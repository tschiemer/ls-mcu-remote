### get version from git tag
# https://ipenguin.ws/2012/11/cmake-automatically-use-git-tags-as.html

include(GetGitRevisionDescription)


function(make_version_src _template_in _src_out)

    git_describe(VERSION --tags)

    #parse the version information into pieces.
    string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" VERSION_MAJOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" VERSION_MINOR "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" VERSION_PATCH "${VERSION}")
    string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" VERSION_SHA1 "${VERSION}")
    set(VERSION_SHORT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

    message(STATUS "git version: ${VERSION} (${VERSION_SHORT})")

    configure_file(${_template_in} ${_src_out})
    set(VERSION_SRC ${_src_out})
endfunction()
