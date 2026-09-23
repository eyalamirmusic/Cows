include(CPM)

# Configure with -D CPM_eacp_SOURCE=<path> to build a local checkout instead of
# this one.
#
# The WebView module pulls in WKWebView / WebView2 and a Node toolchain for its
# schema codegen, none of which a GPU scene touches.
CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY eyalamirmusic/eacp
        GIT_TAG main
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
