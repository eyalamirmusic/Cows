include(CPM)

# A plain value, not a cache entry, so a stale cache can never build an older eacp.
set(COWS_EACP_TAG "7b1a7c805023903c3d3343526fe0449dd7509383")
set(COWS_EACP_REPOSITORY "jamierpond/eacp")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
