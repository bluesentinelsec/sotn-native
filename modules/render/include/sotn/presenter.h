#ifndef SOTN_PRESENTER_H
#define SOTN_PRESENTER_H

#include <SDL3/SDL_stdinc.h>

/** @brief Window, GL context, and GPU resources owned by one main-thread presenter. */
typedef struct SotnPresenter SotnPresenter;

/**
 * @brief Create an OpenGL 3.2 core or OpenGL ES 3.0 window with embedded shaders.
 * @param title Borrowed UTF-8 window title; copied by SDL.
 * @param width Initial window width in logical pixels, greater than zero.
 * @param height Initial window height in logical pixels, greater than zero.
 * @param out Receives ownership, or NULL on failure when non-NULL.
 * @return True on success, otherwise false with context in SDL_GetError().
 * @pre SDL video is initialized. Call all presenter functions on the main thread.
 * @post Release the result with SotnPresenter_Destroy before SDL_Quit.
 */
bool SotnPresenter_Create(const char *title, int width, int height, SotnPresenter **out);

/**
 * @brief Copy top-to-bottom RGBA8 pixels to a nearest-filtered GPU texture.
 * @param presenter Borrowed presenter.
 * @param pixels Borrowed RGBA bytes, consumed before return; never retained.
 * @param size Available bytes; must contain at least width * height * 4 bytes.
 * @param width Positive image width, within the driver's limit.
 * @param height Positive image height, within the driver's limit.
 * @return True on success or false with an SDL error. Images are bounded to 64 MiB.
 * Invalid input or a failed upload leaves the previous image selected.
 */
bool SotnPresenter_SetImage(SotnPresenter *presenter, const Uint8 *pixels, size_t size, int width,
                            int height);

/** @brief Set logical display size independently from the uploaded pixel resolution.
 * @param presenter Borrowed presenter. @param width Positive logical display width, <=16384.
 * @param height Positive logical display height, <=16384. Set both to0 to use image dimensions.
 * @return Success or SDL error without mutation. The title uses320x240 logical display for its
 * 512x240 source. This uploads no image, resizes no window and advances no clock. */
bool SotnPresenter_SetDisplaySize(SotnPresenter *presenter, int width, int height);

/**
 * @brief Draw the uploaded image with preserved aspect ratio and swap the window.
 * @param presenter Borrowed presenter with an uploaded image.
 * @return True on success, otherwise false with an SDL error.
 * @note Prefer integer scaling when the window fits the image. A minimized window draws nothing.
 */
bool SotnPresenter_Draw(SotnPresenter *presenter);

/**
 * @brief Release the window, context, and GPU resources, including partial initialization.
 * @param presenter Owned presenter, or NULL for no operation. All references expire on return.
 */
void SotnPresenter_Destroy(SotnPresenter *presenter);
/** @brief Actual SDL window identity, or zero for NULL; no window or clock mutation. */
Uint32 SotnPresenter_WindowID(const SotnPresenter *presenter);

/**
 * @brief Check real shader compilation, texture orientation, colors, and context recreation.
 * @return True if synthetic framebuffer checks pass; false with an SDL error otherwise.
 * @pre SDL video is initialized; call on the main thread.
 * @note Creates and closes two diagnostic windows. Uses no private game inputs.
 */
bool SotnPresenter_SelfTest(void);

#endif
