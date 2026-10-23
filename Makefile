.DEFAULT_GOAL := debug
CONFIG ?= Debug
BUILD_DIR ?= build/debug
CMAKE_FLAGS ?=
JOBS ?= 4

.PHONY: debug release configure build run test package web android ios fmt clean
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
	python3 scripts/check_repository.py
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
	cmake -S . -B build/ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator -DCMAKE_OSX_DEPLOYMENT_TARGET=15.0
	cmake --build build/ios --config Release --parallel $(JOBS)
fmt:
	clang-format -i src/main.cpp
clean:
	cmake -E rm -rf build dist
