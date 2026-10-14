include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# the fetched one. The pin is a plain value, not a cache entry, so a stale
# cache or preset can never build an older eacp than this file names.
# This release pin combines the Android port, the Vulkan 1.1 compatibility
# path, render scaling used by the adaptive quality tiers and hasCode (#125).
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
set(COWS_EACP_TAG "af7b7037bc8530658b0edb179e16c0facf07758e")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
