include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# the fetched one, or -D COWS_EACP_TAG=<branch|sha> to fetch another revision.
# This branch needs eacp's jp/web, which stacks the Emscripten port (WebGPU,
# the browser's event loop, canvas text; eyalamirmusic/eacp#73) on
# jp/android-integration (the Android and Vulkan 1.1 stack, PRs #66-#69, #71),
# so it is the default on every platform. tools/web.sh builds the web against
# ~/projects/eacp-web when that exists.
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
set(COWS_EACP_TAG "jp/web" CACHE STRING
        "eacp branch, tag or commit to fetch")
set(COWS_EACP_REPOSITORY "jamierpond/eacp" CACHE STRING
        "GitHub repository to fetch eacp from (jamierpond/eacp has jp/web)")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
