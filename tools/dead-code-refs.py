#!/usr/bin/env python3
"""Every function, method, field, variable and type declared in this repo's
sources, against every reference to it in every translation unit, through
libclang. Prints the ones nothing references ("unused") and the ones only
Tests/ references ("test-only").

usage: dead-code-refs.py <build dir with compile_commands.json> [jobs]
"""

import json
import os
import shlex
import subprocess
import sys
from concurrent.futures import ProcessPoolExecutor

LLVM = os.environ.get("LLVM_PREFIX", "/opt/homebrew/opt/llvm")
sys.path.insert(0, next(
    os.path.join(LLVM, "lib", d, "site-packages")
    for d in sorted(os.listdir(os.path.join(LLVM, "lib")), reverse=True)
    if d.startswith("python3")))

from clang import cindex  # noqa: E402

cindex.Config.set_library_file(os.path.join(LLVM, "lib", "libclang.dylib"))

RESOURCE_DIR = subprocess.run([os.path.join(LLVM, "bin", "clang"), "-print-resource-dir"],
                              capture_output=True, text=True).stdout.strip()
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OURS = tuple(os.path.join(ROOT, d) + "/" for d in ("Lib", "Apps", "Tests", "tools"))
K = cindex.CursorKind

# Called from outside this repo's code: the entry points, and Miro's
# serialisation, which finds reflect() by argument-dependent lookup.
ROOTS = {"", "main", "reflect"}

TRACKED = {
    K.FUNCTION_DECL, K.CXX_METHOD, K.FUNCTION_TEMPLATE, K.FIELD_DECL,
    K.STRUCT_DECL, K.CLASS_DECL, K.CLASS_TEMPLATE, K.ENUM_DECL,
    K.TYPE_ALIAS_DECL, K.TYPEDEF_DECL, K.VAR_DECL, K.ENUM_CONSTANT_DECL,
}
SCOPES = {K.NAMESPACE, K.TRANSLATION_UNIT, K.STRUCT_DECL, K.CLASS_DECL,
          K.CLASS_TEMPLATE, K.ENUM_DECL}


def ours(path):
    return path is not None and path.startswith(OURS) and "/_deps/" not in path


def arguments(entry):
    args = shlex.split(entry["command"]) if "command" in entry else entry["arguments"]
    out, skip = ["-resource-dir", RESOURCE_DIR], False
    for a in args[1:]:
        if skip:
            skip = False
        elif a in ("-o", "-c"):
            skip = a == "-o"
        elif a != entry["file"]:
            out.append(a)
    return out


def specialized(cursor):
    try:
        t = cindex.conf.lib.clang_getSpecializedCursorTemplate(cursor)
        return None if t is None or t.kind.is_invalid() else t
    except Exception:
        return None


def qualified(cursor):
    names = []
    while cursor is not None and cursor.kind != K.TRANSLATION_UNIT:
        names.append(cursor.spelling or "(anonymous)")
        cursor = cursor.semantic_parent
    return "::".join(reversed(names))


def scan(entry):
    os.chdir(entry["directory"])
    tu = cindex.Index.create().parse(entry["file"], args=arguments(entry))
    decls, refs = {}, []
    errors = [d for d in tu.diagnostics if d.severity >= cindex.Diagnostic.Error]
    if errors:
        sys.exit(f"{entry['file']}: {errors[0]}")

    # Implicit nodes, such as a conversion operator's member call, have no
    # file: they belong to the code around them.
    def visit(cursor, outer=None):
        for child in cursor.get_children():
            loc = child.location
            path = loc.file.name if loc.file else outer
            if child.kind == K.NAMESPACE or ours(path):
                if ours(path):
                    note(child, path, loc.line)
                visit(child, path)

    def note(c, path, line):
        if c.kind in TRACKED and c.semantic_parent is not None \
                and c.semantic_parent.kind in SCOPES:
            usr = c.get_usr()
            if usr and not (c.kind == K.CXX_METHOD and c.is_virtual_method()) \
                    and c.spelling not in ROOTS \
                    and not c.spelling.startswith("operator"):
                d = decls.setdefault(usr, [qualified(c), c.kind.name, None, []])
                if c.is_definition() or d[2] is None:
                    d[2] = f"{os.path.relpath(path, ROOT)}:{line}"
        where = "test" if path.startswith(os.path.join(ROOT, "Tests") + "/") else "prod"
        if c.kind == K.UNEXPOSED_DECL and c.spelling.startswith("["):
            record = c.type.get_canonical().get_declaration()
            for field in record.get_children():
                if field.kind == K.FIELD_DECL:
                    refs.append((field.get_usr(), where))
        if c.kind.is_declaration():
            return
        lib = cindex.conf.lib
        if c.kind == K.OVERLOADED_DECL_REF:
            # A call a template leaves dependent names every candidate.
            found = [lib.clang_getOverloadedDecl(c, i)
                     for i in range(lib.clang_getNumOverloadedDecls(c))]
            for r in found:
                r._tu = c._tu
        else:
            found = [c.referenced]
        for r in found:
            if r is None or r.kind.is_invalid():
                continue
            for target in (r, r.canonical, specialized(r)):
                if target is not None:
                    usr = target.get_usr()
                    if usr:
                        refs.append((usr, where))

    visit(tu.cursor)
    return decls, refs


def main():
    build = sys.argv[1]
    jobs = int(sys.argv[2]) if len(sys.argv) > 2 else os.cpu_count()
    with open(os.path.join(build, "compile_commands.json")) as f:
        entries = [e for e in json.load(f)
                   if ours(e["file"]) and e["file"].endswith((".cpp", ".mm"))]

    decls, used = {}, {}
    with ProcessPoolExecutor(jobs) as pool:
        for d, r in pool.map(scan, entries):
            for usr, (name, kind, where, _) in d.items():
                known = decls.setdefault(usr, [name, kind, where])
                if known[2] is None or (where and "Tests/" not in where
                                        and where.endswith(".cpp")):
                    known[2] = known[2] or where
            for usr, where in r:
                used.setdefault(usr, set()).add(where)

    def report(title, keep):
        rows = sorted(f"{where}: {kind.lower()} {name}"
                      for usr, (name, kind, where) in decls.items()
                      if where and not where.startswith("Tests/") and keep(usr))
        print(f"## {title} ({len(rows)})")
        print("\n".join(rows))
        print()

    report("unused", lambda usr: usr not in used)
    report("test-only", lambda usr: used.get(usr) == {"test"})


if __name__ == "__main__":
    main()
