#include <SDL3/SDL.h>
#include <sotn/actor_graphics.h>
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
    CHECK(SotnActorGraphics_DrawTransform(g, sprite, 20, 20,
                                          (SotnSpriteTransform){1024, 256, 256, 64}, fixture->frame,
                                          SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 20] == 15);
    CHECK(fixture->frame[21 * SOTN_FRAME_WIDTH + 20] == 0x81e0);
    ClearFrame(fixture);
    CHECK(SotnActorGraphics_DrawTransform(g, sprite, 20, 20,
                                          (SotnSpriteTransform){0, 512, 256, 128}, fixture->frame,
                                          SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 18] == 31 &&
          fixture->frame[19 * SOTN_FRAME_WIDTH + 19] == 31);
    const Uint16 preserved = fixture->frame[19 * SOTN_FRAME_WIDTH + 18];
    CHECK(!SotnActorGraphics_DrawTransform(g, sprite, 20, 20, (SotnSpriteTransform){0, 0, 256, 128},
                                           fixture->frame, SOTN_FRAME_PIXELS));
    CHECK(fixture->frame[19 * SOTN_FRAME_WIDTH + 18] == preserved);
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
    CHECK(!SotnActorGraphics_CreateStage(dra, overlay, art, 2, &g) && g == NULL);
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
    sprite.blend_mode = 0x30;
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = 0x7c00;
    CHECK(SotnActorGraphics_Draw(g, sprite, 10, 10, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[11 * 256 + 9] == 31 && f->frame[11 * 256 + 10] == 0x7c00 &&
          f->frame[11 * 256 + 11] == 0x7fe0);
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

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = StageBankBounds(fixture) && TestRopeCache(fixture) &&
                        TestRotation(fixture) && TestQuad(fixture) && TestSprites(fixture) &&
                        TestFailures(fixture) && TestDepthsAndOrdering(fixture) &&
                        TestWeapons(fixture) && TestDialogue(fixture) && PackedSheets(fixture) &&
                        TestColorLine(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_ACTOR_GRAPHICS_TEST: PASS");
    return 0;
}
