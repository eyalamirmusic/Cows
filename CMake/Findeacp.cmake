include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# the fetched one, or -D COWS_EACP_TAG=<branch|sha> to fetch another revision.
# This release pin combines the Android port, the Vulkan 1.1 compatibility
# path, render scaling used by the adaptive quality tiers and hasCode (#125).
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
set(COWS_EACP_TAG "af7b7037bc8530658b0edb179e16c0facf07758e" CACHE STRING
        "eacp branch, tag or commit to fetch")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp" CACHE STRING
        "GitHub repository to fetch eacp from")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
