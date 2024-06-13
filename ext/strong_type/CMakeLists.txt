fetchcontent_declare(
        STRONGTYPE
        URL https://github.com/rollbear/strong_type/archive/refs/tags/v14.zip
        UPDATE_COMMAND ""
)

fetchcontent_makeavailable(STRONGTYPE)
add_library(thirdparty::strong_type ALIAS strong_type)
set_target_properties(strong_type PROPERTIES FOLDER "ThirdParty")
