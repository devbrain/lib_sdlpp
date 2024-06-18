set(SDL2_VERSION "2.30.3")
set(SDL2_IMAGE_VERSION "2.8.2")
set(SDL2_TTF_VERSION "2.22.0")
set(SDL2_MIXER_VERSION "release-2.8.0")

fetchcontent_declare(
        SDL2
        #GIT_REPOSITORY "https://github.com/libsdl-org/SDL.git"
        URL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip
        #GIT_TAG "main"
        UPDATE_COMMAND ""
        OVERRIDE_FIND_PACKAGE
)

fetchcontent_makeavailable(SDL2)
add_library(thirdparty::sdl2 ALIAS SDL2)
add_library(thirdparty::sdl2-main ALIAS SDL2main)

set_target_properties(SDL2 PROPERTIES FOLDER "ThirdParty")
set_target_properties(SDL2main PROPERTIES FOLDER "ThirdParty")
# -----------------------------------------------------------------------------------------------
fetchcontent_declare(
        SDL_IMAGE
        URL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(SDL_IMAGE)
add_library(thirdparty::sdl2_image ALIAS SDL2_image)
set_target_properties(SDL2_image PROPERTIES FOLDER "ThirdParty")

# -----------------------------------------------------------------------------------------------
fetchcontent_declare(
        SDL_TTF
        URL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(SDL_TTF)
add_library(thirdparty::sdl2_ttf ALIAS SDL2_ttf)
set_target_properties(SDL2_ttf PROPERTIES FOLDER "ThirdParty")

# -----------------------------------------------------------------------------------------------
set(SDL2MIXER_VENDORED ON CACHE BOOL  "" FORCE)

fetchcontent_declare(
        SDL_MIXER
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG "a37e09f"
        GIT_SHALLOW TRUE
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(SDL_MIXER)
add_library(thirdparty::sdl2_mixer ALIAS SDL2_mixer)
set_target_properties(SDL2_mixer PROPERTIES FOLDER "ThirdParty")
set_target_properties(opus PROPERTIES FOLDER "ThirdParty")
set_target_properties(opusfile PROPERTIES FOLDER "ThirdParty")
set_target_properties(ogg PROPERTIES FOLDER "ThirdParty")
set_target_properties(wavpack PROPERTIES FOLDER "ThirdParty")

if (TARGET xmp_shared)
	set_target_properties(xmp_shared PROPERTIES FOLDER "ThirdParty")
endif()

set_target_properties(SDL2_test PROPERTIES FOLDER "ThirdParty")
set_target_properties(sdl_headers_copy PROPERTIES FOLDER "ThirdParty")
set_target_properties(freetype PROPERTIES FOLDER "ThirdParty")
set_target_properties(uninstall PROPERTIES FOLDER "ThirdParty")










