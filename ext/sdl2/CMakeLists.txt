fetchcontent_declare(
        SDL2
        #GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        URL https://github.com/libsdl-org/SDL/releases/download/release-2.30.3/SDL2-2.30.3.zip
        #GIT_TAG "main"
        UPDATE_COMMAND ""
        OVERRIDE_FIND_PACKAGE
)

fetchcontent_makeavailable(SDL2)
add_library(thirdparty::sdl2 ALIAS SDL2)
add_library(thirdparty::sdl2-main ALIAS SDL2main)

set_target_properties(SDL2 PROPERTIES FOLDER "ThirdParty")
set_target_properties(SDL2main PROPERTIES FOLDER "ThirdParty")

