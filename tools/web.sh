#!/usr/bin/env bash
# Usage: tools/web.sh build              build Cows.js/.wasm and index.html into
#                                        build-web/Apps/CowsInLove/
#        tools/web.sh serve [port]       serve it (default 8000) with COOP/COEP
#        tools/web.sh shot [shot.png]    build, serve, and screenshot it in
#                                        headless Chromium with WebGPU on
#
# Needs Emscripten (brew install emscripten) and eacp's jp/web branch:
# EACP=<path> (default ~/projects/eacp-web) builds against that checkout, or,
# when it is absent, fetches jamierpond/eacp@jp/web through CPM. Likewise
# MAKEASOUND=<path> (default ~/projects/MakeASound, when present) builds against
# that MakeASound checkout instead of the pinned one.
# COWS_CONFIG=Release|Debug (default Release) picks the build type, each in its
# own build dir. COWS_QUERY="seed=3&stage=1" is appended to the page's URL for
# serve and shot; the app reads its COWS_* settings from it (see
# Apps/CowsInLove/Source/Settings.h). shot needs node; it installs Playwright
# and its Chromium into COWS_WEB_TOOLS (default $TMPDIR/cows-web-tools), never
# the repo, and waits COWS_SHOT_DELAY seconds (default 8).
set -euo pipefail

root="$(cd "$(dirname "$0")/.." && pwd)"
cd "$root"

eacp="${EACP:-$HOME/projects/eacp-web}"
makeasound="${MAKEASOUND:-$HOME/projects/MakeASound}"
config="${COWS_CONFIG:-Release}"
build=build-web
[[ $config == Release ]] || build="build-web-$(echo "$config" | tr '[:upper:]' '[:lower:]')"
site="$build/Apps/CowsInLove"
tools="${COWS_WEB_TOOLS:-${TMPDIR:-/tmp}/cows-web-tools}"
query="${COWS_QUERY:+?$COWS_QUERY}"

build() {
    command -v emcmake >/dev/null || { echo "no emcmake: brew install emscripten" >&2; exit 1; }

    local source_args=(-DCPM_eacp_SOURCE="$eacp")
    [[ -d $eacp ]] || source_args=(-DCOWS_EACP_REPOSITORY=jamierpond/eacp
                                 -DCOWS_EACP_TAG=jp/web)
    [[ -d $makeasound ]] && source_args+=(-DCPM_MakeASound_SOURCE="$makeasound")

    emcmake cmake -G Ninja -B $build -DCMAKE_BUILD_TYPE="$config" \
        "${source_args[@]}" -DCOWS_BUILD_TESTS=OFF >/dev/null
    cmake --build $build --target Cows
}

serve() {
    local port="${1:-8000}"

    exec python3 - "$site" "$port" <<'PY'
import functools, http.server, sys

class Handler(http.server.SimpleHTTPRequestHandler):
    extensions_map = {**http.server.SimpleHTTPRequestHandler.extensions_map,
                      ".wasm": "application/wasm", ".js": "text/javascript"}

    # Cross-origin isolation, so SharedArrayBuffer and audio worklets can work.
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def log_message(self, *args):
        pass

site, port = sys.argv[1], int(sys.argv[2])
server = http.server.ThreadingHTTPServer(
    ("127.0.0.1", port), functools.partial(Handler, directory=site))
print(f"http://localhost:{port}/", flush=True)
server.serve_forever()
PY
}

install_playwright() {
    command -v node >/dev/null || { echo "no node: skipping the shot" >&2; exit 0; }

    if [[ ! -d $tools/node_modules/playwright ]]; then
        mkdir -p "$tools"
        (cd "$tools" && npm init -y >/dev/null && npm install --silent playwright >&2) || {
            echo "could not install Playwright: skipping the shot" >&2
            exit 0
        }
    fi

    (cd "$tools" && npx playwright install chromium >&2) || {
        echo "no Chromium for Playwright: skipping the shot" >&2
        exit 0
    }

    cat > "$tools/shot.mjs" <<'JS'
import { chromium } from "playwright";

const [url, out, seconds] = process.argv.slice(2);
const browser = await chromium.launch({
    channel: "chromium",
    args: ["--enable-unsafe-webgpu", "--ignore-gpu-blocklist",
           "--autoplay-policy=no-user-gesture-required"],
});
const page = await browser.newPage({ viewport: { width: 1280, height: 800 } });
page.on("console", (message) => console.error(`[page] ${message.text()}`));
page.on("pageerror", (error) => console.error(`[page error] ${error.message}`));

await page.goto(url);
const gpu = await page.evaluate(async () => !navigator.gpu ? "no navigator.gpu"
    : (await navigator.gpu.requestAdapter()) ? "adapter" : "no adapter");
console.error(`[shot] WebGPU: ${gpu}`);
await page.waitForTimeout(Number(seconds) * 1000);
await page.screenshot({ path: out });
await browser.close();
JS
}

shot() {
    local out="${1:-docs/shots/web-$(date +%Y%m%d-%H%M%S).png}"
    local port="${COWS_PORT:-8741}"

    install_playwright
    mkdir -p "$(dirname "$out")"

    serve "$port" >/dev/null &
    server=$!
    trap 'kill "$server" 2>/dev/null || true' EXIT
    sleep 1

    node "$tools/shot.mjs" "http://localhost:$port/$query" "$out" \
        "${COWS_SHOT_DELAY:-8}"
    echo "$out"
}

case "${1:-}" in
    build)
        build
        ;;
    serve)
        [[ -f $site/Cows.js ]] || build
        echo "open http://localhost:${2:-8000}/$query" >&2
        serve "${2:-}"
        ;;
    shot)
        build
        shot "${2:-}"
        ;;
    *)
        sed -n '2,17p' "$0"
        exit 1
        ;;
esac
