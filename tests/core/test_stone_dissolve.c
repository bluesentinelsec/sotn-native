#include "actors_internal.h"
#include "character_fixture.h"
#include <SDL3/SDL.h>
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#include <sotn/stone_dissolve.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0xe180];
    SotnDeathImage resident, displayed[2];
    Uint16 copies, uploads[2];
    bool fail_capture, fail_upload;
} Fixture;
static bool Capture(void *context, SotnDeathImage *out)
{
    Fixture *f = context;
    if (f->fail_capture)
        return SDL_SetError("supplied capture failure");
    *out = f->resident;
    ++f->copies;
    return true;
}
static bool Upload(void *context, const SotnDeathImage *image, bool secondary)
{
    Fixture *f = context;
    if (f->fail_upload)
        return SDL_SetError("supplied upload failure");
    f->displayed[secondary] = *image;
    ++f->uploads[secondary];
    return true;
}
static SotnStoneDissolveInput Input(Fixture *f)
{
    for (Uint16 i = 0; i < 64; ++i)
        f->dra[0xe140 + i] = (Uint8)(i * 35U);
    f->resident.captured = true;
    f->resident.remaining = 64;
    SDL_memset(f->resident.pixels, 0xff, sizeof(f->resident.pixels));
    return (SotnStoneDissolveInput){.dra = {f->dra, sizeof(f->dra)},
                                    .image = {24, 60, -12, -30},
                                    .player_x = 100 * 65536 + 123,
                                    .player_y = 140 * 65536 + 456,
                                    .player_animation = 0x38,
                                    .player_sprite = 1,
                                    .player_step = 11,
                                    .palette = 0x819e,
                                    .priority = 148,
                                    .capture = Capture,
                                    .upload = Upload,
                                    .context = f};
}
static bool Lifecycle(void)
{
    Fixture f = {0};
    SotnStoneDissolveInput in = Input(&f);
    SotnStoneDissolve n = {.x = 45 * 65536 + 900, .y = 50 * 65536 + 800};
    SotnDeathImage image = {0};
    SotnStoneDissolveEvents e = {0};
    Uint32 rng = 17, expected = 17;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && !n.phase && rng == 17 &&
          n.x == 100 * 65536 + 900 && n.y == 140 * 65536 + 800 && !e.reset_afterimages &&
          !e.frame_load);
    in.allocated = true;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.phase == 1 &&
          n.flags == 0x08800000U && !e.frame_load && e.reset_afterimages && !f.copies);
    for (Uint16 i = 0; i < 37; ++i)
        SotnCombat_Random(&expected);
    CHECK(rng == expected && n.quads[0].x[0] == 88 && n.quads[35].x[3] == 112 &&
          n.quads[0].y[0] == 110 && n.quads[35].y[3] == 170 && n.quads[0].u[0] == 132 &&
          n.quads[35].u[3] == 156 && n.quads[0].v[0] == 1 && n.quads[35].v[3] == 61 &&
          n.quads[0].palette == 0x19e && n.quads[0].draw_mode == 0x308);
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.phase == 2 && e.frame_load &&
          e.reset_afterimages && f.copies == 1 && image.captured);
    const SotnStoneDissolve before = n;
    CHECK(SotnStoneDissolve_Rebase(&n, -17, 5, false) && n.x == before.x - 17 * 65536 &&
          n.quads[0].x[0] == before.quads[0].x[0]);
    CHECK(SotnStoneDissolve_Rebase(&n, 17, -5, false));
    in.tapped = 0x1000;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.phase == 3 && rng == expected);
    in.tapped = 0;
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && e.frame_load);
    for (Uint16 i = 0; i < 320; ++i)
        SotnCombat_Random(&expected);
    CHECK(n.phase == 2 && !n.index && f.uploads[0] == 8 && rng == expected &&
          SDL_memcmp(image.pixels, f.resident.pixels, sizeof(image.pixels)));
    in.player_animation = 0x3b;
    in.engine_timer = 0;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.phase == 4 && n.timer == 48 &&
          !e.frame_load && e.reset_afterimages && f.uploads[1] == 1 &&
          !(n.quads[0].draw_mode & 8U));
    SotnCombat_Random(&expected);
    CHECK(rng == expected);
    const Uint8 cursor = n.cursor;
    in.engine_timer = 3;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.timer == 48 && n.cursor == cursor &&
          f.uploads[1] == 1 && rng == expected);
    in.engine_timer = 1;
    n.quads[0].green[0] = 0;
    n.quads[1].green[0] = 1;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.timer == 47 &&
          n.quads[0].green[0] == 255 && !n.quads[0].red[0] && n.quads[1].red[0] == 1 &&
          n.quads[1].red[1] == 32 && n.cursor == (Uint8)(cursor + 210U));
    while (!n.removed)
        CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e));
    CHECK(!n.timer && !e.reset_afterimages && !e.frame_load && f.uploads[1] == 49);
    for (Uint16 i = 0; i < sizeof(image.pixels); ++i)
        CHECK(!image.pixels[i]);
    return true;
}
static bool Failure(void)
{
    Fixture f = {0};
    SotnStoneDissolveInput in = Input(&f);
    SotnStoneDissolve n = {0};
    SotnDeathImage image = {0};
    SotnStoneDissolveEvents e = {0};
    Uint32 rng = 17;
    in.allocated = true;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e));
    const SotnStoneDissolve before = n;
    const SotnDeathImage saved = image;
    const Uint32 previous_rng = rng;
    const SotnStoneDissolveEvents events = e;
    f.fail_capture = true;
    CHECK(!SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) &&
          !SDL_memcmp(&n, &before, sizeof(n)) && !SDL_memcmp(&image, &saved, sizeof(image)) &&
          !SDL_memcmp(&e, &events, sizeof(e)) && rng == previous_rng);
    f.fail_capture = false;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e));
    in.tapped = 0x1000;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e));
    const SotnStoneDissolve held = n;
    const SotnDeathImage original = image;
    f.fail_upload = true;
    CHECK(!SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && !SDL_memcmp(&n, &held, sizeof(n)) &&
          !SDL_memcmp(&image, &original, sizeof(image)) && rng == previous_rng);
    f.fail_upload = false;
    in.player_sprite = 0;
    CHECK(SotnStoneDissolve_Tick(&n, &image, in, &rng, &e) && n.removed && !e.reset_afterimages &&
          rng == previous_rng);
    return true;
}
static bool SharedRows(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *actors = SDL_calloc(1, sizeof(*actors));
    SotnBatRuntime *r = SDL_calloc(1, sizeof(*r));
    SotnCharacter *character = NULL;
    CHECK(dra && actors && r);
    TestCharacter_Fill(dra);
    CHECK(SotnCharacter_Create((SotnDataView){dra, 0x11000}, &character));
    Fixture f = {0};
    (void)Input(&f);
    actors->primitives[0].owner = 99;
    r->serial = 1;
    r->children[47] = (SotnBatRuntimeChild){
        .kind = 32, .generation = 1, .source = {.x = 100 * 65536, .y = 140 * 65536}};
    SotnDeathImage image = {0};
    SotnBatRuntimeInput in = {.dra = {dra, 0x11000},
                              .player = {.x = 100 * 65536,
                                         .y = 140 * 65536,
                                         .step = SOTN_PLAYER_STONE,
                                         .priority = 148,
                                         .palette = 0x819e,
                                         .animation = {.id = 0x38, .sprite = 1}},
                              .image = {24, 60, -12, -30},
                              .dissolve_image = &image,
                              .dissolve_capture = Capture,
                              .dissolve_upload = Upload,
                              .dissolve_context = &f};
    for (Uint16 tick = 0; tick < 3; ++tick)
    {
        CHECK(SotnActors_PrimitiveSlots(actors, &in.primitive_slots) &&
              SotnBatRuntime_Tick(r, character, in) && SotnActors_CommitBatPrimitives(actors, r));
        CHECK(r->children[47].primitive_count == 36 && actors->primitives[0].owner == 99);
        for (Uint16 i = 0; i < 36; ++i)
        {
            const Uint16 row = r->children[47].primitives[i];
            CHECK(row && actors->primitives[row].owner == 1 &&
                  actors->primitives[row].ability_kind == 32);
        }
    }
    SDL_zero(*r);
    CHECK(SotnActors_CommitBatPrimitives(actors, r) && actors->primitives[0].owner == 99);
    for (Uint16 i = 1; i < SOTN_PRIMITIVE_SLOTS; ++i)
        CHECK(!actors->primitives[i].owner);
    SotnCharacter_Destroy(character);
    SDL_free(r);
    SDL_free(actors);
    SDL_free(dra);
    return true;
}
int main(void)
{
    if (!Lifecycle() || !Failure() || !SharedRows())
        return 1;
    SDL_Log("SOTN_STONE_DISSOLVE_TEST: PASS");
    return 0;
}
