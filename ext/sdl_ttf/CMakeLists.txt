fetchcontent_declare(
        SDL_TTF
        URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-2.22.0/SDL2_ttf-2.22.0.zip
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(SDL_TTF)
add_library(thirdparty::sdl2_ttf ALIAS SDL2_ttf)
#set_target_properties(SDL2_TTF PROPERTIES FOLDER "ThirdParty")


