#include <sotn/spirit_capture.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
int main(void)
{
    Uint8 dra[4] = {0}, overlay[0x80] = {0};
    Uint8 *stage = SDL_calloc(1, 0x40000), *common = SDL_calloc(1, 0x42000);
    Uint16 *frame = SDL_malloc(SOTN_FRAME_PIXELS * sizeof(*frame));
    CHECK(stage && common && frame);
    Put32(overlay + 24, 0x80180040);
    Put32(overlay + 0x40, 0x80180060);
    Put32(overlay + 0x60, 5);
    Put32(overlay + 0x64, SDL_MAX_UINT32);
    SotnActorGraphics *g = NULL;
    CHECK(SotnActorGraphics_Create((SotnDataView){dra, sizeof(dra)},
                                   (SotnDataView){overlay, sizeof(overlay)},
                                   (SotnTileSources){{stage, 0x40000}, {common, 0x42000}}, &g));
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        frame[i] = (Uint16)(i ^ 0xa55aU);
    SotnActorSurface target = {0};
    CHECK(SotnActorGraphics_BeginTarget(
        g,
        (SotnActorTarget){
            .x = 512, .y = 448, .width = 64, .height = 64, .clear = true, .color = 0x7777},
        &target));
    CHECK(SotnActorGraphics_EndTarget(g, true));
    CHECK(SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, 10, 20));
    CHECK(SotnActorGraphics_BeginTarget(
        g, (SotnActorTarget){.x = 512, .y = 448, .width = 64, .height = 64}, &target));
    for (size_t y = 0; y < 64; ++y)
        for (size_t x = 0; x < 64; ++x)
            CHECK(target.pixels[y * 64 + x] ==
                  (x < 63 && y < 63 ? frame[(y + 20) * SOTN_FRAME_WIDTH + x + 10] : 0x7777));
    const Uint16 held = target.pixels[0];
    CHECK(!SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, 0, 0) && target.pixels[0] == held);
    CHECK(SotnActorGraphics_EndTarget(g, false));
    CHECK(!SotnSpiritCapture_Frame(NULL, frame, SOTN_FRAME_PIXELS, 10, 20));
    CHECK(!SotnSpiritCapture_Frame(g, NULL, SOTN_FRAME_PIXELS, 10, 20));
    CHECK(!SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS - 1U, 10, 20));
    CHECK(!SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, -1, 20));
    CHECK(!SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, 10, -1));
    CHECK(!SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, SOTN_FRAME_WIDTH - 62, 20));
    CHECK(!SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, 10, SOTN_FRAME_HEIGHT - 62));
    CHECK(SotnActorGraphics_BeginTarget(
        g, (SotnActorTarget){.x = 512, .y = 448, .width = 64, .height = 64}, &target));
    CHECK(target.pixels[0] == held && target.pixels[63] == 0x7777 &&
          target.pixels[63 * 64] == 0x7777);
    CHECK(SotnActorGraphics_EndTarget(g, false));
    CHECK(SotnSpiritCapture_Frame(g, frame, SOTN_FRAME_PIXELS, SOTN_FRAME_WIDTH - 63,
                                  SOTN_FRAME_HEIGHT - 63));
    CHECK(SotnActorGraphics_BeginTarget(
        g, (SotnActorTarget){.x = 512, .y = 448, .width = 64, .height = 64}, &target));
    CHECK(target.pixels[62 * 64 + 62] == frame[SOTN_FRAME_PIXELS - 1U] &&
          target.pixels[63 * 64 + 63] == 0x7777);
    CHECK(SotnActorGraphics_EndTarget(g, false));
    SotnActorGraphics_Destroy(g);
    SDL_free(frame);
    SDL_free(stage);
    SDL_free(common);
    SDL_Log("SOTN_SPIRIT_CAPTURE_TEST: PASS");
    return 0;
}
