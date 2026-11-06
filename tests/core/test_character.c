#include <SDL3/SDL.h>
#include <sotn/character.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x5000);
    CHECK(bytes != NULL);
    Put32(bytes + 0xc0, 0x800a1000);
    Put32(bytes + 0xc4, 0x800a4000);
    bytes[0x100] = 'T' - 32;
    bytes[0x101] = 'e' - 32;
    bytes[0x102] = 's' - 32;
    bytes[0x103] = 't' - 32;
    bytes[0x104] = 255;
    for (Uint32 body = 0; body < 2; ++body)
        for (Uint32 id = 0; id < (body ? 90U : 169U); ++id)
        {
            Uint8 *p = bytes + (body ? 0x4000 + id * 32 : 0x1000 + id * 52);
            Put32(p, 0x800a0100);
            Put32(p + 4, 0x800a0180);
            if (body)
                p[28] = (Uint8)(id == 26 ? 0 : id == 48 ? 2 : (id == 57 || id == 60) ? 3 : 1);
        }
    bytes[0x1000 + 86 * 52 + 8] = 4;
    bytes[0x1000 + 128 * 52 + 8] = 12;
    bytes[0x4000 + 1 * 32 + 8] = 3;
    bytes[0x4000 + 1 * 32 + 10] = 3;
    bytes[0x4000 + 1 * 32 + 12] = 2;
    bytes[0x4000 + 1 * 32 + 13] = 1;
    const SotnDataView dra = {bytes, 0x5000};
    SotnCharacter *c = NULL;
    SotnCharacterState s = {0};
    SotnCharacterStats stats = {0};
    CHECK(SotnCharacter_Create(dra, &c));
    CHECK(SotnCharacter_Get(c, &s, &stats));
    CHECK(s.hp == 70 && s.level == 1 && stats.attack[0] == 6 && stats.unarmed_attack == 6 &&
          stats.defense == 0 && s.equipment[5] == 57);
    CHECK(!SotnCharacter_Equip(c, 0, 86));
    CHECK(SotnCharacter_Collect(c, false, 86));
    CHECK(SotnCharacter_Equip(c, 0, 86));
    CHECK(!SotnCharacter_Equip(c, 1, 86));
    CHECK(SotnCharacter_Collect(c, false, 86));
    CHECK(SotnCharacter_Equip(c, 1, 86));
    CHECK(SotnCharacter_Collect(c, true, 1));
    CHECK(!SotnCharacter_Equip(c, 2, 1));
    CHECK(SotnCharacter_Equip(c, 3, 1));
    CHECK(SotnCharacter_Get(c, &s, &stats));
    CHECK(stats.total[0] == 8 && stats.total[1] == 14 && stats.attack[0] == 15 &&
          stats.unarmed_attack == 11 && stats.defense == 4);
    CHECK(SotnCharacter_Equip(c, 0, 0));
    CHECK(SotnCharacter_Get(c, &s, NULL) && s.hand_count[86] == 1);
    // The original menu never equips Life Apple (item146) in a hand.
    CHECK(SotnCharacter_Collect(c, false, 146));
    CHECK(!SotnCharacter_Equip(c, 0, 146));
    for (Uint32 i = 0; i < 120; ++i)
        CHECK(SotnCharacter_Collect(c, false, 128));
    CHECK(SotnCharacter_Get(c, &s, NULL) && s.hand_count[128] == 99);
    SotnCharacterState invalid = s;
    invalid.hand_order[1] = invalid.hand_order[0];
    CHECK(!SotnCharacter_Restore(c, &invalid));
    invalid = s;
    invalid.equipment[0] = 169;
    CHECK(!SotnCharacter_Restore(c, &invalid));
    invalid = s;
    invalid.equipment[2] = 1;
    CHECK(!SotnCharacter_Restore(c, &invalid));
    CHECK(SotnCharacter_Get(c, &invalid, NULL) && invalid.equipment[0] == s.equipment[0]);
    SotnItem item = {0};
    char name[16] = {0};
    CHECK(SotnItem_Read(dra, false, 86, &item));
    CHECK(SotnItem_Name(item, name, sizeof(name)) && SDL_strcmp(name, "Test") == 0);
    CHECK(!SotnItem_Name(item, name, 2));
    CHECK(!SotnItem_Read(dra, true, 90, &item));
    CHECK(SotnCharacter_Collect(c, true, 60) && SotnCharacter_Equip(c, SOTN_EQUIP_RING1, 60) &&
          SotnCharacter_Get(c, &s, NULL) && s.equipment[SOTN_EQUIP_RING1] == 60 &&
          !s.body_count[60]);
    const Uint16 stolen_hand[] = {123, 16};
    const Uint16 stolen_body[] = {15, 45, 56, 78};
    for (Uint32 i = 0; i < 2; ++i)
    {
        CHECK(SotnCharacter_Collect(c, false, stolen_hand[i]));
        CHECK(SotnCharacter_Collect(c, false, stolen_hand[i]));
    }
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(SotnCharacter_Collect(c, true, stolen_body[i]));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    const Uint16 left_before = s.equipment[0], right_before = s.equipment[1];
    const Uint16 hp_before = s.hp;
    CHECK(SotnCharacter_DeathTheft(c));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    CHECK(s.equipment[0] == left_before && s.equipment[1] == right_before && s.hp == hp_before);
    for (Uint32 i = 0; i < 2; ++i)
        CHECK(s.hand_count[stolen_hand[i]] == 1);
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(s.body_count[stolen_body[i]] == 0);
    CHECK(!SotnCharacter_DeathTheft(NULL));
    bytes[0x4000 + 45 * 32 + 28] = 0;
    bytes[0x4000 + 56 * 32 + 28] = 2;
    bytes[0x4000 + 78 * 32 + 28] = 3;
    CHECK(SotnCharacter_OpeningGear(c));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    CHECK(s.equipment[0] == 123 && s.equipment[1] == 16 && s.equipment[2] == 45 &&
          s.equipment[3] == 15 && s.equipment[4] == 56 && s.equipment[5] == 78);
    CHECK(SotnCharacter_DeathTheft(c));
    CHECK(SotnCharacter_Get(c, &s, NULL));
    CHECK(s.equipment[0] == 0 && s.equipment[1] == 0 && s.equipment[2] == 26 &&
          s.equipment[3] == 0 && s.equipment[4] == 48 && s.equipment[5] == 57);
    CHECK(s.hand_count[123] == 1 && s.hand_count[16] == 1);
    CHECK(!SotnCharacter_OpeningGear(NULL));
    CHECK(!SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 19));
    CHECK(SotnCharacter_Collect(c, false, 19));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 19));
    CHECK(SotnCharacter_Get(c, &s, NULL) && s.equipment[0] == 19 && !s.hand_count[19]);
    CHECK(SotnCharacter_Restore(c, &s));
    bytes[0x4000 + 36 * 32 + 28] = 0;
    bytes[0x4000 + 36 * 32 + 14] = 3;
    CHECK(SotnCharacter_Collect(c, true, 36));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_HEAD, 36));
    SotnCharacterStats mask_stats = {0};
    CHECK(SotnCharacter_Get(c, &s, &mask_stats) && s.equipment[2] == 36 &&
          mask_stats.bonus[2] == 3 && mask_stats.total[2] == s.base[2] + 12);
    CHECK(SotnCharacter_Get(c, &s, NULL));
    const Uint16 previous_mp = s.mp;
    CHECK(previous_mp >= 4 && SotnCharacter_SpendMP(c, 4));
    CHECK(SotnCharacter_Get(c, &s, NULL) && s.mp == previous_mp - 4);
    CHECK(!SotnCharacter_SpendMP(c, (Uint16)(s.mp + 1)) && !SotnCharacter_SpendMP(c, 10000) &&
          !SotnCharacter_SpendMP(NULL, 0));
    CHECK(SotnCharacter_SpendMP(c, 0) && SotnCharacter_Get(c, &s, NULL) && s.mp == previous_mp - 4);
    // Original IsAlucart: Alucart sword168 and shield167 in the hands with Alucart mail89 in the
    // armor slot add LCK30 to the equipment bonus (before its 99 cap); any piece missing, or the
    // sword in both hands, adds nothing.
    CHECK(SotnCharacter_Collect(c, false, 168) && SotnCharacter_Collect(c, false, 168) &&
          SotnCharacter_Collect(c, false, 167) && SotnCharacter_Collect(c, true, 89));
    SotnCharacterStats plain = {0}, alucart = {0};
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 168) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 167) && SotnCharacter_Get(c, &s, &plain));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 89) && SotnCharacter_Get(c, &s, &alucart));
    CHECK(alucart.bonus[3] == plain.bonus[3] + 30 && alucart.total[3] == plain.total[3] + 30 &&
          alucart.total[0] == plain.total[0] && alucart.bonus[0] == plain.bonus[0]);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 168) && SotnCharacter_Get(c, &s, &alucart) &&
          alucart.bonus[3] == plain.bonus[3]);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 167) && SotnCharacter_Get(c, &s, &alucart) &&
          alucart.bonus[3] == plain.bonus[3] + 30);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 0) && SotnCharacter_Get(c, &s, &alucart) &&
          alucart.bonus[3] == plain.bonus[3]);
    // Original CalcAttack: Shield rod4 gains five beside any shield (category9) in the other hand.
    bytes[0x1000 + 4 * 52 + 14] = 4;
    bytes[0x1000 + 4 * 52 + 8] = 7;
    bytes[0x1000 + 5 * 52 + 14] = 9;
    bytes[0x1000 + 167 * 52 + 14] = 9;
    SotnCharacterStats rod = {0}, guarded = {0};
    CHECK(SotnCharacter_Collect(c, false, 4) && SotnCharacter_Collect(c, false, 4) &&
          SotnCharacter_Collect(c, false, 5));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 4) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 0) && SotnCharacter_Get(c, &s, &rod));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 5) && SotnCharacter_Get(c, &s, &guarded) &&
          guarded.attack[0] == rod.attack[0] + 5);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 4) && SotnCharacter_Get(c, &s, &guarded) &&
          guarded.attack[0] == rod.attack[0] && guarded.attack[1] == rod.attack[0]);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 167) && SotnCharacter_Get(c, &s, &guarded) &&
          guarded.attack[1] == rod.attack[0] + 5);
    // CalcDefense: Shield rod beside a shield adds two defense; held Medusa13/Fire15 shields
    // grant stone/fire immunity.
    SotnItem shield = {0};
    SotnCharacterStats lone = {0}, paired = {0};
    CHECK(SotnItem_Read(dra, false, 5, &shield));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 4) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 0) && SotnCharacter_Get(c, &s, &lone));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 5) && SotnCharacter_Get(c, &s, &paired) &&
          paired.defense == lone.defense + 2 + shield.defense);
    CHECK(SotnCharacter_Collect(c, false, 13) && SotnCharacter_Collect(c, false, 15) &&
          !(paired.immunity & 0x8200U));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 13) && SotnCharacter_Get(c, &s, &paired) &&
          (paired.immunity & 0x200U) && !(paired.immunity & 0x8000U));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 15) && SotnCharacter_Get(c, &s, &paired) &&
          (paired.immunity & 0x8000U) && !(paired.immunity & 0x200U));
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 0));
    // Additive-only body records: Ice mail12's resistance and the Ring of Varda74's bonuses.
    bytes[0x4000 + 12 * 32 + 19] = 0x20;
    bytes[0x4000 + 74 * 32 + 28] = 3;
    bytes[0x4000 + 74 * 32 + 12] = 30;
    bytes[0x4000 + 74 * 32 + 15] = 5;
    SotnCharacterStats mailed = {0};
    CHECK(SotnCharacter_Collect(c, true, 12) && SotnCharacter_Collect(c, true, 74) &&
          SotnCharacter_Get(c, &s, &lone) && SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 12) &&
          SotnCharacter_Equip(c, 5, 74) && SotnCharacter_Get(c, &s, &mailed) &&
          (mailed.resistance & 0x2000U) && mailed.bonus[0] == lone.bonus[0] + 30 &&
          mailed.bonus[3] == lone.bonus[3] + 5);
    CHECK(SotnCharacter_Equip(c, 5, 57) && SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 0));
    // Reverse cloak50 and Royal cloak53 are ordinary cape-slot records (their palettes copy).
    bytes[0x4000 + 50 * 32 + 28] = 2;
    bytes[0x4000 + 53 * 32 + 28] = 2;
    bytes[0x4000 + 53 * 32 + 13] = 8;
    SotnCharacterStats caped = {0};
    CHECK(SotnCharacter_Collect(c, true, 50) && SotnCharacter_Collect(c, true, 53) &&
          SotnCharacter_Get(c, &s, &lone) && SotnCharacter_Equip(c, SOTN_EQUIP_CAPE, 50) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_CAPE, 53) && SotnCharacter_Get(c, &s, &caped) &&
          s.equipment[4] == 53 && caped.bonus[1] == lone.bonus[1] + 8);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_CAPE, 48));
    // CalcDefense: Walk armor19 adds the explored-room counter / 60.
    SotnCharacterStats walking = {0};
    CHECK(SotnCharacter_Collect(c, true, 19) && SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 19) &&
          SotnCharacter_Get(c, &s, &lone));
    CHECK(!SotnCharacter_SetRooms(c, 8193));
    CHECK(SotnCharacter_SetRooms(c, 179) && SotnCharacter_Get(c, &s, &walking) &&
          walking.defense == lone.defense + 2);
    CHECK(SotnCharacter_SetRooms(c, 180) && SotnCharacter_Get(c, &s, &walking) &&
          walking.defense == lone.defense + 3);
    CHECK(SotnCharacter_Equip(c, SOTN_EQUIP_ARMOR, 0) && SotnCharacter_Get(c, &s, &walking) &&
          SotnCharacter_SetRooms(c, 0));
    // Original CalcAttack: the Sword Familiar126 adds the Sword familiar's (record4) level.
    bytes[0x1000 + 126 * 52 + 14] = 1;
    bytes[0x1000 + 126 * 52 + 8] = 0;
    SotnCharacterStats familiar = {0}, raised = {0};
    SotnFamiliarProgress progress = {0};
    CHECK(SotnCharacter_Collect(c, false, 126) && SotnCharacter_Equip(c, SOTN_EQUIP_LEFT, 126) &&
          SotnCharacter_Equip(c, SOTN_EQUIP_RIGHT, 0) && SotnCharacter_Get(c, &s, &familiar) &&
          SotnCharacter_GetFamiliars(c, &progress, NULL) && progress.records[4].level == 1);
    progress.records[4] = (SotnFamiliarStats){62, 6150, 3};
    progress.records[2] = (SotnFamiliarStats){9, 800, 1};
    CHECK(SotnCharacter_RestoreFamiliars(c, &progress) && SotnCharacter_Get(c, &s, &raised) &&
          raised.attack[0] == familiar.attack[0] + 61 && raised.attack[1] == familiar.attack[1]);
    // UpdateEquipStatBonuses: Sunstone59 (hours6..17) / Moonstone58 (the rest) add five to every
    // stat per accessory copy; they need the actual playtime owner.
    bytes[0x4000 + 58 * 32 + 28] = 3;
    bytes[0x4000 + 59 * 32 + 28] = 3;
    SotnCharacter *sun = NULL;
    SotnCharacterStats unlit = {0}, lit = {0};
    CHECK(SotnCharacter_Create(dra, &sun) && SotnCharacter_Collect(sun, true, 59) &&
          SotnCharacter_Collect(sun, true, 59) && SotnCharacter_Collect(sun, true, 58) &&
          SotnCharacter_Get(sun, &s, &unlit));
    CHECK(!SotnCharacter_Equip(sun, 5, 59));
    CHECK(SotnCharacter_SetPlaytime(sun, (SotnPlaytime){.hours = 30}) &&
          SotnCharacter_Equip(sun, 5, 59) && SotnCharacter_Equip(sun, 6, 59) &&
          SotnCharacter_Get(sun, &s, &lit));
    for (Uint32 i = 0; i < 4; ++i)
        CHECK(lit.bonus[i] == unlit.bonus[i] + 10 &&
              lit.total[i] == unlit.total[i] + 10 * (i == 1   ? 8
                                                     : i == 2 ? 4
                                                              : 1));
    // Hour 18 (and a negative hour's negative remainder) belongs to the Moonstone.
    CHECK(SotnCharacter_SetPlaytime(sun, (SotnPlaytime){.hours = 42}) &&
          SotnCharacter_Get(sun, &s, &lit) && lit.bonus[0] == unlit.bonus[0]);
    CHECK(SotnCharacter_SetPlaytime(sun, (SotnPlaytime){.hours = 17}) &&
          SotnCharacter_Get(sun, &s, &lit) && lit.bonus[3] == unlit.bonus[3] + 10);
    CHECK(SotnCharacter_Equip(sun, 6, 58) && SotnCharacter_Get(sun, &s, &lit) &&
          lit.bonus[2] == unlit.bonus[2] + 5);
    // A negative hour keeps C's negative remainder (-7, the Moonstone's), never 9.
    CHECK(SotnCharacter_Equip(sun, 5, 57));
    CHECK(SotnCharacter_SetPlaytime(sun, (SotnPlaytime){.hours = -7}) &&
          SotnCharacter_Get(sun, &s, &lit) && lit.bonus[1] == unlit.bonus[1] + 5);
    CHECK(SotnCharacter_SetPlaytime(sun, (SotnPlaytime){.hours = 5}) &&
          SotnCharacter_Get(sun, &s, &lit) && lit.bonus[0] == unlit.bonus[0] + 5);
    // Active Vlad relics: Tooth STR, Rib CON, Ring INT, Eye LCK +10; Heart curse immunity.
    SotnCharacterStats vlad = {0};
    CHECK(!SotnCharacter_SetVladRelics(sun, 32));
    CHECK(SotnCharacter_SetVladRelics(sun, 2) && SotnCharacter_Get(sun, &s, &vlad) &&
          vlad.bonus[0] == lit.bonus[0] + 10 && vlad.bonus[1] == lit.bonus[1] &&
          !(vlad.immunity & 0x100U));
    CHECK(SotnCharacter_SetVladRelics(sun, 4) && SotnCharacter_Get(sun, &s, &vlad) &&
          vlad.bonus[1] == lit.bonus[1] + 10 && vlad.total[1] == lit.total[1] + 80);
    CHECK(SotnCharacter_SetVladRelics(sun, 8) && SotnCharacter_Get(sun, &s, &vlad) &&
          vlad.bonus[2] == lit.bonus[2] + 10 && vlad.total[2] == lit.total[2] + 40);
    CHECK(SotnCharacter_SetVladRelics(sun, 16) && SotnCharacter_Get(sun, &s, &vlad) &&
          vlad.bonus[3] == lit.bonus[3] + 10 && vlad.bonus[0] == lit.bonus[0]);
    CHECK(SotnCharacter_SetVladRelics(sun, 1) && SotnCharacter_Get(sun, &s, &vlad) &&
          (vlad.immunity & 0x100U) && vlad.bonus[3] == lit.bonus[3]);
    CHECK(SotnCharacter_SetVladRelics(sun, 0) && SotnCharacter_Get(sun, &s, &vlad) &&
          !(vlad.immunity & 0x100U));
    SotnCharacter_Destroy(sun);
    bytes[0x104] = 0;
    CHECK(!SotnItem_Read(dra, false, 86, &item));
    SotnCharacter_Destroy(c);
    SDL_free(bytes);
    SDL_Log("SOTN_CHARACTER_TEST: PASS");
    return 0;
}
