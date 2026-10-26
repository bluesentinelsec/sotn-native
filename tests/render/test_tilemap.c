#include <SDL3/SDL.h>
#include <sotn/alucard_sprite.h>
#include <sotn/tilemap.h>

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

typedef struct Buffers
{
    Uint8 stage[0x40000], common[0x42000];
    Uint16 frame[SOTN_FRAME_PIXELS];
    Uint8 rgba[SOTN_FRAME_PIXELS * 4];
} Buffers;

static void Put16(Uint8 *bytes, size_t offset, Uint16 value)
{
    bytes[offset] = (Uint8)value;
    bytes[offset + 1] = (Uint8)(value >> 8);
}

static void Clear(Uint16 *frame, Uint16 color)
{
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        frame[i] = color;
}

static bool TestTiles(Buffers *buffers)
{
    Uint8 tiles[512] = {1};
    Uint8 pages[2] = {0}, graphics[2] = {0}, palettes[2] = {0};
    SotnStageLayer layer = {0};
    layer.present = true;
    layer.flags = 1;
    layer.collision = (SotnCollisionMap){{tiles, sizeof(tiles)}, {0}, 16, 16};
    layer.pages = (SotnDataView){pages, sizeof(pages)};
    layer.graphics = (SotnDataView){graphics, sizeof(graphics)};
    layer.palettes = (SotnDataView){palettes, sizeof(palettes)};
    const SotnTileSources sources = {{buffers->stage, sizeof(buffers->stage)},
                                     {buffers->common, sizeof(buffers->common)}};
    Put16(buffers->stage, 0, 0x3210);
    Put16(buffers->stage, 2, 4);
    /* Palette zero lives at VRAM (512,240), in the lower half of block two. */
    Put16(buffers->stage, 0x5c02, 0x001f);
    Put16(buffers->stage, 0x5c04, 0x03e0);
    Put16(buffers->stage, 0x5c06, 0x7c00);
    Put16(buffers->stage, 0x5c08, 0x8000);
    Clear(buffers->frame, 0x7fff);
    CHECK(SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x7fff && buffers->frame[1] == 0x001f);
    CHECK(buffers->frame[2] == 0x03e0 && buffers->frame[3] == 0x7c00);
    CHECK(buffers->frame[4] == 0x8000 && buffers->frame[16] == 0x7fff);
    CHECK(
        SotnFrame_ToRgba(buffers->frame, SOTN_FRAME_PIXELS, buffers->rgba, sizeof(buffers->rgba)));
    CHECK(buffers->rgba[4] == 255 && buffers->rgba[5] == 0 && buffers->rgba[7] == 255);
    CHECK(buffers->rgba[16] == 0 && buffers->rgba[17] == 0 && buffers->rgba[19] == 255);
    Clear(buffers->frame, 0x7fff);
    CHECK(SotnTilemap_Draw(&layer, sources, 1, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x001f && buffers->frame[15] == 0x7fff);
    Clear(buffers->frame, 0x7fff);
    CHECK(SotnTilemap_Draw(&layer, sources, -1, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x7fff && buffers->frame[2] == 0x001f);
    pages[1] = 1;
    graphics[1] = 0x80;
    Put16(buffers->stage, 0xc000, 0x1111);
    CHECK(SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x001f);
    pages[1] = 16;
    graphics[1] = 0;
    Put16(buffers->common, 0, 0x1111);
    layer.flags = 0x201;
    Put16(buffers->common, 0x40002, 0x03e0);
    CHECK(SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x03e0);
    layer.flags = 0x101;
    pages[1] = 0;
    Put16(buffers->stage, 0, 0x0201);
    CHECK(SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x001f && buffers->frame[1] == 0x03e0);
    layer.flags = 0x81;
    Put16(buffers->stage, 0, 0x1111);
    Put16(buffers->stage, 0x5c02, 0x801f);
    const Uint16 blends[] = {0x01ef, 0x03ff, 0x03e0, 0x03e7};
    for (Uint16 mode = 0; mode < 4; ++mode)
    {
        layer.flags = (Uint16)(0x81 | mode << 5);
        Clear(buffers->frame, 0x03e0);
        CHECK(SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
        CHECK(buffers->frame[0] == blends[mode]);
    }
    layer.flags = 1;
    pages[1] = 8;
    Clear(buffers->frame, 0x1234);
    CHECK(!SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x1234 && buffers->frame[SOTN_FRAME_PIXELS - 1] == 0x1234);
    pages[1] = 0;
    CHECK(SotnTilemap_Draw(&layer, sources, SDL_MAX_SINT32, SDL_MIN_SINT32, buffers->frame,
                           SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x1234);
    layer.flags = 0x1001;
    CHECK(SotnTilemap_Draw(&layer, sources, 256, 256, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[0] == 0x801f);
    layer.flags = 1;
    CHECK(!SotnTilemap_Draw(NULL, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnTilemap_Draw(&layer, (SotnTileSources){0}, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnTilemap_Draw(&layer, sources, 0, 0, buffers->frame, SOTN_FRAME_PIXELS - 1));
    layer.flags = 0;
    CHECK(SotnTilemap_Draw(&layer, (SotnTileSources){0}, 0, 0, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnFrame_ToRgba(NULL, SOTN_FRAME_PIXELS, buffers->rgba, sizeof(buffers->rgba)));
    CHECK(!SotnFrame_ToRgba(buffers->frame, SOTN_FRAME_PIXELS, buffers->rgba,
                            sizeof(buffers->rgba) - 1));
    return true;
}

static void Put32(Uint8 *bytes, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        bytes[offset + i] = (Uint8)(value >> (i * 8));
}

static bool TestSprite(Buffers *buffers)
{
    Uint8 *dra = SDL_calloc(1, 0x30000);
    CHECK(dra != NULL);
    Uint8 arc[128] = {0};
    Put32(dra, 0x2f328, 0x800cf800);
    Put16(dra, 0x2f800, 0x8001);
    Put16(dra, 0x2f802, 0xfffe);
    Put16(dra, 0x2f804, 0xffff);
    Put32(arc, 36, 0x8013c040);
    arc[64] = 4;
    arc[65] = 2;
    arc[66] = 1;
    arc[67] = 2;
    arc[68] = 0x21;
    arc[69] = 0x03;
    arc[70] = 0x40;
    arc[71] = 0x12;
    const SotnDataView image = {dra, 0x30000};
    const SotnDataView sheets = {arc, sizeof(arc)};
    const SotnDataView common = {buffers->common, sizeof(buffers->common)};
    SotnAlucardSprite sprite = {0};
    CHECK(SotnAlucard_DecodeSprite(image, sheets, 1, &sprite));
    CHECK(sprite.width == 4 && sprite.height == 2 && sprite.pixels.size == 4);
    CHECK(sprite.pivot_x == -1 && sprite.pivot_y == 1);
    SDL_memset(buffers->common + 0x40000, 0, 32);
    Put16(buffers->common, 0x40002, 0x001f);
    Put16(buffers->common, 0x40004, 0x03e0);
    Put16(buffers->common, 0x40006, 0x7c00);
    Put16(buffers->common, 0x40008, 0x8000);
    Clear(buffers->frame, 0x7fff);
    CHECK(
        SotnAlucard_DrawSprite(&sprite, common, 10, 10, false, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[11 * 256 + 9] == 0x001f);
    CHECK(buffers->frame[11 * 256 + 10] == 0x03e0);
    CHECK(buffers->frame[11 * 256 + 11] == 0x7c00);
    CHECK(buffers->frame[11 * 256 + 12] == 0x7fff);
    CHECK(buffers->frame[12 * 256 + 10] == 0x8000);
    Clear(buffers->frame, 0x7fff);
    CHECK(SotnAlucard_DrawSprite(&sprite, common, 10, 10, true, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[11 * 256 + 8] == 0x7fff);
    CHECK(buffers->frame[11 * 256 + 9] == 0x7c00);
    CHECK(buffers->frame[11 * 256 + 11] == 0x001f);
    Clear(buffers->frame, 0x7fff);
    CHECK(
        SotnAlucard_DrawGravity(&sprite, common, 10, 10, false, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[13 * 256 + 11] == 0x001f && buffers->frame[13 * 256 + 10] == 0x03e0 &&
          buffers->frame[13 * 256 + 9] == 0x7c00 && buffers->frame[12 * 256 + 10] == 0x8000);
    Clear(buffers->frame, 0x7fff);
    CHECK(
        SotnAlucard_DrawGravity(&sprite, common, 10, 10, true, buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[13 * 256 + 9] == 0x001f && buffers->frame[13 * 256 + 11] == 0x7c00);
    Clear(buffers->frame, 0x1234);
    CHECK(SotnAlucard_DrawGravity(&sprite, common, SDL_MAX_SINT32, SDL_MIN_SINT32, true,
                                  buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnAlucard_DrawGravity(&sprite, (SotnDataView){0}, 10, 10, false, buffers->frame,
                                   SOTN_FRAME_PIXELS) &&
          buffers->frame[13 * 256 + 11] == 0x1234);
    CHECK(!SotnAlucard_DrawGravity(&sprite, common, 10, 10, false, buffers->frame, 0));
    SotnPlayerStone stone = {.active = true, .palette = 0x19e, .pivot = 24};
    Put16(buffers->common, 0x413c2, 0x7c1f);
    Clear(buffers->frame, 0x7fff);
    CHECK(SotnAlucard_DrawPetrified(&sprite, common, 10, 10, false, stone, buffers->frame,
                                    SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[11 * 256 + 9] == 0x7c1f && buffers->frame[11 * 256 + 10] == 0x7fff);
    Clear(buffers->frame, 0x7fff);
    CHECK(SotnAlucard_DrawPetrified(&sprite, common, 10, 10, true, stone, buffers->frame,
                                    SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[11 * 256 + 11] == 0x7c1f && buffers->frame[11 * 256 + 9] == 0x7fff);
    stone.hidden = true;
    Clear(buffers->frame, 0x1234);
    CHECK(SotnAlucard_DrawPetrified(&sprite, common, 10, 10, false, stone, buffers->frame,
                                    SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[11 * 256 + 9] == 0x1234);
    stone.hidden = false;
    stone.rotation = 32;
    CHECK(SotnAlucard_DrawPetrified(&sprite, common, SDL_MAX_SINT32, SDL_MIN_SINT32, false, stone,
                                    buffers->frame, SOTN_FRAME_PIXELS));
    stone.rotation = 33;
    CHECK(!SotnAlucard_DrawPetrified(&sprite, common, 10, 10, false, stone, buffers->frame,
                                     SOTN_FRAME_PIXELS));
    CHECK(buffers->frame[11 * 256 + 9] == 0x1234);
    stone.rotation = 0;
    stone.palette = 0x200;
    CHECK(!SotnAlucard_DrawPetrified(&sprite, common, 10, 10, false, stone, buffers->frame,
                                     SOTN_FRAME_PIXELS));
    CHECK(SotnAlucard_DrawSprite(&sprite, common, SDL_MAX_SINT32, SDL_MIN_SINT32, false,
                                 buffers->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnAlucard_DecodeSprite(image, (SotnDataView){arc, 71}, 1, &sprite));
    CHECK(sprite.width == 4 && sprite.pivot_x == -1);
    Put32(dra, 0x2f328, 0);
    CHECK(!SotnAlucard_DecodeSprite(image, sheets, 1, &sprite));
    CHECK(!SotnAlucard_DecodeSprite(image, sheets, 265, &sprite));
    CHECK(!SotnAlucard_DrawSprite(&sprite, (SotnDataView){0}, 0, 0, false, buffers->frame,
                                  SOTN_FRAME_PIXELS));
    CHECK(SotnAlucard_DecodeSprite(image, sheets, 0, &sprite));
    CHECK(sprite.width == 0 && sprite.pixels.bytes == NULL);
    CHECK(SotnAlucard_DrawSprite(&sprite, common, 0, 0, false, buffers->frame, SOTN_FRAME_PIXELS));
    SDL_free(dra);
    return true;
}

int main(void)
{
    Buffers *buffers = SDL_calloc(1, sizeof(*buffers));
    if (buffers == NULL)
        return 1;
    const bool passed = TestTiles(buffers) && TestSprite(buffers);
    SDL_free(buffers);
    if (!passed)
        return 1;
    SDL_Log("SOTN_TILEMAP_TEST: PASS");
    return 0;
}
