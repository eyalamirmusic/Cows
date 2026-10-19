include(CPM)

# A plain value, not a cache entry, so a stale cache can never build an older eacp.
set(COWS_EACP_TAG "af7b7037bc8530658b0edb179e16c0facf07758e")
set(COWS_EACP_REPOSITORY "eyalamirmusic/eacp")

CPMAddPackage(
        NAME eacp
        GITHUB_REPOSITORY ${COWS_EACP_REPOSITORY}
        GIT_TAG ${COWS_EACP_TAG}
        OPTIONS
            "EACP_BUILD_WEBVIEW OFF")
