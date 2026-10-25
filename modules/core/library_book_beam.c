// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_spellbook_magic_tome.c shared attack and page debris.
#include "actors_internal.h"
static Actor *Owner(SotnActors *s, const Actor *a)
{
    if (a->enemy_state.owner_slot >= SOTN_STAGE_ACTOR_SLOTS)
        return NULL;
    Actor *root = &s->slots[a->enemy_state.owner_slot];
    return root->state.generation && root->state.generation == a->enemy_state.owner_generation &&
                   (root->state.id == 0x30 || root->state.id == 0x32)
               ? root
               : NULL;
}
static bool Debris(SotnActors *s, Actor *a)
{
    if (!a->state.step)
    {
        if (!ActorInitialize(s, a, 0x908))
            return false;
        a->state.frame = (Uint16)((ActorRandom(s) & 15U) + 1);
        const Sint16 angle =
            (Sint16)Wrap16((Sint32)a->state.rotation + (Sint32)ActorRandom(s) * 2 - 256);
        SotnMathTrig trig = {0};
        if (!SotnMath_Trig(s->executable, angle, &trig))
            return false;
        a->vx = (Sint32)trig.cosine * 32;
        a->vy = (Sint32)trig.sine * 32;
        return true;
    }
    if (a->state.step != 1)
        return SDL_SetError("Library book debris entered an invalid step");
    if (!ActorLibraryBookMove(a) || a->vy > SDL_MAX_SINT32 - 8192)
        return SDL_SetError("Library book debris exceeds bounded fixed-point gravity");
    a->vy += 8192;
    return true;
}
bool ActorLibraryBookBeam(SotnActors *s, Actor *a)
{
    if (a->state.id == 0x31)
        return Debris(s, a);
    if (a->state.parameters >= 3)
        return SDL_SetError("Library book beam exceeds its private recipe count");
    SotnDataView recipe = {0};
    if (!Slice(s->nz0, 0x316c + a->state.parameters * 16U, 16, &recipe))
        return false;
    const Sint32 radius = Signed16(recipe.bytes + 8), size = Signed16(recipe.bytes + 10);
    if (radius < 0 || radius > 128 || size < 0 || size > 128 || recipe.bytes[12] > 127)
        return SDL_SetError("Library book beam has an invalid private geometry recipe");
    if (!a->state.step)
    {
        const Uint32 address = Read32(recipe.bytes);
        if (address < 0x80180000 || address % 4 || !ActorInitialize(s, a, address - 0x80180000))
            return SDL_SetError("Library book beam references an invalid private initialization");
        a->state.hitbox_width = a->state.hitbox_height = (Uint16)size;
        return true;
    }
    Actor *root = Owner(s, a);
    if (!root)
    {
        a->remove = true;
        return true;
    }
    const bool dead = (root->state.flags & 0x100U) != 0;
    if (a->state.step == 1)
    {
        a->state.hitbox_state = 0;
        if (a->book.command)
        {
            if (!ActorLibrarySound(s, a, 0x78e))
                return false;
            SetStep(a, 2);
        }
        if (dead)
            a->remove = true;
        return true;
    }
    if (a->state.step != 2 || a->substep > 2)
        return SDL_SetError("Library book beam entered an invalid gameplay step");
    if (!a->substep)
    {
        a->state.x = Wrap16((Sint64)Pixel(root->state.x) + root->book.center_x / 2) * 65536;
        a->state.y = Wrap16((Sint64)Pixel(root->state.y) + root->book.center_y / 2) * 65536;
        Sint16 angle = 0;
        if (!SotnMath_Atan2(s->executable, root->book.center_y, root->book.center_x, &angle))
            return false;
        a->state.frame = recipe.bytes[12];
        a->state.rotation = (Sint16)Wrap16((Sint32)angle + Signed16(recipe.bytes + 4));
        a->state.rotated = a->state.scale_set = true;
        a->state.scale_x = a->state.scale_y = 0;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        a->state.priority = root->book.priority;
        a->state.scale_y = (Uint16)(a->state.scale_y + 8);
        a->state.scale_x = a->state.scale_y;
        if (a->state.scale_x >= 288)
            a->substep = 2;
        if (a->state.scale_x > 128)
        {
            a->state.hitbox_state = 1;
            SotnMathTrig trig = {0};
            if (!SotnMath_Trig(s->executable,
                               (Sint32)a->state.rotation - Signed16(recipe.bytes + 6), &trig))
                return false;
            a->state.hitbox_x =
                (Sint16)ActorShift((Sint64)radius * a->state.scale_x * trig.cosine, 20);
            a->state.hitbox_y =
                (Sint16)ActorShift((Sint64)radius * a->state.scale_x * trig.sine, 20);
        }
        if (dead)
        {
            a->state.hitbox_state = 0;
            a->substep = 2;
        }
    }
    else
    {
        a->state.hitbox_state = 0;
        const Sint32 scale = (Sint32)a->state.scale_y - 4 - (dead ? 16 : 0);
        if (scale < 0)
        {
            a->state.frame = 0;
            a->book.command = false;
            a->state.scale_x = a->state.scale_y = 0;
            SetStep(a, 1);
        }
        else
            a->state.scale_x = a->state.scale_y = (Uint16)scale;
    }
    return true;
}
