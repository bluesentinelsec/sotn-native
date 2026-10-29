#include "character_fixture.h"
#include <sotn/bat_runtime.h>
#include <sotn/combat.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x11000];
    SotnCharacter *c;
    SotnBatRuntime r;
    SotnBatRuntimeInput in;
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void BP(Fixture *f, Uint16 id, Uint8 child, Uint8 amount, Uint8 kind)
{
    Uint8 *p = f->dra + 0xd1d4 + id * 6U;
    p[0] = child;
    p[1] = amount;
    p[2] = (Uint8)(amount | 64);
    p[3] = 1;
    p[4] = kind;
}
static void Anim(Fixture *f, Uint32 at, Uint16 count, bool stop)
{
    for (Uint16 i = 0; i < count - 1; ++i)
    {
        Put16(f->dra + at + i * 4U, 1);
        Put16(f->dra + at + i * 4U + 2U, (Uint16)(i + 1));
    }
    Put16(f->dra + at + (count - 1U) * 4U, stop ? 0xffff : 0);
    Put16(f->dra + at + (count - 1U) * 4U + 2U, 0);
}
static bool Create(Fixture *f)
{
    SotnCharacter_Destroy(f->c);
    SDL_zero(*f);
    TestCharacter_Fill(f->dra);
    BP(f, 33, 25, 1, 0);
    BP(f, 34, 26, 3, 1);
    BP(f, 35, 27, 2, 1);
    BP(f, 36, 24, 1, 0);
    BP(f, 37, 28, 1, 0);
    BP(f, 38, 29, 1, 0);
    BP(f, 4, 5, 1, 0);
    f->dra[0xd4b8] = 48;
    f->dra[0xd4b9] = 63;
    f->dra[0xd4ba] = 17;
    f->dra[0xd4bb] = 47;
    Anim(f, 0x10798, 9, false);
    Anim(f, 0x107c8, 26, false);
    Anim(f, 0xd5fc, 13, true);
    Uint8 *s = f->dra + 0x8410 + 2 * 28;
    s[16] = 2;
    s[20] = 0;
    s[22] = 16;
    s[24] = 25;
    for (Uint16 i = 0; i < 3; ++i)
        TestCharacter_Put32(f->dra + 0x10830 + i * 4U, 32768);
    TestCharacter_Put32(f->dra + 0xdb98 + 10 * 4, 0x800a0300);
    Uint8 *p = f->dra + 0x300;
    p[0] = 2;
    p[1] = 60;
    p[2] = 40;
    p[3] = 20;
    p[4] = p[5] = 2;
    Put16(p + 8, 0x33);
    Put16(p + 10, 10);
    TestCharacter_Put32(p + 12, 0x08800000);
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->c));
    f->in = (SotnBatRuntimeInput){.dra = {f->dra, sizeof(f->dra)},
                                  .player = {.x = 128 * 65536 + 123,
                                             .y = 140 * 65536 + 456,
                                             .priority = 148,
                                             .step = SOTN_PLAYER_HELLFIRE}};
    return true;
}
static bool Tick(Fixture *f)
{
    CHECK(SotnBatRuntime_Tick(&f->r, f->c, f->in));
    ++f->in.game_timer;
    return true;
}
static bool Lifetimes(Fixture *f)
{
    for (Uint16 big = 0; big < 2; ++big)
    {
        CHECK(Create(f));
        f->in.hellfire_up = big != 0;
        bool allocated = false;
        CHECK(SotnBatRuntime_RequestHellfire(&f->r,
                                             (SotnPlayerFactorySource){.x = f->in.player.x,
                                                                       .y = f->in.player.y,
                                                                       .priority = 148},
                                             33, &allocated) &&
              allocated);
        CHECK(Tick(f) && f->r.children[47].kind == 25 && f->r.children[47].primitive_count == 1 &&
              f->r.hellfire_upload && f->r.hellfire_signal == 0 && f->r.sound_count == 1 &&
              f->r.sounds[0] == 0x636);
        const Uint32 root_generation = f->r.children[47].generation;
        Uint16 hidden = 0, appear = 0, projectiles = 0, trails = 0;
        bool signal1 = false, signal2 = false, signal3 = false;
        for (Uint16 t = 1; t < 180; ++t)
        {
            CHECK(Tick(f));
            hidden += (Uint16)(f->r.hellfire_palette == 0x810d);
            appear += (Uint16)(f->r.hellfire_palette == 0x8100);
            signal1 |= f->r.hellfire_signal == 1;
            signal2 |= f->r.hellfire_signal == 2;
            signal3 |= f->r.hellfire_signal == 3;
            CHECK(!f->r.hellfire_upload);
            for (Uint16 k = 0; k < 8; ++k)
            {
                const SotnPlayerFactory *factory = &f->r.factories[k].cursor;
                if (!factory->entity_id)
                    continue;
                if (factory->blueprint == 38 || factory->blueprint == 34 ||
                    factory->blueprint == 35 || factory->blueprint == 4)
                    CHECK(factory->source.priority == 448);
                if (factory->blueprint == 36)
                    CHECK(factory->source.priority == 150);
            }
            for (Uint16 i = 0; i < 48; ++i)
            {
                const SotnBatRuntimeChild *c = &f->r.children[i];
                if (c->kind == 26 || c->kind == 27)
                {
                    ++projectiles;
                    CHECK(c->initialized && c->group >= 3 && c->group <= 10 &&
                          c->state.hellfire_fire.attack.power >= 25 &&
                          c->state.hellfire_fire.attack.width == (big ? 8 : 4) &&
                          c->state.hellfire_fire.frame && f->r.hellfire_mask & ((Uint64)1 << i));
                }
                if (c->kind == 24)
                    ++trails;
                if (c->kind == 25 || c->kind == 28 || c->kind == 29)
                {
                    const SotnPlayerEffectQuad *q = NULL;
                    Uint16 n = 0;
                    CHECK(SotnBatRuntime_Visual(&f->r, i, &q, &n) && n == 1 && q != NULL);
                }
            }
        }
        CHECK(hidden && appear && signal1 && signal2 && signal3 && projectiles);
        if (big)
            CHECK(trails);
        CHECK(!f->r.children[47].kind || f->r.children[47].generation != root_generation);
        f->in.player.step = SOTN_PLAYER_STAND;
        CHECK(Tick(f));
    }
    return true;
}
static bool Failures(Fixture *f)
{
    CHECK(Create(f));
    bool allocated = false;
    const SotnPlayerFactorySource source = {.x = 128 * 65536, .y = 140 * 65536, .priority = 148};
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnBatRuntime_RequestHellfire(&f->r, source, 33, &allocated) && allocated);
    CHECK(SotnBatRuntime_RequestHellfire(&f->r, source, 33, &allocated) && !allocated);
    CHECK(!SotnBatRuntime_RequestHellfire(&f->r, source, 34, &allocated));
    CHECK(Create(f));
    CHECK(SotnBatRuntime_RequestHellfire(&f->r, source, 33, &allocated) && allocated);
    SDL_memset(f->in.primitive_slots.occupied, 255, sizeof(f->in.primitive_slots.occupied));
    SotnCharacterState before = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->c, &before, NULL));
    CHECK(Tick(f) && f->r.hellfire_signal == 65535 && !SotnBatRuntime_Occupied(&f->r) &&
          !f->r.hellfire_upload && SotnCharacter_Get(f->c, &after, NULL) &&
          before.random == after.random);
    CHECK(Create(f));
    CHECK(SotnBatRuntime_RequestHellfire(&f->r, source, 33, &allocated) && Tick(f));
    const SotnHellfireBeam beam = f->r.children[47].state.hellfire_beam;
    f->in.freeze = 3;
    CHECK(Tick(f) && !SDL_memcmp(&beam, &f->r.children[47].state.hellfire_beam, sizeof(beam)));
    f->in.freeze = 0;
    f->in.player.step = SOTN_PLAYER_STAND;
    CHECK(Tick(f) && !f->r.children[47].kind);
    CHECK(Create(f));
    CHECK(SotnBatRuntime_RequestHellfire(&f->r, source, 37, &allocated) && Tick(f));
    CHECK(f->r.children[47].kind == 28);
    f->in.entering = true;
    f->in.screen_shift_x = -3;
    f->in.screen_shift_y = 4;
    const SotnHellfireEffect circle = f->r.children[47].state.hellfire_effect;
    CHECK(Tick(f) && f->r.children[47].state.hellfire_effect.width == circle.width &&
          f->r.children[47].state.hellfire_effect.quad.x[0] == circle.quad.x[0] - 3 &&
          f->r.children[47].state.hellfire_effect.quad.y[0] == circle.quad.y[0] + 4);
    return true;
}
static bool Feedback(Fixture *f)
{
    for (Uint16 big = 0; big < 2; ++big)
    {
        CHECK(Create(f));
        CHECK(SotnPlayerFactory_Begin(&f->r.factories[0].cursor,
                                      (SotnPlayerFactorySource){.x = f->in.player.x,
                                                                .y = f->in.player.y,
                                                                .priority = 148},
                                      big ? 35 : 34, 0) &&
              Tick(f));
        const Uint32 generation = f->r.children[1].generation;
        CHECK(generation && f->r.children[1].kind == (big ? 27 : 26));
        CHECK(!SotnBatRuntime_Hit(&f->r, 1, generation + 1U, 2));
        CHECK(SotnBatRuntime_Hit(&f->r, 1, generation, 2) && Tick(f));
        CHECK(big ? f->r.children[1].generation == generation && f->r.children[1].kind == 27
                  : !f->r.children[1].kind);
    }
    CHECK(Create(f));
    CHECK(SotnPlayerFactory_Begin(
              &f->r.factories[0].cursor,
              (SotnPlayerFactorySource){.x = f->in.player.x, .y = f->in.player.y, .priority = 148},
              35, 0) &&
          Tick(f));
    f->r.children[1].state.hellfire_fire.pose = 23;
    f->r.children[1].state.hellfire_fire.remaining = 1;
    f->in.game_timer = 2;
    SotnCharacterState rng_profile = {0};
    CHECK(SotnCharacter_Get(f->c, &rng_profile, NULL));
    Uint32 seed = 0, trial = 0;
    do
    {
        trial = ++seed;
    } while (!(SotnCombat_Random(&trial) & 1U));
    rng_profile.random = seed;
    CHECK(SotnCharacter_Restore(f->c, &rng_profile));
    const Sint32 previous_x = f->r.children[1].source.x;
    CHECK(Tick(f) && f->r.factories[0].cursor.blueprint == 36 &&
          f->r.factories[0].cursor.source.x == previous_x &&
          f->r.children[1].source.x != previous_x);
    CHECK(Create(f));
    CHECK(SotnPlayerFactory_Begin(
        &f->r.factories[0].cursor,
        (SotnPlayerFactorySource){.x = f->in.player.x, .y = f->in.player.y, .priority = 148}, 34,
        0));
    Put16(f->dra + 0x10798, 0xfffe);
    SotnBatRuntime *before = SDL_malloc(sizeof(*before));
    CHECK(before != NULL);
    *before = f->r;
    SotnCharacterState profile = {0}, after = {0};
    CHECK(SotnCharacter_Get(f->c, &profile, NULL));
    CHECK(!SotnBatRuntime_Tick(&f->r, f->c, f->in) && !SDL_memcmp(before, &f->r, sizeof(*before)) &&
          SotnCharacter_Get(f->c, &after, NULL) && after.random == profile.random);
    SDL_free(before);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Lifetimes(f) && Failures(f) && Feedback(f);
    SotnCharacter_Destroy(f->c);
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_HELLFIRE_RUNTIME_TEST: PASS");
    return ok ? 0 : 1;
}
