include(CPM)

# A plain value, not a cache entry, so a stale cache can never build an older eacp.
set(COWS_EACP_TAG "jp/develop/2026-11-16.6")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
