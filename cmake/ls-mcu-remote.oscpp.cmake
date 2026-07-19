# https://github.com/kaoskorobase/oscpp/blob/master/README.md

include(FetchContent)
FetchContent_Declare(oscpp
        GIT_REPOSITORY https://github.com/kaoskorobase/oscpp.git
        GIT_TAG        1.0.0
        )
FetchContent_MakeAvailable(oscpp)