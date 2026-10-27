// SPDX-License-Identifier: AGPL-3.0-or-later
// dra/84B88.c:EntityBatFireball, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Animation/spell/artwork stay private.
#include <sotn/bat_fireball.h>

static Sint32 Signed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
bool SotnBatFireball_Tick(SotnBatFireball *fireball, SotnBatFireballInput in,
                          SotnBatFireballEvents *events)
{
    if (!fireball || !events || fireball->removed ||
        (!fireball->initialized && (in.player_priority < 2 || in.player_priority > 511)) ||
        (fireball->initialized &&
         (fireball->bank != 9 || fireball->priority > 509 || fireball->width != 4 ||
          fireball->height != 8 || fireball->scale_x < 64 || fireball->scale_x > 384 ||
          fireball->scale_y < 64 || fireball->scale_y > 256 ||
          fireball->velocity_x != (fireball->facing_left ? 229376 : -229376))))
        return SDL_SetError(
            "Bat fireball requires an actual bounded live child and birth priority");
    SotnBatFireball next = *fireball;
    SotnBatFireballEvents out = {0};
    if (!next.initialized)
    {
        // Both original parameter branches ultimately overwrite facing from the player.
        next.facing_left = !in.player_facing_left;
        next.velocity_x = next.facing_left ? 229376 : -229376;
        next.x = Signed((Uint32)next.x + (Uint32)next.velocity_x * 2U);
        next.y = Signed((Uint32)next.y - 4U * 65536U);
        next.priority = (Uint16)(in.player_priority - 2);
        next.bank = 9;
        next.scale_x = next.scale_y = 64;
        next.width = 4;
        next.height = 8;
        next.initialized = true;
        out = (SotnBatFireballEvents){
            .sound = 0x680, .spell = 9, .player_use_ticks = 4, .initialize = true};
    }
    else if (in.hit_flags)
        next.removed = true;
    else
    {
        next.x = Signed((Uint32)next.x + (Uint32)next.velocity_x);
        next.scale_x = (Uint16)SDL_min(384, next.scale_x + 16);
        next.scale_y = (Uint16)SDL_min(256, next.scale_y + 16);
    }
    *fireball = next;
    *events = out;
    return true;
}
