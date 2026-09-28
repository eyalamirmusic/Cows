include(CPM)

# Configure with -D CPM_MakeASound_SOURCE=<path> to build a local checkout
# instead of this one.
CPMAddPackage(
        NAME MakeASound
        GITHUB_REPOSITORY eyalamirmusic/MakeASound
        GIT_TAG 985d0eac662c581048067e0751acdfeb0e11bb06)
