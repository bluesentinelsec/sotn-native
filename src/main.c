#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <sotn/game_data.h>
#include <sotn/presenter.h>
#include <sotn/save_icon.h>
#include <stdbool.h>

/** @brief Application-owned SDL resources, released by SDL_AppQuit. */
typedef struct App
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SotnGameData *game_data;
    SotnPresenter *presenter;
    Uint8 preview_pixels[SOTN_SAVE_ICON_RGBA_SIZE];
    Uint32 preview_index;
    bool smoke_test;
    Uint32 frames;
    char *smoke_report;
} App;

static SDL_AppResult Fail(const char *operation)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s", operation, SDL_GetError());
    return SDL_APP_FAILURE;
}

static bool UpdatePreview(App *app)
{
    SotnDataView dra = {0};
    return SotnGameData_GetFile(app->game_data, SOTN_GAME_DRA, &dra) &&
           SotnSaveIcon_Decode(dra, app->preview_index, app->preview_pixels,
                               sizeof(app->preview_pixels)) &&
           SotnPresenter_SetImage(app->presenter, app->preview_pixels, sizeof(app->preview_pixels),
                                  SOTN_SAVE_ICON_WIDTH, SOTN_SAVE_ICON_HEIGHT);
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
    const char *game_directory = NULL;
    bool validate_only = false;
    bool graphics_test = false;
    bool preview_icons = false;
    for (int i = 1; i < argc; ++i)
    {
        if (SDL_strcmp(argv[i], "--smoke-test") == 0)
        {
            app->smoke_test = true;
        }
        else if (SDL_strcmp(argv[i], "--game-data") == 0)
        {
            if (game_directory != NULL || i + 1 >= argc || argv[i + 1][0] == '\0' ||
                SDL_strncmp(argv[i + 1], "--", 2) == 0)
            {
                SDL_SetError("--game-data requires one directory and may appear only once");
                return Fail("Invalid arguments");
            }
            game_directory = argv[++i];
        }
        else if (SDL_strcmp(argv[i], "--validate-only") == 0)
        {
            validate_only = true;
        }
        else if (SDL_strcmp(argv[i], "--graphics-test") == 0)
        {
            graphics_test = true;
        }
        else if (SDL_strcmp(argv[i], "--preview-save-icons") == 0)
        {
            preview_icons = true;
        }
        else if (SDL_strcmp(argv[i], "--help") == 0)
        {
            SDL_Log("Usage: sotn-native [--game-data DIR] [--validate-only | --preview-save-icons] "
                    "[--smoke-test] | --graphics-test");
            return SDL_APP_SUCCESS;
        }
        else
        {
            SDL_SetError("Unknown argument '%s'; use --help", argv[i]);
            return Fail("Invalid arguments");
        }
    }
    if (validate_only && game_directory == NULL)
    {
        SDL_SetError("--validate-only requires --game-data DIR");
        return Fail("Invalid arguments");
    }
    if ((preview_icons && (game_directory == NULL || validate_only)) ||
        (graphics_test && (game_directory != NULL || preview_icons || validate_only)))
    {
        SDL_SetError("--preview-save-icons requires --game-data and display mode; "
                     "--graphics-test runs separately without game inputs");
        return Fail("Invalid arguments");
    }
    if (game_directory != NULL)
    {
        if (!SotnGameData_Open(game_directory, &app->game_data))
        {
            return Fail("Cannot load game data");
        }
        SDL_Log("SOTN_GAME_DATA: PASS (%s, %d core files)", SotnGameData_ReferenceId(),
                (int)SOTN_GAME_FILE_COUNT);
        SDL_Log("Core data loaded; game logic is not integrated yet.");
        if (validate_only)
        {
            return SDL_APP_SUCCESS;
        }
    }
    SDL_SetAppMetadata("SOTN Native", SOTN_VERSION, "org.bluesentinelsec.sotn-native");
    SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60");
    if (app->smoke_test && SDL_getenv("SOTN_SMOKE_REPORT") != NULL)
    {
        char *directory = SDL_GetPrefPath("bluesentinelsec", "sotn-native");
        if (directory == NULL)
            return Fail("Cannot locate smoke-test report directory");
        const int length = SDL_asprintf(&app->smoke_report, "%ssmoke-result.txt", directory);
        SDL_free(directory);
        if (length < 0 || !SDL_SaveFile(app->smoke_report, "START\n", 6))
            return Fail("Cannot initialize smoke-test report");
    }
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        return Fail("Cannot initialize SDL video");
    }
    if (graphics_test)
    {
        if (!SotnPresenter_SelfTest())
            return Fail("OpenGL presentation test failed");
        SDL_Log("SOTN_GRAPHICS_TEST: PASS");
        return SDL_APP_SUCCESS;
    }
    if (preview_icons)
    {
        if (!SotnPresenter_Create("SOTN save icons - Left/Right selects, Esc quits", 960, 540,
                                  &app->presenter) ||
            !UpdatePreview(app))
        {
            return Fail("Cannot create OpenGL asset preview");
        }
        if (app->smoke_test)
            SDL_Log("SOTN_SMOKE_TEST: START");
        return SDL_APP_CONTINUE;
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
 * @param appstate Borrowed application state.
 * @param event Borrowed event supplied by SDL.
 * @return SDL_APP_SUCCESS when quitting, otherwise SDL_APP_CONTINUE.
 */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    App *app = appstate;
    if (event->type == SDL_EVENT_QUIT ||
        (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE))
    {
        return SDL_APP_SUCCESS;
    }
    if (app->presenter != NULL && event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat &&
        (event->key.key == SDLK_LEFT || event->key.key == SDLK_RIGHT))
    {
        const Uint32 step = event->key.key == SDLK_RIGHT ? 1U : SOTN_SAVE_ICON_COUNT - 1U;
        app->preview_index = (app->preview_index + step) % SOTN_SAVE_ICON_COUNT;
        if (!UpdatePreview(app))
            return Fail("Cannot update save-icon preview");
        SDL_Log("Save icon %u of %u", (unsigned int)app->preview_index + 1U,
                (unsigned int)SOTN_SAVE_ICON_COUNT);
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
    if (app->presenter != NULL)
    {
        if (!SotnPresenter_Draw(app->presenter))
            return Fail("Cannot present save icon");
    }
    else if (!SDL_SetRenderDrawColor(app->renderer, 17, 22, 38, 255) ||
             !SDL_RenderClear(app->renderer) ||
             !SDL_SetRenderDrawColor(app->renderer, 240, 226, 182, 255) ||
             !SDL_RenderDebugText(app->renderer, 276, 160, "Hello world") ||
             !SDL_RenderDebugText(app->renderer, 176, 184, "SOTN Native - development bootstrap") ||
             (app->game_data != NULL &&
              !SDL_RenderDebugText(app->renderer, 176, 208,
                                   "US core data loaded - game integration pending")) ||
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
    char *report = app != NULL ? app->smoke_report : NULL;
    const bool completed =
        app != NULL && app->smoke_test && app->frames == 3 && result == SDL_APP_SUCCESS;
    if (app != NULL)
    {
        SotnPresenter_Destroy(app->presenter);
        SDL_DestroyRenderer(app->renderer);
        SDL_DestroyWindow(app->window);
        SotnGameData_Close(app->game_data);
        SDL_free(app);
    }
    SDL_Quit();
    if (report != NULL && completed)
    {
        static const char contents[] = "SOTN_SMOKE_TEST: PASS\nSOTN_SMOKE_TEST: CLEANUP_COMPLETE\n";
        if (!SDL_SaveFile(report, contents, sizeof(contents) - 1))
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Cannot write smoke-test report: %s",
                         SDL_GetError());
    }
    SDL_free(report);
}
