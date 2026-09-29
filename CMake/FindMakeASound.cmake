include(CPM)

# Configure with -D CPM_MakeASound_SOURCE=<path> to build a local checkout
# instead of this one. jamierpond/MakeASound jp/web is upstream 985d0ea plus the
# web build (eyalamirmusic/MakeASound#2): no recovery thread and Web Audio's
# channel count under Emscripten, and rtmidi's dummy backend on Emscripten and
# Android.
CPMAddPackage(
        NAME MakeASound
        GITHUB_REPOSITORY jamierpond/MakeASound
        GIT_TAG 79116b094d533df85d6a3a8293cdd0253283e14d)
