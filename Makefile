.DEFAULT_GOAL := debug
CONFIG ?= Debug
BUILD_DIR ?= build/debug
CMAKE_FLAGS ?=
JOBS ?= 4
SOTN_FORMAT_SOURCES := tests/render/test_launch_renderer.c \
  modules/core/launch_menu.c modules/core/include/sotn/launch_menu.h \
  modules/render/launch_renderer.c modules/render/include/sotn/launch_renderer.h tests/core/test_launch_menu.c \
  modules/render/castle_map.c modules/render/include/sotn/castle_map.h tests/render/test_castle_map.c \
  modules/data/stage_assets.c modules/data/include/sotn/stage_assets.h \
  tests/data/test_stage_assets.c \
  src/main.c src/lab_preview.c src/lab_preview.h \
  modules/data/game_data.c \
  modules/data/sha1.c \
  modules/data/resource.c \
  modules/data/data_internal.h \
  modules/data/include/sotn/game_data.h \
  modules/data/include/sotn/resource.h \
  tests/data/test_data.c \
  modules/render/presenter.c \
  modules/render/gl_api.c \
  modules/render/gl_api.h \
  modules/render/layout.c \
  modules/render/layout.h \
  modules/render/save_icon.c \
  modules/render/tilemap.c \
  modules/render/alucard_sprite.c \
  modules/render/actor_graphics.c modules/render/include/sotn/actor_graphics.h \
  tests/render/test_actor_graphics.c \
  modules/render/shaders.h \
  modules/render/include/sotn/presenter.h \
  modules/render/include/sotn/save_icon.h \
  modules/render/include/sotn/tilemap.h \
  modules/render/include/sotn/alucard_sprite.h \
  tests/render/test_images.c \
  tests/render/test_tilemap.c tests/game/test_input.c
SOTN_FORMAT_SOURCES += modules/core/collision.c \
  modules/core/animation.c \
  modules/core/decompress.c \
  modules/core/stage.c \
  modules/core/actor_layout.c modules/core/room_actors.c \
  modules/core/actors.c modules/core/include/sotn/actors.h tests/core/test_actors.c \
  modules/core/combat.c modules/core/include/sotn/combat.h tests/core/test_combat.c \
  modules/core/include/sotn/actor_layout.h tests/core/test_actor_layout.c \
  modules/core/player.c \
  modules/core/player_sensors.c \
  modules/core/player_internal.h \
  modules/core/include/sotn/collision.h \
  modules/core/include/sotn/animation.h \
  modules/core/include/sotn/decompress.h \
  modules/core/include/sotn/stage.h \
  modules/core/include/sotn/player.h \
  tests/core/test_core.c \
  tests/core/test_animation.c \
  tests/core/test_player.c \
  tests/core/test_stage.c
SOTN_FORMAT_SOURCES += modules/game/laboratory.c modules/game/input.c modules/game/input.h \
  modules/game/include/sotn/laboratory.h

SOTN_FORMAT_SOURCES += modules/data/stream.c modules/data/include/sotn/stream.h \
  modules/audio/music.c modules/audio/include/sotn/music.h tests/audio/test_music.c tests/data/test_stream.c
SOTN_FORMAT_SOURCES += modules/audio/sound.c modules/audio/voice.h modules/audio/gauss.h \
  modules/audio/include/sotn/sound.h tests/audio/test_sound.c

SOTN_FORMAT_SOURCES += modules/audio/playback.c modules/audio/playback_internal.h \
  modules/audio/include/sotn/playback.h tests/audio/test_playback.c

SOTN_FORMAT_SOURCES += modules/core/equipment.c modules/core/include/sotn/equipment.h tests/core/test_equipment.c

SOTN_FORMAT_SOURCES += modules/render/hud.c modules/render/include/sotn/hud.h tests/render/test_hud.c

SOTN_FORMAT_SOURCES += modules/render/game_over.c modules/render/include/sotn/game_over.h tests/render/test_game_over.c

SOTN_FORMAT_SOURCES += modules/save/save.c modules/save/store.c modules/save/save_internal.h \
  modules/save/include/sotn/save.h tests/save/test_save.c

SOTN_FORMAT_SOURCES += modules/core/save_point.c modules/core/include/sotn/save_point.h \
  modules/render/save_point.c modules/render/include/sotn/save_point_renderer.h \
  tests/core/test_save_point.c tests/render/test_save_point.c

SOTN_FORMAT_SOURCES += modules/core/character.c modules/core/include/sotn/character.h tests/core/test_character.c

SOTN_FORMAT_SOURCES += modules/core/inventory_menu.c modules/core/include/sotn/inventory_menu.h

SOTN_FORMAT_SOURCES += modules/render/inventory.c modules/render/include/sotn/inventory.h

SOTN_FORMAT_SOURCES += tests/core/character_fixture.h tests/core/test_inventory_menu.c tests/render/test_inventory.c

SOTN_FORMAT_SOURCES += tests/core/test_progression.c

SOTN_FORMAT_SOURCES += modules/core/subweapon.c modules/core/include/sotn/subweapon.h

SOTN_FORMAT_SOURCES += modules/render/subweapon.c modules/render/include/sotn/subweapon_renderer.h

SOTN_FORMAT_SOURCES += tests/core/test_subweapon.c

SOTN_FORMAT_SOURCES += modules/core/actors_internal.h modules/core/actor_doors.c \
	  modules/core/actor_cannon.c \
	  modules/core/actor_objects.c \
	  modules/core/actor_mechanisms.c \
	  modules/core/actor_collision.c modules/core/enemy_skeleton.c

SOTN_FORMAT_SOURCES += tests/core/test_boss.c \
	  tests/core/test_game_math.c \
	  tests/core/test_camera.c \
	  tests/core/test_world.c \
	  tests/core/test_enemy.c

SOTN_FORMAT_SOURCES += modules/core/actor_primitives.c modules/core/enemy_knight.c

SOTN_FORMAT_SOURCES += modules/core/enemy_spittle.c

SOTN_FORMAT_SOURCES += modules/core/boss_projection.c \
	  modules/core/boss_projection.h \
	  modules/core/boss_reward.c \
	  modules/core/boss_effects.c \
	  modules/core/boss_event.c \
	  modules/core/boss_slogra.c \
	  modules/core/boss_gaibon.c \
	  modules/core/game_math.c \
	  modules/core/include/sotn/game_math.h \
	  modules/core/camera.c \
	  modules/core/include/sotn/camera.h \
	  modules/core/actor_camera.c \
	  modules/core/world.c modules/core/include/sotn/world.h

.PHONY: debug release configure build run test sanitizer package web android ios fmt clean
configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(CONFIG) $(CMAKE_FLAGS)
build: configure
	cmake --build $(BUILD_DIR) --config $(CONFIG) --parallel $(JOBS)
debug:
	$(MAKE) build CONFIG=Debug BUILD_DIR=build/debug
release:
	$(MAKE) build CONFIG=Release BUILD_DIR=build/release
run: build
	cmake --build $(BUILD_DIR) --config $(CONFIG) --target run
test: build
	ctest --test-dir $(BUILD_DIR) -C $(CONFIG) --output-on-failure
sanitizer:
	$(MAKE) test CONFIG=Debug BUILD_DIR=build/sanitizer CMAKE_FLAGS="$(CMAKE_FLAGS) -DSOTN_ENABLE_SANITIZERS=ON"
package:
	$(MAKE) build CONFIG=Release BUILD_DIR=build/release
	ctest --test-dir build/release -C Release --output-on-failure
	cmake --build build/release --config Release --target package
web:
	emcmake cmake -S . -B build/web -DCMAKE_BUILD_TYPE=Release
	cmake --build build/web --parallel $(JOBS)
	cmake --build build/web --target package
android:
	cd platforms/android && ./gradlew :app:assembleDebug :app:assembleRelease
ios:
	cmake -S . -B build/ios-simulator -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
	cmake --build build/ios-simulator --parallel $(JOBS)
fmt:
	clang-format -i $(SOTN_FORMAT_SOURCES)
clean:
	cmake -E rm -rf build dist

SOTN_FORMAT_SOURCES += modules/core/actor_wide_animation.c modules/core/enemy_flea.c \
  modules/core/enemy_slinger.c modules/core/enemy_ouija.c modules/core/enemy_rose.c \
  modules/core/enemy_rose_seed.c tests/core/test_marble_enemies.c
