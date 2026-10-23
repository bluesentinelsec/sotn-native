#include "layout.h"
#include <SDL3/SDL.h>
#include <sotn/presenter.h>
#include <sotn/save_icon.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_LogError(SDL_LOG_CATEGORY_TEST, "Line %d: %s: %s", __LINE__, #condition,           \
                         SDL_GetError());                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static void SetColor(Uint8 *bytes, size_t offset, Uint16 color)
{
    bytes[offset] = (Uint8)color;
    bytes[offset + 1] = (Uint8)(color >> 8);
}

static bool TestDecoding(void)
{
    Uint8 dra[0x1f18] = {0};
    Uint8 rgba[SOTN_SAVE_ICON_RGBA_SIZE] = {0};
    SetColor(dra, 0x51a, 0x001f);
    SetColor(dra, 0x51c, 0x03e0);
    SetColor(dra, 0x51e, 0x7c00);
    SetColor(dra, 0x520, 0x8000);
    SetColor(dra, 0x522, 0x0003);
    dra[0x718] = 0x21;
    dra[0x719] = 0x03;
    dra[0x71a] = 0x54;
    const SotnDataView view = {dra, sizeof(dra)};
    CHECK(SotnSaveIcon_Decode(view, 0, rgba, sizeof(rgba)));
    static const Uint8 expected[] = {255, 0, 0, 255, 0, 255, 0, 255, 0,  0, 255, 255,
                                     0,   0, 0, 0,   0, 0,   0, 255, 25, 0, 0,   255};
    CHECK(SDL_memcmp(rgba, expected, sizeof(expected)) == 0);
    CHECK(rgba[sizeof(rgba) - 1] == 0);
    SetColor(dra, 0x518 + 15 * 32 + 2, 0x7fff);
    dra[0x718 + 15 * 384 + 383] = 0x10;
    CHECK(SotnSaveIcon_Decode(view, 15, rgba, sizeof(rgba)));
    CHECK(rgba[sizeof(rgba) - 4] == 255 && rgba[sizeof(rgba) - 3] == 255 &&
          rgba[sizeof(rgba) - 2] == 255 && rgba[sizeof(rgba) - 1] == 255);
    SDL_memset(rgba, 0x55, sizeof(rgba));
    CHECK(!SotnSaveIcon_Decode((SotnDataView){dra, sizeof(dra) - 1}, 15, rgba, sizeof(rgba)));
    CHECK(!SotnSaveIcon_Decode(view, 16, rgba, sizeof(rgba)));
    CHECK(!SotnSaveIcon_Decode(view, SDL_MAX_UINT32, rgba, sizeof(rgba)));
    CHECK(!SotnSaveIcon_Decode(view, 0, rgba, sizeof(rgba) - 1));
    CHECK(!SotnSaveIcon_Decode((SotnDataView){0}, 0, rgba, sizeof(rgba)));
    CHECK(!SotnSaveIcon_Decode(view, 0, NULL, sizeof(rgba)));
    for (size_t i = 0; i < sizeof(rgba); ++i)
        CHECK(rgba[i] == 0x55);
    return true;
}

static bool TestLayout(void)
{
    static const struct
    {
        int window_w, window_h, image_w, image_h;
        SDL_Rect expected;
    } cases[] = {
        {96, 64, 2, 2, {16, 0, 64, 64}},
        {1000, 600, 320, 240, {180, 60, 640, 480}},
        {100, 80, 320, 240, {0, 2, 100, 75}},
        {1, 1, 16, 48, {0, 0, 1, 1}},
        {1, 1, 48, 16, {0, 0, 1, 1}},
        {SDL_MAX_SINT32, 2, SDL_MAX_SINT32, SDL_MAX_SINT32, {1073741822, 0, 2, 2}},
        {SDL_MAX_SINT32, 1, 1, 65536, {1073741823, 0, 1, 1}},
        {1, SDL_MAX_SINT32, 65536, 1, {0, 1073741823, 1, 1}},
    };
    SDL_Rect rect = {0};
    for (size_t i = 0; i < SDL_arraysize(cases); ++i)
    {
        CHECK(SotnRender_FitImage(cases[i].window_w, cases[i].window_h, cases[i].image_w,
                                  cases[i].image_h, &rect));
        CHECK(rect.x == cases[i].expected.x && rect.y == cases[i].expected.y &&
              rect.w == cases[i].expected.w && rect.h == cases[i].expected.h);
    }
    CHECK(!SotnRender_FitImage(0, 1, 1, 1, &rect));
    CHECK(!SotnRender_FitImage(1, -1, 1, 1, &rect));
    CHECK(!SotnRender_FitImage(1, 1, 0, 1, &rect));
    CHECK(!SotnRender_FitImage(1, 1, 1, 1, NULL));
    return true;
}

static bool TestInvalidPresenterArguments(void)
{
    SotnPresenter *presenter = NULL;
    Uint8 pixel[4] = {0};
    CHECK(!SotnPresenter_Create(NULL, 1, 1, &presenter));
    CHECK(presenter == NULL);
    CHECK(!SotnPresenter_Create("invalid", 0, 1, &presenter));
    CHECK(!SotnPresenter_Create("invalid", 1, 1, NULL));
    CHECK(!SotnPresenter_SetImage(NULL, pixel, sizeof(pixel), 1, 1));
    CHECK(!SotnPresenter_Draw(NULL));
    SotnPresenter_Destroy(NULL);
    return true;
}

int main(void)
{
    if (!TestDecoding() || !TestLayout() || !TestInvalidPresenterArguments())
        return 1;
    SDL_Log("SOTN_IMAGE_TEST: PASS");
    return 0;
}
