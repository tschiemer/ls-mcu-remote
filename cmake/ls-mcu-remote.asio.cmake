# https://github.com/chriskohlhoff/asio/issues/1208#issuecomment-1875658450
FetchContent_Declare(asio
        GIT_REPOSITORY  https://github.com/chriskohlhoff/asio.git
        GIT_TAG         asio-1-38-1
        GIT_SHALLOW     TRUE
        )
FetchContent_MakeAvailable(asio)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)
# Use as standalone library and do not allow deprecated features
target_compile_definitions(asio INTERFACE ASIO_STANDALONE ASIO_NO_DEPRECATED)
# Link threads as dependency
#target_link_libraries(asio INTERFACE <threads>)