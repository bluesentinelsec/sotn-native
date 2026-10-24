# Local SDL 3.4.18 macOS playback shutdown fix; preserve recording behavior.
# The release archive remains integrity-checked. Fail on source drift and accept
# the exact patched file on repeated configurations.
set(source "${SOTN_SDL_SOURCE_DIR}/src/audio/coreaudio/SDL_coreaudio.m")
file(SHA256 "${source}" digest)
if(digest STREQUAL "215e4192c701fd70f663d9c3660b02010c59be53bc44de63a1cc669b2cf3b301")
  return()
endif()
if(NOT digest STREQUAL "c1ed4d18205056f615ae3f7098b53cd69ccbb939e1e8f212f029c0e8c1cc44af")
  message(FATAL_ERROR "SDL CoreAudio source changed; review the pinned playback shutdown patch")
endif()
file(READ "${source}" content)
string(REPLACE [=[        AudioQueueFlush(device->hidden->audioQueue);
        AudioQueueStop(device->hidden->audioQueue, 0);]=] [=[        // sotn-native modification: let disposal drain playback without a pending async stop.
        if (device->recording) {
            AudioQueueFlush(device->hidden->audioQueue);
            AudioQueueStop(device->hidden->audioQueue, 0);
        }]=] content "${content}")
# Keep the byte fingerprint identical on Windows hosts as well as macOS/Linux.
file(CONFIGURE OUTPUT "${source}" CONTENT "@content@" @ONLY NEWLINE_STYLE UNIX)
file(SHA256 "${source}" digest)
if(NOT digest STREQUAL "215e4192c701fd70f663d9c3660b02010c59be53bc44de63a1cc669b2cf3b301")
  message(FATAL_ERROR "SDL CoreAudio playback shutdown patch did not produce the expected source")
endif()
