include(CPM)

# Jamie's fork until MakeASound takes the iOS session fix (no device ever opened).
CPMAddPackage(
        NAME MakeASound
        GITHUB_REPOSITORY jamierpond/MakeASound
        GIT_TAG 6541045)
