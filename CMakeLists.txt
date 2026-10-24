cmake_minimum_required(VERSION 3.24)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" SOTN_VERSION LIMIT_COUNT 1)
project(sotn-native VERSION ${SOTN_VERSION} LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
option(SOTN_ENABLE_SANITIZERS "Instrument the app and SDL with ASan and UBSan on Linux" OFF)
if(SOTN_ENABLE_SANITIZERS)
  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" OR
     NOT CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "SOTN_ENABLE_SANITIZERS requires Linux with GCC or Clang")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
include(cmake/Dependencies.cmake)
include(cmake/ProjectOptions.cmake)
include(cmake/ReferenceData.cmake)
add_subdirectory(modules/data)
add_subdirectory(modules/core)
add_subdirectory(modules/save)
add_subdirectory(modules/audio)
add_subdirectory(modules/render)
add_subdirectory(modules/game)

if(ANDROID)
  # SDL's Android HID backend needs the static C++ runtime; application sources remain C.
  enable_language(CXX)
  add_library(sotn-native SHARED src/main.c src/lab_preview.c src/lab_preview.h)
  set_target_properties(sotn-native PROPERTIES OUTPUT_NAME main LINKER_LANGUAGE CXX)
  set_property(TARGET sotn-native PROPERTY LINK_LIBRARY_OVERRIDE "WHOLE_ARCHIVE,SDL3::SDL3-static")
  if(DEFINED SOTN_ANDROID_JAVA_DIR)
    file(MAKE_DIRECTORY "${SOTN_ANDROID_JAVA_DIR}")
    file(COPY "${sdl3_SOURCE_DIR}/android-project/app/src/main/java/org"
         DESTINATION "${SOTN_ANDROID_JAVA_DIR}")
  endif()
else()
  add_executable(sotn-native src/main.c src/lab_preview.c src/lab_preview.h)
endif()
target_link_libraries(sotn-native PRIVATE sotn-game)
sotn_configure_c_target(sotn-native)
target_compile_definitions(sotn-native PRIVATE SOTN_VERSION="${PROJECT_VERSION}")

if(APPLE)
  if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    set(SOTN_IOS_PLIST "<key>LSRequiresIPhoneOS</key><true/><key>UILaunchScreen</key><dict/><key>UIApplicationSupportsIndirectInputEvents</key><true/>")
  endif()
  set_target_properties(sotn-native PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER org.bluesentinelsec.sotn-native
    MACOSX_BUNDLE_BUNDLE_NAME "SOTN Native"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/platforms/apple/Info.plist.in"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED NO
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
  )
endif()

if(EMSCRIPTEN)
  set_target_properties(sotn-native PROPERTIES OUTPUT_NAME index SUFFIX .html)
  target_link_options(sotn-native PRIVATE
    "--shell-file=${CMAKE_CURRENT_SOURCE_DIR}/platforms/web/shell.html"
    -sALLOW_MEMORY_GROWTH=1 -sEXIT_RUNTIME=1 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
  )
  set_property(TARGET sotn-native APPEND PROPERTY LINK_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/platforms/web/shell.html")
endif()

include(CTest)
if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
  add_test(NAME hello-world COMMAND ${CMAKE_COMMAND}
    "-DAPP=$<TARGET_FILE:sotn-native>" -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/run_smoke.cmake")
  set_tests_properties(hello-world PROPERTIES
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy"
    TIMEOUT 20)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_subdirectory(tests/data)
  add_subdirectory(tests/core)
  add_subdirectory(tests/render)
  add_subdirectory(tests/game)
  add_subdirectory(tests/audio)
  add_subdirectory(tests/save)
  add_test(NAME repository-policy-tests COMMAND ${Python3_EXECUTABLE}
    -m unittest discover -s "${CMAKE_CURRENT_SOURCE_DIR}/tests" -v)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    add_test(NAME repository-policy COMMAND ${Python3_EXECUTABLE}
      "${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_repository.py")
  endif()
endif()

if(NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS" AND NOT EMSCRIPTEN)
  add_custom_target(run COMMAND $<TARGET_FILE:sotn-native> DEPENDS sotn-native USES_TERMINAL)
endif()

include(GNUInstallDirs)
if(EMSCRIPTEN)
  install(FILES "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.html"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.js"
                "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/index.wasm" DESTINATION .)
elseif(ANDROID)
  install(TARGETS sotn-native LIBRARY DESTINATION lib)
else()
  install(TARGETS sotn-native RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} BUNDLE DESTINATION .)
endif()
install(FILES LICENSE README.md THIRD_PARTY.md DESTINATION .)
install(FILES "${sdl3_SOURCE_DIR}/LICENSE.txt" DESTINATION licenses RENAME SDL3.txt)
install(FILES licenses/AGPL-3.0-or-later.txt DESTINATION licenses)
install(FILES licenses/MPL-2.0.txt licenses/PSY-Z-decomp-MIT.txt DESTINATION licenses)
set(CPACK_PACKAGE_NAME sotn-native)
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR bluesentinelsec)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Asset-free SDL3 bootstrap for SOTN Native")
string(TOLOWER "${CMAKE_SYSTEM_NAME}" SOTN_PACKAGE_SYSTEM)
set(SOTN_PACKAGE_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
if(EMSCRIPTEN)
  set(SOTN_PACKAGE_ARCH wasm32)
elseif(APPLE AND CMAKE_OSX_ARCHITECTURES)
  string(REPLACE ";" "-" SOTN_PACKAGE_ARCH "${CMAKE_OSX_ARCHITECTURES}")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  string(TOLOWER "${CMAKE_OSX_SYSROOT}" SOTN_APPLE_SYSROOT)
  if(SOTN_APPLE_SYSROOT MATCHES "iphonesimulator")
    set(SOTN_PACKAGE_SYSTEM ios-simulator)
  else()
    set(SOTN_PACKAGE_SYSTEM ios-device-unsigned)
  endif()
endif()
set(CPACK_PACKAGE_FILE_NAME "sotn-native-${PROJECT_VERSION}-${SOTN_PACKAGE_SYSTEM}-${SOTN_PACKAGE_ARCH}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/dist")
set(CPACK_GENERATOR ZIP)
set(CPACK_MONOLITHIC_INSTALL ON)
include(CPack)
