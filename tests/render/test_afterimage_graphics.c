#include <SDL3/SDL.h>
#include <sotn/actor_graphics.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
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
    Uint8 *dra = SDL_calloc(1, 0x4000), *stage = SDL_calloc(1, 0x40000),
          *common = SDL_calloc(1, 0x42000);
    CHECK(dra && stage && common);
    Uint8 overlay[0x80] = {0};
    Put32(overlay + 24, 0x80180040);
    Put32(overlay + 0x40, 0x80180060);
    Put32(overlay + 0x60, 5);
    Put32(overlay + 0x64, SDL_MAX_UINT32);
    for (Uint16 i = 1; i <= 3; ++i)
    {
        Put16(dra + 0x21b8 + i * 4U, (Uint16)(609 + i * 32U));
        Put16(dra + 0x21ba + i * 4U, 257);
    }
    SotnActorGraphics *g = NULL;
    CHECK(SotnActorGraphics_Create((SotnDataView){dra, 0x4000},
                                   (SotnDataView){overlay, sizeof(overlay)},
                                   (SotnTileSources){{stage, 0x40000}, {common, 0x42000}}, &g));
    Uint16 colors[16] = {0};
    colors[1] = 0x801f;
    colors[2] = 0x3e0;
    colors[3] = 0xfc00;
    CHECK(SotnActorGraphics_SetPlayerPalette(g, colors));
    const Uint8 pixels[] = {0x21, 0x33, 0x12, 0};
    const Uint8 alternate[] = {0x22, 0x22, 0x22, 0x22};
    SotnAlucardSprite image = {
        .width = 4, .height = 2, .pivot_x = -2, .pivot_y = -1, .pixels = {pixels, sizeof(pixels)}};
    SotnAfterimageDisplay d = {.sprite = 1,
                               .palette = 0x8100,
                               .priority = 146,
                               .bank = 1,
                               .texture_slot = 1,
                               .red = 128,
                               .green = 128,
                               .blue = 128,
                               .color_enabled = true};
    SotnPlayerEffectQuad q = {0};
    Uint16 raster[32 * 32] = {0};
    SotnActorSurface surface = {raster, SDL_arraysize(raster), 32, 32};
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q));
    CHECK(q.x[0] == 8 && q.x[1] == 12 && q.y[0] == 9 && q.y[2] == 11 && q.u[0] == 4 &&
          q.u[1] == 8 && q.v[0] == 1 && q.page == 26 && q.palette == 0x100 && q.priority == 146 &&
          q.draw_mode == 4);
    CHECK(SotnActorGraphics_PlayerEffect(g, &q, 0, 0, surface) && raster[9 * 32 + 8] == 0x801f &&
          raster[9 * 32 + 9] == 0x3e0);
    image.pixels = (SotnDataView){alternate, sizeof(alternate)};
    d.sprite = 0x8001;
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) &&
          SotnActorGraphics_PlayerEffect(g, &q, 0, 0, surface) && raster[9 * 32 + 8] == 0x801f);
    d.texture_slot = 2;
    d.sprite = 1;
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) && q.u[0] == 132 &&
          SotnActorGraphics_PlayerEffect(g, &q, 0, 0, surface) && raster[9 * 32 + 8] == 0x3e0);
    d.texture_slot = 1;
    d.sprite = 0x8001;
    d.red = 64;
    d.green = 128;
    d.blue = 128;
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) &&
          SotnActorGraphics_PlayerEffect(g, &q, 0, 0, surface) && raster[9 * 32 + 8] == 0x800f);
    colors[1] = 0x83e0;
    CHECK(SotnActorGraphics_SetPlayerPalette(g, colors) &&
          SotnActorGraphics_PlayerEffect(g, &q, 0, 0, surface) && raster[9 * 32 + 8] == 0x83e0);
    d.facing_left = true;
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) && q.x[0] == 9 && q.x[1] == 13 &&
          q.u[0] == 7 && q.u[1] == 3);
    Put16(dra + 0x21ba + 4, 0x301);
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) && q.x[0] == 8 && q.x[1] == 12 &&
          q.u[0] == 4);
    d.facing_left = false;
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) && q.x[0] == 9 && q.u[0] == 7);
    d.texture_slot = 3;
    d.sprite = 1;
    d.blend = 16;
    CHECK(SotnActorGraphics_Afterimage(g, &image, d, 10, 10, &q) && q.page == 27 &&
          q.draw_mode == 5);
    CHECK(SotnActorGraphics_Afterimage(g, NULL, d, 513, 0, &q) && q.draw_mode == 8);
    d.sprite = 0;
    CHECK(SotnActorGraphics_Afterimage(g, NULL, d, 0, 0, &q) && q.draw_mode == 8);
    d.sprite = 1;
    const SotnPlayerEffectQuad before = q;
    d.texture_slot = 4;
    CHECK(!SotnActorGraphics_Afterimage(g, &image, d, 0, 0, &q) &&
          !SDL_memcmp(&q, &before, sizeof(q)));
    d.texture_slot = 1;
    Put16(dra + 0x21b8 + 4, 1023);
    CHECK(!SotnActorGraphics_Afterimage(g, &image, d, 0, 0, &q) &&
          !SDL_memcmp(&q, &before, sizeof(q)));
    CHECK(!SotnActorGraphics_Afterimage(NULL, &image, d, 0, 0, &q));
    CHECK(pixels[0] == 0x21 && pixels[3] == 0);
    SotnActorGraphics_Destroy(g);
    SDL_free(common);
    SDL_free(stage);
    SDL_free(dra);
    return 0;
}
