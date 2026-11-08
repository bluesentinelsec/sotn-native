#include <sotn/combat.h>
#include <sotn/subweapon.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Definition(Uint8 *bytes, Uint16 id, Uint8 cost, Uint8 chain)
{
    Uint8 *p = bytes + 0x4a00 + id * 20;
    p[0] = 7;
    p[2] = cost;
    p[4] = 0x40;
    p[6] = chain;
    p[7] = 4;
    p[8] = 5;
    p[10] = 0x5d;
    p[12] = 2;
    p[14] = 3;
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x11000);
    CHECK(bytes != NULL);
    const SotnDataView dra = {bytes, 0x11000};
    Definition(bytes, 1, 3, 2);
    Definition(bytes, 6, 20, 1);
    bytes[0x4a00 + 1 * 20 + 16] = 9;
    bytes[0x4a00 + 1 * 20 + 18] = 0x30;
    SotnSubweaponDefinition d = {0};
    CHECK(SotnSubweapon_Read(dra, 1, &d) && d.attack.power == 7 && d.hearts == 3 && d.chain == 2 &&
          d.attack.element == 0x40 && d.attack.invincibility_frames == 4 &&
          d.attack.stun_frames == 5 && d.animation == 0x5d && d.blueprint == 0 &&
          d.attack.hit_type == 2 && d.attack.hit_effect == 3 && d.crash == 9 &&
          d.attack.critical_rate == 0x30 && d.attack.active);
    CHECK(SotnSubweapon_Read(dra, 6, &d) && d.hearts == 20 && d.chain == 1);
    // Out-of-range ids, a short image and malformed records fail without mutation.
    const SotnSubweaponDefinition kept = d;
    CHECK(!SotnSubweapon_Read(dra, 13, &d) && !SDL_memcmp(&d, &kept, sizeof(d)));
    CHECK(!SotnSubweapon_Read((SotnDataView){bytes, 0x4a00 + 12 * 20}, 1, &d));
    CHECK(!SotnSubweapon_Read(dra, 2, &d) && !SDL_memcmp(&d, &kept, sizeof(d)));
    CHECK(SotnSubweapon_Read(dra, 0, &d) && d.chain == 0);
    // CheckSubwpnChainLimit counts at most sixteen slots.
    bytes[0x4a00 + 1 * 20 + 6] = 17;
    CHECK(!SotnSubweapon_Read(dra, 1, &d));
    bytes[0x4a00 + 1 * 20 + 6] = 16;
    bytes[0x4a00 + 1 * 20 + 11] = 123;
    CHECK(!SotnSubweapon_Read(dra, 1, &d));
    bytes[0x4a00 + 1 * 20 + 11] = 0;
    bytes[0x4a00 + 1 * 20 + 1] = 4;
    CHECK(!SotnSubweapon_Read(dra, 1, &d));
    bytes[0x4a00 + 1 * 20 + 1] = 0;
    bytes[0x4a00 + 1 * 20 + 19] = 1;
    CHECK(!SotnSubweapon_Read(dra, 1, &d));
    CHECK(!SotnSubweapon_Read(dra, 1, NULL) && !SotnSubweapon_Read((SotnDataView){0}, 1, &d));
    // func_800FE3C4: one Heart broach77 halves the cost, two divide by three, minimum one.
    SotnCharacterState broach = {0};
    CHECK(SotnSubweapon_Cost(broach, 20) == 20 && SotnSubweapon_Cost(broach, 0) == 1);
    broach.equipment[5] = 77;
    CHECK(SotnSubweapon_Cost(broach, 20) == 10 && SotnSubweapon_Cost(broach, 1) == 1);
    broach.equipment[6] = 77;
    CHECK(SotnSubweapon_Cost(broach, 20) == 6 && SotnSubweapon_Cost(broach, 3) == 1 &&
          SotnSubweapon_Cost(broach, 100) == 33);
    broach.equipment[5] = 0;
    CHECK(SotnSubweapon_Cost(broach, 5) == 2);
    // func_800FE3C4: Brilliant mail20 adds ten; each Staurolite61 multiplies the cross (4) and
    // subweapon12 only; one libc roll adds (INT * 2 + rand() % 12) / 10.
    {
        SotnCharacterState gear = {0};
        Uint32 cursor = 0x1234, expected = cursor;
        const Uint16 roll = SotnCombat_Random(&expected) % 12;
        Uint16 power = 0;
        CHECK(!SotnSubweapons_Power(0, 7, &gear, 30, &cursor, &power) && cursor == 0x1234);
        CHECK(SotnSubweapons_Power(1, 7, &gear, 30, &cursor, &power) && cursor == expected &&
              power == 7 + (60 + roll) / 10);
        gear.equipment[3] = 20;
        cursor = 0x1234;
        CHECK(SotnSubweapons_Power(1, 7, &gear, 30, &cursor, &power) &&
              power == 17 + (60 + roll) / 10);
        gear.equipment[5] = 61;
        cursor = 0x1234;
        CHECK(SotnSubweapons_Power(1, 7, &gear, 30, &cursor, &power) &&
              power == 17 + (60 + roll) / 10);
        cursor = 0x1234;
        CHECK(SotnSubweapons_Power(4, 7, &gear, 30, &cursor, &power) &&
              power == 34 + (60 + roll) / 10);
        gear.equipment[6] = 61;
        gear.equipment[3] = 0;
        cursor = 0x1234;
        CHECK(SotnSubweapons_Power(12, 7, &gear, 30, &cursor, &power) &&
              power == 21 + (60 + roll) / 10);
        cursor = 0x1234;
        CHECK(SotnSubweapons_Power(3, 7, &gear, 30, &cursor, &power) &&
              power == 7 + (60 + roll) / 10);
    }
    SDL_free(bytes);
    SDL_Log("SOTN_SUBWEAPON_TEST: PASS");
    return 0;
}
