#include "player_internal.h"
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
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Casting(f) && FrozenPlayer(f) && FreezeEdges();
    SotnPlayer_Close(f->p);
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_PLAYER_SPIRIT_TEST: PASS");
    return ok ? 0 : 1;
}
