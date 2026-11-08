// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/collision.h enemy drops and e_collect.h rare-drop records at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. The drop tables stay in each private stage overlay.
#include "actors_internal.h"
#include <sotn/enemy_drop.h>

// Each integrated overlay's file-static g_testCollLuckCutoff (its other drop tables follow the
// same layout around it).
static size_t LuckOffset(Uint16 stage)
{
    switch (stage)
    {
    case 0:
        return 0x1488; // NO0
    case 1:
        return 0x1da8; // NO1
    case 2:
        return 0x1fc8; // LIB
    case 4:
        return 0x1374; // NO2
    case 6:
        return 0x13bc; // DAI
    case 7:
        return 0x1998; // NP3
    case 12:
        return 0x18c0; // NZ0
    case 25:
        return 0x10ac; // BO5
    case 26:
        return 0x9fc; // BO4
    case 30:
        return 0x17c4; // BO0
    case 65:
        return 0x200c; // NO3
    default:
        return 0;
    }
}
void ActorDropRecord(SotnActors *actors, const Actor *drop)
{
    if (!drop->drop_castle_flag || !actors->world)
        return;
    const Uint16 index = (Uint16)(drop->drop_castle_flag - 1U);
    actors->world->flags[0x1b0 + index / 8U] |= (Uint8)(1U << (index & 7U));
}
bool ActorEnemyDrop(SotnActors *actors, const Actor *part, const Actor *root, Uint16 hit_effect)
{
    const Uint32 kind = root->state.flags & 0xc00U;
    if (!kind)
        return true;
    // Standalone actor fixtures without a character own no luck, equipment or drop roll.
    if (!actors->character)
        return true;
    const size_t luck_offset = LuckOffset(actors->stage_id);
    SotnEnemyDropTables tables = {0};
    if (!luck_offset ||
        !SotnEnemyDrop_ReadTables(actors->nz0.bytes, actors->nz0.size, luck_offset, &tables))
        return SDL_SetError("Enemy drop requires its stage's original collision tables");
    SotnCharacterState progress = {0};
    SotnCharacterStats stats = {0};
    if (!SotnCharacter_Get(actors->character, &progress, &stats))
        return false;
    const Sint32 luck = stats.total[3];
    Uint32 random = actors->combat_random;
    const Sint32 cutoff = SotnEnemyDrop_LuckCutoff(tables.luck_cutoff[kind >> 10], luck);
    if ((Sint32)(SotnCombat_Random(&random) & 0xffU) >= cutoff)
    {
        actors->combat_random = random;
        return true;
    }
    Actor *drop = NULL;
    if (!ActorSpawn(actors, part, 3, 0, 160, 192, &drop))
        return false;
    if (!drop)
    {
        actors->combat_random = random; // AllocEntity found no slot: no further rolls.
        return true;
    }
    Uint16 item = 0, castle = 0;
    if ((hit_effect & 0x7fU) == 5)
        item = SotnEnemyDrop_Jewel(&tables, &random);
    else
    {
        SotnDataView api = {0}, definition = {0};
        const Uint32 id = root->enemy_definition ? root->enemy_definition - 1U : 400U;
        if (id >= 400 || !Slice(actors->dra, 0x98, 4, &api))
            return SDL_SetError("Enemy drop requires the killed root's original enemy definition");
        const Uint32 address = Read32(api.bytes);
        if (address < 0x800a0000 ||
            !Slice(actors->dra, (size_t)(address - 0x800a0000) + id * 40U + 26U, 8, &definition))
            return SDL_SetError("Enemy drop definition lies outside the private DRA image");
        const SotnEnemyDropRates rates = {Read16(definition.bytes), Read16(definition.bytes + 2),
                                          Read16(definition.bytes + 4),
                                          Read16(definition.bytes + 6)};
        const Uint16 arcana =
            (Uint16)((progress.equipment[5] == 75) + (progress.equipment[6] == 75));
        Sint32 choice = 0;
        if (!SotnEnemyDrop_Kind(rates, luck, arcana, &random, &choice))
            return false;
        if (choice & 0x40)
        {
            item = rates.rare_item;
            // Paranthropus: a Ring of Varda only after the game is cleared, else a Turquoise.
            if (item == 0x173 && !(actors->world && actors->world->game_clear))
                item = 0x16a;
            else
                castle = tables.enemy_lookup[id];
        }
        else if (choice & 0x20)
            item = rates.uncommon_item;
        else if (choice >= 0 && choice < 32)
            item = tables.prize[choice];
        else
            return SDL_SetError("Enemy drop chose a prize outside the original table");
    }
    drop->state.id = item >= 0x80 ? 0xa : 3;
    drop->state.parameters = item >= 0x80 ? (Uint16)(item - 0x80U) : item;
    drop->state.facing_left = false;
    drop->vy = -0x38000; // FIX(-3.5): the item pops up when spawned.
    drop->drop_castle_flag = castle;
    actors->combat_random = random;
    return true;
}
