#include <SDL3/SDL.h>
#include <sotn/actor_graphics.h>
#include <sotn/actor_order.h>
#include <sotn/dialogue_renderer.h>

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

typedef struct Fixture
{
    Uint8 nz0[0x2000], dra[0x4000], stage[0x40000], common[0x42000], weapons[0x1a0000];
    Uint16 frame[SOTN_FRAME_PIXELS];
} Fixture;

static void Put16(Uint8 *bytes, Uint16 value)
{
    bytes[0] = (Uint8)value;
    bytes[1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *bytes, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        bytes[i] = (Uint8)(value >> (i * 8));
}

static void MakeFixture(Fixture *fixture)
{
    SDL_memset(fixture, 0, sizeof(*fixture));
    Uint8 *image = fixture->nz0;
    Put32(image + 20, 0x80180040);
    Put32(image + 0x44, 0x80180600);
    Put32(image + 24, 0x801800b0);
    Put32(image + 0xb0, 0x801800c0);
    Put32(image + 0xc0, 5);
    Put32(image + 0xc4, 0x2000);
    Put32(image + 0xc8, 32);
    Put32(image + 0xcc, 0x80180200);
    Put32(image + 0xd0, SDL_MAX_UINT32);
    Put16(image + 0x202, 31);
    Put16(image + 0x204, 0x8000);
    Put16(image + 0x206, 0x83e0);
    Put32(image + 36, 0x80180300);
    Put32(image + 0x300, 0x80180400);
    Put32(image + 0x304, 0x80180420);
    for (Uint32 i = 0; i < 2; ++i)
    {
        const size_t bank = i == 0 ? 0x400 : 0x420;
        Put32(image + bank, i == 0 ? 1 : 4);
        Put32(image + bank + 4, (129U << 16) | 64);
        Put32(image + bank + 8, (4U << 16) | 1);
        Put32(image + bank + 12, i == 0 ? 0x80180500 : 0x80180520);
        Put32(image + bank + 16, SDL_MAX_UINT32);
    }
    image[0x500] = 0x21;
    image[0x501] = 0x03;
    static const Uint8 literals[] = {0x11, 0x12, 0x13, 0x10, 0xf0};
    SDL_memcpy(image + 0x528, literals, sizeof(literals));
    Put32(image + 0x604, 0x80180608);
    Put16(image + 0x608, 1);
    Uint8 *part = image + 0x60a;
    Put16(part + 2, 65535);
    Put16(part + 4, 65535);
    Put16(part + 6, 4);
    Put16(part + 8, 1);
    Put16(part + 12, 8);
    Put16(part + 14, 4);
    Put16(part + 16, 64);
    Put16(part + 18, 8);
    Put16(part + 20, 65);
    Put32(fixture->dra + 0x3b70 + 3 * 4, 0x800a0100);
    Put32(fixture->dra + 0x104, 0x800a0108);
    SDL_memcpy(fixture->dra + 0x108, image + 0x608, 24);
    Put16(fixture->dra + 0x10a + 10, 0x100);
    Put16(fixture->dra + 0x10a + 12, 24 * 4);
    Put16(fixture->dra + 0x10a + 16, 0);
    Put16(fixture->dra + 0x10a + 20, 1);
    fixture->common[2] = 0x21;
    fixture->common[3] = 0x03;
    Put16(fixture->common + 0x40002, 0x7c00);
}

static bool Create(Fixture *fixture, SotnActorGraphics **out)
{
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)};
    const SotnDataView nz0 = {fixture->nz0, sizeof(fixture->nz0)};
    const SotnTileSources sources = {{fixture->stage, sizeof(fixture->stage)},
                                     {fixture->common, sizeof(fixture->common)}};
    return SotnActorGraphics_Create(dra, nz0, sources, out);
}

static void ClearFrame(Fixture *fixture)
{
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        fixture->frame[i] = 7;
}

static bool Unchanged(const Fixture *fixture)
{
    for (size_t i = 0; i < SOTN_FRAME_PIXELS; ++i)
        if (fixture->frame[i] != 7)
            return false;
    return true;
}

static bool TestSprites(Fixture *fixture)
{
    MakeFixture(fixture);
    SotnActorGraphics *graphics = NULL;
    CHECK(Create(fixture, &graphics));
    SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
    ClearFrame(fixture);
    CHECK(!SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(Unchanged(fixture));
    CHECK(SotnActorGraphics_LoadBank(graphics, 0));
    CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[1] == 0x8000 && fixture->frame[2] == 0x83e0 &&
          fixture->frame[3] == 7);
    ClearFrame(fixture);
    sprite.facing_left = true;
    CHECK(SotnActorGraphics_Draw(graphics, sprite, 2, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 7 && fixture->frame[1] == 0x83e0 && fixture->frame[2] == 0x8000 &&
          fixture->frame[3] == 31);
    ClearFrame(fixture);
    sprite.facing_left = false;
    sprite.blend_mode = 0x10;
    CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[1] == 3 && fixture->frame[2] == (3 | 15 << 5));
    CHECK(SotnActorGraphics_LoadBank(graphics, 1));
    ClearFrame(fixture);
    sprite.blend_mode = 0;
    CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[2] == 0x83e0);
    ClearFrame(fixture);
    sprite.animation_set = 3;
    sprite.palette = 0;
    CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 0x7c00 && fixture->frame[1] == 7);
    ClearFrame(fixture);
    sprite.frame = 2;
    CHECK(!SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(Unchanged(fixture));
    sprite = (SotnActorSprite){0x8001, 1, 512, 0, 0, false};
    CHECK(SotnActorGraphics_Draw(graphics, sprite, SDL_MIN_SINT32, SDL_MAX_SINT32, fixture->frame,
                                 SOTN_FRAME_PIXELS));
    CHECK(Unchanged(fixture));
    SotnActorGraphics_Destroy(graphics);
    return true;
}

static bool TestFailures(Fixture *fixture)
{
    MakeFixture(fixture);
    Put32(fixture->nz0 + 0x428, (8U << 16) | 1);
    SotnActorGraphics *graphics = NULL;
    CHECK(Create(fixture, &graphics));
    CHECK(SotnActorGraphics_LoadBank(graphics, 0));
    CHECK(!SotnActorGraphics_LoadBank(graphics, 1));
    const SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[2] == 0x83e0);
    SotnActorGraphics_Destroy(graphics);
    MakeFixture(fixture);
    Put16(fixture->nz0 + 0x608, 2);
    SDL_memcpy(fixture->nz0 + 0x620, fixture->nz0 + 0x60a, 22);
    Put16(fixture->nz0 + 0x60a, 0x200);
    CHECK(Create(fixture, &graphics));
    CHECK(SotnActorGraphics_LoadBank(graphics, 0));
    ClearFrame(fixture);
    CHECK(!SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(Unchanged(fixture));
    CHECK(!SotnActorGraphics_LoadBank(graphics, 25));
    SotnActorGraphics_Destroy(graphics);
    MakeFixture(fixture);
    Put32(fixture->nz0 + 0xc4, 0x3000);
    CHECK(!Create(fixture, &graphics) && graphics == NULL);
    SotnActorGraphics_Destroy(NULL);
    return true;
}

static bool TestDepthsAndOrdering(Fixture *fixture)
{
    for (Uint32 depth = 1; depth <= 2; ++depth)
    {
        MakeFixture(fixture);
        Put32(fixture->nz0 + 0x400, depth + 1);
        Put32(fixture->nz0 + 0x404, ((depth == 1 ? 130U : 132U) << 16) | 64);
        Put16(fixture->nz0 + 0x60a + 12, (Uint16)(8U + depth * 512));
        if (depth == 1)
        {
            fixture->nz0[0x500] = 1;
            fixture->nz0[0x501] = 2;
            fixture->nz0[0x502] = 3;
            fixture->nz0[0x503] = 0;
        }
        else
        {
            Put16(fixture->nz0 + 0x500, 31);
            Put16(fixture->nz0 + 0x502, 0x8000);
            Put16(fixture->nz0 + 0x504, 0x83e0);
            Put16(fixture->nz0 + 0x506, 0);
        }
        SotnActorGraphics *graphics = NULL;
        CHECK(Create(fixture, &graphics));
        CHECK(SotnActorGraphics_LoadBank(graphics, 0));
        ClearFrame(fixture);
        const SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
        CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
        CHECK(fixture->frame[0] == 31 && fixture->frame[1] == 0x8000 &&
              fixture->frame[2] == 0x83e0 && fixture->frame[3] == 7);
        SotnActorGraphics_Destroy(graphics);
    }
    for (unsigned int priority = 0; priority < 2; ++priority)
    {
        MakeFixture(fixture);
        Put16(fixture->nz0 + 0x608, 2);
        SDL_memcpy(fixture->nz0 + 0x620, fixture->nz0 + 0x60a, 22);
        Put16(fixture->nz0 + 0x620 + 10, 1);
        Put16(fixture->nz0 + 0x222, 0x7c00);
        Put16(fixture->nz0 + 0x60a, priority != 0 ? 0x100 : 0);
        SotnActorGraphics *graphics = NULL;
        CHECK(Create(fixture, &graphics));
        CHECK(SotnActorGraphics_LoadBank(graphics, 0));
        ClearFrame(fixture);
        const SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
        CHECK(SotnActorGraphics_Draw(graphics, sprite, 1, 1, fixture->frame, SOTN_FRAME_PIXELS));
        CHECK(fixture->frame[0] == (priority != 0 ? 0x7c00 : 31));
        SotnActorGraphics_Destroy(graphics);
    }
    return true;
}

static bool TestWeapons(Fixture *fixture)
{
    MakeFixture(fixture);
    const SotnDataView weapons = {fixture->weapons, sizeof(fixture->weapons)};
    for (Uint32 family = 0; family <= 2; family += 2)
    {
        Uint8 *overlay = fixture->weapons + family * 0x7000 + 0x4000;
        Put32(overlay + 0x44, 0x8017a048);
        Put16(overlay + 0x48, 1);
        Uint8 *part = overlay + 0x4a;
        Put16(part + 6, 4);
        Put16(part + 8, 1);
        Put16(part + 18, 4);
        Put16(part + 20, 1);
        Put32(overlay + (family == 0 ? 0x10bc : 0xbd4), 0x8017b100);
        Put16(overlay + 0x1102, 31);
        fixture->weapons[family * 0x7000] = 0x11;
        fixture->weapons[family * 0x7000 + 1] = 0x11;
    }
    SotnActorGraphics *graphics = NULL;
    CHECK(Create(fixture, &graphics));
    for (Uint32 hand = 0; hand < 2; ++hand)
    {
        const SotnEquipment equipment = {.id = (Uint16)(hand == 0 ? 86 : 128),
                                         .family = (Uint8)(hand * 2)};
        CHECK(SotnActorGraphics_LoadWeapon(graphics, weapons, hand, equipment));
        ClearFrame(fixture);
        const SotnActorSprite sprite = {.animation_set = (Uint16)(hand == 0 ? 16 : 18),
                                        .frame = 1,
                                        .palette = (Uint16)(hand == 0 ? 0x110 : 0x128),
                                        .texture_offset = (Uint16)(hand == 0 ? 0x64 : 0x66)};
        CHECK(SotnActorGraphics_Draw(graphics, sprite, 16, 16, fixture->frame, SOTN_FRAME_PIXELS));
        CHECK(fixture->frame[16 * SOTN_FRAME_WIDTH + 16] == 31);
        CHECK(fixture->frame[16 * SOTN_FRAME_WIDTH + 19] == 31);
        CHECK(!SotnActorGraphics_LoadWeapon(graphics, (SotnDataView){0}, hand, equipment));
        CHECK(SotnActorGraphics_LoadWeapon(graphics, weapons, hand, (SotnEquipment){0}));
        CHECK(!SotnActorGraphics_Draw(graphics, sprite, 16, 16, fixture->frame, SOTN_FRAME_PIXELS));
    }
    Uint8 *short_sword = fixture->weapons + 37 * 0x7000 + 0x4000;
    SDL_memcpy(short_sword, fixture->weapons + 0x4000, 0x3000);
    // Exactly five palettes fit; a seven-palette read would exceed this synthetic overlay.
    Put32(short_sword + 0x7b8, 0x8017cf60);
    Put16(short_sword + 0x2f62, 31);
    fixture->weapons[37 * 0x7000] = 0x11;
    for (Uint32 hand = 0; hand < 2; ++hand)
    {
        CHECK(SotnActorGraphics_LoadWeapon(graphics, weapons, hand,
                                           (SotnEquipment){.id = 19, .family = 37}));
        ClearFrame(fixture);
        const SotnActorSprite sprite = {.animation_set = (Uint16)(hand == 0 ? 16 : 18),
                                        .frame = 1,
                                        .palette = (Uint16)(hand == 0 ? 0x110 : 0x128),
                                        .texture_offset = (Uint16)(hand == 0 ? 0x64 : 0x66)};
        CHECK(SotnActorGraphics_Draw(graphics, sprite, 16, 16, fixture->frame, SOTN_FRAME_PIXELS));
        CHECK(fixture->frame[16 * SOTN_FRAME_WIDTH + 16] == 31);
    }
    SotnActorGraphics_Destroy(graphics);
    return true;
}

static bool TestQuad(Fixture *fixture)
{
    MakeFixture(fixture);
    SotnActorGraphics *graphics = NULL;
    CHECK(Create(fixture, &graphics) && SotnActorGraphics_LoadBank(graphics, 0));
    ClearFrame(fixture);
    SotnTextureQuad q = {.x = {0, 4, 0, 4},
                         .y = {0, 0, 1, 1},
                         .u = {4, 8, 4, 8},
                         .v = {64, 64, 65, 65},
                         .page = 2,
                         .palette = 512};
    CHECK(SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[1] == 0x8000 && fixture->frame[2] == 0x83e0);
    q.modulate = true;
    q.opacity = 64;
    CHECK(SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 15 && fixture->frame[1] == 0x8000 && fixture->frame[2] == 0x81e0);
    q.modulate = false;
    const Uint16 first = fixture->frame[0];
    q.page = 0x1ff;
    CHECK(!SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS) &&
          fixture->frame[0] == first);
    q.page = 2;
    q.x[3] = 5;
    CHECK(SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS) &&
          fixture->frame[0] == 31);
    ClearFrame(fixture);
    q = (SotnTextureQuad){.x = {0, 4, 0, 4},
                          .y = {0, 0, 4, 1},
                          .u = {4, 4, 4, 4},
                          .v = {64, 64, 64, 64},
                          .page = 2,
                          .palette = 512};
    CHECK(SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[2 * SOTN_FRAME_WIDTH] == 31 &&
          fixture->frame[2 * SOTN_FRAME_WIDTH + 3] == 7);
    q = (SotnTextureQuad){.x = {0, 4, 0, 4},
                          .y = {0, 0, 4, 4},
                          .u = {4, 4, 4, 4},
                          .v = {64, 64, 64, 64},
                          .page = 2,
                          .palette = 512,
                          .gouraud = true,
                          .vertex_red = {128, 0, 128, 0}};
    CHECK(SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 27 && fixture->frame[1] == 19 && fixture->frame[2] == 11 &&
          fixture->frame[3] == 3);
    q.x[3] = 4;
    q.x[0] = SDL_MAX_SINT32;
    CHECK(!SotnActorGraphics_Quad(graphics, q, fixture->frame, SOTN_FRAME_PIXELS));
    SotnActorGraphics_Destroy(graphics);
    return true;
}

static bool TestColorQuad(Fixture *f)
{
    ClearFrame(f);
    SotnTextureQuad q = {.x = {0, 4, 0, 4},
                         .y = {0, 0, 4, 4},
                         .untextured = true,
                         .gouraud = true,
                         .vertex_red = {255, 0, 255, 0}};
    CHECK(SotnActorGraphics_Quad(NULL, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 27 && f->frame[1] == 19 && f->frame[2] == 11 && f->frame[3] == 3);
    q.gouraud = false;
    CHECK(SotnActorGraphics_Quad(NULL, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 0);
    q.red = 128;
    q.blend = true;
    const Uint16 expected[4] = {12, 24, 0, 12};
    for (Uint16 mode = 0; mode < 4; ++mode)
    {
        for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
            f->frame[i] = 8;
        q.page = (Uint16)(mode * 32);
        CHECK(SotnActorGraphics_Quad(NULL, q, f->frame, SOTN_FRAME_PIXELS));
        for (Uint32 row = 0; row < 4; ++row)
            for (Uint32 col = 0; col < 4; ++col)
                CHECK((f->frame[row * SOTN_FRAME_WIDTH + col] & 31U) == expected[mode]);
        CHECK(f->frame[4] == 8 && f->frame[4 * SOTN_FRAME_WIDTH] == 8);
    }
    q.x[0] = 2049;
    const Uint16 before = f->frame[0];
    CHECK(!SotnActorGraphics_Quad(NULL, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == before);
    q.x[0] = 0;
    CHECK(!SotnActorGraphics_Quad(NULL, q, f->frame, SOTN_FRAME_PIXELS - 1));
    CHECK(!SotnActorGraphics_Quad(NULL, q, NULL, SOTN_FRAME_PIXELS));
    q.untextured = false;
    CHECK(!SotnActorGraphics_Quad(NULL, q, f->frame, SOTN_FRAME_PIXELS));
    return true;
}

static bool TestRopeCache(Fixture *fixture)
{
    MakeFixture(fixture);
    for (Uint32 row = 64; row < 80; ++row)
        for (Uint32 col = 30; col < 32; ++col)
            Put16(fixture->stage + 27U * 8192 + (row * 32 + col) * 2, 0x1111);
    SotnActorGraphics *graphics = NULL;
    CHECK(Create(fixture, &graphics));
    CHECK(!SotnActorGraphics_PrepareOuterRope(NULL));
    CHECK(SotnActorGraphics_PrepareOuterRope(graphics));
    ClearFrame(fixture);
    const SotnTextureQuad rope = {.x = {0, 16, 0, 16},
                                  .y = {0, 0, 16, 16},
                                  .u = {0, 16, 0, 16},
                                  .v = {240, 240, 256, 256},
                                  .page = 0x10,
                                  .palette = 512};
    CHECK(SotnActorGraphics_Quad(graphics, rope, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[0] == 31 && fixture->frame[15 * SOTN_FRAME_WIDTH + 7] == 31 &&
          fixture->frame[8] == 7 && fixture->frame[15 * SOTN_FRAME_WIDTH + 15] == 7);
    SotnActorGraphics_Destroy(graphics);
    return true;
}

static bool TestTransformFlags(Fixture *fixture)
{
    MakeFixture(fixture);
    SotnActorGraphics *g = NULL;
    CHECK(Create(fixture, &g) && SotnActorGraphics_LoadBank(g, 0));
    const SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_Draw(g, sprite, 20, 20, fixture->frame, SOTN_FRAME_PIXELS));
    Uint16 expected[SOTN_FRAME_PIXELS] = {0};
    SDL_memcpy(expected, fixture->frame, sizeof(expected));
    // Original routines may retain all transform values when clearing their enable bits.
    SotnActorState a = {.scale_x = 768,
                        .scale_y = 512,
                        .rotation = 1024,
                        .opacity = 0,
                        .scale_set = true,
                        .rotated = true,
                        .opacity_set = true,
                        .transform_flags_set = true};
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_DrawTransform(g, sprite, 20, 20, SotnActorGraphics_GetTransform(a),
                                          fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(SDL_memcmp(expected, fixture->frame, sizeof(expected)) == 0);
    a.transform_flags = SOTN_ACTOR_SCALE_Y;
    a.scale_y = 0;
    SotnSpriteTransform t = SotnActorGraphics_GetTransform(a);
    CHECK(t.scale_x == 256 && t.scale_y == 0 && t.rotation == 0 && t.opacity == 128);
    a.transform_flags = SOTN_ACTOR_SCALE_X | SOTN_ACTOR_ROTATE | SOTN_ACTOR_OPACITY;
    a.scale_x = 0xff00;
    t = SotnActorGraphics_GetTransform(a);
    CHECK(t.scale_x == -256 && t.scale_y == 256 && t.rotation == 1024 && t.opacity == 0);
    a = (SotnActorState){.scale_x = 512, .opacity = 64};
    t = SotnActorGraphics_GetTransform(a);
    CHECK(t.scale_x == 512 && t.scale_y == 256 && t.opacity == 64);
    a = (SotnActorState){.scale_set = true, .opacity_set = true};
    t = SotnActorGraphics_GetTransform(a);
    CHECK(!t.scale_x && !t.scale_y && !t.opacity);
    SotnActorGraphics_Destroy(g);
    return true;
}

static bool TestRotation(Fixture *fixture)
{
    MakeFixture(fixture);
    SotnActorGraphics *g = NULL;
    CHECK(Create(fixture, &g) && SotnActorGraphics_LoadBank(g, 0));
    SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
    ClearFrame(fixture);
    CHECK(
        SotnActorGraphics_DrawRotated(g, sprite, 20, 20, 1024, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 20] == 31);
    CHECK(fixture->frame[20 * SOTN_FRAME_WIDTH + 20] == 0x8000);
    CHECK(fixture->frame[21 * SOTN_FRAME_WIDTH + 20] == 0x83e0);
    CHECK(fixture->frame[22 * SOTN_FRAME_WIDTH + 20] == 7);
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_DrawTransform(
        g, sprite, 20, 20,
        (SotnSpriteTransform){.rotation = 1024, .scale_x = 256, .scale_y = 256, .opacity = 64},
        fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 20] == 15);
    CHECK(fixture->frame[21 * SOTN_FRAME_WIDTH + 20] == 0x81e0);
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_DrawTransform(
        g, sprite, 20, 20, (SotnSpriteTransform){.scale_x = 512, .scale_y = 256, .opacity = 128},
        fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 18] == 31 &&
          fixture->frame[19 * SOTN_FRAME_WIDTH + 19] == 31);
    const Uint16 preserved = fixture->frame[19 * SOTN_FRAME_WIDTH + 18];
    CHECK(!SotnActorGraphics_DrawTransform(g, sprite, 20, 20,
                                           (SotnSpriteTransform){.scale_y = 256, .opacity = 128},
                                           fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 18] == preserved);
    Uint16 reference[SOTN_FRAME_PIXELS] = {0};
    SDL_memcpy(reference, fixture->frame, sizeof(reference));
    for (Uint16 axis = 0; axis < 3; ++axis)
    {
        ClearFrame(fixture);
        CHECK(
            SotnActorGraphics_DrawTransform(g, sprite, 20, 20,
                                            (SotnSpriteTransform){.scale_x = axis == 1 ? 512 : -512,
                                                                  .scale_y = axis == 0 ? 256 : -256,
                                                                  .opacity = 128},
                                            fixture->frame, SOTN_FRAME_PIXELS));
        for (Uint16 y = 0; y < 40; ++y)
            for (Uint16 x = 0; x < 40; ++x)
            {
                const Uint16 source_x = axis == 1 ? x : (Uint16)(39 - x);
                const Uint16 source_y = axis == 0 ? y : (Uint16)(39 - y);
                CHECK(fixture->frame[y * SOTN_FRAME_WIDTH + x] ==
                      reference[source_y * SOTN_FRAME_WIDTH + source_x]);
            }
    }
    CHECK(!SotnActorGraphics_DrawTransform(
        g, sprite, 20, 20, (SotnSpriteTransform){.scale_x = -1025, .scale_y = 256, .opacity = 128},
        fixture->frame, SOTN_FRAME_PIXELS));
    sprite.facing_left = true;
    ClearFrame(fixture);
    CHECK(
        SotnActorGraphics_DrawRotated(g, sprite, 20, 20, 1024, fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[18 * SOTN_FRAME_WIDTH + 20] == 7);
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 20] == 0x83e0);
    CHECK(fixture->frame[20 * SOTN_FRAME_WIDTH + 20] == 0x8000);
    CHECK(fixture->frame[21 * SOTN_FRAME_WIDTH + 20] == 31);
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_DrawRotated(g, sprite, SDL_MAX_SINT32, 20, 128, fixture->frame,
                                        SOTN_FRAME_PIXELS));
    CHECK(Unchanged(fixture));
    SotnActorGraphics_Destroy(g);
    return true;
}

static bool TestDialogue(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    CHECK(SotnActorGraphics_Create(
        (SotnDataView){f->dra, sizeof(f->dra)}, (SotnDataView){f->nz0, sizeof(f->nz0)},
        (SotnTileSources){{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}}, &g));
    SotnDialogueState d = {.x = 60, .y = 148, .visible = true};
    SDL_strlcpy(d.lines[0], "TEST", sizeof(d.lines[0]));
    CHECK(SotnDialogueRenderer_Draw(g, d, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[148 * 256 + 4] == 0x4210 && f->frame[149 * 256 + 5] == 0x2400);
    d.row = 5;
    CHECK(!SotnDialogueRenderer_Draw(g, d, f->frame, SOTN_FRAME_PIXELS));
    d.row = 0;
    SDL_memset(d.lines[0], 'T', sizeof(d.lines[0]));
    CHECK(!SotnDialogueRenderer_Draw(g, d, f->frame, SOTN_FRAME_PIXELS));
    CHECK(SotnActorGraphics_LoadPortrait(g, 0x500, 0));
    CHECK(!SotnActorGraphics_LoadPortrait(g, 0x500, 2));
    CHECK(!SotnActorGraphics_LoadPortrait(g, 0x1fff, 0));
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = 0x7fff;
    CHECK(!SotnDialogueRenderer_Background(241, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 0x7fff);
    CHECK(SotnDialogueRenderer_Background(240, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 0x421);
    CHECK(!SotnDialogueRenderer_Item((SotnDataView){f->dra, sizeof(f->dra)},
                                     (SotnDataView){f->nz0, sizeof(f->nz0)}, 6, 100, 100, 14, 14,
                                     false, f->frame, SOTN_FRAME_PIXELS));
    SotnActorGraphics_Destroy(g);
    // Chapel's portraits occupy the original page 0x90, independently from Library/NZ0.
    MakeFixture(f);
    for (Uint32 y = 0; y < 72; ++y)
        for (Uint32 x = 0; x < 48; ++x)
            f->nz0[0x800 + y * 48 + x] = (Uint8)(x < 24 ? 1 : 3);
    CHECK(SotnActorGraphics_CreateStage(
        (SotnDataView){f->dra, sizeof(f->dra)}, (SotnDataView){f->nz0, sizeof(f->nz0)},
        (SotnTileSources){{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}}, 6, &g));
    for (Uint16 slot = 0; slot < 2; ++slot)
    {
        CHECK(SotnActorGraphics_LoadPortrait(g, 0x800, slot));
        d = (SotnDialogueState){.x = 60,
                                .y = 148,
                                .visible = true,
                                .portrait_visible = true,
                                .portrait_page = 0x90,
                                .palette = 0x200,
                                .portrait_slot = (Uint8)slot,
                                .mirrored = slot != 0};
        d.portraits[slot] = 0x800;
        CHECK(SotnDialogueRenderer_Draw(g, d, f->frame, SOTN_FRAME_PIXELS));
        CHECK(f->frame[184 * 256 + 10] == (slot ? 0x83e0 : 31) &&
              f->frame[184 * 256 + 50] == (slot ? 31 : 0x83e0));
    }
    SotnActorGraphics_Destroy(g);
    // BO5's copied words occupy the original cache rows; spaces stay transparent.
    MakeFixture(f);
    for (Uint32 y = 0; y < 8; ++y)
    {
        Put16(f->common + 24U * 8192U + (16U + y) * 64U + 4, 0x1111);
        Put16(f->common + 24U * 8192U + (16U + y) * 64U + 6, 0x3333);
    }
    CHECK(SotnActorGraphics_CreateStage(
        (SotnDataView){f->dra, sizeof(f->dra)}, (SotnDataView){f->nz0, sizeof(f->nz0)},
        (SotnTileSources){{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}}, 25, &g));
    d = (SotnDialogueState){.primitive_text = true};
    SDL_strlcpy(d.lines[0], "A A", sizeof(d.lines[0]));
    SDL_strlcpy(d.lines[4], "A", sizeof(d.lines[4]));
    CHECK(SotnActorGraphics_LoadDialogue(g, d));
    SotnTextureQuad cache = {.x = {0, 24, 0, 24},
                             .y = {0, 0, 8, 8},
                             .u = {8, 32, 8, 32},
                             .v = {128, 128, 136, 136},
                             .page = 16,
                             .palette = 0x200};
    ClearFrame(f);
    CHECK(SotnActorGraphics_Quad(g, cache, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 31 && f->frame[4] == 0x83e0 && f->frame[8] == 7 && f->frame[16] == 31);
    cache.v[0] = cache.v[1] = 176;
    cache.v[2] = cache.v[3] = 184;
    ClearFrame(f);
    CHECK(SotnActorGraphics_Quad(g, cache, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    d.lines[0][0] = 1;
    CHECK(!SotnActorGraphics_LoadDialogue(g, d));
    d.lines[0][0] = 'A';
    d.column = 25;
    CHECK(!SotnActorGraphics_LoadDialogue(g, d) && !SotnActorGraphics_LoadDialogue(NULL, d));
    Put16(f->nz0 + 0xb84, 1001);
    CHECK(!SotnActorGraphics_LoadPortrait(g, 0x800, 0));
    Put16(f->nz0 + 0xb84, 32);
    CHECK(SotnActorGraphics_LoadPortrait(g, 0x800, 0));
    SotnActorGraphics_Destroy(g);
    return true;
}

static bool StageBankBounds(Fixture *f)
{
    MakeFixture(f);
    Put32(f->nz0 + 0x40 + 24 * 4, 0x80180600);
    SotnActorGraphics *g = NULL;
    CHECK(Create(f, &g));
    SotnActorSprite sprite = {.animation_set = 0x8018, .frame = 1, .palette = 0x200};
    ClearFrame(f);
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(Unchanged(f));
    SotnActorGraphics_Destroy(g);
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, overlay = {f->nz0, sizeof(f->nz0)};
    const SotnTileSources art = {{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}};
    CHECK(SotnActorGraphics_CreateStage(dra, overlay, art, 0, &g));
    CHECK(SotnActorGraphics_LoadBank(g, 0));
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(!Unchanged(f));
    sprite.animation_set = 0x8019;
    ClearFrame(f);
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(Unchanged(f));
    SotnActorGraphics_Destroy(g);
    CHECK(SotnActorGraphics_CreateStage(dra, overlay, art, 2, &g));
    // Stage-start bank zero must already be resident before a room asks for a bank.
    sprite.animation_set = 0x8001;
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    ClearFrame(f);
    sprite.animation_set = 0x8018;
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS) && Unchanged(f));
    SotnActorGraphics_Destroy(g);
    CHECK(!SotnActorGraphics_CreateStage(dra, overlay, art, 3, &g) && g == NULL);
    return true;
}

static bool DefaultBo4Bank(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, stage = {f->nz0, sizeof(f->nz0)};
    const SotnTileSources art = {{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}};
    CHECK(SotnActorGraphics_CreateStage(dra, stage, art, 26, &g));
    SDL_zero(f->frame);
    const SotnTextureQuad q = {.x = {0, 4, 0, 4},
                               .y = {0, 0, 1, 1},
                               .u = {4, 8, 4, 8},
                               .v = {64, 64, 65, 65},
                               .page = 2,
                               .palette = 0x200};
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    SotnActorGraphics_Destroy(g);
    g = NULL;
    Put32(f->nz0 + 0x400, 7);
    CHECK(!SotnActorGraphics_CreateStage(dra, stage, art, 26, &g) && g == NULL);
    MakeFixture(f);
    CHECK(SotnActorGraphics_CreateStage(dra, stage, art, 25, &g));
    SDL_zero(f->frame);
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    SotnActorGraphics_Destroy(g);
    g = NULL;
    Put32(f->nz0 + 0x400, 7);
    CHECK(!SotnActorGraphics_CreateStage(dra, stage, art, 25, &g) && g == NULL);
    return true;
}

static bool PackedSheets(Fixture *f)
{
    MakeFixture(f);
    Uint8 *image = f->nz0;
    Put32(image + 0x44, 0x80181800);
    Put32(image + 0x48, 0x80181800);
    Put32(image + 0x1804, 0x80181810);
    Put32(image + 0x1808, 0x80181810);
    Put16(image + 0x1810, 0x8000);
    Put16(image + 0x1812, (Uint16)-2);
    Put16(image + 0x1814, (Uint16)-1);
    Put32(image + 44, 0x80181820);
    Put32(image + 48, 0x80181820);
    for (Uint32 i = 0; i < 4; ++i)
        Put32(image + 0x1820 + i * 4, 0x80181830);
    image[0x1830] = 4;
    image[0x1831] = 2;
    image[0x1832] = 1;
    image[0x1833] = 2;
    image[0x1834] = 0x21;
    image[0x1835] = 0x03;
    image[0x1836] = 0x40;
    image[0x1837] = 0x12;
    Put16(f->dra + 0x21d8, 1);
    Put16(f->dra + 0x21da, 257);
    SotnActorGraphics *g = NULL;
    CHECK(SotnActorGraphics_CreateStage(
        (SotnDataView){f->dra, sizeof(f->dra)}, (SotnDataView){image, sizeof(f->nz0)},
        (SotnTileSources){{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}}, 26, &g));
    SotnActorSprite sprite = {.animation_set = 0x8001, .frame = 1, .palette = 0x8200};
    SotnActorSprite retained = sprite;
    retained.frame |= 0x8000U;
    CHECK(!SotnActorGraphics_LoadDopplePose(g, retained));
    CHECK(SotnActorGraphics_LoadDopplePose(g, sprite));
    const SotnTextureQuad cached = {.x = {0, 4, 0, 4},
                                    .y = {0, 0, 2, 2},
                                    .u = {4, 8, 4, 8},
                                    .v = {1, 1, 3, 3},
                                    .page = 0x10,
                                    .palette = 0x200};
    SDL_zero(f->frame);
    CHECK(SotnActorGraphics_Quad(g, cached, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 31 && f->frame[1] == 0x8000 && f->frame[2] == 0x83e0);
    image[0x1834] = 0x22;
    CHECK(SotnActorGraphics_LoadDopplePose(g, retained));
    SDL_zero(f->frame);
    CHECK(SotnActorGraphics_Quad(g, cached, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    image[0x1830] = 3;
    CHECK(!SotnActorGraphics_LoadDopplePose(g, sprite));
    image[0x1830] = 4;
    CHECK(SotnActorGraphics_Quad(g, cached, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    CHECK(SotnActorGraphics_LoadDopplePose(g, sprite));
    CHECK(SotnActorGraphics_Quad(g, cached, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 0x8000);
    image[0x1834] = 0x21;
    CHECK(SotnActorGraphics_LoadDopplePose(g, sprite));
    CHECK(!SotnActorGraphics_LoadDopplePose(NULL, sprite));

    SDL_memset(f->frame, 0, sizeof(f->frame));
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 31 && f->frame[11 * 256 + 10] == 0x8000 &&
          f->frame[11 * 256 + 11] == 0x83e0 && f->frame[11 * 256 + 12] == 0);
    sprite.facing_left = true;
    sprite.frame |= 0x8000;
    SDL_memset(f->frame, 0, sizeof(f->frame));
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 11] == 31 && f->frame[11 * 256 + 10] == 0x8000 &&
          f->frame[11 * 256 + 9] == 0x83e0);
    sprite.facing_left = false;
    sprite.animation_set = 0x8002;
    SDL_memset(f->frame, 0, sizeof(f->frame));
    CHECK(SotnActorGraphics_DrawTransform(
        g, sprite, 10, 10, (SotnSpriteTransform){.scale_x = 512, .scale_y = 512, .opacity = 128},
        f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[12 * 256 + 8] == 31 && f->frame[12 * 256 + 9] == 31);
    Uint16 reference[SOTN_FRAME_PIXELS] = {0};
    SDL_memcpy(reference, f->frame, sizeof(reference));
    SDL_zero(f->frame);
    CHECK(SotnActorGraphics_DrawTransform(
        g, sprite, 10, 10, (SotnSpriteTransform){.scale_x = -512, .scale_y = -512, .opacity = 128},
        f->frame, SOTN_FRAME_PIXELS));
    for (Uint16 y = 0; y < 20; ++y)
        for (Uint16 x = 0; x < 20; ++x)
            CHECK(f->frame[y * 256U + x] == reference[(19U - y) * 256U + 19U - x]);
    sprite.blend_mode = 0x30;
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = 0x7c00;
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 31 && f->frame[11 * 256 + 10] == 0x7c00 &&
          f->frame[11 * 256 + 11] == 0x7fe0);
    Put16(image + 0x1fd8, 0);
    Put16(image + 0x1fda, 256);
    Put16(image + 0x1fdc, 16);
    Put16(image + 0x1fde, 64);
    SotnActorSprite dying = {.animation_set = 0x8001, .frame = 0x8001, .palette = 0x8200};
    Uint8 erased[512] = {0};
    erased[68 / 8] = 1U << (68 & 7U);
    SotnActorGraphics *cold = NULL;
    CHECK(SotnActorGraphics_CreateStage(
        (SotnDataView){f->dra, sizeof(f->dra)}, (SotnDataView){image, sizeof(f->nz0)},
        (SotnTileSources){{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}}, 26,
        &cold));
    CHECK(SotnActorGraphics_DoppleDissolve(cold, dying, (SotnDataView){erased, sizeof(erased)}));
    f->frame[11 * 256 + 9] = 0x7c00;
    CHECK(SotnActorGraphics_Draw(cold, dying, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 0x7c00 && f->frame[11 * 256 + 10] == 0x8000);
    SotnActorGraphics_Destroy(cold);
    CHECK(SotnActorGraphics_DoppleDissolve(g, dying, (SotnDataView){erased, sizeof(erased)}));
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = 0x7c00;
    CHECK(SotnActorGraphics_Draw(g, dying, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 0x7c00 && f->frame[11 * 256 + 10] == 0x8000);
    SDL_memset(erased, 255, sizeof(erased));
    CHECK(SotnActorGraphics_DoppleDissolve(g, dying, (SotnDataView){erased, sizeof(erased)}));
    f->frame[11 * 256 + 10] = 0x7c00;
    CHECK(SotnActorGraphics_Draw(g, dying, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 10] == 0x7c00);
    SDL_zero(erased);
    CHECK(!SotnActorGraphics_DoppleDissolve(g, dying, (SotnDataView){erased, sizeof(erased) - 1}));
    CHECK(SotnActorGraphics_Draw(g, dying, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 10] == 0x7c00);
    CHECK(SotnActorGraphics_DoppleDissolve(g, dying, (SotnDataView){erased, sizeof(erased)}));
    CHECK(SotnActorGraphics_DrawTransform(g, dying, 10, 10,
                                          (SotnSpriteTransform){.scale_x = 256,
                                                                .scale_y = 256,
                                                                .opacity = 128,
                                                                .tint = true,
                                                                .red = 64,
                                                                .green = 0,
                                                                .blue = 0},
                                          f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 15);
    Put16(image + 0x1fdc, 17);
    CHECK(!SotnActorGraphics_DoppleDissolve(g, dying, (SotnDataView){erased, sizeof(erased)}));
    Put16(image + 0x1fdc, 16);
    CHECK(!SotnActorGraphics_DoppleDissolve(NULL, dying, (SotnDataView){erased, sizeof(erased)}));
    dying.frame = 1;
    CHECK(SotnActorGraphics_LoadDopplePose(g, dying));
    CHECK(SotnActorGraphics_Draw(g, dying, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    Put16(image + 0x11ea, (Uint16)-3);
    CHECK(SotnActorGraphics_UpdateDoppleBat(g, 1, 2));
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 10] == 31 && image[0x1812] == 0xfe);
    CHECK(!SotnActorGraphics_UpdateDoppleBat(g, 1, 65));
    CHECK(!SotnActorGraphics_UpdateDoppleBat(g, 18, 0));
    CHECK(!SotnActorGraphics_UpdateDoppleBat(NULL, 1, 0));
    // Validate all input references before modifying a pixel.
    Put16(image + 0x1810, 0x8004);
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 31);
    Put16(image + 0x1810, 0x8000);
    image[0x1830] = 3;
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    image[0x1830] = 4;
    CHECK(SotnActorGraphics_Draw(g, sprite, SDL_MAX_SINT32, SDL_MIN_SINT32, f->frame,
                                 SOTN_FRAME_PIXELS));
    Put32(image + 0x1820, 0x80180000 + (Uint32)sizeof(f->nz0) - 4);
    image[sizeof(f->nz0) - 4] = image[sizeof(f->nz0) - 3] = 252;
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    SotnActorGraphics_Destroy(g);
    g = NULL;
    Put32(image + 0x1820, 0x80181830);
    CHECK(Create(f, &g));
    CHECK(!SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    SotnActorGraphics_Destroy(g);
    return true;
}

static bool TestColorLine(Fixture *f)
{
    SDL_zero(f->frame);
    SotnColorLine line = {.x = {2, 6}, .y = {3, 3}, .red = {248, 0}, .green = {0, 248}};
    CHECK(SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[3 * 256 + 2] == 31 && f->frame[3 * 256 + 6] == (31 << 5) &&
          f->frame[3 * 256 + 4] == (15 | (15 << 5)) && f->frame[3 * 256 + 7] == 0);
    line.x[0] = 6;
    line.x[1] = 2;
    line.red[0] = 0;
    line.red[1] = 248;
    line.green[0] = 248;
    line.green[1] = 0;
    line.blend = true;
    line.blend_mode = 1;
    f->frame[3 * 256 + 4] = 0x7c00;
    CHECK(SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[3 * 256 + 4] == 0x7def);
    SDL_zero(f->frame);
    line = (SotnColorLine){.x = {1, 5}, .y = {1, 3}, .red = {248, 248}};
    CHECK(SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[1 * 256 + 1] == 31 && f->frame[1 * 256 + 2] == 31 &&
          f->frame[2 * 256 + 3] == 31 && f->frame[2 * 256 + 4] == 31 &&
          f->frame[3 * 256 + 5] == 31);
    line = (SotnColorLine){.x = {-2, 2}, .y = {0, 0}, .red = {248, 248}};
    CHECK(SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    line = (SotnColorLine){
        .x = {0, 0}, .y = {0, 0}, .red = {248, 248}, .blend = true, .blend_mode = 2};
    f->frame[0] = 3;
    CHECK(SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 0);
    line.blend_mode = 3;
    f->frame[0] = 3;
    CHECK(SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 10);
    line.blend_mode = 4;
    CHECK(!SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 10);
    line.blend_mode = 0;
    line.x[0] = 2049;
    CHECK(!SotnActorGraphics_Line(line, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 10);
    line.x[0] = 0;
    CHECK(!SotnActorGraphics_Line(line, f->frame, 1));
    CHECK(!SotnActorGraphics_Line(line, NULL, SOTN_FRAME_PIXELS));
    return true;
}

static bool TestActorOrder(void)
{
    SotnActorDraw items[4] = {0};
    size_t count = 0;
    CHECK(SotnActorOrder_Add(
        items, 4, &count, (SotnActorDraw){.priority = 136, .actor = 0, .kind = SOTN_DRAW_SPRITE}));
    CHECK(SotnActorOrder_Add(
        items, 4, &count,
        (SotnActorDraw){.priority = 208, .actor = 0, .primitive = 0, .kind = SOTN_DRAW_PRIMITIVE}));
    CHECK(SotnActorOrder_Add(
        items, 4, &count,
        (SotnActorDraw){.priority = 64, .actor = 0, .primitive = 1, .kind = SOTN_DRAW_PRIMITIVE}));
    CHECK(SotnActorOrder_Add(
        items, 4, &count, (SotnActorDraw){.priority = 136, .actor = 1, .kind = SOTN_DRAW_ACTOR}));
    // dra/42398.c calls forward ClearOTag, then DrawOTag from its FIRST bucket.
    // Bucket 64 precedes sprites at 136; bucket 208 covers them. Equal entries stay stable.
    CHECK(count == 4 && items[0].primitive == 1 && items[0].priority == 64 &&
          items[1].kind == SOTN_DRAW_SPRITE && items[2].actor == 1 && items[3].priority == 208);
    Uint16 *frame = SDL_calloc(SOTN_FRAME_PIXELS, sizeof(*frame));
    CHECK(frame != NULL);
    for (size_t i = 0; i < count; ++i)
    {
        const bool foreground = items[i].priority == 208;
        const SotnColorLine pixel = {.x = {0, 0},
                                     .y = {0, 0},
                                     .red = {foreground ? 248 : 0, foreground ? 248 : 0},
                                     .green = {0, 0},
                                     .blue = {foreground ? 0 : 248, foreground ? 0 : 248}};
        CHECK(SotnActorGraphics_Line(pixel, frame, SOTN_FRAME_PIXELS));
    }
    CHECK(frame[0] == 31); // The higher-priority foreground must cover the blue background.
    SDL_free(frame);
    SotnActorDraw saved[4] = {0};
    SDL_memcpy(saved, items, sizeof(items));
    CHECK(!SotnActorOrder_Add(items, 4, &count, (SotnActorDraw){0}) && count == 4 &&
          SDL_memcmp(saved, items, sizeof(items)) == 0);
    count = 0;
    CHECK(!SotnActorOrder_Add(items, 4, &count, (SotnActorDraw){.priority = 512}) && count == 0);
    CHECK(!SotnActorOrder_Add(items, 4, &count, (SotnActorDraw){.kind = 99}) && count == 0);
    CHECK(!SotnActorOrder_Add(items, 4, &count,
                              (SotnActorDraw){.kind = SOTN_DRAW_PRIMITIVE, .primitive = 384}));
    CHECK(!SotnActorOrder_Add(NULL, 4, &count, (SotnActorDraw){0}));
    CHECK(!SotnActorOrder_Add(items, 4, NULL, (SotnActorDraw){0}));
    return true;
}
static bool TestPaletteBlend(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    CHECK(Create(f, &g) && SotnActorGraphics_LoadBank(g, 0));
    SotnPaletteBlend blend = {.source = 512,
                              .destination = 517,
                              .steps = 16,
                              .red = 24,
                              .green = 48,
                              .blue = 80,
                              .active = true};
    CHECK(SotnActorGraphics_BlendPalette(g, blend));
    SotnTextureQuad q = {.x = {0, 4, 0, 4},
                         .y = {0, 0, 1, 1},
                         .u = {4, 8, 4, 8},
                         .v = {64, 64, 65, 65},
                         .page = 2,
                         .palette = 517};
    ClearFrame(f);
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 31 && f->frame[1] == 0x8000 && f->frame[2] == 0x83e0 && f->frame[3] == 7);
    q.palette = 532;
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    const Uint16 red = (Uint16)(4U | (5U << 5) | (9U << 10)),
                 green = (Uint16)(0x8000U | 2U | (7U << 5) | (9U << 10));
    CHECK(f->frame[0] == red && f->frame[1] == 0x8000 && f->frame[2] == green);
    CHECK(f->nz0[0x202] == 31 && f->nz0[0x204] == 0 && f->nz0[0x205] == 0x80);
    const SotnPaletteBlend invalid[] = {
        {.source = 768, .destination = 517, .steps = 16, .active = true},
        {.source = 600, .destination = 517, .steps = 16, .active = true},
        {.source = 512, .destination = 767, .steps = 2, .active = true},
        {.source = 512, .destination = 517, .steps = 0, .active = true},
        {.source = 512, .destination = 517, .steps = 17, .active = true},
        {.source = 512, .destination = 510, .steps = 4, .active = true}};
    for (size_t i = 0; i < SDL_arraysize(invalid); ++i)
    {
        CHECK(!SotnActorGraphics_BlendPalette(g, invalid[i]));
        CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == red &&
              f->frame[1] == 0x8000 && f->frame[2] == green);
    }
    CHECK(SotnActorGraphics_BlendPalette(g, (SotnPaletteBlend){0}));
    blend.destination = 600;
    blend.steps = 1;
    q.palette = 600;
    CHECK(SotnActorGraphics_BlendPalette(g, blend) &&
          SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    CHECK(!SotnActorGraphics_BlendPalette(NULL, blend));
    SotnActorGraphics_Destroy(g);
    // A missing final source color must not partially overwrite a valid destination.
    MakeFixture(f);
    Put32(f->nz0 + 0xc8, 31);
    CHECK(Create(f, &g) && SotnActorGraphics_LoadBank(g, 0));
    blend.source = 513;
    blend.destination = 512;
    CHECK(!SotnActorGraphics_BlendPalette(g, blend));
    q.palette = 512;
    ClearFrame(f);
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    SotnActorGraphics_Destroy(g);
    return true;
}
static bool TestPaletteRedShift(Fixture *f)
{
    MakeFixture(f);
    const Uint16 color = (Uint16)(0x8000U | 4U | (13U << 5) | (3U << 10));
    Put32(f->nz0 + 0xc4, 32 * 16);
    Put16(f->nz0 + 0x200, 0x8000);
    Put16(f->nz0 + 0x202, color);
    Put16(f->nz0 + 0x204, 31);
    Put16(f->nz0 + 0x206, 0);
    SotnActorGraphics *g = NULL;
    CHECK(Create(f, &g));
    SotnPaletteRedShift shift = {.palettes = {32}, .count = 1};
    CHECK(SotnActorGraphics_ReddenPalette(g, shift));
    SotnTextureQuad q = {.x = {0, 4, 0, 4},
                         .y = {0, 0, 1, 1},
                         .u = {4, 8, 4, 8},
                         .v = {0, 0, 1, 1},
                         .page = 24,
                         .palette = 32};
    ClearFrame(f);
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == color + 1 && f->frame[1] == 31 && f->frame[2] == 1 &&
          f->frame[3] == 0x8000);
    const SotnPaletteRedShift invalid[] = {
        {.palettes = {32, 256}, .count = 2}, {.palettes = {32, 32}, .count = 2}, {.count = 8}};
    for (size_t i = 0; i < SDL_arraysize(invalid); ++i)
    {
        CHECK(!SotnActorGraphics_ReddenPalette(g, invalid[i]));
        CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) &&
              f->frame[0] == color + 1);
    }
    for (Uint16 i = 0; i < 40; ++i)
        CHECK(SotnActorGraphics_ReddenPalette(g, shift));
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) &&
          f->frame[0] == (Uint16)((color & 0xffe0U) | 31U) && f->frame[1] == 31 &&
          f->frame[2] == 31 && f->frame[3] == 0x8000);
    CHECK(f->nz0[0x202] == (Uint8)color && f->nz0[0x203] == (Uint8)(color >> 8));
    CHECK(SotnActorGraphics_ReddenPalette(g, (SotnPaletteRedShift){0}));
    CHECK(!SotnActorGraphics_ReddenPalette(NULL, shift));
    SotnActorGraphics_Destroy(g);
    return true;
}
static bool TestFramebuffer(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    CHECK(Create(f, &g));
    SotnTextureQuad q = {.framebuffer = true,
                         .page = 0x100,
                         .x = {1, 5, 1, 5},
                         .y = {0, 0, 1, 1},
                         .u = {0, 4, 0, 4},
                         .v = {0, 0, 1, 1}};
    ClearFrame(f);
    CHECK(!SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    for (Uint16 i = 0; i < 5; ++i)
        f->frame[i] = (Uint16)(i + 1);
    CHECK(SotnActorGraphics_CaptureFrame(g, f->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnActorGraphics_CaptureFrame(g, f->frame, 1));
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    // Overlapping output samples the immutable snapshot, not earlier writes in this quad.
    CHECK(f->frame[0] == 1 && f->frame[1] == 1 && f->frame[2] == 2 && f->frame[3] == 3 &&
          f->frame[4] == 4);
    q.page = 0x104;
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[3] == 3 && f->frame[4] == 4);
    CHECK(SotnActorGraphics_CaptureFrame(g, f->frame, SOTN_FRAME_PIXELS));
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[2] == 1 && f->frame[3] == 2 && f->frame[4] == 3);
    q.v[0] = q.v[1] = 240;
    q.v[2] = q.v[3] = 241;
    const Uint16 saved = f->frame[4];
    CHECK(!SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[4] == saved);
    q.page = 0x80;
    CHECK(!SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnActorGraphics_CaptureFrame(NULL, f->frame, SOTN_FRAME_PIXELS));
    CHECK(!SotnActorGraphics_CaptureFrame(g, NULL, SOTN_FRAME_PIXELS));
    SotnActorGraphics_Destroy(g);
    return true;
}
static bool TestTargets(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    CHECK(Create(f, &g) && SotnActorGraphics_LoadBank(g, 0));
    const SotnActorTarget target = {.x = 0, .y = 256, .width = 128, .height = 256, .clear = true};
    SotnActorSurface surface = {0};
    CHECK(!SotnActorGraphics_EndTarget(g, true));
    CHECK(SotnActorGraphics_BeginTarget(g, target, &surface));
    CHECK(surface.width == 128 && surface.height == 256 && surface.count == 32768);
    SotnActorSurface rejected = {0};
    CHECK(!SotnActorGraphics_BeginTarget(g, target, &rejected) && !rejected.pixels);
    const SotnSpriteTransform identity = {.scale_x = 256, .scale_y = 256, .opacity = 128};
    const SotnActorSprite sprite = {0x8001, 1, 512, 0, 0, false};
    CHECK(SotnActorGraphics_DrawSurface(g, sprite, 1, 255, identity, surface));
    CHECK(surface.pixels[254 * 128] == 31 && surface.pixels[254 * 128 + 2] == 0x83e0 &&
          surface.pixels[253 * 128] == 0);
    CHECK(surface.pixels[254 * 128 + 4] == 0);
    SotnSpriteTransform wide = identity;
    wide.scale_x = 512;
    CHECK(SotnActorGraphics_DrawSurface(g, sprite, 1, 255, wide, surface));
    CHECK(surface.pixels[254 * 128 + 4] != 0);
    SotnTextureQuad q = {
        .untextured = true, .red = 248, .x = {4, 6, 4, 6}, .y = {255, 255, 256, 256}};
    CHECK(SotnActorGraphics_QuadSurface(g, q, surface));
    CHECK(surface.pixels[255 * 128 + 4] == 31 && surface.pixels[255 * 128 + 5] == 31);
    CHECK(SotnActorGraphics_EndTarget(g, true));
    ClearFrame(f);
    q = (SotnTextureQuad){.page = 0x110,
                          .x = {0, 2, 0, 2},
                          .y = {0, 0, 1, 1},
                          .u = {4, 6, 4, 6},
                          .v = {255, 255, 256, 256}};
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 31 && f->frame[1] == 31); // Includes the final original VRAM row.
    SotnActorTarget retained = target;
    retained.clear = false;
    CHECK(SotnActorGraphics_BeginTarget(g, retained, &surface) &&
          surface.pixels[255 * 128 + 4] == 31);
    surface.pixels[255 * 128 + 4] = 15;
    CHECK(SotnActorGraphics_EndTarget(g, false));
    CHECK(SotnActorGraphics_BeginTarget(g, retained, &surface) &&
          surface.pixels[255 * 128 + 4] == 31);
    CHECK(SotnActorGraphics_EndTarget(g, false));
    retained.x = 1024;
    CHECK(!SotnActorGraphics_BeginTarget(g, retained, &surface));
    retained = target;
    retained.y = 500;
    CHECK(!SotnActorGraphics_BeginTarget(g, retained, &surface));
    retained = target;
    retained.width = 257;
    CHECK(!SotnActorGraphics_BeginTarget(g, retained, &surface));
    CHECK(!SotnActorGraphics_BeginTarget(NULL, target, &surface));
    CHECK(!SotnActorGraphics_BeginTarget(g, target, NULL));
    CHECK(!SotnActorGraphics_DrawSurface(g, sprite, 0, 0, identity,
                                         (SotnActorSurface){f->frame, 1, 128, 256}));
    CHECK(!SotnActorGraphics_QuadSurface(g, q, (SotnActorSurface){f->frame, 1, 128, 256}));
    SotnActorGraphics_Destroy(g);
    return true;
}
static bool TestTargetSequence(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    CHECK(Create(f, &g));
    const SotnActorSurface main = {f->frame, SOTN_FRAME_PIXELS, 256, 240};
    SotnActorTargetCursor cursor = {.surface = main};
    SotnActorPrimitive begin = {.target = 1,
                                .clip_right = 64,
                                .clip_top = 256,
                                .clip_bottom = 512,
                                .target_offset_y = 256,
                                .target_clear = true};
    CHECK(!SotnActorGraphics_ApplyTarget(g, main, (SotnActorPrimitive){.target = 2}, &cursor));
    CHECK(SotnActorGraphics_ApplyTarget(g, main, begin, &cursor) && cursor.mode == 1 &&
          cursor.surface.width == 64 && cursor.surface.height == 256 && !cursor.offset_y);
    cursor.surface.pixels[255 * 64] = 31;
    CHECK(!SotnActorGraphics_ApplyTarget(g, main, begin, &cursor) && cursor.mode == 1 &&
          cursor.surface.pixels[255 * 64] == 31);
    CHECK(!SotnActorGraphics_ApplyTarget(g, main, (SotnActorPrimitive){.target = 4}, &cursor));
    CHECK(SotnActorGraphics_ApplyTarget(g, main, (SotnActorPrimitive){.target = 2}, &cursor) &&
          !cursor.mode && cursor.surface.pixels == main.pixels);
    begin.target = 3;
    CHECK(SotnActorGraphics_ApplyTarget(g, main, begin, &cursor));
    cursor.surface.pixels[0] = 31;
    begin.clip_left = 64;
    begin.clip_right = 128;
    begin.target_offset_x = 64;
    CHECK(SotnActorGraphics_ApplyTarget(g, main, begin, &cursor) && cursor.mode == 3 &&
          !cursor.offset_x && !cursor.surface.pixels[0]);
    cursor.surface.pixels[0] = 0x3e0;
    CHECK(!SotnActorGraphics_ApplyTarget(g, main, (SotnActorPrimitive){.target = 2}, &cursor));
    CHECK(SotnActorGraphics_ApplyTarget(g, main, (SotnActorPrimitive){.target = 4}, &cursor) &&
          !cursor.mode && cursor.surface.pixels == main.pixels);
    CHECK(SotnActorGraphics_ApplyTarget(g, main, (SotnActorPrimitive){.target = 4}, &cursor));
    ClearFrame(f);
    SotnTextureQuad q = {
        .page = 0x110, .x = {0, 1, 0, 1}, .y = {0, 0, 1, 1}, .u = {0, 1, 0, 1}, .v = {0, 0, 1, 1}};
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    q.u[0] = q.u[2] = 64;
    q.u[1] = q.u[3] = 65;
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 0x3e0);
    begin.clip_right = 129;
    begin.clip_bottom = 513;
    CHECK(!SotnActorGraphics_ApplyTarget(g, main, begin, &cursor) && !cursor.mode);
    CHECK(!SotnActorGraphics_ApplyTarget(NULL, main, begin, &cursor));
    CHECK(!SotnActorGraphics_ApplyTarget(g, (SotnActorSurface){0}, begin, &cursor));
    SotnActorGraphics_Destroy(g);
    return true;
}

static bool ClearedBo0Atlas(Fixture *f)
{
    MakeFixture(f);
    SotnActorGraphics *g = NULL;
    const SotnTileSources sources = {{f->stage, sizeof(f->stage)}, {f->common, sizeof(f->common)}};
    CHECK(SotnActorGraphics_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                        (SotnDataView){f->nz0, sizeof(f->nz0)}, sources, 30, &g));
    ClearFrame(f);
    f->frame[0] = 31;
    SotnTextureQuad q = {.page = 0x15,
                         .palette = 0x200,
                         .x = {0, 1, 0, 1},
                         .y = {0, 0, 1, 1},
                         .u = {128, 129, 128, 129},
                         .v = {129, 129, 130, 130}};
    CHECK(SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    q.page = 0x10;
    q.u[0] = q.u[1] = q.u[2] = q.u[3] = 0;
    CHECK(!SotnActorGraphics_Quad(g, q, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 31);
    SotnActorGraphics_Destroy(g);
    return true;
}
int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed =
        TestActorOrder() && StageBankBounds(fixture) && TestRopeCache(fixture) &&
        TestTransformFlags(fixture) && TestRotation(fixture) && TestColorQuad(fixture) &&
        TestQuad(fixture) && TestSprites(fixture) && TestFailures(fixture) &&
        TestDepthsAndOrdering(fixture) && TestWeapons(fixture) && TestDialogue(fixture) &&
        DefaultBo4Bank(fixture) && PackedSheets(fixture) && TestColorLine(fixture) &&
        TestPaletteBlend(fixture) && TestPaletteRedShift(fixture) && TestFramebuffer(fixture) &&
        TestTargets(fixture) && TestTargetSequence(fixture) && ClearedBo0Atlas(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_ACTOR_GRAPHICS_TEST: PASS");
    return 0;
}
