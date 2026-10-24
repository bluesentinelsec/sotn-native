add_library(sotn-save STATIC
  save.c
  store.c
  save_internal.h
  include/sotn/save.h
)
sotn_configure_c_target(sotn-save)
target_include_directories(sotn-save PUBLIC include)
target_link_libraries(sotn-save PUBLIC sotn-data)
