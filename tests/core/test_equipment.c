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
    Uint8 dra[0x11000], weapons[0x15000], tiles[32 * 16 * 2], types[2];
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

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = Metadata(fixture) && Movement(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_EQUIPMENT_TEST: PASS");
    return 0;
}
