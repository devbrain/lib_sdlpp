set(BUILD_SHARED_LIBS TRUE CACHE INTERNAL "")
set(bsw_ENABLE_TESTING FALSE CACHE INTERNAL "")


# SET(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE INTERNAL "")

fetchcontent_declare(
        MATH_EXT
        GIT_REPOSITORY https://github.com/devbrain/neutrino_math.git
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_TAG main
        TIMEOUT 10
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(MATH_EXT)

ADD_TO_FOLDER(neutrino_math "Libs")



