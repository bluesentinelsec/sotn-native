#include "character_fixture.h"
#include <sotn/combat.h>
#include <sotn/gas_cloud.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0xe000);
    CHECK(bytes);
    TestCharacter_Fill(bytes);
    // Authored descriptor fields exercise the selected original factory contract.
    Uint8 *bp = bytes + 0xd1d4 + 83 * 6, *spell = bytes + 0x8410 + 12 * 28;
    bp[0] = 48;
    bp[1] = 17;
    bp[2] = 0x80 | 0x40 | 17;
    bp[4] = 2 | (2 << 4);
    spell[13] = 7;
    spell[14] = 9;
    spell[16] = 2;
    spell[18] = 3;
    spell[22] = 0x20;
    spell[24] = 31;
    const SotnDataView dra = {bytes, 0xe000};
    SotnCharacter *character = NULL;
    CHECK(SotnCharacter_Create(dra, &character));
    SotnCharacterState before = {0}, after = {0};
    SotnGasCloud cloud = {0};
    CHECK(SotnCharacter_Get(character, &before, NULL));
    CHECK(SotnGasCloud_Begin(&cloud, dra, character, 17, SDL_MAX_UINT16));
    CHECK(cloud.generation == 17 && cloud.mask == SDL_MAX_UINT16);
    Uint64 occupied = 0;
    CHECK(SotnGasCloud_Occupied(&cloud, &occupied) && occupied == (Uint64)0xffff << 32);
    for (Uint16 i = 0; i < SOTN_GAS_CLOUD_SLOTS; ++i)
        CHECK(cloud.children[i].slot == 32U + i);
    Uint32 random = before.random;
    for (Uint16 i = 0; i < SOTN_GAS_CLOUD_SLOTS; ++i)
    {
        const Uint16 expected = (Uint16)(31 + (12 + SotnCombat_Random(&random) % 12) / 10);
        const SotnPlayerAttack a = cloud.children[i].attack;
        CHECK(a.power == expected && a.active && a.hit_type == 2 && a.element == 0x20 &&
              a.invincibility_frames == 7 && a.stun_frames == 9 && a.hit_effect == 3 &&
              a.width == (i ? 8 : 28) && a.height == a.width && !a.critical_rate);
    }
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == random);
    CHECK(SotnCharacter_Restore(character, &before));
    CHECK(SotnGasCloud_Begin(&cloud, dra, character, 17, 0xaaaa) && cloud.mask == 0xff);
    CHECK(SotnGasCloud_Occupied(&cloud, &occupied) && occupied == (Uint64)0xaaaa << 32);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(cloud.children[i].slot == 33U + 2U * i);
    const SotnGasCloud valid_slots = cloud;
    cloud.children[1].slot = cloud.children[0].slot;
    occupied = 123;
    CHECK(!SotnGasCloud_Occupied(&cloud, &occupied) && occupied == 123);
    cloud = valid_slots;
    cloud.children[1].slot = 48;
    CHECK(!SotnGasCloud_Occupied(&cloud, &occupied) && occupied == 123);
    cloud = valid_slots;
    cloud.generation = 0;
    CHECK(!SotnGasCloud_Occupied(&cloud, &occupied) && occupied == 123);
    cloud = valid_slots;
    random = before.random;
    for (Uint16 i = 0; i < 8; ++i)
        (void)SotnCombat_Random(&random);
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == random);
    CHECK(SotnGasCloud_Begin(&cloud, dra, character, 18, 0) && cloud.mask == 0);
    CHECK(SotnCharacter_Get(character, &before, NULL) && before.random == random);
    const SotnGasCloud unchanged = cloud;
    bp[1] = 18;
    CHECK(!SotnGasCloud_Begin(&cloud, dra, character, 19, SDL_MAX_UINT16));
    CHECK(!SDL_memcmp(&cloud, &unchanged, sizeof(cloud)));
    CHECK(SotnCharacter_Get(character, &after, NULL) && after.random == random);
    bp[1] = 17;
    CHECK(!SotnGasCloud_Begin(&cloud, (SotnDataView){bytes, 100}, character, 19, 1));
    CHECK(!SotnGasCloud_Begin(&cloud, dra, character, 0, 1));
    CHECK(SotnCharacter_Collect(character, true, 21));
    CHECK(SotnCharacter_Equip(character, SOTN_EQUIP_ARMOR, 21));
    CHECK(SotnGasCloud_Begin(&cloud, dra, character, 19, 1));
    const Uint16 unbuffed = (Uint16)(31 + (12 + SotnCombat_Random(&random) % 12) / 10);
    CHECK(cloud.children[0].attack.power == unbuffed + unbuffed / 2);
    CHECK(SotnGasCloud_Begin(&cloud, dra, character, 19, SDL_MAX_UINT16));
    SotnPlayerState player = {.x = 99 * 65536 + 123,
                              .y = 198 * 65536 + 456,
                              .status = 2,
                              .mist = {.generation = 19, .active = true}};
    SotnMistEffect effect = {.x = -2 * 65536 + 32768, .y = -3 * 65536 + 32768};
    for (Uint16 i = 0; i < 16; ++i)
    {
        effect.points[i].x = ((Sint32)i - 8) * 65536 + 42;
        effect.points[i].y = ((Sint32)i - 8) * 65536 + 56;
    }
    CHECK(SotnGasCloud_Tick(&cloud, player, &effect, 19, 100, 200, true));
    CHECK(cloud.children[0].x == 99 && cloud.children[0].y == 198);
    for (Uint16 i = 1; i < 16; ++i)
        CHECK(cloud.children[i].x == 100 + i - 9 && cloud.children[i].y == 200 + i - 9);
    const SotnGasCloud positioned = cloud;
    CHECK(!SotnGasCloud_Tick(&cloud, player, NULL, 0, 100, 200, true));
    CHECK(!SotnGasCloud_Tick(&cloud, player, &effect, 20, 100, 200, true));
    CHECK(!SDL_memcmp(&cloud, &positioned, sizeof(cloud)));
    CHECK(SotnGasCloud_Tick(&cloud, player, &effect, 19, 100, 200, false));
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(!cloud.children[i].attack.active);
    CHECK(SotnGasCloud_Tick(&cloud, player, &effect, 19, 100, 200, true));
    CHECK(cloud.children[0].attack.active);
    player.status = 0;
    CHECK(SotnGasCloud_Tick(&cloud, player, NULL, 0, 100, 200, true));
    CHECK(!cloud.generation && !cloud.mask);
    player.status = 2;
    CHECK(SotnGasCloud_Tick(&cloud, player, &effect, 19, 100, 200, true));
    CHECK(!cloud.generation && !cloud.mask);
    SotnCharacter_Destroy(character);
    SDL_free(bytes);
    return 0;
}
