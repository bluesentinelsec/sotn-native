// SPDX-License-Identifier: AGPL-3.0-or-later
// Ordinary EntityWeaponAttack in weapon/w_052.c and alu_anim.c:PlayAnimation,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. All tables remain private.
#include <SDL3/SDL.h>
#include <sotn/shield.h>
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool View(SotnDataView overlay, size_t offset, size_t size, const Uint8 **out)
{
    if (overlay.bytes == NULL || offset > overlay.size || size > overlay.size - offset)
        return SDL_SetError("Shield animation references truncated private weapon data");
    *out = overlay.bytes + offset;
    return true;
}
static bool Frame(SotnDataView overlay, Uint16 animation, Uint16 pose, Uint16 *time, Uint16 *packed)
{
    const Uint8 *pointer = NULL, *record = NULL;
    if (animation >= 14 || pose >= 64 || !View(overlay, 0xb84 + animation * 4U, 4, &pointer))
        return SDL_SetError("Shield animation or pose exceeds its bounded table");
    const Uint32 address = Read32(pointer);
    if (address < 0x8017a000 || address % 4 != 0 ||
        !View(overlay, (size_t)(address - 0x8017a000) + pose * 4U, 4, &record))
        return SDL_SetError("Shield frame has an invalid private animation address");
    *time = Read16(record);
    *packed = Read16(record + 2);
    return true;
}
static Uint16 Movement(Uint16 animation, bool crouched, bool *follow)
{
    *follow = true;
    switch (animation)
    {
    case 9:
    case 10:
    case 11:
        return 9;
    case 7:
        return 8;
    case 8:
        return 7;
    case 12:
        return 6;
    case 13:
        return 5;
    case 24:
    case 25:
        return 4;
    case 14:
    case 15:
        return 3;
    case 26:
        return 2;
    default:
        *follow = false;
        return crouched ? 1 : 0;
    }
}
bool SotnShield_Tick(SotnDataView weapons, Uint16 player_animation, Uint16 player_pose,
                     bool crouched, bool held, bool disabled, SotnShield *cursor)
{
    if (cursor == NULL || cursor->phase > 3 || cursor->remaining < -1 ||
        cursor->remaining > 32767 || player_pose >= 4096)
        return SDL_SetError("Shield requires an initialized bounded animation cursor");
    if (disabled || (!held && cursor->phase == 0))
    {
        *cursor = (SotnShield){0};
        return true;
    }
    const SotnEquipment equipment = {.id = 16, .family = 52};
    SotnDataView overlay = {0};
    if (!SotnWeapon_Overlay(weapons, equipment, &overlay))
        return false;
    SotnShield next = *cursor;
    if (!held && next.phase < 3)
    {
        next.phase = 3;
        next.pose = 0;
        next.remaining = 0;
    }
    if (next.phase == 0)
        next.phase = 1;
    if (next.phase == 1 && next.remaining < 0)
        next.phase = 2;
    if (next.phase == 3 && next.remaining < 0)
    {
        *cursor = (SotnShield){0};
        return true;
    }
    if (next.phase == 2)
    {
        bool follow = false;
        next.animation = Movement(player_animation, crouched, &follow);
        next.pose = follow ? player_pose : 0;
        next.remaining = 2;
    }
    else
        next.animation = (Uint16)((next.phase == 1 ? 10 : 12) + (crouched ? 1 : 0));
    Uint16 duration = 0, packed = 0;
    if (next.remaining > 0)
        --next.remaining;
    if (next.remaining == 0)
    {
        if (cursor->remaining > 0 && next.phase == cursor->phase)
            ++next.pose;
        if (!Frame(overlay, next.animation, next.pose, &duration, &packed))
            return false;
        if (duration == 0)
        {
            next.pose = packed;
            if (!Frame(overlay, next.animation, next.pose, &duration, &packed))
                return false;
        }
        if (duration == 0xffff)
        {
            if (next.pose == 0)
                return SDL_SetError("Shield animation ends without a drawable frame");
            --next.pose;
            next.remaining = -1;
        }
        else if (duration == 0 || duration > 32767)
            return SDL_SetError("Shield animation resolves to an invalid duration");
        else
            next.remaining = duration;
    }
    if (!Frame(overlay, next.animation, next.pose, &duration, &packed))
        return false;
    const Uint16 property = (packed >> 9) & 127;
    const Uint8 *box = NULL;
    if (property >= 18 || !View(overlay, 0xb3c + property * 4U, 4, &box))
        return SDL_SetError("Shield hitbox exceeds its private frame-property table");
    next.frame = (SotnWeaponFrame){.x = (Sint16)(box[0] < 128 ? box[0] : (Sint32)box[0] - 256),
                                   .y = (Sint16)(box[1] < 128 ? box[1] : (Sint32)box[1] - 256),
                                   .width = box[2],
                                   .height = box[3],
                                   .sprite = packed & 511};
    *cursor = next;
    return true;
}
