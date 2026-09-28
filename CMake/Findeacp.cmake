include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# the fetched one, or -D COWS_EACP_TAG=<branch|sha> to fetch another revision
# (the Android build needs eacp's android-mvp branch until it lands on main:
# -D COWS_EACP_REPOSITORY=jamierpond/eacp -D COWS_EACP_TAG=android-mvp).
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
set(COWS_EACP_TAG "main" CACHE STRING "eacp branch, tag or commit to fetch")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp" CACHE STRING
        "GitHub repository to fetch eacp from (jamierpond/eacp has android-mvp)")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
