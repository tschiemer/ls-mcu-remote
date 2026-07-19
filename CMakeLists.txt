cmake_minimum_required(VERSION 3.24)
project(ls_mcu_remote)

set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(FetchContent)

include(ls-mcu-remote.oscpp)
include(ls-mcu-remote.json)
include(ls-mcu-remote.libremidi)

#include(ls-mcu-remote.asio)

find_library(COREMIDI_LIBRARY CoreMIDI)
find_library(COREAUDIO_LIBRARY CoreAudio)
find_library(COREFOUNDATION_LIBRARY CoreFoundation)


add_library(lsmcuremote
        src/include/ls-mcu-remote/LsOscDef.h
        src/include/ls-mcu-remote/LsProxy.h
        src/core/LsProxy.cpp src/core/Controller.cpp src/include/ls-mcu-remote/Controller.h src/include/ls-mcu-remote/version.h)
target_include_directories(lsmcuremote PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include/ls-mcu-remote)

target_compile_definitions(lsmcuremote PUBLIC LIBREMIDI_COREMIDI)
target_link_libraries(lsmcuremote PUBLIC oscpp::oscpp nlohmann_json::nlohmann_json libremidi::libremidi
        ${COREFOUNDATION_LIBRARY}
        ${COREAUDIO_LIBRARY}
        ${COREMIDI_LIBRARY})

#target_compile_definitions(libremidi ${_public} LIBREMIDI_COREMIDI)

add_executable(ls-mcu-remote src/apps/ls-mcu-remote/ls-mcu-remote.cpp)
target_include_directories(ls-mcu-remote PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include/ls-mcu-remote)
#target_include_directories(ls_mcu_remote PUBLIC /src/include)
target_link_libraries(ls-mcu-remote PRIVATE lsmcuremote)
target_compile_definitions(ls-mcu-remote PUBLIC LIBREMIDI_COREMIDI)

add_executable(playground main.cpp)
target_include_directories(playground PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include/ls-mcu-remote)
#target_include_directories(ls_mcu_remote PUBLIC /src/include)
target_link_libraries(playground PRIVATE lsmcuremote)
