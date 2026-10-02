include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# the fetched one, or -D COWS_EACP_TAG=<branch|sha> to fetch another revision.
# This branch needs eacp's jp/android-vulkan-1-1 until the PRs it stacks land:
# #84 (the Android port) and #69 (Vulkan 1.1 phones such as the Galaxy S22). It
# is develop plus both, so that is the default here.
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
set(COWS_EACP_TAG "jp/android-vulkan-1-1" CACHE STRING "eacp branch, tag or commit to fetch")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp" CACHE STRING
        "GitHub repository to fetch eacp from")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
