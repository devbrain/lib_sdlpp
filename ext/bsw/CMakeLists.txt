set(BUILD_SHARED_LIBS TRUE CACHE INTERNAL "")
set(bsw_ENABLE_TESTING FALSE CACHE INTERNAL "")


# SET(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE INTERNAL "")

fetchcontent_declare(
        BSW_EXT
        GIT_REPOSITORY https://github.com/devbrain/lib_bsw.git
        GIT_PROGRESS TRUE
        GIT_TAG master
        TIMEOUT 10
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(BSW_EXT)

