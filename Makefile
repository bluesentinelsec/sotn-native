.DEFAULT_GOAL := debug
CONFIG ?= Debug
BUILD_DIR ?= build/debug
CMAKE_FLAGS ?=
JOBS ?= 4
SOTN_FORMAT_SOURCES := src/main.c src/lab_preview.c src/lab_preview.h \
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
