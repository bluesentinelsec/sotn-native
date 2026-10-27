#include <SDL3/SDL.h>
#include <sotn/actor_graphics.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8U));
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0xe900), *stage = SDL_calloc(1, 0x40000),
          *common = SDL_calloc(1, 0x42000);
    CHECK(dra && stage && common);
    Uint8 overlay[0x80] = {0};
    Put32(overlay + 24, 0x80180040);
    Put32(overlay + 0x40, 0x80180060);
    Put32(overlay + 0x60, 5);
    Put32(overlay + 0x64, SDL_MAX_UINT32);
    Put16(dra + 0x21b8, 513);
    Put16(dra + 0x21ba, 257);
    Put16(dra + 0xd670 + 29U * 20U + 14U, 0x700a);
    for (Uint16 i = 0; i < 4; ++i)
    {
        const Uint16 id = i == 3 ? 31 : (Uint16)(27 + i);
        const Uint32 offset = 0x3000U + i * 24U;
        Put32(dra + 0x3bb8 + id * 4U, 0x800a0000U + offset);
        Put32(dra + offset, i ? 0x20002 : 1);
        Put32(dra + offset + 4, 0x10f0);
        Put32(dra + offset + 8, 16);
        // Authored palette sources: initial red, target green, restored blue, alternate yellow.
        const Uint32 source = i == 0 ? 0x10a0 : i == 1 ? 0x10c0 : i == 2 ? 0x1000 : 0x10b0;
        Put32(dra + offset + 12, 0x8006cbccU + source * 2U);
        Put32(dra + offset + 16, SDL_MAX_UINT32);
    }
    for (Uint16 i = 0; i < 16; ++i)
    {
        Put16(common + 0x40000 + 0xa0U * 2U + i * 2U, 0x801f);
        Put16(common + 0x40000 + 0xc0U * 2U + i * 2U, 0x3e0);
        Put16(common + 0x40000 + i * 2U, 0x7c00);
        Put16(common + 0x40000 + 0xb0U * 2U + i * 2U, 0x3ff);
    }
    Put32(dra + 0x3bb8 + 22U * 4U, 0x800a2800U);
    Put32(dra + 0x2800, 1);
    Put32(dra + 0x2804, 0x1000);
    Put32(dra + 0x2808, 0xe0);
    Put32(dra + 0x280c, 0x800a5000U);
    SDL_memcpy(dra + 0x5000, common + 0x40000, 0xe0U * 2U);
    // Raw artwork carries a different authored source; character initialization must replace it.
    for (Uint16 i = 0; i < 16; ++i)
        Put16(common + 0x40000 + 0xa0U * 2U + i * 2U, 0x7c1f);
    SotnActorGraphics *graphics = NULL;
    CHECK(SotnActorGraphics_Create(
        (SotnDataView){dra, 0xe900}, (SotnDataView){overlay, sizeof(overlay)},
        (SotnTileSources){{stage, 0x40000}, {common, 0x42000}}, &graphics));
    CHECK(SotnActorGraphics_InitPlayerPalettes(graphics));
    CHECK(!SotnActorGraphics_InitPlayerPalettes(NULL));
    Uint8 pixels[16U * 17U / 2U] = {0};
    SDL_memset(pixels, 0x11, sizeof(pixels));
    const SotnAlucardSprite sprite = {{pixels, sizeof(pixels)}, 16, 17, -8, -12};
    SotnMistWedge wedges[8] = {0};
    CHECK(SotnActorGraphics_CapturePlayer(graphics, &sprite, 100, 80, false, wedges));
    CHECK(wedges[0].x[0] == 92 && wedges[0].x[1] == 100 && wedges[0].y[0] == 68 &&
          wedges[0].x[2] == 100 && wedges[0].y[2] == 76);
    CHECK(wedges[6].y[0] == 85 && wedges[6].y[1] == 76 && wedges[7].y[1] == 68);
    CHECK(wedges[2].u[0] == 20 && wedges[2].v[1] == 9 && wedges[3].v[1] == 18);
    SotnActorSurface upload = {0};
    CHECK(SotnActorGraphics_BeginTarget(
        graphics, (SotnActorTarget){.x = 512, .y = 256, .width = 7, .height = 23}, &upload));
    CHECK(upload.pixels[1U * 7U + 1U] == 0x1111 && upload.pixels[17U * 7U + 4U] == 0x1111 &&
          upload.pixels[1U * 7U + 5U] == 0 && upload.pixels[18U * 7U + 1U] == 0 &&
          upload.pixels[21U * 7U + 5U] == 0);
    CHECK(SotnActorGraphics_EndTarget(graphics, false));
    CHECK(SotnActorGraphics_CapturePlayer(graphics, &sprite, -20, -30, true, wedges));
    CHECK(wedges[0].x[0] == -12 && wedges[0].x[1] == -20 && wedges[0].x[2] == -20);
    SotnMistWedge previous[8] = {0};
    SDL_memcpy(previous, wedges, sizeof(previous));
    SotnAlucardSprite bad = sprite;
    --bad.pixels.size;
    CHECK(!SotnActorGraphics_CapturePlayer(graphics, &bad, 100, 80, false, wedges));
    CHECK(!SDL_memcmp(previous, wedges, sizeof(previous)));
    bool accepted = false;
    CHECK(SotnActorGraphics_RequestMistPalette(graphics, 27, &accepted) && accepted);
    CHECK(SotnActorGraphics_RequestMistPalette(graphics, 0x11c, &accepted) && accepted);
    CHECK(SotnActorGraphics_CancelMistPalette(graphics));
    CHECK(SotnActorGraphics_TickMistPalette(graphics));
    Uint16 colors[16] = {0};
    CHECK(SotnActorGraphics_GetMistPalette(graphics, colors) && colors[0] == 0x801f);
    CHECK(SotnActorGraphics_RequestMistPalette(graphics, 0x11c, &accepted) && accepted);
    CHECK(SotnActorGraphics_TickMistPalette(graphics));
    CHECK(SotnActorGraphics_GetMistPalette(graphics, colors) && colors[0] == 0x803e);
    CHECK(SotnActorGraphics_TickMistPalette(graphics));
    CHECK(SotnActorGraphics_GetMistPalette(graphics, colors) && colors[0] == 0x803e);
    for (Uint16 i = 0; i < 62; ++i)
        CHECK(SotnActorGraphics_TickMistPalette(graphics));
    CHECK(SotnActorGraphics_GetMistPalette(graphics, colors) && colors[0] == 0x83e0);
    for (Uint16 i = 0; i < 16; ++i)
        colors[i] = 0x7c00;
    CHECK(SotnActorGraphics_SetPlayerPalette(graphics, colors));
    CHECK(SotnActorGraphics_RequestMistPalette(graphics, 0x11d, &accepted) && accepted);
    CHECK(SotnActorGraphics_TickMistPalette(graphics));
    CHECK(SotnActorGraphics_GetMistPalette(graphics, colors) && colors[0] == 0x87c0);
    CHECK(SotnActorGraphics_CancelMistPalette(graphics));
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(SotnActorGraphics_RequestMistPalette(graphics, 0x11f, &accepted) && accepted);
    CHECK(SotnActorGraphics_RequestMistPalette(graphics, 27, &accepted) && !accepted);
    CHECK(SotnActorGraphics_CancelMistPalette(graphics));
    CHECK(SotnActorGraphics_RequestMistPalette(graphics, 27, &accepted) && accepted);
    Put32(dra + 0x3000 + 12, 0x8006cbccU + 0x10f0U * 2U);
    Uint16 before[16] = {0};
    CHECK(SotnActorGraphics_GetMistPalette(graphics, before));
    CHECK(!SotnActorGraphics_TickMistPalette(graphics));
    CHECK(SotnActorGraphics_GetMistPalette(graphics, colors) &&
          !SDL_memcmp(before, colors, sizeof(colors)));
    CHECK(!SotnActorGraphics_RequestMistPalette(graphics, 0x120, &accepted));
    CHECK(!SotnActorGraphics_SetPlayerPalette(graphics, NULL));
    CHECK(!SotnActorGraphics_CapturePlayer(graphics, &sprite, 32768, 0, false, wedges));
    Put16(dra + 0xd670 + 29U * 20U + 14U, 0);
    CHECK(!SotnActorGraphics_CapturePlayer(graphics, &sprite, 0, 0, false, wedges));
    // Bat/player display uploads have no dependency on the Mist capture profile.
    CHECK(SotnActorGraphics_UploadPlayer(graphics, &sprite));
    // Raw Bat body packets carry zero color bytes; only DRAW_COLORS may shade them.
    Uint16 raster[32U * 32U] = {0};
    const SotnActorSurface surface = {raster, SDL_arraysize(raster), 32, 32};
    SotnPlayerEffectQuad packet = {.page = 0x18, .palette = 0x100};
    for (Uint16 v = 0; v < 4; ++v)
    {
        packet.x[v] = (Sint16)((v & 1U) ? 16 : 0);
        packet.y[v] = (Sint16)((v & 2U) ? 17 : 0);
        packet.u[v] = (Uint8)((v & 1U) ? 20 : 4);
        packet.v[v] = (Uint8)((v & 2U) ? 18 : 1);
    }
    CHECK(SotnActorGraphics_PlayerEffect(graphics, &packet, 2, 3, surface));
    CHECK((raster[4U * 32U + 3U] & 0x7fffU) == 0x7c00);
    SDL_memset(raster, 0, sizeof(raster));
    packet.draw_mode = 4;
    CHECK(SotnActorGraphics_PlayerEffect(graphics, &packet, 2, 3, surface));
    CHECK((raster[4U * 32U + 3U] & 0x7fffU) == 0);
    raster[4U * 32U + 3U] = 0x3e0;
    packet.draw_mode = 8;
    CHECK(SotnActorGraphics_PlayerEffect(graphics, &packet, 2, 3, surface));
    CHECK(raster[4U * 32U + 3U] == 0x3e0);
    CHECK(!SotnActorGraphics_PlayerEffect(graphics, &packet, SDL_MAX_SINT32, 0, surface));
    CHECK(!SotnActorGraphics_PlayerEffect(graphics, NULL, 0, 0, surface));
    CHECK(!SotnActorGraphics_UploadPlayer(graphics, &bad));
    CHECK(!SotnActorGraphics_UploadPlayer(NULL, &sprite));
    CHECK(!SotnActorGraphics_GetMistPalette(NULL, colors));
    SotnActorGraphics_Destroy(graphics);
    SDL_free(dra);
    SDL_free(stage);
    SDL_free(common);
    return 0;
}
