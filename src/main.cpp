#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstring>
#include <new>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace
{
struct App
{
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    bool smoke_test = false;
    int frames = 0;
};

SDL_AppResult Fail()
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s", SDL_GetError());
    return SDL_APP_FAILURE;
}
} // namespace

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    auto *app = new (std::nothrow) App;
    if (app == nullptr)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot allocate app state");
        return SDL_APP_FAILURE;
    }
    *appstate = app;
    app->smoke_test = SDL_getenv("SOTN_SMOKE_TEST") != nullptr;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--smoke-test") == 0)
        {
            app->smoke_test = true;
        }
    }
#ifdef __EMSCRIPTEN__
    app->smoke_test =
        EM_ASM_INT({ return new URLSearchParams(window.location.search).has('smoke-test'); }) != 0;
#endif
    SDL_SetAppMetadata("SOTN Native", SOTN_VERSION, "org.bluesentinelsec.sotn-native");
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return Fail();
    }
    const SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
    if (!SDL_CreateWindowAndRenderer("SOTN Native", 960, 540, flags, &app->window,
                                     &app->renderer) ||
        !SDL_SetRenderLogicalPresentation(app->renderer, 640, 360,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        return Fail();
    }
    SDL_Log("Hello world");
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT ||
        (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE))
    {
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    auto &app = *static_cast<App *>(appstate);
    if (!SDL_SetRenderDrawColor(app.renderer, 17, 22, 38, 255) || !SDL_RenderClear(app.renderer) ||
        !SDL_SetRenderDrawColor(app.renderer, 240, 226, 182, 255) ||
        !SDL_RenderDebugText(app.renderer, 276, 160, "Hello world") ||
        !SDL_RenderDebugText(app.renderer, 176, 184, "SOTN Native - development bootstrap") ||
        !SDL_RenderPresent(app.renderer))
    {
        return Fail();
    }
    if (app.smoke_test && ++app.frames == 3)
    {
        SDL_Log("SOTN_SMOKE_TEST: PASS");
#ifdef __EMSCRIPTEN__
        EM_ASM({ document.body.dataset.smokeResult = 'pass'; });
#endif
        return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult)
{
    auto *app = static_cast<App *>(appstate);
    if (app != nullptr)
    {
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        delete app;
    }
    SDL_Quit();
}
