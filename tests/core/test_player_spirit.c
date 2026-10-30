#include "player_internal.h"
#include <sotn/bat_runtime.h>
#include <sotn/scene_freeze.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
typedef struct Fixture
{
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnBatRuntime runtime;
    SotnCollisionMap map;
} Fixture;
static bool Create(Fixture *f, bool enabled)
{
    SotnPlayer_Close(f->p);
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
        Put32(f->dra + 0x200 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    }
    Put32(f->dra + 0x118, 0);
    Put32(f->dra + 0x218, 65535);
    Put32(f->dra + 0x101b8 + 0xba * 4U, 0x800a0200);
    SDL_memset(f->dra + 0xcf18, 3, 20);
    SDL_memset(f->dra + 0xcf2c, 5, 16);
    SDL_memset(f->dra + 0xcf3c, 28, 16);
    f->dra[0x8410 + 12] = 10;
    /* Authored ordinary unarmed fallback for an unaffordable/disabled command. */
    Put32(f->dra + 0xc0, 0x800a0800);
    f->dra[0x812] = 0x24;
    f->dra[0x815] = 5;
    f->dra[0x81a] = 8;
    f->dra[0x828] = 2;
    Put32(f->dra + 0xd53c, 0x800a0900);
    Put32(f->dra + 0xd540, 0x800a0a00);
    f->dra[0xd549] = 1;
    for (Uint16 i = 0; i < 7; ++i)
        Put32(f->dra + 0x900 + i * 4U, 0x800a0b00);
    f->dra[0xa08] = 18;
    f->dra[0xa09] = 240;
    f->dra[0xa0a] = 10;
    f->dra[0xa0b] = 7;
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 32, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 128, 135, &f->p));
    CHECK(SotnPlayer_SetSpiritContext(f->p, enabled));
    f->dra[0x8410 + 28 + 12] = 5;
    f->dra[0x8410 + 84 + 12] = 20;
    Put32(f->dra + 0x101b8 + 0xf0 * 4U, 0x800a0200);
    Put32(f->dra + 0x101b8 + 0xf1 * 4U, 0x800a0200);
    return true;
}

static bool Command(Fixture *f, bool face, bool tetra)
{
    const Uint16 back = face ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT,
                 forward = face ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    f->p->facing_left = face;
    if (tetra)
    {
        for (Uint16 i = 0; i < 33; ++i)
            CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
        CHECK(SotnPlayer_Tick(f->p, (Uint16)(SOTN_BUTTON_UP | forward)));
        CHECK(SotnPlayer_Tick(f->p, forward));
        CHECK(SotnPlayer_Tick(f->p, (Uint16)(SOTN_BUTTON_DOWN | forward)));
    }
    else
    {
        CHECK(SotnPlayer_Tick(f->p, back));
        CHECK(SotnPlayer_Tick(f->p, 0));
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
    }
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT_HAND));
    return true;
}
static bool Casting(Fixture *f)
{
    for (Uint16 kind = 0; kind < 2; ++kind)
        for (Uint16 face = 0; face < 2; ++face)
        {
            CHECK(Create(f, true));
            const Uint16 cost = kind ? 20 : 5;
            f->p->mp = cost;
            CHECK(Command(f, face != 0, kind != 0));
            SotnPlayerState s = {0};
            CHECK(SotnPlayer_GetState(f->p, &s));
            CHECK(s.step == (kind ? SOTN_PLAYER_TETRA : SOTN_PLAYER_SUMMON) &&
                  s.animation.id == (kind ? 0xf1 : 0xf0) && s.mp_spent == cost && !s.mp &&
                  s.spirit.started && s.spirit.spell == (kind ? 3 : 1) &&
                  s.spirit.blueprint == (kind ? 0x10075U : 117U) && s.spell_use_ticks == 4 &&
                  s.status == 0x38001320U && s.sound == 0x6f1 && !s.extra_sound && !s.velocity_x &&
                  !s.velocity_y && !s.attack.active);
            const SotnPlayerSpirit source = s.spirit;
            CHECK(SotnPlayer_ChangeRoom(f->p, f->map, 16, -16));
            CHECK(f->p->spirit.source.x == source.source.x + 16 * 65536 &&
                  f->p->spirit.source.y == source.source.y - 16 * 65536);
            CHECK(SotnPlayer_ChangeRoom(f->p, f->map, -16, 16));
            Uint16 t = 0;
            while (f->p->step == (kind ? SOTN_PLAYER_TETRA : SOTN_PLAYER_SUMMON) && t < 20)
            {
                CHECK(SotnPlayer_Tick(f->p, 0) && !f->p->spirit.started && !f->p->mp_spent);
                ++t;
            }
            CHECK(t < 20 && f->p->step == SOTN_PLAYER_CROUCH && !f->p->substep &&
                  f->p->animation.id == 17 && !f->p->spell_use_ticks);
            CHECK(Create(f, true));
            f->p->mp = (Uint16)(cost - 1U);
            CHECK(Command(f, face != 0, kind != 0) && !f->p->spirit.started &&
                  f->p->mp == cost - 1U);
            CHECK(Create(f, false) && Command(f, face != 0, kind != 0) && !f->p->spirit.started &&
                  f->p->mp == 20);
        }
    CHECK(Create(f, true));
    f->p->attack_mode = 1;
    f->p->shield_cursor.phase = 1;
    f->p->step = SOTN_PLAYER_CROUCH;
    f->p->spell_commands.spells[1] = (SotnSpellCommand){.phase = 4, .timer = 20};
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT_HAND) && f->p->spirit.started &&
          f->p->attack_mode == 1 && !f->p->shield_cursor.phase);
    for (Uint16 t = 0; t < 20; ++t)
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN));
    CHECK(f->p->step == SOTN_PLAYER_CROUCH && f->p->substep < 0x40);
    for (Uint16 t = 0; t < 20; ++t)
        CHECK(SotnPlayer_Tick(f->p, 0));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && f->p->step == SOTN_PLAYER_WALK);
    CHECK(Create(f, true));
    f->p->step = SOTN_PLAYER_CROUCH;
    f->p->spell_commands.spells[1] = (SotnSpellCommand){.phase = 4, .timer = 1};
    const SotnPlayer before = *f->p;
    Put32(f->dra + 0x101b8 + 0xf0 * 4U, 0x800b1000);
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT_HAND) &&
          !SDL_memcmp(&before, f->p, sizeof(before)));
    CHECK(!SotnPlayer_SetSpiritContext(NULL, true));
    return true;
}
static bool FrozenPlayer(Fixture *f)
{
    const Uint16 ids[] = {0xba, 0xbb, 0xbe, 0xf0, 0xf1, 17, 1, 0xda};
    for (Uint16 i = 0; i < SDL_arraysize(ids); ++i)
    {
        CHECK(Create(f, true));
        Put32(f->dra + 0x101b8 + ids[i] * 4U, 0x800a0200);
        CHECK(SotnAnimation_Set(&f->p->animation, ids[i]));
        f->p->vx = 12345;
        f->p->vy = -67890;
        f->p->dark.ticks = 99;
        f->p->healing.pending = 8;
        f->p->poison_ticks = 40;
        f->p->spell_use_ticks = 4;
        f->p->invincibility = 16;
        f->p->afterimage_timer15 = 4;
        f->p->timers[0] = 10;
        SotnPlayer expected = *f->p;
        const Uint16 buttons = SOTN_BUTTON_RIGHT | SOTN_BUTTON_JUMP;
        expected.previous_buttons = buttons;
        if (i < 5)
            CHECK(SotnAnimation_Tick(expected.dra, &expected.animation));
        CHECK(SotnPlayer_TickSpellFreeze(f->p, buttons, true) &&
              !SDL_memcmp(f->p, &expected, sizeof(expected)));
        expected.previous_buttons = SOTN_BUTTON_START;
        CHECK(SotnPlayer_TickSpellFreeze(f->p, SOTN_BUTTON_START, false) &&
              !SDL_memcmp(f->p, &expected, sizeof(expected)));
        CHECK(SotnPlayer_EndSpellFreeze(f->p) && f->p->invincibility == 48);
        f->p->invincibility = 100;
        CHECK(SotnPlayer_EndSpellFreeze(f->p) && f->p->invincibility == 100);
    }
    CHECK(Create(f, true) && Command(f, false, false));
    CHECK(f->p->spirit.started && f->p->sound && f->p->mp_spent);
    CHECK(SotnPlayer_TickSpellFreeze(f->p, SOTN_BUTTON_LEFT_HAND, true) && !f->p->spirit.started &&
          !f->p->dark.started && !f->p->sound && !f->p->mp_spent);
    const SotnPlayer before = *f->p;
    Put32(f->dra + 0x101b8 + 0xf0 * 4U, 0x800b1000);
    CHECK(!SotnPlayer_TickSpellFreeze(f->p, 0, true) && !SDL_memcmp(f->p, &before, sizeof(before)));
    CHECK(!SotnPlayer_TickSpellFreeze(f->p, 1, true) && !SDL_memcmp(f->p, &before, sizeof(before)));
    CHECK(!SotnPlayer_TickSpellFreeze(NULL, 0, true) && !SotnPlayer_EndSpellFreeze(NULL));
    CHECK(Create(f, true) && SotnPlayer_Tick(f->p, 0));
    CHECK(SotnPlayer_TickSpellFreeze(f->p, SOTN_BUTTON_JUMP, true) &&
          SotnPlayer_TickSpellFreeze(f->p, SOTN_BUTTON_JUMP, false) &&
          SotnPlayer_EndSpellFreeze(f->p));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_STAND);
    CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) &&
          f->p->step == SOTN_PLAYER_JUMP);
    return true;
}
static bool FreezeEdges(void)
{
    SotnSceneFreeze f = {0};
    SotnSceneFreezeFrame frame = SOTN_SCENE_RELEASE;
    bool enter = true;
    CHECK(SotnSceneFreeze_BeginFrame(&f, &frame) && frame == SOTN_SCENE_NORMAL);
    CHECK(SotnSceneFreeze_Request(&f, 3) && !f.active && frame == SOTN_SCENE_NORMAL);
    CHECK(SotnSceneFreeze_EndFrame(&f, &enter) && enter && f.active);
    for (Uint16 i = 0; i < 20; ++i)
    {
        CHECK(SotnSceneFreeze_BeginFrame(&f, &frame) && frame == SOTN_SCENE_FROZEN);
        CHECK(SotnSceneFreeze_EndFrame(&f, &enter) && !enter);
    }
    CHECK(SotnSceneFreeze_Request(&f, 0) && frame == SOTN_SCENE_FROZEN);
    CHECK(SotnSceneFreeze_EndFrame(&f, &enter) && !enter);
    CHECK(SotnSceneFreeze_BeginFrame(&f, &frame) && frame == SOTN_SCENE_RELEASE && !f.active);
    CHECK(SotnSceneFreeze_BeginFrame(&f, &frame) && frame == SOTN_SCENE_NORMAL);
    const SotnSceneFreeze before = f;
    CHECK(!SotnSceneFreeze_Request(&f, 0) && !SDL_memcmp(&f, &before, sizeof(f)));
    CHECK(!SotnSceneFreeze_Request(&f, 1) && !SDL_memcmp(&f, &before, sizeof(f)));
    f.request = 255;
    const SotnSceneFreeze invalid = f;
    frame = SOTN_SCENE_RELEASE;
    enter = true;
    CHECK(!SotnSceneFreeze_BeginFrame(&f, &frame) && frame == SOTN_SCENE_RELEASE &&
          !SDL_memcmp(&f, &invalid, sizeof(f)));
    CHECK(!SotnSceneFreeze_EndFrame(&f, &enter) && enter);
    CHECK(!SotnSceneFreeze_BeginFrame(NULL, &frame));
    return true;
}
static bool HellfireFactory(void *context, SotnPlayerFactorySource source, Uint32 packed,
                            bool *allocated)
{
    return SotnBatRuntime_RequestHellfire(context, source, packed, allocated);
}
static bool HellfireCommand(Fixture *f, bool face)
{
    const Uint16 forward = face ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    f->p->facing_left = face;
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(f->p, (Uint16)(SOTN_BUTTON_DOWN | forward)));
    CHECK(SotnPlayer_Tick(f->p, forward));
    CHECK(SotnPlayer_Tick(f->p, (Uint16)(forward | SOTN_BUTTON_LEFT_HAND)));
    return true;
}
static bool Hellfire(Fixture *f)
{
    for (Uint16 face = 0; face < 2; ++face)
    {
        CHECK(Create(f, true));
        f->dra[0x8410 + 2 * 28 + 12] = 8;
        f->p->mp = 8;
        Put32(f->dra + 0x101b8 + 0x3c * 4, 0x800a0400);
        for (Uint16 i = 0; i < 12; ++i)
            Put32(f->dra + 0x400 + i * 4U, 1U | ((Uint32)i + 1U) << 16);
        Put32(f->dra + 0x430, 65535);
        CHECK(SotnPlayer_SetHellfireContext(f->p, HellfireFactory, &f->runtime, 0, 0, 0));
        CHECK(HellfireCommand(f, face != 0));
        SotnPlayerState state = {0};
        CHECK(SotnPlayer_GetState(f->p, &state) && state.hellfire_started && state.step == 34 &&
              state.substep == 1 && state.mp_spent == 8 && !state.mp && state.invulnerable &&
              (state.status & 0x38000200U) == 0x38000200U &&
              f->runtime.factories[0].cursor.blueprint == 33);
        CHECK(SotnPlayer_Tick(f->p, 0) && f->p->substep == 1 && !f->p->hellfire_started &&
              !f->p->mp_spent);
        CHECK(SotnPlayer_CommitHellfire(f->p, 1, 0x810d) && SotnPlayer_Tick(f->p, 0) &&
              f->p->substep == 2 && SotnPlayer_GetState(f->p, &state) && state.hellfire_hidden);
        CHECK(SotnPlayer_CommitHellfire(f->p, 2, 0x8100) && SotnPlayer_Tick(f->p, 0) &&
              f->p->substep == 3);
        CHECK(SotnPlayer_CommitHellfire(f->p, 3, 0x8100) && SotnPlayer_Tick(f->p, 0) &&
              f->p->substep == 4);
        f->p->poison_ticks = 1000;
        f->p->curse_ticks = 100;
        bool circle = false, timer = false;
        for (Uint16 t = 0; t < 20 && f->p->step == 34; ++t)
        {
            CHECK(SotnPlayer_Tick(f->p, 0));
            timer |= f->p->spell_use_ticks == 4;
            for (Uint16 i = 0; i < 8; ++i)
                circle |= f->runtime.factories[i].cursor.entity_id &&
                          f->runtime.factories[i].cursor.blueprint == 37;
        }
        CHECK(f->p->step == SOTN_PLAYER_STAND && f->p->animation.id == 0x3d && circle && timer);
        bool poison = false, curse = false;
        for (Uint16 i = 0; i < 8; ++i)
        {
            const SotnPlayerFactory factory = f->runtime.factories[i].cursor;
            poison |= factory.entity_id && factory.blueprint == 44 && factory.parameter == 0x1600;
            curse |= factory.entity_id && factory.blueprint == 44 && factory.parameter == 0x1700;
        }
        CHECK(poison && curse);
    }
    CHECK(Create(f, true));
    f->dra[0x8410 + 2 * 28 + 12] = 8;
    f->p->mp = 8;
    CHECK(SotnPlayer_SetHellfireContext(f->p, HellfireFactory, &f->runtime, 0, 0, 0));
    for (Uint16 i = 0; i < 8; ++i)
    {
        bool allocated = false;
        CHECK(SotnBatRuntime_RequestHellfire(
                  &f->runtime, (SotnPlayerFactorySource){.priority = 148}, 33, &allocated) &&
              allocated);
    }
    CHECK(HellfireCommand(f, false) && f->p->step == SOTN_PLAYER_STAND && f->p->substep == 1 &&
          f->p->hellfire_started && f->p->mp_spent == 8 && !f->p->mp);
    CHECK(Create(f, true));
    f->dra[0x8410 + 2 * 28 + 12] = 8;
    f->p->mp = 7;
    CHECK(SotnPlayer_SetHellfireContext(f->p, HellfireFactory, &f->runtime, 0, 0, 0) &&
          HellfireCommand(f, false) && f->p->step != 34 && !f->p->hellfire_started &&
          !f->p->mp_spent && f->p->mp == 7 && !f->runtime.factories[0].cursor.entity_id);
    CHECK(!SotnPlayer_SetHellfireContext(f->p, HellfireFactory, NULL, 0, 0, 0));
    CHECK(!SotnPlayer_SetHellfireContext(f->p, NULL, NULL, 0, 1, 0));
    CHECK(!SotnPlayer_CommitHellfire(f->p, 4, 0));
    CHECK(!SotnPlayer_CommitHellfire(f->p, 0, 0x8123));
    return true;
}
static bool SoulCommand(Fixture *f, bool face)
{
    const Uint16 back = face ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT,
                 forward = face ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    f->p->facing_left = face;
    const Uint16 sequence[] = {back,
                               forward,
                               (Uint16)(SOTN_BUTTON_DOWN | forward),
                               SOTN_BUTTON_DOWN,
                               (Uint16)(SOTN_BUTTON_DOWN | back),
                               back,
                               forward,
                               (Uint16)(forward | SOTN_BUTTON_LEFT_HAND)};
    for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
        CHECK(SotnPlayer_Tick(f->p, sequence[i]));
    return true;
}
static bool Soul(Fixture *f)
{
    CHECK(SOTN_PLAYER_SOUL == 37);
    for (Uint16 face = 0; face < 2; ++face)
    {
        CHECK(Create(f, true) && SotnPlayer_SetSoulContext(f->p, true));
        f->dra[0x8410 + 5 * 28 + 12] = 6;
        Put32(f->dra + 0x101b8 + 0xda * 4U, 0x800a0300);
        for (Uint16 i = 0; i < 8; ++i)
            Put32(f->dra + 0x300 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
        Put32(f->dra + 0x320, 65535);
        f->p->mp = 6;
        CHECK(SoulCommand(f, face != 0) && f->p->step == SOTN_PLAYER_SOUL &&
              f->p->animation.id == 0xda && f->p->soul.started && !f->p->soul.attack &&
              f->p->mp_spent == 6 && !f->p->mp && f->p->sound == 0x6f4 &&
              f->p->spell_use_ticks == 4 && f->p->invincibility == 16);
        SotnPlayerState state = {0};
        CHECK(SotnPlayer_GetState(f->p, &state) && state.soul.started &&
              (state.status & 0x38000200U) == 0x38000200U && !(state.status & 0x20U));
        CHECK(!SotnPlayer_SetSoulContext(f->p, false));
        const SotnPlayerSoul source = f->p->soul;
        CHECK(SotnPlayer_ChangeRoom(f->p, f->map, 16, 0) &&
              f->p->soul.source.x == source.source.x + 16 * 65536 && f->p->mp_spent == 6);
        Uint16 attacks = 0;
        for (Uint16 t = 0; t < 24; ++t)
        {
            CHECK(SotnPlayer_Tick(f->p, 0) && !f->p->mp_spent && !f->p->soul.started);
            if (f->p->soul.attack)
            {
                ++attacks;
                CHECK(f->p->sound == 0x67d && f->p->soul.source.priority == 148);
            }
        }
        CHECK(attacks == 1 && f->p->step == SOTN_PLAYER_STAND &&
              SotnPlayer_SetSoulContext(f->p, false));
    }
    CHECK(Create(f, true));
    f->dra[0x8410 + 5 * 28 + 12] = 6;
    f->p->mp = 6;
    CHECK(SoulCommand(f, false) && f->p->step != SOTN_PLAYER_SOUL && !f->p->soul.started &&
          !f->p->mp_spent && f->p->mp == 6);
    CHECK(Create(f, true) && SotnPlayer_SetSoulContext(f->p, true));
    f->dra[0x8410 + 5 * 28 + 12] = 6;
    f->p->mp = 5;
    CHECK(SoulCommand(f, false) && f->p->step != SOTN_PLAYER_SOUL && !f->p->soul.started &&
          !f->p->mp_spent && f->p->mp == 5);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Casting(f) && FrozenPlayer(f) && FreezeEdges() && Hellfire(f) && Soul(f);
    SotnPlayer_Close(f->p);
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_PLAYER_SPIRIT_TEST: PASS");
    return ok ? 0 : 1;
}
