#include <SDL3/SDL.h>
#include <sotn/save_point_renderer.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0x4000), *common = SDL_calloc(1, 0x42000);
    Uint16 *frame = SDL_calloc(SOTN_FRAME_PIXELS, sizeof(*frame));
    CHECK(dra != NULL && common != NULL && frame != NULL);
    SotnSavePointRenderer *renderer = NULL;
    CHECK(!SotnSavePointRenderer_Create((SotnDataView){dra, 0x4000}, (SotnDataView){common, 1},
                                        &renderer));
    Put32(dra + 0x3a74, 1);
    Put32(dra + 0x3a78, 0x1f00);
    Put32(dra + 0x3a7c, 16);
    Put32(dra + 0x3a80, 0x800a3c00);
    /* Original synthetic triangle; no extracted mesh or pixels. */
    dra[0x3b00] = 246;
    dra[0x3b01] = 255;
    dra[0x3b02] = 246;
    dra[0x3b03] = 255;
    dra[0x3b08] = 10;
    dra[0x3b0a] = 246;
    dra[0x3b0b] = 255;
    dra[0x3b12] = 10;
    for (Uint32 mesh = 0; mesh < 2; ++mesh)
        for (Uint32 i = 0; i < 24; ++i)
            for (Uint32 j = 0; j < 3; ++j)
                Put32(dra + (mesh == 0 ? 0x3210 : 0x33a0) + i * 12 + j * 4, 0x800a3b00 + j * 8);
    SDL_memset(common, 0x11, 0x40000);
    for (Uint32 i = 0; i < 256 * 16; ++i)
        common[0x40000 + i * 2] = 31;
    CHECK(SotnSavePointRenderer_Create((SotnDataView){dra, 0x4000}, (SotnDataView){common, 0x42000},
                                       &renderer));
    SotnSavePoint point = {0};
    CHECK(SotnSavePoint_Reset(&point, 15, 1, 0));
    CHECK(SotnSavePointRenderer_Draw(renderer, point, false, frame, SOTN_FRAME_PIXELS));
    CHECK(frame[136 * 256 + 128] == 31);
    Uint16 previous[SOTN_FRAME_PIXELS] = {0};
    SDL_memcpy(previous, frame, sizeof(previous));
    SDL_memset(frame, 0, sizeof(previous));
    CHECK(
        SotnSavePointRenderer_DrawOffset(renderer, point, false, 3, -2, frame, SOTN_FRAME_PIXELS));
    for (Sint32 y = 2; y < SOTN_FRAME_HEIGHT; ++y)
        for (Sint32 x = 0; x < SOTN_FRAME_WIDTH - 3; ++x)
            CHECK(frame[(y - 2) * 256 + x + 3] == previous[y * 256 + x]);
    SDL_memcpy(previous, frame, sizeof(previous));
    CHECK(!SotnSavePointRenderer_DrawOffset(renderer, point, false, 257, 0, frame,
                                            SOTN_FRAME_PIXELS));
    CHECK(!SDL_memcmp(previous, frame, sizeof(previous)));
    point.phase = SOTN_SAVE_POINT_CONFIRM;
    CHECK(SotnSavePointRenderer_Dialog(renderer, point, false, frame, SOTN_FRAME_PIXELS));
    CHECK(frame[76 * 256 + 24] == 0x7fff);
    CHECK(!SotnSavePointRenderer_Draw(renderer, point, true, frame, 1));
    point.scale = 999;
    CHECK(!SotnSavePointRenderer_Draw(renderer, point, true, frame, SOTN_FRAME_PIXELS));
    SotnSavePointRenderer_Destroy(renderer);
    Put32(dra + 0x3210, 0x800a3fff);
    CHECK(!SotnSavePointRenderer_Create((SotnDataView){dra, 0x4000},
                                        (SotnDataView){common, 0x42000}, &renderer));
    CHECK(renderer == NULL);
    SDL_free(dra);
    SDL_free(common);
    SDL_free(frame);
    SDL_Log("SOTN_SAVE_POINT_RENDER_TEST: PASS");
    return 0;
}
