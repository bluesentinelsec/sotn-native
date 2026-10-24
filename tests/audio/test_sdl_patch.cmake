file(MAKE_DIRECTORY "${FIXTURE}/src/audio/coreaudio")
set(source "${FIXTURE}/src/audio/coreaudio/SDL_coreaudio.m")
file(COPY_FILE "${SDL_SOURCE}/src/audio/coreaudio/SDL_coreaudio.m" "${source}")
# Exercise initial application even when the macOS build already patched its SDL.
file(READ "${source}" content)
string(REPLACE [=[        // sotn-native modification: let disposal drain playback without a pending async stop.
        if (device->recording) {
            AudioQueueFlush(device->hidden->audioQueue);
            AudioQueueStop(device->hidden->audioQueue, 0);
        }]=] [=[        AudioQueueFlush(device->hidden->audioQueue);
        AudioQueueStop(device->hidden->audioQueue, 0);]=] content "${content}")
string(REGEX REPLACE "\n$" "" content "${content}")
file(CONFIGURE OUTPUT "${source}" CONTENT "@content@" @ONLY NEWLINE_STYLE UNIX)
file(SHA256 "${source}" original_digest)
if(NOT original_digest STREQUAL "c1ed4d18205056f615ae3f7098b53cd69ccbb939e1e8f212f029c0e8c1cc44af")
  message(FATAL_ERROR "SDL patch test did not restore the pinned original source")
endif()
set(SOTN_SDL_SOURCE_DIR "${FIXTURE}")
include("${PATCH}")
file(SHA256 "${source}" first_digest)
include("${PATCH}")
file(SHA256 "${source}" second_digest)
if(NOT first_digest STREQUAL second_digest)
  message(FATAL_ERROR "Repeated SDL patch changed the file")
endif()
file(APPEND "${source}" "\n/* Unexpected source drift. */\n")
execute_process(COMMAND "${CMAKE_COMMAND}" "-DSOTN_SDL_SOURCE_DIR=${FIXTURE}"
  -P "${PATCH}" RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(result STREQUAL "0" OR NOT error MATCHES "SDL CoreAudio source changed")
  message(FATAL_ERROR "SDL patch accepted unexpected source: ${output}${error}")
endif()
file(REMOVE_RECURSE "${FIXTURE}")
