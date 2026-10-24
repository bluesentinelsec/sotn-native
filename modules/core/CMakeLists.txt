add_library(sotn-core STATIC
  collision.c
  decompress.c
  include/sotn/collision.h
  include/sotn/decompress.h
)
sotn_configure_c_target(sotn-core)
target_include_directories(sotn-core PUBLIC include)
target_link_libraries(sotn-core PUBLIC sotn-data)
