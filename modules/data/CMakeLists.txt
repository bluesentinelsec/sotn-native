sotn_generate_reference("${PROJECT_BINARY_DIR}/generated/reference_data.h")
add_library(sotn-data STATIC
  game_data.c
  sha1.c
  data_internal.h
  include/sotn/game_data.h
)
sotn_configure_c_target(sotn-data)
target_include_directories(sotn-data PUBLIC include PRIVATE "${PROJECT_BINARY_DIR}/generated")
target_link_libraries(sotn-data PUBLIC SDL3::SDL3-static)
