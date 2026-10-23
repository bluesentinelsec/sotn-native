#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <stdbool.h>

/** @brief Application-owned SDL resources, released by SDL_AppQuit. */
typedef struct App
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool smoke_test;
    Uint32 frames;
} App;

static SDL_AppResult Fail(const char *operation)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", operation, SDL_GetError());
    return SDL_APP_FAILURE;
}

/**
 * @brief Initialize the application and its display.
 * @param appstate Receives owned state retained until SDL_AppQuit, including on failure.
 * @param argc Number of command-line arguments.
 * @param argv Borrowed command-line arguments, valid during this call.
 * @return SDL_APP_CONTINUE on success or SDL_APP_FAILURE with a logged error.
 */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    App *app = SDL_calloc(1, sizeof(*app));
    if (app == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot allocate app state");
        return SDL_APP_FAILURE;
    }
    *appstate = app;
    app->smoke_test = SDL_getenv("SOTN_SMOKE_TEST") != NULL;
    for (int i = 1; i < argc; ++i)
    {
        if (SDL_strcmp(argv[i], "--smoke-test") == 0)
        {
            app->smoke_test = true;
        }
    }
    SDL_SetAppMetadata("SOTN Native", SOTN_VERSION, "org.bluesentinelsec.sotn-native");
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return Fail("Cannot initialize SDL video");
    }
    const SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
    if (!SDL_CreateWindowAndRenderer("SOTN Native", 960, 540, flags, &app->window,
                                     &app->renderer) ||
        !SDL_SetRenderLogicalPresentation(app->renderer, 640, 360,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        return Fail("Cannot create or configure the display");
    }
    SDL_Log("Hello world");
    if (app->smoke_test)
    {
        SDL_Log("SOTN_SMOKE_TEST: START");
    }
    return SDL_APP_CONTINUE;
}

/**
 * @brief Handle close and keyboard events.
 * @param appstate Borrowed application state; unused by this callback.
 * @param event Borrowed event supplied by SDL.
 * @return SDL_APP_SUCCESS when quitting, otherwise SDL_APP_CONTINUE.
 */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    (void)appstate;
    if (event->type == SDL_EVENT_QUIT ||
        (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE))
    {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

/**
 * @brief Draw one frame and complete the smoke test after three successful frames.
 * @param appstate Borrowed state initialized by SDL_AppInit.
 * @return Continue, successful test completion, or failure with a logged error.
 */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    App *app = appstate;
    if (!SDL_SetRenderDrawColor(app->renderer, 17, 22, 38, 255) ||
        !SDL_RenderClear(app->renderer) ||
        !SDL_SetRenderDrawColor(app->renderer, 240, 226, 182, 255) ||
        !SDL_RenderDebugText(app->renderer, 276, 160, "Hello world") ||
        !SDL_RenderDebugText(app->renderer, 176, 184, "SOTN Native - development bootstrap") ||
        !SDL_RenderPresent(app->renderer))
    {
        return Fail("Cannot render the bootstrap frame");
    }
    if (app->smoke_test && ++app->frames == 3)
    {
        SDL_Log("SOTN_SMOKE_TEST: PASS");
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

/**
 * @brief Release application state and shut down SDL after success or partial initialization.
 * @param appstate Owned state from SDL_AppInit, or NULL if allocation failed.
 * @param result Final application result; resource cleanup is identical for every result.
 */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    App *app = appstate;
    (void)result;
    if (app != NULL)
    {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SDL_free(app);
    }
    SDL_Quit();
}
