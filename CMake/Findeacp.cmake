include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# the fetched one, or -D COWS_EACP_TAG=<branch|sha> to fetch another revision.
# develop has the controller feed (#106) on top of the Android port (#84, #93).
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
set(COWS_EACP_TAG "develop" CACHE STRING "eacp branch, tag or commit to fetch")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp" CACHE STRING
        "GitHub repository to fetch eacp from")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
