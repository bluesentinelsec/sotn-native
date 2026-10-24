add_library(sotn-render STATIC
  presenter.c
  gl_api.c
  gl_api.h
  layout.c
  layout.h
  save_icon.c
  hud.c
  include/sotn/hud.h
  tilemap.c
  alucard_sprite.c
  actor_graphics.c
  shaders.h
  include/sotn/presenter.h
  include/sotn/save_icon.h
  include/sotn/tilemap.h
  include/sotn/alucard_sprite.h
  include/sotn/actor_graphics.h
)
sotn_configure_c_target(sotn-render)
target_include_directories(sotn-render PUBLIC include)
target_link_libraries(sotn-render PUBLIC sotn-core)
