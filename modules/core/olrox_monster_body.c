// SPDX-License-Identifier: AGPL-3.0-or-later
// Reconstructed BO0 313A8.c:func_us_801B365C at pinned sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Profiles and artwork remain private.
#include "olrox_monster_internal.h"

static Sint16 Word(Sint32 n)
{
    return (Sint16)Wrap16(n);
}
static void Whole(Sint32 *p, Sint32 n)
{
    *p = (Sint32)((Sint64)Word(n) * 65536 + ((Uint32)*p & 65535U));
}
static Sint32 AbsoluteX(const Actor *a)
{
    const Sint16 x = Word(Pixel(a->state.x));
    return Word(a->state.facing_left ? 512 - x : x);
}
static bool Step(SotnActors *s, Actor *a, Uint16 step)
{
    return OlroxMonster_ChangeStep(s, a, step);
}
static bool Anchor(SotnActors *s, Actor *a)
{
    return OlroxMonster_Anchor(s, a, false);
}
static bool Pose(SotnActors *s, Actor *a, Uint16 leg, OlroxLegPose pose, bool *arrived)
{
    return OlroxMonster_Pose(s, a, leg, pose, arrived);
}
static bool Tilt(SotnActors *s, Actor *a, Sint16 target, Sint16 speed, bool *arrived)
{
    (void)s;
    return OlroxMonster_Smooth(&a->olrox.body_angle, &a->olrox.body_velocity, target, speed,
                               arrived);
}
static void ClearPhases(SotnActors *s)
{
    s->olrox_monster.legs[0].phase = s->olrox_monster.legs[1].phase = 0;
}
static bool Move(Actor *a)
{
    const Sint64 x = (Sint64)a->state.x + a->vx, y = (Sint64)a->state.y + a->vy,
                 vy = (Sint64)a->vy + 0x2800;
    if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32 ||
        vy < SDL_MIN_SINT32 || vy > SDL_MAX_SINT32)
        return SDL_SetError("Olrox monster motion exceeds fixed-point coordinates");
    a->state.x = (Sint32)x;
    a->state.y = (Sint32)y;
    a->vy = (Sint32)vy;
    return true;
}
static bool Reserved(SotnActors *s, Actor *a, Uint16 role, Uint16 id, Uint16 parameters,
                     Actor **out)
{
    if (!out || !role || role > 10 || (Uint32)a->slot + role >= SOTN_STAGE_ACTOR_SLOTS ||
        s->effect_serial >= 0x7fffffffU)
        return SDL_SetError("Olrox monster child exceeds its original reserved block");
    *out = NULL;
    const Uint16 slot = (Uint16)(a->slot + role);
    Actor *old = &s->slots[slot];
    SotnActorPlacement placement = {0};
    Uint32 placed = 0;
    if (!SotnRoomActors_Get(s->spawns, slot, &placement, &placed))
        return false;
    // Only the same morph lineage may hand off released humanoid columns to this body.
    Actor *morph = NULL;
    if (a->enemy_state.owner_generation && a->enemy_state.owner_slot < SOTN_STAGE_ACTOR_SLOTS)
    {
        Actor *candidate = &s->slots[a->enemy_state.owner_slot];
        if (candidate->state.id == 0x32 &&
            candidate->state.generation == a->enemy_state.owner_generation)
            morph = candidate;
    }
    const bool sibling = morph && morph->enemy_state.owner_generation &&
                         old->enemy_state.owner_slot == morph->enemy_state.owner_slot &&
                         old->enemy_state.owner_generation == morph->enemy_state.owner_generation &&
                         old->state.id >= 0x2d && old->state.id <= 0x31;
    if (placed || (old->state.generation && !OlroxChild(s, a, role) && !sibling))
        return SDL_SetError("Olrox monster child would replace an unrelated actor in slot %u",
                            slot);
    if (old->state.generation)
    {
        ActorPrimitivesClear(s, old);
        *old = (Actor){0};
    }
    if (!ActorSpawnAt(s, a, id, parameters, slot, out))
        return false;
    (*out)->state.x = Pixel(a->state.x) * 65536;
    (*out)->state.y = Pixel(a->state.y) * 65536;
    (*out)->state.facing_left = false;
    a->olrox.children[role] = (*out)->state.generation;
    return true;
}
static bool Initialize(SotnActors *s, Actor *a)
{
    if (!ActorInitialize(s, a, 0x708))
        return false;
    a->state.hitbox_width = 8;
    a->state.hitbox_height = 16;
    a->state.hitbox_y = -8;
    a->state.frame = 11;
    a->state.priority = s->olrox_monster.priority = 0xa0;
    Actor *child = NULL;
    const Uint16 roles[] = {1, 2, 3, 4, 7, 9, 8};
    const Uint16 ids[] = {0x34, 0x34, 0x35, 0x36, 0x37, 0x38, 0x38};
    for (Uint16 i = 0; i < SDL_arraysize(roles); ++i)
        if (!Reserved(s, a, roles[i], ids[i], roles[i] == 8 ? 1 : 0, &child))
            return false;
    if (!ActorPrimitives(s, a, 17))
    {
        SDL_ClearError();
        a->state.step = 0;
        return true;
    }
    a->state.flags |= 0x800000U;
    for (Uint16 i = 5; i <= 6; ++i)
        if (!Reserved(s, a, i, 0x3b, 0, &child))
            return false;
    s->olrox_monster.legs[0].upper = -512;
    s->olrox_monster.legs[0].lower = 768;
    s->olrox_monster.legs[1].upper = -896;
    s->olrox_monster.legs[1].lower = 256;
    a->olrox.support_leg = 1;
    return true;
}
static bool Entry(SotnActors *s, Actor *a)
{
    switch (a->substep)
    {
    case 0:
        a->state.priority = s->olrox_monster.priority = 0xd0;
        if (!Anchor(s, a))
            return false;
        ++a->substep;
        break;
    case 1: {
        Actor *foot = OlroxChild(s, a, 2);
        if (!foot)
            return SDL_SetError("Olrox entry lost its owned support foot");
        a->state.priority = s->olrox_monster.priority = 0xa0;
        Whole(&foot->state.x, 0);
        Whole(&foot->state.y, -32);
        if (!Anchor(s, a))
            return false;
        ++a->substep;
        break;
    }
    case 2:
        if (!s->olrox_morph.transforming)
        {
            for (Uint16 i = 0; i < 8; ++i)
            {
                Actor *part = i ? OlroxChild(s, a, i) : a;
                if (!part)
                    return SDL_SetError("Olrox entry lost an owned original part");
                part->state.flags |= 0x08000000U;
            }
            a->state.facing_left = a->state.parameters != 0;
            Actor *foot = OlroxChild(s, a, 2);
            foot->state.x =
                (Sint32)((Sint64)Pixel(a->state.x) * 65536 + ((Uint32)foot->state.x & 65535U));
            Whole(&foot->state.y, Pixel(a->state.y));
            if (!Anchor(s, a))
                return false;
            a->olrox.timer = 32;
            return Step(s, a, 12);
        }
        break;
    default:
        break;
    }
    return true;
}
static bool Decision(SotnActors *s, Actor *a)
{
    if (!Anchor(s, a))
        return false;
    const Sint16 dx = Word(Pixel(a->state.x) - Pixel(s->player.x)),
                 dy = Word(Pixel(a->state.y) - Pixel(s->player.y));
    const Sint32 x = AbsoluteX(a), distance = SDL_abs((Sint32)dx);
    const bool facing = dx < 1;
    Uint16 step = 10;
    if (a->olrox.decision)
    {
        a->olrox.decision = 0;
        if (distance < 128)
            step = x < 384 ? 4 : 3;
        else if (a->state.facing_left == facing)
            step = x >= 209 ? 3 : 4;
        else
            step = x < 256 ? 17 : 13;
    }
    else
    {
        a->olrox.decision = 1;
        if (a->state.facing_left != facing)
            step = 13;
        else if (distance < 96)
            step = dy > 0 ? 18 : ((ActorRandom(s) & 1U) ? 6 : 8);
        else
            step = dy >= 65 || (ActorRandom(s) & 1U) ? 9 : 8;
    }
    return Step(s, a, step);
}
static bool Walk(SotnActors *s, Actor *a, Uint16 kind)
{
    Actor *head = OlroxChild(s, a, 3), *tail = OlroxChild(s, a, 4), *arms = OlroxChild(s, a, 7);
    if (!head || !tail || !arms)
        return SDL_SetError("Olrox gait lost an owned command receiver");
    const bool backward = kind != 3;
    if (!a->substep)
    {
        const Sint32 x = AbsoluteX(a);
        if ((kind == 3 && x < 192) || (kind == 4 && x >= 385) || (kind == 14 && x >= 385))
            return Step(s, a, 10);
        arms->state.step = kind == 14 ? 15 : 2;
        tail->state.step = kind == 14 ? 9 : 3;
        head->state.step = kind == 14 ? 7 : 1;
        a->olrox.walk_count = 0;
        ++a->substep;
    }
    if (kind != 14 && AbsoluteX(a) < 384 && (head->hit_flags & 3U) >= 2)
        if (!Step(s, a, 14))
            return false;
    const Uint16 support = a->olrox.support_leg, swing = support ^ 1U;
    bool ignored = false;
    if (!Pose(s, a, support, backward ? OLROX_BACKWARD_SUPPORT : OLROX_FORWARD_SUPPORT, &ignored) ||
        !Pose(s, a, swing, backward ? OLROX_BACKWARD_SWING : OLROX_FORWARD_SWING, &ignored) ||
        !Anchor(s, a))
        return false;
    Sint16 shift = 0;
    if (!OlroxMonster_Floor(s, a, swing, &shift))
        return false;
    OlroxLeg *leg = &s->olrox_monster.legs[swing], *other = &s->olrox_monster.legs[support];
    if (leg->contact &&
        (backward ? leg->horizontal < other->horizontal : other->horizontal < leg->horizontal))
    {
        if (!ActorSound(s, 0x647))
            return false;
        ++a->olrox.walk_count;
        ClearPhases(s);
        Whole(&leg->foot_y, Pixel(leg->foot_y) + shift);
        const Sint32 x = AbsoluteX(a);
        const bool stop = kind == 14 ? x >= 385 : backward ? x >= 385 : x < 192;
        if (stop || a->olrox.walk_count >= 2)
        {
            if (kind == 14)
            {
                a->olrox.walk_count = 0;
                tail->state.step = 3;
            }
            return Step(s, a, 10);
        }
        a->olrox.support_leg ^= 1U;
    }
    return true;
}
static bool Jump(SotnActors *s, Actor *a)
{
    Uint16 support = a->olrox.support_leg, other = support ^ 1U;
    Actor *tail = OlroxChild(s, a, 4);
    if (!tail)
        return SDL_SetError("Olrox jump lost its owned tail");
    bool arrived = false;
    if (!a->substep)
    {
        support = 0;
        other = 1;
        a->olrox.support_leg =
            s->olrox_monster.legs[1].horizontal >= s->olrox_monster.legs[0].horizontal;
        if (!Anchor(s, a))
            return false;
        ++a->substep;
    }
    if (a->substep == 1)
    {
        if (!Anchor(s, a) || !Pose(s, a, support, OLROX_TAKEOFF, &arrived))
            return false;
        if (arrived)
        {
            ClearPhases(s);
            a->vx = a->state.facing_left ? -0x40000 : 0x40000;
            a->vy = -0x30000;
            tail->state.step = 8;
            ++a->substep;
        }
    }
    else if (a->substep == 2)
    {
        if (!Move(a) || !OlroxMonster_Pair(s, a, other, -768, -384, 40, &arrived) ||
            !OlroxMonster_Pair(s, a, support, -640, 384, 40, &arrived) ||
            !OlroxMonster_Anchor(s, a, true))
            return false;
        Sint16 shift = 0;
        if (!OlroxMonster_Floor(s, a, support, &shift))
            return false;
        OlroxLeg *leg = &s->olrox_monster.legs[support];
        if (leg->contact)
        {
            if (!ActorSound(s, 0x63d))
                return false;
            Whole(&leg->foot_y, Pixel(leg->foot_y) + shift);
            if (!Anchor(s, a))
                return false;
            tail->state.step = 3;
            ++a->substep;
            return Step(s, a, 3);
        }
    }
    return true;
}
static bool Swipe(SotnActors *s, Actor *a)
{
    Actor *head = OlroxChild(s, a, 3), *arms = OlroxChild(s, a, 7);
    if (!head || !arms)
        return SDL_SetError("Olrox swipe lost its owned head or arms");
    const Uint16 support = a->olrox.support_leg;
    bool arrived = false;
    if (!Anchor(s, a))
        return false;
    if (arms->olrox_arms.damaged)
        return Step(s, a, 10);
    switch (a->substep)
    {
    case 0:
        arms->state.step = 3;
        a->olrox.body_style = 2;
        head->state.step = 6;
        ++a->substep;
        a->olrox.support_leg =
            s->olrox_monster.legs[1].horizontal < s->olrox_monster.legs[0].horizontal;
        break;
    case 1:
    case 2:
        if (!Pose(s, a, support, a->substep == 1 ? OLROX_LOWER_BODY : OLROX_RAISE_BODY, &arrived))
            return false;
        if (arrived && arms->state.step == 1)
        {
            arms->state.step = 4;
            ClearPhases(s);
            ++a->substep;
        }
        break;
    case 3:
        arms->state.step = 2;
        head->state.step = 1;
        a->olrox.body_style = 0;
        return Step(s, a, 10);
    default:
        break;
    }
    return true;
}
static bool Slam(SotnActors *s, Actor *a)
{
    const Uint16 support = a->olrox.support_leg, other = support ^ 1U;
    Actor *head = OlroxChild(s, a, 3), *tail = OlroxChild(s, a, 4), *arms = OlroxChild(s, a, 7);
    if (!head || !tail || !arms)
        return SDL_SetError("Olrox slam lost its owned command receivers");
    bool arrived = false;
    OlroxLeg *leg = &s->olrox_monster.legs[support];
    switch (a->substep)
    {
    case 0:
        a->olrox.support_leg =
            s->olrox_monster.legs[1].horizontal >= s->olrox_monster.legs[0].horizontal;
        if (!Anchor(s, a))
            return false;
        arms->state.step = 11;
        a->olrox.body_style = 2;
        head->state.step = 10;
        ++a->substep;
        break;
    case 1:
    case 2:
        if (!Anchor(s, a))
            return false;
        if (arms->olrox_arms.damaged)
            return Step(s, a, 10);
        if (!Pose(s, a, support, a->substep == 1 ? OLROX_WIDE_STANCE : OLROX_LANDING, &arrived))
            return false;
        if (arrived && (a->substep == 2 || arms->state.step == 1))
        {
            ClearPhases(s);
            if (a->substep == 1)
            {
                arms->state.step = 12;
                tail->state.step = 6;
            }
            else
            {
                a->olrox.body_style = 3;
                a->vx = a->state.facing_left ? 65536 : -65536;
                const Sint32 x = AbsoluteX(a);
                if (x < 256)
                    a->vx /= 2;
                if (x < 192)
                    a->vx = 0;
                a->vy = -0x60000;
            }
            ++a->substep;
        }
        break;
    case 3:
        if (!Move(a) || !OlroxMonster_Anchor(s, a, true) || !Tilt(s, a, -256, 48, &arrived) ||
            !OlroxMonster_Pair(s, a, support, -128, 256, 16, &arrived) ||
            !OlroxMonster_Pair(s, a, other, -640, 640, 16, &arrived))
            return false;
        if (a->vy > -65536)
        {
            tail->state.step = 6;
            a->olrox.support_leg ^= 1U;
            ++a->substep;
        }
        if (head->state.rotation < 128)
            head->state.rotation = Word(head->state.rotation + 8);
        break;
    case 4: {
        if (!Move(a) || !Tilt(s, a, 256, 32, &arrived) ||
            !OlroxMonster_Pair(s, a, support, -384, -256, 24, &arrived) ||
            !OlroxMonster_Pair(s, a, other, -832, 704, 24, &arrived))
            return false;
        arms->state.step = 13;
        if (!OlroxMonster_Anchor(s, a, true))
            return false;
        Sint16 shift = 0;
        if (!OlroxMonster_Floor(s, a, support, &shift))
            return false;
        if (leg->contact)
        {
            if (!ActorSound(s, 0x63d))
                return false;
            arms->state.step = 14;
            Whole(&leg->foot_y, Pixel(leg->foot_y) + shift);
            a->olrox.body_style = 0;
            tail->state.step = 3;
            ++a->substep;
        }
        if (head->state.rotation >= -511)
            head->state.rotation = Word(head->state.rotation - 64);
        break;
    }
    case 5:
        if (!Pose(s, a, support, OLROX_CROUCH, &arrived))
            return false;
        if (arrived)
        {
            ClearPhases(s);
            ++a->substep;
        }
        if (!Anchor(s, a))
            return false;
        head->state.rotation = head->state.rotation > 0 ? Word(head->state.rotation - 256) : 0;
        break;
    case 6:
        arms->state.step = 2;
        a->olrox.body_style = 0;
        head->state.step = 1;
        return Step(s, a, 10);
    default:
        break;
    }
    return true;
}
static bool Rank(SotnActors *s, Actor *head, Uint8 *rank)
{
    SotnDataView api = {0}, definition = {0};
    if (!Slice(s->dra, 0x98, 4, &api))
        return false;
    const Uint32 address = Read32(api.bytes);
    if (address < 0x800a0000U || address % 4 ||
        !Slice(s->dra, (size_t)(address - 0x800a0000U) + 56U * 40U + 4U, 2, &definition))
        return false;
    const Sint32 max_hp = Signed16(definition.bytes);
    if (max_hp <= 0 || head->state.hit_points < 0 || head->state.hit_points > max_hp)
        return SDL_SetError("Olrox attack rank requires its original current/maximum head HP");
    *rank = (Uint8)(3 - head->state.hit_points / (max_hp / 4 + 1));
    return true;
}
static bool Laser(SotnActors *s, Actor *a)
{
    Actor *head = OlroxChild(s, a, 3), *arms = OlroxChild(s, a, 7);
    if (!head || !arms)
        return SDL_SetError("Olrox laser request lost its owned head or arms");
    bool arrived = false;
    if (!Anchor(s, a))
        return false;
    switch (a->substep)
    {
    case 0:
        arms->state.step = 6;
        head->state.step = 2;
        a->olrox.laser_pattern = a->olrox.laser_busy = 0;
        head->olrox.laser_pattern = 0;
        ++a->substep;
        if (!Rank(s, head, &a->olrox.laser_rank))
            return false;
        break;
    case 1:
    case 2: {
        SotnDataView target = {0};
        if (a->olrox.laser_pattern > 3 ||
            !Slice(s->nz0, (a->substep == 1 ? 0x11cc : 0x11ce) + a->olrox.laser_pattern * 8U, 2,
                   &target))
            return SDL_SetError("Olrox torso laser pose exceeds its original private table");
        if (!Tilt(s, a, (Sint16)Signed16(target.bytes), 16, &arrived))
            return false;
        if (a->substep == 1)
        {
            if (a->olrox.laser_busy)
                ++a->substep;
        }
        else
        {
            a->olrox.body_style = 0;
            if (!a->olrox.laser_busy)
            {
                ++a->olrox.laser_pattern;
                if ((a->olrox.laser_pattern & 1U) && AbsoluteX(a) < 248)
                    a->olrox.laser_pattern = 8;
                if (a->olrox.laser_pattern <= a->olrox.laser_rank)
                {
                    head->state.step = 2;
                    head->olrox.laser_busy = 0;
                    head->olrox.laser_pattern = a->olrox.laser_pattern;
                    --a->substep;
                }
                else
                    ++a->substep;
            }
        }
        break;
    }
    default:
        head->state.step = 1;
        arms->state.step = 2;
        return Step(s, a, 10);
    }
    return true;
}
static bool Fire(SotnActors *s, Actor *a)
{
    Actor *head = OlroxChild(s, a, 3), *arms = OlroxChild(s, a, 7);
    if (!head || !arms)
        return SDL_SetError("Olrox fire request lost its owned head or arms");
    if (!Anchor(s, a))
        return false;
    switch (a->substep)
    {
    case 0:
        a->olrox.fire_busy = 0;
        arms->state.step = 6;
        head->state.step = 4;
        head->substep = 0;
        head->olrox.fire_busy = 0;
        if (!Rank(s, head, &a->olrox.fire_rank))
            return false;
        head->olrox.fire_rank = a->olrox.fire_rank;
        ++a->substep;
        break;
    case 1:
        if (a->olrox.fire_busy)
            ++a->substep;
        break;
    case 2:
        if (!a->olrox.fire_busy)
            ++a->substep;
        break;
    default:
        head->state.step = 1;
        arms->state.step = 2;
        return Step(s, a, 10);
    }
    return true;
}
static bool Roar(SotnActors *s, Actor *a)
{
    const Uint16 support = a->olrox.support_leg, other = support ^ 1U;
    Actor *head = OlroxChild(s, a, 3), *arms = OlroxChild(s, a, 7);
    if (!head || !arms)
        return SDL_SetError("Olrox recovery lost its owned head or arms");
    bool arrived = false;
    if (!Anchor(s, a))
        return false;
    switch (a->substep)
    {
    case 0:
        arms->state.step = 7;
        ++a->substep;
        break;
    case 1: {
        if (!Pose(s, a, support, OLROX_RECOVER, &arrived) || !Anchor(s, a))
            return false;
        Sint16 shift = 0;
        if (!OlroxMonster_Floor(s, a, other, &shift))
            return false;
        OlroxLeg *leg = &s->olrox_monster.legs[other];
        if (leg->contact)
        {
            if (!ActorSound(s, 0x647))
                return false;
            Whole(&leg->foot_y, Pixel(leg->foot_y) + shift);
            ClearPhases(s);
            head->state.step = 5;
            a->olrox.roar_busy = 1;
            ++a->substep;
        }
        break;
    }
    case 2:
        if (!Tilt(s, a, -256, 16, &arrived))
            return false;
        if (!a->olrox.roar_busy)
            ++a->substep;
        break;
    default:
        head->state.step = 1;
        arms->state.step = 2;
        return Step(s, a, 10);
    }
    return true;
}
static bool Turn(SotnActors *s, Actor *a)
{
    const Uint16 support = a->olrox.support_leg, other = support ^ 1U;
    Actor *tail = OlroxChild(s, a, 4), *arms = OlroxChild(s, a, 7);
    if (!tail || !arms)
        return SDL_SetError("Olrox turn lost its owned tail or arms");
    bool arrived = false, first = false, second = false;
    if (!Anchor(s, a))
        return false;
    switch (a->substep)
    {
    case 0:
        arms->state.step = 8;
        tail->state.step = 4;
        ++a->substep;
        break;
    case 1:
        if (!Tilt(s, a, -256, 48, &arrived) ||
            !OlroxMonster_Pair(s, a, support, -64, 64, 32, &first) ||
            !OlroxMonster_Pair(s, a, other, -256, 256, 32, &second))
            return false;
        if (arrived && first && second)
        {
            OlroxLeg *leg = &s->olrox_monster.legs[support];
            Whole(&leg->foot_x, Pixel(leg->foot_x) + (a->state.facing_left ? -48 : 48));
            if (!Anchor(s, a))
                return false;
            a->state.facing_left = !a->state.facing_left;
            ++a->substep;
        }
        break;
    case 2:
        arms->state.step = 2;
        tail->state.step = 3;
        return Step(s, a, 3);
    default:
        break;
    }
    return true;
}
static bool Geometry(SotnActors *s, Actor *a)
{
    if (!OlroxMonster_LegGeometry(s, a, 0, 2, 0) || !OlroxMonster_LegGeometry(s, a, 1, 6, 1) ||
        !OlroxMonster_BodyGeometry(s, a, 10))
        return false;
    for (Uint16 i = 2; i < a->state.primitive_count; ++i)
    {
        ActorPrimitive *p = ActorPrimitiveGet(s, a, i);
        if (!p)
            return false;
        if (s->olrox_morph.transforming)
            p->state.blend = false;
        if (a->state.palette & 0x8000U)
            p->state.palette = a->state.palette & 0xfffU;
        if (a->olrox.death_phase)
        {
            p->state.page = 0x15;
            p->state.palette = 0x219;
        }
    }
    return true;
}
bool ActorOlroxMonster(SotnActors *s, Actor *a)
{
    if (!OlroxMonster_Bind(s, a))
        return false;
    if (a->olrox.support_leg > 1)
        return SDL_SetError("Olrox body has an invalid support-leg role");
    if (!a->olrox.death_phase && (a->state.flags & 0x100U))
    {
        a->state.hitbox_state = 0;
        if (a->state.step < 16 && !Step(s, a, 16))
            return false;
    }
    if (!a->state.step)
    {
        if (!Initialize(s, a))
            return false;
        if (!a->state.step)
            return true;
    }
    bool ok = true;
    switch (a->state.step)
    {
    case 1:
        ok = Entry(s, a);
        break;
    case 2:
        if (!--a->olrox.timer)
            ok = Step(s, a, 10);
        if (ok)
            ok = Anchor(s, a);
        break;
    case 10:
        ok = Decision(s, a);
        break;
    case 3:
    case 4:
    case 14:
        ok = Walk(s, a, a->state.step);
        break;
    case 17:
        ok = Jump(s, a);
        break;
    case 6:
        ok = Swipe(s, a);
        break;
    case 18:
        ok = Slam(s, a);
        break;
    case 8:
        ok = Laser(s, a);
        break;
    case 9:
        ok = Fire(s, a);
        break;
    case 12:
        ok = Roar(s, a);
        break;
    case 13:
        ok = Turn(s, a);
        break;
    case 16:
        return SDL_SetError("Olrox monster defeat remains outside this body component");
    default:
        break;
    }
    return ok && Geometry(s, a);
}
