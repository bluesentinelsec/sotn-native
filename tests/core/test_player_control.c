#include "character_fixture.h"
#include "player_internal.h"
#include <sotn/subweapon.h>

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
    Uint8 dra[0x11000], tiles[64 * 16 * 2], types[2];
    Uint8 *weapons;
    size_t weapon_size;
    SotnCollisionMap map;
    SotnPlayer *p;
} Fixture;

static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}

static bool Create(Fixture *f)
{
    SotnPlayer_Close(f->p);
    f->p = NULL;
    SDL_memset(f->dra, 0, sizeof(f->dra));
    SDL_memset(f->tiles, 0, sizeof(f->tiles));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 16; ++i)
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    Put32(f->dra + 0x140, 0);
    Put32(f->dra + 0xc0, 0x800a0800);
    f->dra[0x812] = 0x24;
    f->dra[0x815] = 5;
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 256, 135, &f->p));
    return true;
}

static bool LedgeGrace(Fixture *f)
{
    CHECK(Create(f));
    // An actual floor edge creates Fall with the walking animation still selected.
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 17; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 0;
    Uint16 ticks = 0;
    while (f->p->step != SOTN_PLAYER_FALL && ++ticks < 40)
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT));
    CHECK(f->p->step == SOTN_PLAYER_FALL && f->p->timers[6] == 8 && f->p->animation.id != 28 &&
          (f->p->movement_flags & 0x10));
    const SotnPlayer at_edge = *f->p;
    for (Uint16 i = 0; i < 7; ++i)
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && f->p->timers[6] == 7 - i &&
              f->p->animation.id != 28);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && !f->p->timers[6] &&
          f->p->animation.id == 28 && !(f->p->movement_flags & 0x10));
    *f->p = at_edge;
    for (Uint16 i = 0; i < 6; ++i)
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_JUMP &&
          f->p->vy == -319488);
    return true;
}

static bool ExpiryRollback(Fixture *f)
{
    CHECK(Create(f));
    f->p->step = SOTN_PLAYER_FALL;
    f->p->y.value = 80 * 65536;
    f->p->contact = 0;
    f->p->animation.id = 7;
    f->p->timers[6] = 1;
    f->p->movement_flags = 0x10;
    Put32(f->dra + 0x101b8 + 28 * 4U, 0x800b1000);
    const SotnPlayer before = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, 0) && !SDL_memcmp(&before, f->p, sizeof(before)));
    Put32(f->dra + 0x101b8 + 28 * 4U, 0x800a0100);
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->animation.id == 28 && !f->p->timers[6] &&
          !(f->p->movement_flags & 0x10));
    CHECK(Create(f));
    f->p->timers[6] = 1;
    f->p->movement_flags = 0x10;
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->animation.id == 4 && (f->p->movement_flags & 0x10));
    return true;
}

static bool Landing(Fixture *f)
{
    for (Uint16 step = 3; step <= 4; ++step)
        for (Uint16 hand = 0; hand < 2; ++hand)
        {
            CHECK(Create(f));
            f->p->step = (SotnPlayerStep)step;
            f->p->substep = 1;
            f->p->vy = 450561;
            f->p->vx = 196608;
            f->p->attack_mode = hand;
            f->p->x.parts.low = 123;
            const Sint32 x = f->p->x.value;
            CHECK(SotnPlayer_Tick(f->p, 0) && f->p->step == SOTN_PLAYER_CROUCH &&
                  f->p->substep == (step == 4 && !hand ? 3 : 4) &&
                  f->p->vx == (step == 4 && !hand ? (196608 - 4096) / 2 : 0) &&
                  !f->p->attack_mode && f->p->sound == 0x647 && f->p->movement_effects.smoke &&
                  f->p->movement_effects.smoke_source.x == x &&
                  f->p->movement_effects.smoke_source.y == 135 * 65536);
        }
    for (Uint16 face = 0; face < 2; ++face)
    {
        CHECK(Create(f));
        f->p->step = SOTN_PLAYER_JUMP;
        f->p->substep = 1;
        f->p->vy = 65536;
        f->p->vx = face ? -180224 : 180224;
        CHECK(SotnPlayer_Tick(f->p, 0) && f->p->step == SOTN_PLAYER_STAND &&
              f->p->animation.id == 14 && f->p->vx == (face ? -176128 : 176128) &&
              f->p->sound == 0x647 && f->p->movement_effects.smoke);
    }
    return true;
}

static bool AttackCancels(Fixture *f)
{
    CHECK(Create(f));
    f->p->substep = 0x40;
    f->p->attack_mode = 0x8001;
    f->p->attack_lock = 5;
    f->p->animation.pose = 5;
    f->p->animation.remaining = 2;
    // Looking up is an unlocked hand cancel, before the attack's last pose.
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP) && f->p->substep == 1 && f->p->animation.id == 0 &&
          !f->p->attack_mode);
    CHECK(Create(f));
    f->p->substep = 0x40;
    f->p->attack_mode = 1;
    // Jump starts first and retains the original unlocked hand word until its owner retires.
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_JUMP &&
          f->p->attack_mode == 1 && !f->p->attack.active);
    CHECK(Create(f));
    f->p->step = SOTN_PLAYER_CROUCH;
    f->p->substep = 0x40;
    f->p->attack_mode = 1;
    f->p->attack_lock = 5;
    f->p->animation.pose = 7;
    f->p->animation.remaining = 2;
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->substep == 2 && f->p->animation.id == 19 &&
          !f->p->attack_mode && f->p->animation.pose == 0);
    CHECK(Create(f));
    f->p->step = SOTN_PLAYER_CROUCH;
    f->p->substep = 0x51;
    f->p->attack_mode = 0x8012;
    f->p->curse_failure = true;
    f->p->animation.remaining = -1;
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->substep == 2 && f->p->animation.id == 19 &&
          !f->p->attack_mode && !f->p->curse_failure);
    return true;
}

static bool AirRecovery(Fixture *f)
{
    for (Uint16 ended = 0; ended < 2; ++ended)
    {
        CHECK(Create(f));
        f->p->step = SOTN_PLAYER_JUMP;
        f->p->substep = 0x51;
        f->p->attack_mode = 0x8012;
        f->p->curse_failure = true;
        f->p->y.value = 80 * 65536;
        f->p->vy = -65536;
        f->p->vx = 65536;
        f->p->contact = 0;
        f->p->movement_flags = 0x100;
        f->p->animation.remaining = ended ? -1 : 2;
        CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && f->p->vx == (ended ? 98304 : 0) &&
              f->p->substep == (ended ? 0 : 0x51));
    }
    return true;
}

static bool WeaponFixture(Fixture *f, Uint16 family)
{
    if (!f->weapons)
    {
        f->weapon_size = 53U * 0x7000;
        f->weapons = SDL_calloc(1, f->weapon_size);
        CHECK(f->weapons);
    }
    SDL_memset(f->weapons, 0, f->weapon_size);
    f->p->weapons = (SotnDataView){f->weapons, f->weapon_size};
    Uint8 *overlay = f->weapons + family * 0x7000U + 0x4000;
    if (family == 52)
    {
        for (Uint16 i = 0; i < 14; ++i)
            Put32(overlay + 0xb84 + i * 4U, 0x8017a100);
        Put32(overlay + 0x100, 0x00010002);
        const size_t definition = 0x800 + 16 * 52;
        f->dra[definition + 15] = 52;
        f->dra[definition + 17] = 128;
        f->dra[definition + 18] = 0x63;
        f->p->equipment[1] = 16;
        f->p->shield_button = SOTN_BUTTON_RIGHT_HAND;
    }
    else
    {
        Put32(overlay + 0x8ec, 0x8017a100);
        Put32(overlay + 0x8f0, 0x8017a300);
        overlay[0x8f8] = 0x6f;
        for (Uint16 i = 0; i < 7; ++i)
            Put32(overlay + 0x100 + i * 4U, 0x8017a200);
        f->p->attack_equipment =
            (SotnEquipment){.id = 18, .kind = 7, .family = 31, .animation = 0x6f};
    }
    return true;
}

static bool HeldHands(Fixture *f)
{
    for (Uint16 crouched = 0; crouched < 2; ++crouched)
    {
        CHECK(Create(f) && WeaponFixture(f, 31));
        f->p->step = crouched ? SOTN_PLAYER_CROUCH : SOTN_PLAYER_STAND;
        f->p->substep = 0x47;
        f->p->attack_mode = 0x8008;
        f->p->attack_lock = 5;
        f->p->attack_button = SOTN_BUTTON_LEFT_HAND;
        f->p->animation.id = (Uint16)(0x6f + (crouched ? 2 : 0));
        f->p->animation.pose = 6;
        f->p->animation.remaining = 1;
        f->p->previous_buttons = SOTN_BUTTON_LEFT_HAND;
        for (Uint16 i = 0; i < 8; ++i)
            CHECK(
                SotnPlayer_Tick(f->p, SOTN_BUTTON_LEFT_HAND | (crouched ? SOTN_BUTTON_DOWN : 0)) &&
                f->p->animation.pose == 6 && f->p->animation.remaining == 1);
        CHECK(SotnPlayer_Tick(f->p, crouched ? SOTN_BUTTON_DOWN : 0) && f->p->animation.pose == 7);
    }
    CHECK(Create(f) && WeaponFixture(f, 52));
    f->p->previous_buttons = SOTN_BUTTON_RIGHT_HAND;
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT_HAND) && f->p->substep == 2 &&
          f->p->animation.id == 0x63 && f->p->shield_cursor.phase == 1);
    // The prior child flag remains authoritative until its later lowering update.
    f->p->step = SOTN_PLAYER_CROUCH;
    f->p->substep = 1;
    f->p->animation = (SotnAnimation){.id = 0x65, .remaining = 2};
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT_HAND) && f->p->substep == 2 &&
          f->p->animation.id == 19 && f->p->animation.pose == 2);
    f->p->step = SOTN_PLAYER_STAND;
    f->p->substep = 2;
    f->p->animation = (SotnAnimation){.id = 10, .remaining = 2};
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->substep == 2 && f->p->animation.id == 10 &&
          f->p->shield_cursor.phase == 3);
    f->p->animation = (SotnAnimation){.id = 0x63, .remaining = 2};
    CHECK(SotnPlayer_Tick(f->p, 0) && f->p->substep == 0 && f->p->animation.id == 4);
    return true;
}

static bool BatCollisionPasses(Fixture *f)
{
    CHECK(Create(f));
    for (Uint16 state = 0; state < 2; ++state)
    {
        f->p->step = state ? SOTN_PLAYER_UNMORPH_BAT : SOTN_PLAYER_BAT;
        f->p->status = 1;
        f->p->x.value = 256 * 65536;
        f->p->y.value = 80 * 65536;
        f->p->contact = 0;
        f->p->vx = -65537;
        f->p->vy = 65535;
        SotnPlayer_Collide(f->p);
        CHECK(!f->p->failed && f->p->vx == -65544 && f->p->vy == 65528 &&
              f->p->x.value == 256 * 65536 - 65544 && f->p->y.value == 80 * 65536 + 65528);
    }
    return true;
}

static bool HeldShieldSubweapon(Fixture *f)
{
    CHECK(Create(f) && WeaponFixture(f, 52));
    Uint8 *bytes = SDL_calloc(1, 0x11000);
    CHECK(bytes);
    TestCharacter_Fill(bytes);
    TestCharacter_Progression(bytes);
    // An authored dagger definition with a real cost and projectile owner.
    Uint8 *definition = bytes + 0x4a00 + 20;
    definition[0] = 7;
    definition[2] = 3;
    definition[6] = 2;
    definition[7] = 4;
    definition[8] = 5;
    definition[10] = 0x5d;
    SotnCharacter *character = NULL;
    SotnSubweapons *subweapons = NULL;
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Create((SotnDataView){bytes, 0x11000}, &character));
    CHECK(SotnCharacter_Get(character, &state, NULL));
    state.subweapon = 1;
    state.hearts = 20;
    CHECK(SotnCharacter_Restore(character, &state));
    CHECK(SotnSubweapons_Create((SotnDataView){bytes, 0x11000}, character, f->map, &subweapons));
    CHECK(SotnPlayer_SetSubweapons(f->p, subweapons));
    f->p->previous_buttons = SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT_HAND;
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT_HAND));
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 20 && !f->p->timers[0] &&
          f->p->shield_cursor.phase == 1);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT_HAND));
    CHECK(SotnCharacter_Get(character, &state, NULL) && state.hearts == 17 && f->p->timers[0] == 4);
    CHECK(SotnPlayer_SetSubweapons(f->p, NULL));
    SotnSubweapons_Destroy(subweapons);
    SotnCharacter_Destroy(character);
    SDL_free(bytes);
    return true;
}

static bool PhysicalInputRebase(Fixture *f)
{
    CHECK(Create(f));
    CHECK(SotnPlayer_SetScriptHold(f->p, true));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->simulated_input && !f->p->buttons);
    CHECK(SotnPlayer_SetScriptHold(f->p, false));
    Put32(f->dra + 0x101b8 + 4 * 4U, 0x800b1000);
    const SotnPlayer before = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && !SDL_memcmp(&before, f->p, sizeof(before)));
    Put32(f->dra + 0x101b8 + 4 * 4U, 0x800a0100);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && !f->p->simulated_input && !f->p->tapped &&
          f->p->held == SOTN_BUTTON_JUMP);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_STAND);
    CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) &&
          f->p->step == SOTN_PLAYER_JUMP);
    CHECK(Create(f));
    SotnPlayer_HoldInput(f->p, 1);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->simulated_input &&
          !f->p->simulated_pad.remaining && !f->p->buttons);
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_STAND &&
          !f->p->tapped && !f->p->simulated_input);
    CHECK(SotnPlayer_Tick(f->p, 0) && SotnPlayer_Tick(f->p, SOTN_BUTTON_JUMP) &&
          f->p->step == SOTN_PLAYER_JUMP);
    return true;
}

int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = LedgeGrace(f) && ExpiryRollback(f) && Landing(f) && AttackCancels(f) &&
                    AirRecovery(f) && HeldHands(f) && BatCollisionPasses(f) &&
                    HeldShieldSubweapon(f) && PhysicalInputRebase(f);
    SotnPlayer_Close(f->p);
    SDL_free(f->weapons);
    SDL_free(f);
    return ok ? 0 : 1;
}
