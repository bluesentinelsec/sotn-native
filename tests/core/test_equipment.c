#include <SDL3/SDL.h>
#include <sotn/equipment.h>
#include <sotn/player.h>

#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #value, SDL_GetError());                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0x11000], weapons[0x1a0000], tiles[32 * 16 * 2], types[2];
} Fixture;

static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}

static SotnCollisionMap MakeFixture(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0xc0, 0x800a0300);
    const Uint16 items[3] = {0, 86, 128};
    for (Uint32 i = 0; i < 3; ++i)
    {
        Uint8 *e = f->dra + 0x300 + (size_t)items[i] * 52;
        Put16(e + 8, (Uint16)(i == 0 ? 0 : 12));
        Put16(e + 12, 0x20);
        e[15] = (Uint8)(i == 2 ? 2 : 0);
        e[17] = (Uint8)i;
        e[18] = (Uint8)(i == 0 ? 0x24 : i == 1 ? 0x41 : 0x48);
        e[21] = 5;
        e[26] = 8;
        Put16(e + 40, 2);
        Put16(e + 48, 5);
    }
    Uint8 *dagger = f->dra + 0x300 + 18 * 52;
    SDL_memcpy(dagger, f->dra + 0x300 + 86 * 52, 52);
    dagger[15] = 31;
    dagger[17] = 7;
    dagger[18] = 0x6f;
    dagger[21] = 5;
    Uint8 *sword = f->dra + 0x300 + 123 * 52;
    SDL_memcpy(sword, f->dra + 0x300 + 86 * 52, 52);
    sword[15] = 43;
    Uint8 *short_sword = f->dra + 0x300 + 19 * 52;
    SDL_memcpy(short_sword, sword, 52);
    short_sword[15] = 37;
    short_sword[17] = 15;
    short_sword[18] = 0xa7;
    Uint8 *shield = f->dra + 0x300 + 16 * 52;
    SDL_memcpy(shield, sword, 52);
    shield[15] = 52;
    shield[17] = 128;
    shield[18] = 0x63;
    for (Uint32 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4, 0x800a3000);
    for (Uint32 pose = 0; pose < 14; ++pose)
        Put32(f->dra + 0x3000 + pose * 4, 1 | (1U << 16));
    Put32(f->dra + 0x3038, 0xffff);
    for (Uint32 family = 0; family <= 2; family += 2)
    {
        Uint8 *overlay = f->weapons + family * 0x7000 + 0x4000;
        Uint8 *descriptor = overlay + (family == 0 ? 0x106c : 0xbc4);
        Put32(descriptor, 0x8017b100);
        Put32(descriptor + 4, 0x8017b300);
        Put16(descriptor + 10, 0x55);
        descriptor[12] = (Uint8)(family == 0 ? 0x41 : 0x48);
        descriptor[13] = 4;
        for (Uint32 variant = 0; variant < 7; ++variant)
            Put32(overlay + 0x1100 + variant * 4, 0x8017b200);
        for (Uint32 pose = 0; pose < 14; ++pose)
            Put16(overlay + 0x1200 + pose * 2, 0x201);
        overlay[0x1304] = 18;
        overlay[0x1305] = 252;
        overlay[0x1306] = 20;
        overlay[0x1307] = 8;
    }
    Uint8 *overlay = f->weapons + 0xdd000, *descriptor = overlay + 0x8ec;
    Put32(descriptor, 0x8017b100);
    Put32(descriptor + 4, 0x8017b300);
    descriptor[12] = 0x6f;
    descriptor[13] = 4;
    for (Uint32 variant = 0; variant < 7; ++variant)
        Put32(overlay + 0x1100 + variant * 4, 0x8017b200);
    for (Uint32 pose = 0; pose < 14; ++pose)
        Put16(overlay + 0x1200 + pose * 2, 0x201);
    overlay[0x1304] = 16;
    overlay[0x1306] = 12;
    overlay[0x1307] = 8;
    overlay = f->weapons + 37 * 0x7000 + 0x4000;
    SDL_memcpy(overlay, f->weapons + 0x4000, 0x3000);
    SDL_memcpy(overlay + 0x6a4, overlay + 0x106c, 16);
    overlay[0x6b0] = 0xa7;
    f->types[1] = 3;
    for (Uint32 y = 10; y < 16; ++y)
        for (Uint32 x = 0; x < 32; ++x)
            f->tiles[(y * 32 + x) * 2] = 1;
    return (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
}

static bool Metadata(Fixture *f)
{
    (void)MakeFixture(f);
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, weapons = {f->weapons, sizeof(f->weapons)};
    SotnEquipment equipment = {0};
    CHECK(SotnEquipment_Read(dra, 86, &equipment));
    CHECK(equipment.attack == 12 && equipment.family == 0 && equipment.animation == 0x41);
    CHECK(!SotnEquipment_Read(dra, 1, &equipment) && equipment.id == 86);
    SotnWeaponFrame frame = {0};
    CHECK(SotnWeapon_ReadFrame(weapons, equipment, 0x41, 4, 1, &frame));
    CHECK(frame.x == 18 && frame.y == -4 && frame.width == 20 && frame.sprite == 1 &&
          frame.sound == 0x55);
    CHECK(SotnWeapon_ReadFrame(weapons, equipment, 0x41, 4, 2, &frame) && frame.sound == 0);
    CHECK(SotnWeapon_ReadFrame(weapons, equipment, 0x41, 4, -1, &frame) && frame.sprite == 0);
    CHECK(!SotnWeapon_ReadFrame(weapons, equipment, 0x41, 32, 1, &frame));
    CHECK(!SotnWeapon_ReadFrame((SotnDataView){0}, equipment, 0x41, 0, 1, &frame));
    Put32(f->weapons + 0x506c, 0xfffffffc);
    CHECK(!SotnWeapon_ReadFrame(weapons, equipment, 0x41, 0, 1, &frame));
    CHECK(frame.sprite == 0);
    CHECK(SotnEquipment_Read(dra, 18, &equipment) && equipment.family == 31 &&
          equipment.animation == 0x6f);
    CHECK(SotnWeapon_ReadFrame(weapons, equipment, 0x6f, 4, 1, &frame) && frame.width == 12 &&
          frame.sprite == 1);
    CHECK(SotnEquipment_Read(dra, 86, &equipment));
    CHECK(SotnEquipment_Read(dra, 123, &equipment) && equipment.family == 43);
    SotnDataView overlay = {0};
    CHECK(SotnWeapon_Overlay(weapons, equipment, &overlay) && overlay.size == 0x3000);
    CHECK(SotnEquipment_Read(dra, 16, &equipment) && equipment.kind == 128);
    CHECK(SotnWeapon_Overlay(weapons, equipment, &overlay));
    CHECK(!SotnWeapon_ReadFrame(weapons, equipment, 0x63, 0, 1, &frame));
    CHECK(SotnEquipment_Read(dra, 86, &equipment));
    CHECK(SotnEquipment_Read(dra, 19, &equipment) && equipment.family == 37 &&
          equipment.animation == 0xa7 && equipment.kind == 15);
    CHECK(SotnWeapon_ReadFrame(weapons, equipment, 0xa7, 4, 1, &frame) && frame.width == 20 &&
          frame.sound == 0x55);
    Put16(f->weapons + 37 * 0x7000 + 0x4000 + 0x1208, (15U << 9) | 1U);
    CHECK(!SotnWeapon_ReadFrame(weapons, equipment, 0xa7, 4, 1, &frame));
    equipment.family = 0;
    CHECK(!SotnWeapon_Overlay(weapons, equipment, &overlay));
    CHECK(SotnEquipment_Read(dra, 86, &equipment));
    Put32(f->dra + 0xc0, 0xfffffffc);
    CHECK(!SotnEquipment_Read(dra, 86, &equipment) && equipment.id == 86);
    return true;
}

static bool Movement(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, weapons = {f->weapons, sizeof(f->weapons)};
    SotnPlayer *player = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &player));
    CHECK(!SotnPlayer_SetEquipment(player, weapons, 86, 1));
    CHECK(SotnPlayer_SetEquipment(player, weapons, 86, 128));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_GetState(player, &state));
    CHECK(state.animation.id == 0x41 && state.attack.active && state.attack.equipment == 86 &&
          state.attack.power == 15);
    CHECK(!SotnPlayer_SetEquipment(player, weapons, 0, 0));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT));
    CHECK(SotnPlayer_GetState(player, &state) && state.facing_left && state.animation.id == 0x43);
    for (Uint32 tick = 0; tick < 32; ++tick)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.attack.right_hand &&
          state.animation.id == 0x4a && state.attack.equipment == 128);
    for (Uint32 tick = 0; tick < 32; ++tick)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && !state.attack.active);
    for (Uint32 tick = 0; tick < 32; ++tick)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0x45 && state.attack.active);
    SotnPlayer_Close(player);
    return true;
}

static bool ShortSword(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, weapons = {f->weapons, sizeof(f->weapons)};
    SotnPlayer *player = NULL;
    CHECK(SotnPlayer_Create(dra, map, 64, 135, &player));
    CHECK(SotnPlayer_SetEquipment(player, weapons, 19, 19));
    const Uint16 stances[] = {0, SOTN_BUTTON_DOWN, SOTN_BUTTON_JUMP};
    for (Uint32 hand = 0; hand < 2; ++hand)
        for (Uint32 stance = 0; stance < 3; ++stance)
        {
            for (Uint32 tick = 0; tick < 64; ++tick)
                CHECK(SotnPlayer_Tick(player, 0));
            CHECK(SotnPlayer_Tick(player, stances[stance]));
            CHECK(SotnPlayer_Tick(player,
                                  (Uint16)(stances[stance] | (hand ? SOTN_BUTTON_RIGHT_HAND
                                                                   : SOTN_BUTTON_LEFT_HAND))));
            SotnPlayerState state = {0};
            CHECK(SotnPlayer_GetState(player, &state));
            CHECK(state.attack.equipment == 19 && state.attack.active &&
                  state.attack.right_hand == (hand != 0) &&
                  state.animation.id == 0xa7 + (stance == 0   ? 0U
                                                : stance == 1 ? 2U
                                                              : 4U));
        }
    SotnPlayer_Close(player);
    return true;
}

static bool Curse(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, weapons = {f->weapons, sizeof(f->weapons)};
    SotnPlayer *player = NULL;
    SotnPlayerState state = {0};
    CHECK(SotnPlayer_Create(dra, map, 128, 135, &player));
    CHECK(SotnPlayer_SetEquipment(player, weapons, 86, 16));
    CHECK(SotnPlayer_CurseDamage(player, 1, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.hp == 69 && state.curse_ticks == 1000 &&
          (state.status & 0x28108000U) == 0x28108000U && state.extra_sound == 0x6f2);
    CHECK(SotnPlayer_CurseDamage(player, 10, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.hp == 69 && state.curse_ticks == 1000);
    CHECK(!SotnPlayer_RestoreCurse(player, 1025));
    CHECK(!SotnPlayer_CurseDamage(player, 10000, 96, 0));
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.step == SOTN_PLAYER_STAND &&
          state.curse_ticks == 936 && state.extra_sound == 0);
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0xb5 &&
          !state.attack.active && state.sound == 0x6e7 && !state.can_pause);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.can_pause && !state.attack.active);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0xb6 &&
          !state.attack.active);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0xb7 &&
          !state.attack.active);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_RIGHT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0xb5 &&
          !state.shield.active && !state.attack.active);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_RestoreCurse(player, 2));
    CHECK(SotnPlayer_ChangeRoom(player, map, 16, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.curse_ticks == 2);
    CHECK(SotnPlayer_Tick(player, 0) && SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.curse_ticks == 0 &&
          !(state.status & 0x8000U));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.attack.active &&
          state.attack.equipment == 86);
    for (Uint16 i = 0; i < 32; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    // Unarmed attacks remain available under curse in the original routine.
    Put32(f->dra + 0xd53c, 0x800a7000);
    Put32(f->dra + 0xd540, 0x800a7100);
    f->dra[0x7104] = 12;
    f->dra[0x7105] = (Uint8)-8;
    f->dra[0x7106] = f->dra[0x7107] = 8;
    for (Uint16 i = 0; i < 7; ++i)
        Put32(f->dra + 0x7000 + i * 4U, 0x800a7200);
    for (Uint16 i = 0; i < 16; ++i)
        Put16(f->dra + 0x7200 + i * 2U, 0x201);
    CHECK(SotnPlayer_SetEquipment(player, weapons, 0, 0));
    CHECK(SotnPlayer_RestoreCurse(player, 1024));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0x24 &&
          state.attack.active && state.curse_ticks == 1023);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_SetEquipment(player, weapons, 86, 16));
    CHECK(SotnPlayer_RestoreCurse(player, 1024));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_Petrify(player, 0, 96, 0));
    for (Uint16 i = 0; i < 100; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_GetState(player, &state));
    const Uint16 stone_hp = state.hp, curse_before = state.curse_ticks;
    CHECK(SotnPlayer_CurseDamage(player, 1, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.hp == stone_hp - 1 &&
          state.curse_ticks == curse_before);
    for (Uint16 i = 0; i < 400; ++i)
        CHECK(SotnPlayer_Tick(player, i % 24 == 0 ? SOTN_BUTTON_LEFT : 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.step == SOTN_PLAYER_STAND &&
          !state.stone.active && state.can_pause && state.curse_ticks != 0);
    CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_Tick(player, SOTN_BUTTON_LEFT_HAND));
    CHECK(SotnPlayer_GetState(player, &state) && state.animation.id == 0xb5 &&
          !state.attack.active);
    CHECK(!SotnPlayer_CurseDamage(NULL, 1, 0, 0) && !SotnPlayer_RestoreCurse(NULL, 0));
    SotnPlayer_Close(player);
    return true;
}

static bool CurseBounds(Fixture *f)
{
    const SotnCollisionMap map = MakeFixture(f);
    const SotnDataView dra = {f->dra, sizeof(f->dra)};
    SotnPlayer *player = NULL;
    SotnPlayerState state = {0};
    SotnCharacterState profile = {.hp = 70, .hp_max = 70};
    SotnCharacterStats stats = {.total = {6, 255, 6, 6}, .attack = {6, 6}};
    CHECK(SotnPlayer_Create(dra, map, 128, 135, &player));
    CHECK(SotnPlayer_SetCharacter(player, &profile, stats));
    CHECK(SotnPlayer_CurseDamage(player, 1, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.curse_ticks == 64);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    stats.total[1] = 0;
    CHECK(SotnPlayer_SetCharacter(player, &profile, stats));
    for (Uint16 i = 0x2e; i <= 0x34; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0xfffffffc);
    CHECK(!SotnPlayer_CurseDamage(player, 1, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.hp == 70 && state.curse_ticks == 0);
    for (Uint16 i = 0x2e; i <= 0x34; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a3000);
    CHECK(SotnPlayer_CurseDamage(player, 1, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.curse_ticks == 1024);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnPlayer_Tick(player, 0));
    CHECK(SotnPlayer_RestoreCurse(player, 0));
    CHECK(SotnPlayer_CurseDamage(player, 70, 96, 0));
    CHECK(SotnPlayer_GetState(player, &state) && state.hp == 0 && state.curse_ticks == 0);
    CHECK(!SotnPlayer_RestoreCurse(player, 10));
    SotnPlayer_Close(player);
    return true;
}

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = Metadata(fixture) && Movement(fixture) && ShortSword(fixture) &&
                        Curse(fixture) && CurseBounds(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_EQUIPMENT_TEST: PASS");
    return 0;
}
