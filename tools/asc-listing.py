#!/usr/bin/env python3
"""Fills the App Store Connect listing for Cows In Love and submits it for review.

Usage: just asc <command> [--platform ios|macos]
       [--write] [flags]

Commands: check, info, version, screenshots, build, price, testflight,
invite [--internal] <email> [--first NAME] [--last NAME], submit, all.
Without --write every POST/PATCH/DELETE and upload is printed, not sent.
Credentials come from COWS_ASC_KEY_ID, COWS_ASC_ISSUER_ID and COWS_ASC_KEY
(path to the .p8) or COWS_ASC_KEY_P8 (its contents): `just asc` fills them
from 1Password through Deploy/asc.env.
"""

import argparse
import atexit
import base64
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUNDLE_ID = "com.cowsinlove.play"
LOCALE = "en-US"
API = "https://api.appstoreconnect.apple.com"
PLATFORMS = {"ios": "IOS", "macos": "MAC_OS"}
IOS_METADATA = ROOT / "Deploy/Apple-iOS/Metadata/app-store.md"
MACOS_METADATA = ROOT / "Deploy/Apple-macOS/Metadata/app-store.md"
SCREENSHOTS = {
    "ios": [ROOT / "Deploy/Apple-iOS/Screenshots/iPhone-6.9",
            ROOT / "Deploy/Apple-iOS/Screenshots/iPhone-6.5"],
    "macos": [ROOT / "Deploy/Apple-macOS/Screenshots"],
}
DISPLAY_TYPES = {
    (1320, 2868): "APP_IPHONE_67", (1290, 2796): "APP_IPHONE_67",
    (1284, 2778): "APP_IPHONE_65", (1242, 2688): "APP_IPHONE_65",
    (1280, 800): "APP_DESKTOP", (1440, 900): "APP_DESKTOP",
    (2560, 1600): "APP_DESKTOP", (2880, 1800): "APP_DESKTOP",
}
IOS_REVIEW_NOTES = ("No account needed. Walk with the on-screen stick, Moo, "
                    "find the other cow.")
FREQUENCY_RATINGS = [
    "alcoholTobaccoOrDrugUseOrReferences", "contests", "gamblingSimulated",
    "gunsOrOtherWeapons", "medicalOrTreatmentInformation",
    "profanityOrCrudeHumor", "sexualContentGraphicAndNudity",
    "sexualContentOrNudity", "horrorOrFearThemes", "matureOrSuggestiveThemes",
    "violenceCartoonOrFantasy", "violenceRealisticProlongedGraphicOrSadistic",
    "violenceRealistic",
]
BOOLEAN_RATINGS = [
    "advertising", "gambling", "healthOrWellnessTopics", "lootBox",
    "messagingAndChat", "parentalControls", "ageAssurance", "socialMedia",
    "socialMediaAgeRestricted", "unrestrictedWebAccess", "userGeneratedContent",
]
EDITABLE_VERSION_STATES = {
    "PREPARE_FOR_SUBMISSION", "DEVELOPER_REJECTED", "REJECTED",
    "METADATA_REJECTED", "INVALID_BINARY", "WAITING_FOR_EXPORT_COMPLIANCE",
}
BETA_GROUP = "Cows"
FRIENDS_GROUP = "Friends"
WRITE = False
NEW_ID = "(new)"


def fail(message):
    print(f"error: {message}", file=sys.stderr)
    sys.exit(1)


# ---- auth -------------------------------------------------------------------

def b64url(data):
    return base64.urlsafe_b64encode(data).rstrip(b"=").decode()


def der_to_raw(der):
    def read_int(at):
        if der[at] != 0x02:
            fail("unexpected ECDSA signature encoding")
        length = der[at + 1]
        value = der[at + 2:at + 2 + length]
        return value.lstrip(b"\0").rjust(32, b"\0"), at + 2 + length

    at = 3 if der[1] & 0x80 else 2
    r, at = read_int(at)
    s, _ = read_int(at)
    return r + s


def make_token():
    key_id = os.environ.get("COWS_ASC_KEY_ID")
    issuer = os.environ.get("COWS_ASC_ISSUER_ID")
    key = os.environ.get("COWS_ASC_KEY")
    if not key and os.environ.get("COWS_ASC_KEY_P8"):
        key = os.path.join(tempfile.mkdtemp(), f"AuthKey_{key_id}.p8")
        with open(key, "w") as f:
            f.write(os.environ["COWS_ASC_KEY_P8"] + "\n")
        os.chmod(key, 0o600)
        atexit.register(shutil.rmtree, os.path.dirname(key), ignore_errors=True)
    if not (key_id and issuer and key):
        fail("COWS_ASC_KEY_ID, COWS_ASC_ISSUER_ID and COWS_ASC_KEY must be set; "
             "run through `just asc`")
    now = int(time.time())
    header = {"alg": "ES256", "kid": key_id, "typ": "JWT"}
    claims = {"iss": issuer, "iat": now, "exp": now + 15 * 60,
              "aud": "appstoreconnect-v1"}
    signing_input = (b64url(json.dumps(header).encode()) + "."
                     + b64url(json.dumps(claims).encode())).encode()
    der = subprocess.run(["openssl", "dgst", "-sha256", "-sign", key],
                         input=signing_input, capture_output=True, check=True).stdout
    return signing_input.decode() + "." + b64url(der_to_raw(der))


TOKEN = {"value": None, "made": 0}


def token():
    if time.time() - TOKEN["made"] > 10 * 60:
        TOKEN["value"] = make_token()
        TOKEN["made"] = time.time()
    return TOKEN["value"]


# ---- api --------------------------------------------------------------------

def api(method, path, body=None, params=None, missing_ok=False):
    if method != "GET" and not WRITE:
        print(f"  would {method} {path}"
              + (f" {json.dumps(body)}" if body else ""))
        return {"data": {"id": NEW_ID, "type": "", "attributes": {}}}
    if NEW_ID in path:
        return {"data": []}
    url = path if path.startswith("http") else API + path
    if params:
        url += "?" + urllib.parse.urlencode(params, safe="[],")
    request = urllib.request.Request(
        url, method=method,
        data=json.dumps(body).encode() if body is not None else None,
        headers={"Authorization": f"Bearer {token()}",
                 "Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(request) as response:
            text = response.read()
            return json.loads(text) if text else {}
    except urllib.error.HTTPError as error:
        if error.code == 404 and missing_ok:
            return {"data": None}
        detail = error.read().decode()
        try:
            detail = json.dumps(json.loads(detail), indent=2)
        except ValueError:
            pass
        fail(f"{method} {path} -> HTTP {error.code}\n{detail}")


def get_all(path, params=None):
    items = []
    result = api("GET", path, params=params)
    while True:
        data = result.get("data") or []
        items += data if isinstance(data, list) else [data]
        next_url = result.get("links", {}).get("next")
        if not next_url:
            return items
        result = api("GET", next_url)


def resource(type_, id_):
    return {"data": {"type": type_, "id": id_}}


def create(type_, attributes, relationships):
    body = {"data": {"type": type_, "attributes": attributes,
                     "relationships": relationships}}
    return api("POST", f"/v1/{type_}", body)["data"]


def update(type_, id_, attributes, current=None):
    if current is not None:
        attributes = {k: v for k, v in attributes.items() if current.get(k) != v}
    if not attributes:
        print(f"  {type_} {id_}: up to date")
        return
    print(f"  {type_} {id_}: set {', '.join(attributes)}")
    api("PATCH", f"/v1/{type_}/{id_}",
        {"data": {"type": type_, "id": id_, "attributes": attributes}})


# ---- metadata ---------------------------------------------------------------

def table(text):
    rows = {}
    for line in text.splitlines():
        cells = [c.strip() for c in line.strip().strip("|").split("|")]
        if line.startswith("|") and len(cells) == 2 and not cells[0].startswith("-"):
            rows[re.sub(r"\s*\(\d+\)$", "", cells[0])] = cells[1]
    return rows


def paragraphs(block):
    out = []
    for para in re.split(r"\n\s*\n", block.strip()):
        lines = para.strip().splitlines()
        if all(l.startswith("- ") for l in lines):
            out.append("\n".join(lines))
        else:
            out.append(" ".join(l.strip() for l in lines))
    return "\n\n".join(out)


def section(text, title):
    match = re.search(r"^\*\*" + re.escape(title) + r"(?: \(\d+\))?\*\*\n(.*?)"
                      r"(?=^\*\*|^## |\Z)", text, re.S | re.M)
    if not match:
        fail(f"no '{title}' in the metadata")
    return paragraphs(match.group(1))


def quote_after(text, marker):
    lines = text.split(marker, 1)[1].strip().splitlines()
    quoted = []
    for line in lines:
        if not line.startswith(">"):
            break
        quoted.append(line.lstrip("> ").strip())
    return " ".join(quoted)


def metadata(platform):
    text = IOS_METADATA.read_text()
    rows = table(text)
    description = section(text, "Description")
    review_notes = IOS_REVIEW_NOTES
    if platform == "macos":
        mac = MACOS_METADATA.read_text()
        controls = quote_after(mac, "Description differences: replace the touch "
                                    "controls line with")
        description = description.replace("\n\n- ", f"\n\n{controls}\n\n- ", 1)
        review_notes = quote_after(mac, "Review notes:")
    return {
        "name": rows["Name"],
        "subtitle": rows["Subtitle"],
        "marketingUrl": rows["Marketing URL"].split()[0],
        "promotionalText": section(text, "Promotional text"),
        "description": description,
        "keywords": section(text, "Keywords"),
        "whatsNew": section(text, "What's New"),
        "reviewNotes": review_notes,
    }


def version_string():
    match = re.search(r"^project\(Cows VERSION ([0-9.]+)",
                      (ROOT / "CMakeLists.txt").read_text(), re.M)
    return match.group(1)


def required(args, name):
    value = getattr(args, name)
    if not value:
        flag = "--" + name.replace("_", "-")
        env = "COWS_" + name.upper()
        fail(f"{flag} (or {env}) is required")
    return value


# ---- lookups ----------------------------------------------------------------

def find_app():
    apps = get_all("/v1/apps", {"filter[bundleId]": BUNDLE_ID})
    if not apps:
        fail(f"no app record for {BUNDLE_ID} in App Store Connect yet")
    return apps[0]


def versions(app_id, platform):
    return get_all(f"/v1/apps/{app_id}/appStoreVersions",
                   {"filter[platform]": PLATFORMS[platform], "limit": 200})


def find_version(app_id, platform, wanted):
    for v in versions(app_id, platform):
        if v["attributes"]["versionString"] == wanted:
            return v
    if not WRITE:
        for v in versions(app_id, platform):
            if v["attributes"].get("appVersionState") in EDITABLE_VERSION_STATES:
                print(f"  (dry run: using editable version "
                      f"{v['attributes']['versionString']}, `version` renames it)")
                return v
        return {"id": NEW_ID, "attributes": {}}
    fail(f"no {PLATFORMS[platform]} version {wanted}; run `version` first")


def editable_app_info(app_id):
    infos = get_all(f"/v1/apps/{app_id}/appInfos")
    for info in infos:
        if info["attributes"].get("state") != "READY_FOR_DISTRIBUTION":
            return info
    fail("no editable appInfo (every one is READY_FOR_DISTRIBUTION)")


def localization(path, locale=LOCALE):
    for item in get_all(path):
        if item["attributes"].get("locale") == locale:
            return item
    return None


# ---- commands ---------------------------------------------------------------

def cmd_check(args, app):
    print(f"app {app['id']}: {app['attributes'].get('name')} "
          f"({app['attributes'].get('bundleId')}, "
          f"primary locale {app['attributes'].get('primaryLocale')})")
    for platform in PLATFORMS:
        for v in versions(app["id"], platform):
            a = v["attributes"]
            print(f"  version {a['versionString']} {a['platform']}: "
                  f"{a.get('appVersionState')} (store state {a.get('appStoreState')})"
                  f", id {v['id']}")
    raw = api("GET", "/v1/builds", params={"filter[app]": app["id"],
                                          "sort": "-uploadedDate",
                                          "include": "preReleaseVersion",
                                          "limit": 200})
    builds = raw["data"]
    pre = {i["id"]: i["attributes"] for i in raw.get("included", [])}
    if not builds:
        print("  no builds")
    for b in builds:
        a = b["attributes"]
        rel = b.get("relationships", {}).get("preReleaseVersion", {}).get("data")
        p = pre.get(rel["id"], {}) if rel else {}
        print(f"  build {p.get('version', '?')} ({a['version']}) "
              f"{p.get('platform', '?')}: {a['processingState']}, uploaded "
              f"{a.get('uploadedDate')}, usesNonExemptEncryption "
              f"{a.get('usesNonExemptEncryption')}, id {b['id']}")
        beta = api("GET", f"/v1/builds/{b['id']}/buildBetaDetail",
                   missing_ok=True).get("data") or {}
        review = api("GET", f"/v1/builds/{b['id']}/betaAppReviewSubmission",
                     missing_ok=True).get("data") or {}
        print(f"    beta review: "
              f"{review.get('attributes', {}).get('betaReviewState', 'not submitted')}")
        print(f"    TestFlight: internal "
              f"{beta.get('attributes', {}).get('internalBuildState')}, external "
              f"{beta.get('attributes', {}).get('externalBuildState')}")
    for g in get_all("/v1/betaGroups", {"filter[app]": app["id"]}):
        ga = g["attributes"]
        testers = get_all(f"/v1/betaGroups/{g['id']}/betaTesters")
        print(f"  beta group {ga['name']} {g['id']}: "
              f"{'internal' if ga['isInternalGroup'] else 'external'}, "
              f"all builds {ga.get('hasAccessToAllBuilds')}, testers "
              + (", ".join(f"{t['attributes'].get('email')} "
                           f"({t['attributes'].get('state')})" for t in testers)
                 or "none"))
    for s in get_all("/v1/reviewSubmissions", {"filter[app]": app["id"]}):
        print(f"  review submission {s['id']} {s['attributes']['platform']}: "
              f"{s['attributes']['state']}")


def cmd_info(args, app):
    meta = metadata("ios")
    print("app:")
    update("apps", app["id"],
           {"contentRightsDeclaration": "DOES_NOT_USE_THIRD_PARTY_CONTENT"},
           app["attributes"])

    info = editable_app_info(app["id"])
    print(f"app info {info['id']} ({info['attributes'].get('state')}):")
    categories = {"primaryCategory": "GAMES",
                  "primarySubcategoryOne": "GAMES_CASUAL",
                  "primarySubcategoryTwo": "GAMES_ADVENTURE"}
    if args.secondary_category:
        categories["secondaryCategory"] = args.secondary_category
    current = {}
    for rel in categories:
        data = api("GET", f"/v1/appInfos/{info['id']}/{rel}", missing_ok=True)
        current[rel] = (data.get("data") or {}).get("id")
    changed = {k: v for k, v in categories.items() if current.get(k) != v}
    if changed:
        print(f"  categories: set {changed}")
        api("PATCH", f"/v1/appInfos/{info['id']}",
            {"data": {"type": "appInfos", "id": info["id"], "relationships": {
                k: resource("appCategories", v) for k, v in changed.items()}}})
    else:
        print("  categories: up to date")

    rating = api("GET", f"/v1/appInfos/{info['id']}/ageRatingDeclaration")["data"]
    answers = {k: "NONE" for k in FREQUENCY_RATINGS}
    answers.update({k: False for k in BOOLEAN_RATINGS})
    print("age rating:")
    update("ageRatingDeclarations", rating["id"], answers, rating["attributes"])

    wanted = {"name": meta["name"], "subtitle": meta["subtitle"],
              "privacyPolicyUrl": required(args, "privacy_url")}
    loc = localization(f"/v1/appInfos/{info['id']}/appInfoLocalizations")
    print(f"app info localization {LOCALE}:")
    if loc:
        update("appInfoLocalizations", loc["id"], wanted, loc["attributes"])
    else:
        create("appInfoLocalizations", {"locale": LOCALE, **wanted},
               {"appInfo": resource("appInfos", info["id"])})


def ensure_version(args, app):
    platform = PLATFORMS[args.platform]
    wanted = args.version_string
    existing = versions(app["id"], args.platform)
    for v in existing:
        if v["attributes"]["versionString"] == wanted:
            return v, existing
    for v in existing:
        if v["attributes"].get("appVersionState") in EDITABLE_VERSION_STATES:
            print(f"  renaming editable {platform} version "
                  f"{v['attributes']['versionString']} to {wanted}")
            update("appStoreVersions", v["id"], {"versionString": wanted})
            v["attributes"]["versionString"] = wanted
            return v, existing
    print(f"  creating {platform} version {wanted}")
    v = create("appStoreVersions",
               {"platform": platform, "versionString": wanted,
                "releaseType": "AFTER_APPROVAL",
                "copyright": required(args, "copyright")},
               {"app": resource("apps", app["id"])})
    return v, existing


def cmd_version(args, app):
    meta = metadata(args.platform)
    print(f"version {args.version_string} {PLATFORMS[args.platform]}:")
    v, existing = ensure_version(args, app)
    update("appStoreVersions", v["id"],
           {"copyright": required(args, "copyright"),
            "releaseType": "AFTER_APPROVAL"}, v["attributes"])
    wanted = {k: meta[k] for k in ("description", "keywords", "promotionalText",
                                   "marketingUrl")}
    wanted["supportUrl"] = required(args, "support_url")
    if any(e["id"] != v["id"] for e in existing):
        wanted["whatsNew"] = meta["whatsNew"]
    loc = localization(f"/v1/appStoreVersions/{v['id']}/appStoreVersionLocalizations")
    print(f"version localization {LOCALE}:")
    if loc:
        update("appStoreVersionLocalizations", loc["id"], wanted, loc["attributes"])
    else:
        create("appStoreVersionLocalizations", {"locale": LOCALE, **wanted},
               {"appStoreVersion": resource("appStoreVersions", v["id"])})


def pixel_size(path):
    out = subprocess.run(["sips", "-g", "pixelWidth", "-g", "pixelHeight", str(path)],
                         capture_output=True, text=True, check=True).stdout
    width = int(re.search(r"pixelWidth: (\d+)", out).group(1))
    height = int(re.search(r"pixelHeight: (\d+)", out).group(1))
    return width, height


def upload_screenshot(set_id, path):
    data = path.read_bytes()
    print(f"    uploading {path.name} ({len(data)} bytes)")
    shot = create("appScreenshots", {"fileName": path.name, "fileSize": len(data)},
                  {"appScreenshotSet": resource("appScreenshotSets", set_id)})
    if not WRITE:
        return shot["id"]
    for op in shot["attributes"]["uploadOperations"]:
        chunk = data[op["offset"]:op["offset"] + op["length"]]
        headers = {h["name"]: h["value"] for h in op.get("requestHeaders") or []}
        request = urllib.request.Request(op["url"], data=chunk, method=op["method"],
                                         headers=headers)
        try:
            urllib.request.urlopen(request).read()
        except urllib.error.HTTPError as error:
            fail(f"upload of {path.name} chunk at {op['offset']} -> HTTP "
                 f"{error.code}\n{error.read().decode()}")
    update("appScreenshots", shot["id"],
           {"uploaded": True, "sourceFileChecksum": hashlib.md5(data).hexdigest()})
    return shot["id"]


def cmd_screenshots(args, app):
    v = find_version(app["id"], args.platform, args.version_string)
    loc = localization(f"/v1/appStoreVersions/{v['id']}/appStoreVersionLocalizations")
    if not loc and not WRITE:
        loc = {"id": NEW_ID}
    if not loc:
        fail(f"no {LOCALE} localization on the version; run `version` first")
    sets = {s["attributes"]["screenshotDisplayType"]: s for s in get_all(
        f"/v1/appStoreVersionLocalizations/{loc['id']}/appScreenshotSets")}
    for folder in SCREENSHOTS[args.platform]:
        files = sorted(folder.glob("*.png"))
        sizes = {pixel_size(f) for f in files}
        if len(sizes) != 1:
            fail(f"{folder} mixes sizes {sizes}")
        size = sizes.pop()
        display = DISPLAY_TYPES.get(size)
        if not display:
            fail(f"{folder}: {size[0]}x{size[1]} matches no display type")
        print(f"{folder.relative_to(ROOT)} ({size[0]}x{size[1]}) -> {display}:")
        shot_set = sets.get(display)
        if not shot_set:
            print(f"  creating set {display}")
            shot_set = create("appScreenshotSets", {"screenshotDisplayType": display},
                              {"appStoreVersionLocalization": resource(
                                  "appStoreVersionLocalizations", loc["id"])})
        existing = get_all(f"/v1/appScreenshotSets/{shot_set['id']}/appScreenshots")
        local = {f.name: f for f in files}
        kept = {}
        for shot in existing:
            a = shot["attributes"]
            state = (a.get("assetDeliveryState") or {}).get("state")
            f = local.get(a.get("fileName"))
            if f and a.get("fileSize") == f.stat().st_size and state in (
                    "COMPLETE", "UPLOAD_COMPLETE") and f.name not in kept:
                kept[f.name] = shot["id"]
                print(f"    keeping {f.name}")
            else:
                print(f"    deleting {a.get('fileName')} ({state})")
                api("DELETE", f"/v1/appScreenshots/{shot['id']}")
        order = []
        for f in files:
            order.append(kept.get(f.name) or upload_screenshot(shot_set["id"], f))
        current = [s["id"] for s in existing]
        if order != current:
            api("PATCH", f"/v1/appScreenshotSets/{shot_set['id']}/relationships/"
                         "appScreenshots",
                {"data": [{"type": "appScreenshots", "id": i} for i in order]})


def find_build(app_id, platform, version, number):
    params = {"filter[app]": app_id,
              "filter[preReleaseVersion.platform]": PLATFORMS[platform],
              "filter[preReleaseVersion.version]": version,
              "filter[expired]": "false", "sort": "-uploadedDate", "limit": 1}
    if number:
        params["filter[version]"] = number
    builds = api("GET", "/v1/builds", params=params)["data"]
    return builds[0] if builds else None


def withdraw_from_review(app, platform, v):
    submissions = get_all("/v1/reviewSubmissions", {
        "filter[app]": app["id"], "filter[platform]": PLATFORMS[platform],
        "filter[state]": "WAITING_FOR_REVIEW,IN_REVIEW,UNRESOLVED_ISSUES"})
    for submission in submissions:
        print(f"  cancelling review submission {submission['id']} "
              f"({submission['attributes']['state']})")
        update("reviewSubmissions", submission["id"], {"canceled": True})
    if not WRITE:
        return
    deadline = time.time() + 10 * 60
    while True:
        state = api("GET", f"/v1/appStoreVersions/{v['id']}")["data"][
            "attributes"]["appVersionState"]
        print(f"{time.strftime('%H:%M:%S')} version {v['id']}: {state}")
        if state in EDITABLE_VERSION_STATES:
            return
        if time.time() > deadline:
            fail(f"version {v['id']} still {state} after 10 minutes")
        time.sleep(15)


def cmd_build(args, app):
    v = find_version(app["id"], args.platform, args.version_string)
    deadline = time.time() + 20 * 60
    while True:
        build = find_build(app["id"], args.platform, args.version_string, args.number)
        state = build["attributes"]["processingState"] if build else "NOT FOUND"
        label = (f"build {args.version_string} ({build['attributes']['version']})"
                 if build else f"build {args.version_string} ({args.number or 'any'})")
        print(f"{time.strftime('%H:%M:%S')} {label}: {state}")
        if state == "VALID":
            break
        if state in ("FAILED", "INVALID"):
            fail(f"{label} is {state}")
        if time.time() > deadline:
            fail(f"{label} not VALID after 20 minutes")
        time.sleep(30)
    if build["attributes"].get("usesNonExemptEncryption") is None:
        update("builds", build["id"], {"usesNonExemptEncryption": False})
    attached = api("GET", f"/v1/appStoreVersions/{v['id']}/build",
                   missing_ok=True).get("data")
    if attached and attached["id"] == build["id"]:
        print(f"  build {build['id']} already attached")
        return
    if v["attributes"].get("appVersionState") not in EDITABLE_VERSION_STATES:
        if not args.number:
            fail(f"version {v['id']} is {v['attributes'].get('appVersionState')}; "
                 "pass --number to pull it from review and swap the build")
        withdraw_from_review(app, args.platform, v)
    print(f"  attaching build {build['id']} to version {v['id']}")
    api("PATCH", f"/v1/appStoreVersions/{v['id']}/relationships/build",
        resource("builds", build["id"]))


def ensure_review_detail(args, version_id, notes):
    name = required(args, "review_name").split()
    wanted = {"contactFirstName": " ".join(name[:-1]) or name[0],
              "contactLastName": name[-1],
              "contactPhone": required(args, "review_phone"),
              "contactEmail": required(args, "review_email"),
              "demoAccountRequired": False, "notes": notes}
    detail = api("GET", f"/v1/appStoreVersions/{version_id}/appStoreReviewDetail",
                 missing_ok=True).get("data")
    print("review detail:")
    if detail:
        update("appStoreReviewDetails", detail["id"], wanted, detail["attributes"])
    else:
        create("appStoreReviewDetails", wanted,
               {"appStoreVersion": resource("appStoreVersions", version_id)})


def cmd_submit(args, app):
    platform = PLATFORMS[args.platform]
    meta = metadata(args.platform)
    v = find_version(app["id"], args.platform, args.version_string)
    ensure_review_detail(args, v["id"], args.review_notes or meta["reviewNotes"])
    submissions = get_all("/v1/reviewSubmissions", {
        "filter[app]": app["id"], "filter[platform]": platform,
        "filter[state]": "READY_FOR_REVIEW,WAITING_FOR_REVIEW,IN_REVIEW,"
                         "UNRESOLVED_ISSUES"})
    for s in submissions:
        if s["attributes"]["state"] != "READY_FOR_REVIEW":
            print(f"review submission {s['id']} is already {s['attributes']['state']}")
            return
    if submissions:
        submission = submissions[0]
        print(f"review submission {submission['id']}: reusing (READY_FOR_REVIEW)")
    else:
        print(f"review submission: creating for {platform}")
        submission = create("reviewSubmissions", {"platform": platform},
                            {"app": resource("apps", app["id"])})
    items = get_all(f"/v1/reviewSubmissions/{submission['id']}/items",
                    {"include": "appStoreVersion"})
    if any((i.get("relationships", {}).get("appStoreVersion", {}).get("data") or {})
           .get("id") == v["id"] for i in items):
        print(f"  version {v['id']} already in the submission")
    else:
        print(f"  adding version {v['id']}")
        create("reviewSubmissionItems", {}, {
            "reviewSubmission": resource("reviewSubmissions", submission["id"]),
            "appStoreVersion": resource("appStoreVersions", v["id"])})
    print("  submitting")
    update("reviewSubmissions", submission["id"], {"submitted": True})
    if WRITE:
        state = api("GET", f"/v1/reviewSubmissions/{submission['id']}")["data"]
        print(f"review submission {submission['id']}: "
              f"{state['attributes']['state']}")


def cmd_price(args, app):
    schedule = api("GET", f"/v1/apps/{app['id']}/appPriceSchedule",
                   missing_ok=True).get("data")
    prices = schedule and api(
        "GET", f"/v1/appPriceSchedules/{schedule['id']}/manualPrices",
        missing_ok=True).get("data")
    if prices:
        print(f"price schedule {schedule['id']}: {len(prices)} manual price(s)")
    else:
        points = get_all(f"/v1/apps/{app['id']}/appPricePoints",
                         {"filter[territory]": "USA", "limit": 200})
        free = [p for p in points
                if float(p["attributes"].get("customerPrice") or 0) == 0]
        if not free:
            fail("no USA price point with customerPrice 0")
        print(f"price schedule: creating, free (price point {free[0]['id']})")
        api("POST", "/v1/appPriceSchedules", {
            "data": {"type": "appPriceSchedules", "relationships": {
                "app": resource("apps", app["id"]),
                "baseTerritory": resource("territories", "USA"),
                "manualPrices": {"data": [{"type": "appPrices", "id": "${free}"}]}}},
            "included": [{"type": "appPrices", "id": "${free}",
                          "attributes": {"startDate": None},
                          "relationships": {"appPricePoint": resource(
                              "appPricePoints", free[0]["id"])}}]})
    availability = api("GET", f"/v1/apps/{app['id']}/appAvailabilityV2",
                       missing_ok=True).get("data")
    if availability:
        print(f"availability {availability['id']}: exists")
        return
    territories = [t["id"] for t in get_all("/v1/territories", {"limit": 200})]
    print(f"availability: creating, all {len(territories)} territories")
    api("POST", "/v2/appAvailabilities", {
        "data": {"type": "appAvailabilities",
                 "attributes": {"availableInNewTerritories": True},
                 "relationships": {
                     "app": resource("apps", app["id"]),
                     "territoryAvailabilities": {"data": [
                         {"type": "territoryAvailabilities", "id": f"${{{t}}}"}
                         for t in territories]}}},
        "included": [{"type": "territoryAvailabilities", "id": f"${{{t}}}",
                      "attributes": {"available": True},
                      "relationships": {"territory": resource("territories", t)}}
                     for t in territories]})


def ensure_beta_group(app, name, internal):
    groups = get_all("/v1/betaGroups", {"filter[app]": app["id"],
                                        "filter[name]": name})
    if groups:
        print(f"beta group {name} {groups[0]['id']}: exists")
        return groups[0]
    kind = "internal, all builds" if internal else "external"
    print(f"beta group {name}: creating ({kind})")
    attributes = {"name": name, "isInternalGroup": internal}
    if internal:
        attributes["hasAccessToAllBuilds"] = True
    return create("betaGroups", attributes, {"app": resource("apps", app["id"])})


def add_to_group(group, build, label):
    in_group = [b["id"] for b in get_all(f"/v1/betaGroups/{group['id']}/builds")]
    if build["id"] in in_group:
        print(f"  {label} already in the group")
    elif group["attributes"].get("hasAccessToAllBuilds"):
        print(f"  {label}: the group gets every build")
    else:
        print(f"  adding {label}")
        api("POST", f"/v1/betaGroups/{group['id']}/relationships/builds",
            {"data": [{"type": "builds", "id": build["id"]}]})


def valid_build(app, platform, args):
    build = find_build(app["id"], platform, args.version_string, args.number)
    if not build or build["attributes"]["processingState"] != "VALID":
        fail(f"no VALID {PLATFORMS[platform]} build {args.version_string}")
    return build, f"{platform} build {args.version_string} " \
                  f"({build['attributes']['version']})"


def add_internal_tester(group, user):
    a = user["attributes"]
    testers = get_all(f"/v1/betaGroups/{group['id']}/betaTesters")
    if any(t["attributes"].get("email", "").lower() == a["username"].lower()
           for t in testers):
        print(f"  tester {a['username']}: already in the group")
        return
    print(f"  adding tester {a['firstName']} {a['lastName']} <{a['username']}>")
    create("betaTesters", {"email": a["username"], "firstName": a["firstName"],
                           "lastName": a["lastName"]},
           {"betaGroups": {"data": [{"type": "betaGroups", "id": group["id"]}]}})


def invite_internal(args, app):
    email = args.email.lower()
    users = [u for u in get_all("/v1/users", {"limit": 200})
             if u["attributes"]["username"].lower() == email]
    if users:
        print(f"team user {email}: exists")
        group = ensure_beta_group(app, BETA_GROUP, internal=True)
        add_internal_tester(group, users[0])
        return
    invitations = get_all("/v1/userInvitations", {"filter[email]": email})
    if invitations:
        a = invitations[0]["attributes"]
        print(f"team invitation {invitations[0]['id']} for {email}: pending, "
              f"expires {a.get('expirationDate')}; rerun once accepted")
        return
    if not (args.first and args.last):
        fail("invite --internal needs --first and --last")
    print(f"team invitation for {args.first} {args.last} <{email}>: creating "
          "(CUSTOMER_SUPPORT, Cows In Love only)")
    invitation = create("userInvitations",
                        {"email": email, "firstName": args.first,
                         "lastName": args.last, "roles": ["CUSTOMER_SUPPORT"],
                         "allAppsVisible": False, "provisioningAllowed": False},
                        {"visibleApps": {"data": [{"type": "apps",
                                                   "id": app["id"]}]}})
    print(f"  invitation {invitation['id']}; rerun once accepted to add the "
          f"tester to {BETA_GROUP}")


def beta_review_in_progress(app, build):
    pre = api("GET", f"/v1/builds/{build['id']}/preReleaseVersion")["data"]
    others = get_all("/v1/builds", {"filter[app]": app["id"],
                                    "filter[preReleaseVersion]": pre["id"]})
    for other in others:
        review = api("GET", f"/v1/builds/{other['id']}/betaAppReviewSubmission",
                     missing_ok=True).get("data")
        if review and review["attributes"]["betaReviewState"] in (
                "WAITING_FOR_REVIEW", "IN_REVIEW"):
            return other["attributes"]["version"]
    return None


def cmd_invite(args, app):
    if not args.email:
        fail("invite needs an email")
    if args.internal:
        invite_internal(args, app)
        return
    meta = metadata("ios")
    print("beta app localization:")
    wanted = {"description": meta["promotionalText"],
              "feedbackEmail": required(args, "review_email"),
              "marketingUrl": meta["marketingUrl"],
              "privacyPolicyUrl": required(args, "privacy_url")}
    loc = localization(f"/v1/apps/{app['id']}/betaAppLocalizations")
    if loc:
        update("betaAppLocalizations", loc["id"], wanted, loc["attributes"])
    else:
        create("betaAppLocalizations", {"locale": LOCALE, **wanted},
               {"app": resource("apps", app["id"])})
    name = required(args, "review_name").split()
    detail = api("GET", f"/v1/apps/{app['id']}/betaAppReviewDetail")["data"]
    print("beta review detail:")
    update("betaAppReviewDetails", detail["id"],
           {"contactFirstName": " ".join(name[:-1]) or name[0],
            "contactLastName": name[-1],
            "contactPhone": required(args, "review_phone"),
            "contactEmail": required(args, "review_email"),
            "demoAccountRequired": False,
            "notes": args.review_notes or IOS_REVIEW_NOTES}, detail["attributes"])

    group = ensure_beta_group(app, FRIENDS_GROUP, internal=False)
    platforms = [args.platform] if args.number else list(PLATFORMS)
    builds = [valid_build(app, platform, args) for platform in platforms]
    for build, label in builds:
        add_to_group(group, build, label)

    email = args.email.lower()
    testers = get_all("/v1/betaTesters", {"filter[email]": email,
                                          "filter[apps]": app["id"]})
    if not testers:
        print(f"  adding tester {args.first} {args.last} <{email}>")
        create("betaTesters", {"email": email, "firstName": args.first,
                               "lastName": args.last},
               {"betaGroups": {"data": [{"type": "betaGroups", "id": group["id"]}]}})
    else:
        tester = testers[0]
        groups = [g["id"] for g in get_all(f"/v1/betaTesters/{tester['id']}/betaGroups")]
        if group["id"] in groups:
            print(f"  tester {email}: already in the group "
                  f"({tester['attributes'].get('state')})")
        else:
            print(f"  adding tester {email} to the group")
            api("POST", f"/v1/betaTesters/{tester['id']}/relationships/betaGroups",
                {"data": [{"type": "betaGroups", "id": group["id"]}]})

    for build, label in builds:
        review = api("GET", f"/v1/builds/{build['id']}/betaAppReviewSubmission",
                     missing_ok=True).get("data")
        if review:
            print(f"{label}: beta review {review['attributes']['betaReviewState']}")
            continue
        busy = beta_review_in_progress(app, build)
        if busy:
            print(f"{label}: not submitted, build {busy} of the same version is "
                  "still in beta review (Apple allows one); rerun once it is done")
        else:
            print(f"{label}: submitting for beta review")
            create("betaAppReviewSubmissions", {},
                   {"build": resource("builds", build["id"])})


def cmd_testflight(args, app):
    build, label = valid_build(app, args.platform, args)
    print(f"{label} {build['id']}:")
    loc = localization(f"/v1/builds/{build['id']}/betaBuildLocalizations")
    wanted = {"whatsNew": "First build."}
    if loc:
        update("betaBuildLocalizations", loc["id"], wanted, loc["attributes"])
    else:
        print(f"  creating beta localization {LOCALE}")
        create("betaBuildLocalizations", {"locale": LOCALE, **wanted},
               {"build": resource("builds", build["id"])})

    group = ensure_beta_group(app, BETA_GROUP, internal=True)
    add_to_group(group, build, label)

    email = args.review_email
    users = get_all("/v1/users", {"limit": 200})
    user = next((u for u in users if u["attributes"]["username"].lower()
                 == (email or "").lower()), None)
    if not user:
        admins = [u for u in users if "ADMIN" in u["attributes"]["roles"]]
        if len(admins) != 1:
            fail(f"no user {email} and {len(admins)} admins; pick one")
        user = admins[0]
    add_internal_tester(group, user)


def cmd_all(args, app):
    for step in (cmd_info, cmd_version, cmd_screenshots, cmd_build, cmd_price,
                 cmd_submit):
        print(f"== {step.__name__[4:]}")
        step(args, app)


COMMANDS = {"check": cmd_check, "info": cmd_info, "version": cmd_version,
            "screenshots": cmd_screenshots, "build": cmd_build,
            "price": cmd_price, "testflight": cmd_testflight,
            "invite": cmd_invite, "submit": cmd_submit, "all": cmd_all}


def main():
    global WRITE
    env = os.environ.get
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("command", choices=COMMANDS)
    parser.add_argument("email", nargs="?", help="invite: the tester's email")
    parser.add_argument("--first")
    parser.add_argument("--last")
    parser.add_argument("--internal", action="store_true",
                        help="invite: a team invitation and the internal group")
    parser.add_argument("--platform", choices=PLATFORMS, default="ios")
    parser.add_argument("--write", action="store_true",
                        help="send writes; without it they are only printed")
    parser.add_argument("--version-string", default=version_string())
    parser.add_argument("--number", help="build number (CFBundleVersion)")
    parser.add_argument("--copyright", default=env("COWS_COPYRIGHT"))
    parser.add_argument("--support-url", default=env("COWS_SUPPORT_URL"))
    parser.add_argument("--privacy-url", default=env("COWS_PRIVACY_URL"))
    parser.add_argument("--secondary-category", default=env("COWS_SECONDARY_CATEGORY"))
    parser.add_argument("--review-name", default=env("COWS_REVIEW_NAME"))
    parser.add_argument("--review-phone", default=env("COWS_REVIEW_PHONE"))
    parser.add_argument("--review-email", default=env("COWS_REVIEW_EMAIL"))
    parser.add_argument("--review-notes", default=env("COWS_REVIEW_NOTES"))
    args = parser.parse_args()
    WRITE = args.write
    if not WRITE and args.command != "check":
        print("dry run: writes are printed, not sent (--write to send)")
    COMMANDS[args.command](args, find_app())


if __name__ == "__main__":
    main()
