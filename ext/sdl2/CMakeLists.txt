fetchcontent_declare(
        SDL2
        #GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        URL https://www.libsdl.org/release/SDL2-2.0.22.zip
        #GIT_TAG "main"
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(SDL2)
add_library(thirdparty::sdl2 ALIAS SDL2)
add_library(thirdparty::sdl2-main ALIAS SDL2main)

set_target_properties(SDL2 PROPERTIES FOLDER "ThirdParty")
set_target_properties(SDL2main PROPERTIES FOLDER "ThirdParty")

