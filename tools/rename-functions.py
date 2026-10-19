#!/usr/bin/env python3
"""Lists every function this repo declares, and renames the ones a list says
to, through libclang: each name is resolved to its declaration's USR, so a
rename touches exactly the declarations, definitions, calls and member
pointers that mean that function, never another with the same spelling.

usage:
  rename-functions.py inventory <build dir>            > docs/naming/functions.csv
  rename-functions.py rename <build dir> <names.csv> [--write]

inventory writes one row per function: where it is first declared, its
qualified name, kind, parameters, return type, what it overrides, and the
flags that keep a name (main, operator, std protocol, an override of a base
outside this repo). The decision, new_name and reason columns start empty.

rename reads the rows of <names.csv> whose decision is "rename" and, without
--write, prints how many edits each name would make and every token with the
old spelling it could not resolve. --write rewrites the files. Identifiers in
C++ comments and `backticked` names in CLAUDE.md, AGENTS.md and docs/ are
rewritten too when the old spelling means one function only.
"""

import csv
import ctypes
import importlib.util
import json
import os
import re
import sys
from concurrent.futures import ProcessPoolExecutor

HERE = os.path.dirname(os.path.abspath(__file__))
spec = importlib.util.spec_from_file_location(
    "dead_code_refs", os.path.join(HERE, "dead-code-refs.py"))
refs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(refs)

cindex, K, ROOT = refs.cindex, refs.K, refs.ROOT

FUNCTIONS = {K.FUNCTION_DECL, K.CXX_METHOD, K.FUNCTION_TEMPLATE, K.CONSTRUCTOR,
             K.DESTRUCTOR, K.CONVERSION_FUNCTION}
STD_PROTOCOL = {"begin", "end", "cbegin", "cend", "rbegin", "rend", "size",
                "empty", "data", "at", "swap", "hash", "get"}
COLUMNS = ["location", "qualified_name", "name", "kind", "parameters",
           "return_type", "overrides", "external_override", "is_main",
           "is_operator", "is_std_protocol", "usr", "decision", "new_name",
           "reason"]


def entries(build):
    with open(os.path.join(build, "compile_commands.json")) as f:
        return [e for e in json.load(f)
                if refs.ours(e["file"]) and e["file"].endswith((".cpp", ".mm"))]


def parse(entry):
    os.chdir(entry["directory"])
    return cindex.Index.create().parse(entry["file"], args=refs.arguments(entry))


def is_callback_field(c):
    return c.kind == K.FIELD_DECL and "std::function" in c.type.get_canonical().spelling


def is_lambda_variable(c):
    return c.kind == K.VAR_DECL and any(
        ch.kind == K.LAMBDA_EXPR or any(g.kind == K.LAMBDA_EXPR for g in ch.get_children())
        for ch in c.get_children())


def kind_of(c):
    if c.kind == K.FIELD_DECL:
        return "callback member"
    if c.kind == K.VAR_DECL:
        return "lambda variable"
    if c.kind == K.CONSTRUCTOR:
        return "constructor"
    if c.kind == K.DESTRUCTOR:
        return "destructor"
    if c.kind == K.CONVERSION_FUNCTION:
        return "conversion operator"
    in_class = c.semantic_parent.kind in (K.STRUCT_DECL, K.CLASS_DECL, K.CLASS_TEMPLATE)
    template = "template " if c.kind == K.FUNCTION_TEMPLATE else ""
    if in_class:
        if c.is_static_method():
            return template + "static method"
        if c.is_virtual_method():
            return template + "virtual method"
        return template + "method"
    anonymous = False
    p = c.semantic_parent
    while p is not None and p.kind != K.TRANSLATION_UNIT:
        anonymous |= p.kind == K.NAMESPACE and p.spelling == ""
        p = p.semantic_parent
    local = anonymous or c.storage_class == cindex.StorageClass.STATIC
    return template + ("file-local function" if local else "free function")


def parameters(c):
    if c.kind in (K.FIELD_DECL, K.VAR_DECL):
        return c.type.spelling
    return ", ".join(f"{a.type.spelling} {a.spelling}".strip() for a in c.get_arguments())


def overridden(c):
    """The methods c overrides, directly, through clang_getOverriddenCursors,
    which these Python bindings do not wrap."""
    if c.kind != K.CXX_METHOD:
        return []
    lib = cindex.conf.lib
    found = ctypes.POINTER(cindex.Cursor)()
    count = ctypes.c_uint()
    get = lib.clang_getOverriddenCursors
    get.argtypes = [cindex.Cursor, ctypes.POINTER(ctypes.POINTER(cindex.Cursor)),
                    ctypes.POINTER(ctypes.c_uint)]
    get(c, ctypes.byref(found), ctypes.byref(count))
    bases = []
    for i in range(count.value):
        base = cindex.Cursor.from_buffer_copy(found[i])
        base._tu = c._tu
        bases.append(base)
    if count.value:
        lib.clang_disposeOverriddenCursors.argtypes = [ctypes.POINTER(cindex.Cursor)]
        lib.clang_disposeOverriddenCursors(found)
    return bases


def describe(c, path):
    bases = overridden(c)
    outside = [b for b in bases if not refs.ours(b.location.file.name if b.location.file else None)]
    name = c.spelling
    returns = "" if c.kind in (K.FIELD_DECL, K.VAR_DECL, K.CONSTRUCTOR, K.DESTRUCTOR) \
        else c.result_type.spelling
    return {
        "location": f"{os.path.relpath(path, ROOT)}:{c.location.line}",
        "qualified_name": refs.qualified(c),
        "name": name,
        "kind": kind_of(c),
        "parameters": parameters(c),
        "return_type": returns,
        "overrides": "; ".join(refs.qualified(b) for b in bases),
        "external_override": "yes" if outside else "",
        "is_main": "yes" if name == "main" else "",
        "is_operator": "yes" if name.startswith("operator") else "",
        "is_std_protocol": "yes" if name in STD_PROTOCOL else "",
        "usr": c.get_usr(),
        "decision": "", "new_name": "", "reason": "",
        "_definition": c.is_definition(),
        "_family": [b.get_usr() for b in bases],
    }


def walk(cursor, visit, outer=None):
    for child in cursor.get_children():
        loc = child.location
        path = loc.file.name if loc.file else outer
        if child.kind == K.NAMESPACE or refs.ours(path):
            if refs.ours(path):
                visit(child, path)
            walk(child, visit, path)


def scan_declarations(entry):
    found = {}

    def visit(c, path):
        if c.kind == K.LAMBDA_EXPR:
            return
        wanted = c.kind in FUNCTIONS or is_callback_field(c) or is_lambda_variable(c)
        if not wanted:
            return
        usr = c.get_usr()
        if not usr:
            return
        row = describe(c, path)
        known = found.get(usr)
        if known is None or (not known["_definition"] and row["_definition"]
                             and path.endswith(".h")):
            found[usr] = row
        elif known is not None and path.endswith(".h") and not known["location"].split(":")[0].endswith(".h"):
            found[usr] = row

    walk(parse(entry).cursor, visit)
    return found


def inventory(build):
    rows = {}
    with ProcessPoolExecutor() as pool:
        for found in pool.map(scan_declarations, entries(build)):
            for usr, row in found.items():
                known = rows.get(usr)
                header = row["location"].split(":")[0].endswith(".h")
                if known is None or (header and not known["location"].split(":")[0].endswith(".h")):
                    rows[usr] = row
    out = csv.DictWriter(sys.stdout, COLUMNS, extrasaction="ignore")
    out.writeheader()
    for row in sorted(rows.values(), key=lambda r: (r["location"].split(":")[0],
                                                     int(r["location"].split(":")[1]))):
        out.writerow(row)


# --- rename -----------------------------------------------------------------

def targets_of(c):
    """The USRs a cursor declares or refers to: itself for a declaration,
    every candidate of a dependent call, and what a method overrides."""
    lib = cindex.conf.lib
    if c.kind == K.OVERLOADED_DECL_REF:
        found = [lib.clang_getOverloadedDecl(c, i)
                 for i in range(lib.clang_getNumOverloadedDecls(c))]
        for r in found:
            r._tu = c._tu
    elif c.kind.is_declaration():
        found = [c]
    else:
        found = [c.referenced]
    usrs = set()
    for r in found:
        if r is None or r.kind.is_invalid():
            continue
        for t in (r, r.canonical, refs.specialized(r)):
            if t is not None and t.get_usr():
                usrs.add(t.get_usr())
        for b in overridden(r):
            usrs.add(b.get_usr())
    return usrs


def scan_references(job):
    """Every cursor in our files that declares or names a USR in `usrs`, at
    the location libclang gives its name."""
    entry, usrs, names = job
    hits, others = set(), set()

    def visit(c, path):
        if c.kind in (K.CALL_EXPR, K.UNEXPOSED_EXPR):
            return
        found = targets_of(c) & usrs
        if found:
            hits.add((path, c.location.offset, min(found)))
        elif c.spelling in names:
            others.add((path, c.location.offset))

    walk(parse(entry).cursor, visit)
    return hits, others


def read_bytes_as_text(path):
    """One character per byte, so offsets match libclang's byte offsets."""
    with open(path, encoding="latin-1") as f:
        return f.read()


def code_spans(text):
    """Offsets of the text outside comments, strings and preprocessor
    includes, where an identifier is code."""
    blank = re.sub(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\\n])*"|#\s*include[^\n]*',
                   lambda m: " " * len(m.group(0)), text, flags=re.S)
    return blank


def looks_like_code(text, start, end, owner, in_doc):
    """In prose, an old name is rewritten only where it reads as this code:
    camelCase, or qualified by the class that owns it, or (in a C++ comment)
    called bare. A one-word name after another qualifier (`EventLoop::run`,
    `android.text`) is someone else's; in docs a bare one-word call is left
    for a hand edit, since plans name eacp's functions the same way."""
    name = text[start:end]
    qualifier = re.search(r"(\w+)(::|\.)$", text[max(0, start - 80):start])
    if qualifier:
        return qualifier.group(1) == owner
    if any(ch.isupper() for ch in name[1:]):
        return True
    return not in_doc and text[end:end + 1] == "("


def rename(build, names_csv, write):
    with open(names_csv) as f:
        rows = [r for r in csv.DictReader(f) if r["decision"] == "rename" and r["new_name"]]
    by_usr = {r["usr"]: r for r in rows}
    names = {r["name"] for r in rows}

    hits, others = set(), set()
    jobs = [(e, set(by_usr), names) for e in entries(build)]
    with ProcessPoolExecutor() as pool:
        for found, other in pool.map(scan_references, jobs):
            hits |= found
            others |= other

    edits, counts, wrong = {}, {}, []
    for path, offset, usr in hits:
        row = by_usr[usr]
        old = row["name"].encode()
        data = open(path, "rb").read()
        if data[offset:offset + len(old)] != old:
            wrong.append(f"{os.path.relpath(path, ROOT)}@{offset}: {row['qualified_name']}")
            continue
        edits.setdefault(path, {})[offset] = (row["name"], row["new_name"])
        key = f"{row['qualified_name']} -> {row['new_name']}"
        counts[key] = counts.get(key, 0) + 1

    # Code tokens with an old spelling that no cursor claimed, as ours or as
    # another thing with that name (a field, a local, an eacp function): misses.
    old_word = re.compile(r"\b(" + "|".join(map(re.escape, sorted(names, key=len, reverse=True))) + r")\b", re.ASCII)
    unclaimed = []
    for path in source_files():
        text = read_bytes_as_text(path)
        code = code_spans(text)
        line_starts = [0] + [m.end() for m in re.finditer("\n", text)]
        for m in old_word.finditer(code):
            if m.start() not in edits.get(path, {}) and (path, m.start()) not in others:
                line = next(i for i in range(len(line_starts) - 1, -1, -1)
                            if line_starts[i] <= m.start()) + 1
                unclaimed.append(f"{os.path.relpath(path, ROOT)}:{line}: {m.group(1)}")

    # Comments and docs: only names one row owns, and only where they read as code.
    owners = {}
    for r in rows:
        owners.setdefault(r["name"], set()).add(r["new_name"])
    single = {n: next(iter(v)) for n, v in owners.items() if len(v) == 1}
    owner_of = {r["name"]: r["qualified_name"].split("::")[-2]
                if "::" in r["qualified_name"] else "" for r in rows}
    word = re.compile(r"\b(" + "|".join(map(re.escape, sorted(single, key=len, reverse=True))) + r")\b", re.ASCII)
    text_counts, plain = {}, []

    def rewrite_prose(path, spans, in_doc):
        text = read_bytes_as_text(path)
        for m in re.finditer(spans, text, re.S):
            for w in word.finditer(m.group(0)):
                start, end = m.start() + w.start(), m.start() + w.end()
                if looks_like_code(text, start, end, owner_of[w.group(1)], in_doc):
                    edits.setdefault(path, {})[start] = (w.group(1), single[w.group(1)])
                    text_counts[w.group(1)] = text_counts.get(w.group(1), 0) + 1
                    if not any(ch.isupper() for ch in w.group(1)):
                        line = text.count("\n", 0, start) + 1
                        context = text[max(0, start - 30):end + 10].replace("\n", " ")
                        plain.append(f"{os.path.relpath(path, ROOT)}:{line}: {context}")

    for path in source_files():
        rewrite_prose(path, r"//[^\n]*|/\*.*?\*/", False)
    for path in doc_files():
        rewrite_prose(path, r"`[^`\n]+`", True)
    ambiguous = sorted(n for n, v in owners.items() if len(v) > 1)

    print(f"## code edits per name ({sum(counts.values())})")
    for key in sorted(counts):
        print(f"{counts[key]:5}  {key}")
    never = sorted(f"{r['qualified_name']} -> {r['new_name']}" for r in rows
                   if f"{r['qualified_name']} -> {r['new_name']}" not in counts)
    print(f"\n## names with no code edit ({len(never)})")
    print("\n".join(never))
    print(f"\n## comment and doc edits per name ({sum(text_counts.values())})")
    for n in sorted(text_counts):
        print(f"{text_counts[n]:5}  {n} -> {single[n]}")
    print(f"\n## prose edits of one-word names, to check by eye ({len(plain)})")
    print("\n".join(sorted(plain)))
    print(f"\n## old names several rows rename differently; comments and docs left alone ({len(ambiguous)})")
    print("\n".join(ambiguous))
    print(f"\n## cursors whose location does not spell the old name ({len(wrong)})")
    print("\n".join(sorted(wrong)))
    print(f"\n## code tokens with an old spelling no cursor claimed ({len(unclaimed)})")
    print("\n".join(sorted(unclaimed)))
    total = sum(len(v) for v in edits.values())
    print(f"\n## total edits {total} in {len(edits)} files")

    if write:
        for path, spans in edits.items():
            data = open(path, "rb").read()
            for off in sorted(spans, reverse=True):
                old, new = spans[off]
                if data[off:off + len(old.encode())] != old.encode():
                    sys.exit(f"{path}:{off}: expected {old}")
                data = data[:off] + new.encode() + data[off + len(old.encode()):]
            open(path, "wb").write(data)


def source_files():
    for d in ("Lib", "Apps", "Tests", "tools"):
        for dp, _, fs in os.walk(os.path.join(ROOT, d)):
            for f in fs:
                if f.endswith((".h", ".cpp", ".mm")):
                    yield os.path.join(dp, f)


def doc_files():
    for f in ("CLAUDE.md", "AGENTS.md"):
        if os.path.isfile(os.path.join(ROOT, f)):
            yield os.path.join(ROOT, f)
    for dp, _, fs in os.walk(os.path.join(ROOT, "docs")):
        for f in fs:
            if f.endswith(".md") and "naming" not in dp:
                yield os.path.join(dp, f)


def main():
    if len(sys.argv) >= 3 and sys.argv[1] == "inventory":
        inventory(sys.argv[2])
    elif len(sys.argv) >= 4 and sys.argv[1] == "rename":
        rename(sys.argv[2], sys.argv[3], "--write" in sys.argv[4:])
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
