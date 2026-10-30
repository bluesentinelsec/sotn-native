#include <sotn/hellfire_graphics.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
int main(void)
{
    Uint8 dra[4] = {0}, overlay[0x80] = {0};
    Uint8 *stage = SDL_calloc(1, 0x40000), *common = SDL_calloc(1, 0x42000),
          *grad = SDL_calloc(1, 0x10798);
    CHECK(stage && common && grad);
    Put32(overlay + 24, 0x80180040);
    Put32(overlay + 0x40, 0x80180060);
    Put32(overlay + 0x60, 5);
    Put32(overlay + 0x64, SDL_MAX_UINT32);
    SotnActorGraphics *g = NULL;
    CHECK(SotnActorGraphics_Create((SotnDataView){dra, sizeof(dra)},
                                   (SotnDataView){overlay, sizeof(overlay)},
                                   (SotnTileSources){{stage, 0x40000}, {common, 0x42000}}, &g));
    for (Uint16 row = 0; row < 2; ++row)
    {
        Uint8 *r = grad + 0x10788 + row * 8;
        Put16(r, (Uint16)(10 + row * 8));
        Put16(r + 2, 7);
        Put16(r + 4, 3);
        Put16(r + 6, 1);
        for (Uint16 x = 0; x < 3; ++x)
            Put16(grad + 0x106c8 + row * 96 + x * 2, (Uint16)(0x8111 + row * 100 + x));
    }
    CHECK(SotnHellfireGraphics_Upload(g, (SotnDataView){grad, 0x10798}));
    SotnActorSurface target = {0};
    CHECK(SotnActorGraphics_BeginTarget(
        g, (SotnActorTarget){.x = 10, .y = 7, .width = 3, .height = 1}, &target));
    CHECK(target.pixels[0] == 0x8111 && target.pixels[2] == 0x8113);
    CHECK(!SotnHellfireGraphics_Upload(g, (SotnDataView){grad, 0x10798}));
    CHECK(SotnActorGraphics_EndTarget(g, false));
    Put16(grad + 0x106c8, 0x8222);
    Put16(grad + 0x10790 + 4, 49);
    CHECK(!SotnHellfireGraphics_Upload(g, (SotnDataView){grad, 0x10798}));
    CHECK(SotnActorGraphics_BeginTarget(
              g, (SotnActorTarget){.x = 10, .y = 7, .width = 3, .height = 1}, &target) &&
          target.pixels[0] == 0x8111);
    CHECK(SotnActorGraphics_EndTarget(g, false));
    Put16(grad + 0x10790 + 4, 3);
    CHECK(!SotnHellfireGraphics_Upload(NULL, (SotnDataView){grad, 0x10798}));
    CHECK(!SotnHellfireGraphics_Upload(g, (SotnDataView){grad, 0x10797}));
    Uint16 pixels[8 * 8] = {0};
    SotnActorSurface surface = {pixels, 64, 8, 8};
    SotnPlayerEffectQuad flash = {
        .x = {1, 4, 1, 4}, .y = {1, 1, 4, 4}, .red = {128, 128, 128, 128}, .draw_mode = 4};
    CHECK(SotnHellfireGraphics_Flash(g, &flash, 0, 0, surface) && pixels[2 * 8 + 2] == 0x10 &&
          pixels[0] == 0);
    flash.draw_mode |= 8;
    pixels[2 * 8 + 2] = 0x4567;
    CHECK(SotnHellfireGraphics_Flash(g, &flash, 0, 0, surface) && pixels[2 * 8 + 2] == 0x4567);
    CHECK(!SotnHellfireGraphics_Flash(g, &flash, 0, 0, (SotnActorSurface){0}));
    CHECK(!SotnHellfireGraphics_Flash(g, &flash, 2049, 0, surface));
    SotnActorGraphics_Destroy(g);
    SDL_free(stage);
    SDL_free(common);
    SDL_free(grad);
    SDL_Log("SOTN_HELLFIRE_GRAPHICS_TEST: PASS");
    return 0;
}
