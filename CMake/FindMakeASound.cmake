include(CPM)

# Configure with -D CPM_MakeASound_SOURCE=<path> to build a local checkout
# instead of this one. Until MakeASound takes the iOS session fix (Playback
# asked for Bluetooth and AirPlay, which an iPhone refuses, so no device ever
# opened), this is that branch on Jamie's fork.
CPMAddPackage(
        NAME MakeASound
        GITHUB_REPOSITORY jamierpond/MakeASound
        GIT_TAG 6541045)
