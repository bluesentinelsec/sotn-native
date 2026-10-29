// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/71830.c:PlayerStepHellfire, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No game data is embedded.
#include <sotn/hellfire_cast.h>
#include <sotn/player.h>
static void Action(SotnHellfireCastEvents *e, SotnHellfireCastActionKind kind, Uint16 value)
{
    e->actions[e->count++] = (SotnHellfireCastAction){kind, value};
}
static bool Direction(SotnHellfireCast *c, SotnHellfireCastInput in, SotnHellfireCastEvents *e)
{
    if (in.movement_flags & 2U)
        return false;
    if (in.buttons & SOTN_BUTTON_RIGHT)
    {
        e->turned |= c->facing_left;
        c->facing_left = false;
        return true;
    }
    if (in.buttons & SOTN_BUTTON_LEFT)
    {
        e->turned |= !c->facing_left;
        c->facing_left = true;
        return true;
    }
    return false;
}
static void Normal(SotnHellfireCast *c, SotnHellfireCastInput in, SotnHellfireCastEvents *e)
{
    Action(e, SOTN_HELLFIRE_NORMAL, 0);
    c->velocity_x = 0;
    c->phase = in.normal_phase;
    c->finished = true;
}
bool SotnHellfireCast_Tick(SotnHellfireCast *cast, SotnHellfireCastInput in,
                           SotnHellfireCastEvents *out)
{
    if (!cast || !out || cast->finished || cast->phase > 4 ||
        (in.normal_phase != 0 && in.normal_phase != 2))
        return SDL_SetError("Hellfire casting requires its actual live controller/helper state");
    SotnHellfireCast c = *cast;
    SotnHellfireCastEvents e = {0};
    bool finish = false;
    switch (c.phase)
    {
    case 0:
        Action(&e, SOTN_HELLFIRE_FACTORY, 33);
        if (!in.root_created)
            Normal(&c, in, &e);
        Action(&e, SOTN_HELLFIRE_ANIMATION, 1);
        ++c.phase;
        break;
    case 1:
        if (in.signal == 1)
            ++c.phase;
        break;
    case 2:
        c.velocity_x = 0;
        if (Direction(&c, in, &e))
        {
            const bool right = (in.buttons & SOTN_BUTTON_RIGHT) != 0;
            if (((right ? in.floor_right : in.floor_left) & 0x8001U) ||
                (in.floor_center & 0x8000U) || (right ? in.x > 248 : in.x < 8))
                c.velocity_x = c.facing_left ? -196608 : 196608;
        }
        if (in.signal == 2)
        {
            c.velocity_x = 0;
            Action(&e, SOTN_HELLFIRE_ANIMATION, 0x3d);
            ++c.phase;
        }
        break;
    case 3:
        Direction(&c, in, &e);
        if (in.signal == 3)
        {
            Action(&e, SOTN_HELLFIRE_ANIMATION, 0x3c);
            ++c.phase;
            finish = (in.buttons & SOTN_BUTTON_DOWN) != 0;
        }
        break;
    case 4:
        if (in.pose == 10 && in.remaining == 1)
        {
            Action(&e, SOTN_HELLFIRE_USE_TIMER, 4);
            Action(&e, SOTN_HELLFIRE_FACTORY, 37);
        }
        finish = in.remaining < 0;
        break;
    }
    if (finish || in.signal == 0xffffU)
    {
        Normal(&c, in, &e);
        Action(&e, SOTN_HELLFIRE_ANIMATION, 0x3d);
        Action(&e, SOTN_HELLFIRE_RESTORE_FLASHES, 0);
    }
    *cast = c;
    *out = e;
    return true;
}
