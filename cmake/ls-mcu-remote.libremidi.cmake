# https://github.com/kaoskorobase/oscpp/blob/master/README.md

include(FetchContent)
FetchContent_Declare(libremidi
        GIT_REPOSITORY https://github.com/celtera/libremidi.git
        GIT_TAG        master
        )
FetchContent_MakeAvailable(libremidi)