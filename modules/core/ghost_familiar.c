// Ghost familiar (tt_001): src/servant/tt_001/ghost.c and the shared servant helpers
// (set_entity_animation.h, check_entity_valid.h, calculate_angle_to_entity.h,
// step_angle_towards.h, servant_unk0.h, create_event_entity.h, destroy_servant_entity.h) plus
// DRA alu_anim.c:UpdateAnim, sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5,
// AGPL-3.0-or-later.
#include <sotn/game_math.h>
#include <sotn/ghost_familiar.h>

static const Uint32 BASE = 0x80170000U;
static const Uint32 SERVANT_CLUT = 0x80170448U; // g_ServantClut
static const Uint32 GHOST_CLUT = 0x80170580U;   // g_GhostClut
static const Uint32 FRAMES = 0x801704a8U;       // g_DefaultGhostAnimationFrame
static const Uint32 FRAME_TABLE = 0x80170500U;  // g_GhostAnimationFrames
static const Uint32 ABILITIES = 0x80170508U;    // g_GhostAbilityStats
static const Uint32 OFFSETS_X = 0x801705a0U;    // g_ConfusedOffsetsX
static const Uint32 OFFSETS_Y = 0x801705a8U;    // g_ConfusedOffsetsY (word-aligned)
static const Uint32 CAMERA_LOCKED = 0x08000000U;
static const Uint32 KEEP_ALIVE = 0x04000000U;
static const Uint32 HAS_PRIMS = 0x00800000U;
static const Uint32 RETAINED = 0x00020000U;    // FLAG_UNK_20000: survives room entry
static const Uint32 STATUS_LOST = 0x01000001U; // PLAYER_STATUS_BAT_FORM | AXEARMOR

static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Long(Uint32 value)
{
    return value <= SDL_MAX_SINT32 ? (Sint32)value : (Sint32)((Sint64)value - 4294967296LL);
}
static Sint16 Pixel(Sint32 value)
{
    return Word((Sint32)(((Sint64)value - (Sint64)(Uint16)(Uint32)value) / 65536));
}
static Sint32 Fix(Sint32 pixels)
{
    return Long((Uint32)pixels << 16);
}
// posX.i.hi = value, keeping the low fraction word.
static Sint32 SetPixel(Sint32 position, Sint32 pixel)
{
    return Long(((Uint32)(Uint16)(Uint32)pixel << 16) | ((Uint32)position & 0xffffU));
}
static bool View(SotnDataView o, Uint32 address, Uint32 size, const Uint8 **out)
{
    if (!o.bytes || address < BASE || address - BASE > o.size || o.size - (address - BASE) < size)
        return SDL_SetError("Ghost familiar reads outside its private overlay");
    *out = o.bytes + (address - BASE);
    return true;
}
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return (Uint32)p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static bool Trig(SotnDataView exe, Sint32 angle, Sint32 *sine, Sint32 *cosine)
{
    SotnMathTrig t = {0};
    if (!SotnMath_Trig(exe, angle, &t))
        return false;
    if (sine)
        *sine = t.sine;
    if (cosine)
        *cosine = t.cosine;
    return true;
}
// One original statsFamiliars record: level = EXP/100 + 1, EXP0..9899, loads0..9999.
static bool Stats(SotnFamiliarStats s)
{
    return s.experience >= 0 && s.experience <= 9899 && s.level == s.experience / 100 + 1 &&
           s.loads >= 0 && s.loads <= 9999;
}
// g_GhostAbilityStats[level / 10]: delayFrames, spellId, makeBadAttacks.
static bool Ability(SotnDataView o, Sint32 level, Sint32 out[3])
{
    const Uint8 *p = NULL;
    if (level < 1 || level > 99 || !View(o, ABILITIES + (Uint32)(level / 10) * 12U, 12, &p))
        return SDL_SetError("Ghost familiar ability row requires level1..99 and its private table");
    for (Uint16 i = 0; i < 3; ++i)
        out[i] = Long(Read32(p + i * 4U));
    return true;
}
// One overlay halfword, with the live ProcessEvent timer/next words the original's tables hold.
static bool OverlayHalf(SotnDataView o, const SotnFamiliarEvents *events,
                        SotnFamiliarEventSource source, Uint32 address, Uint16 *out)
{
    const Uint8 *p = NULL;
    if (!View(o, address, 2, &p))
        return false;
    *out = Read16(p);
    const Uint32 span = 48U * source.count;
    if (address >= source.table && address - source.table < span)
    {
        const Uint32 record = (address - source.table) / 48U,
                     offset = (address - source.table) % 48U;
        if (offset < 8)
        {
            const Uint32 word = offset < 4 ? events->timer[record]
                                : events->next[record]
                                    ? source.table + (Uint32)events->next[record] * 48U
                                    : 0;
            *out = (Uint16)(word >> ((offset & 2U) * 8U));
        }
    }
    return true;
}
static bool Clut(SotnDataView o, const SotnFamiliarEvents *events, SotnFamiliarEventSource source,
                 Uint16 out[SOTN_GHOST_CLUT_WORDS])
{
    // The original copies 256 words from the 48-word g_ServantClut and 32 from the 16-word
    // g_GhostClut, reading the overlay data that follows each table.
    for (Uint32 i = 0; i < SOTN_GHOST_CLUT_WORDS; ++i)
    {
        if (!OverlayHalf(o, events, source, SERVANT_CLUT + i * 2U, &out[i]))
            return false;
        if (i % 16U)
            out[i] |= 0x8000U;
    }
    for (Uint32 i = 0; i < 32; ++i)
        if (!OverlayHalf(o, events, source, GHOST_CLUT + i * 2U, &out[0x30 + i]))
            return false;
    return true;
}
// ServantUnk0: which side of a flag20 room the familiar takes.
static Uint16 Side(SotnGhostInput in)
{
    const bool castle2 = in.stage >= 0x20 && in.stage < 0x35;
    if (in.room_flags == 0x22)
        return castle2 ? 1 : 0;
    if (in.room_flags == 0x20)
        return castle2 ? 0 : 1;
    return 2;
}
static void Birth(SotnGhostRuntime *r, Uint16 index, const SotnGhostSlot *parent, Uint16 identity,
                  Uint16 parameter)
{
    SDL_memset(r->rows[index - 1U], 0, sizeof(r->rows[0]));
    r->slots[index] = (SotnGhostSlot){.generation = ++r->generation,
                                      .identity = identity,
                                      .parameter = parameter,
                                      .priority = parent->priority,
                                      .facing_left = parent->facing_left,
                                      .flags = KEEP_ALIVE,
                                      .x = parent->x,
                                      .y = parent->y};
}
static void Retire(SotnGhostRuntime *r, Uint16 index)
{
    SDL_zero(r->slots[index]);
    if (index)
        SDL_memset(r->rows[index - 1U], 0, sizeof(r->rows[0]));
}
// CreateChildEntity: the first empty slot5..7 (zero, the original NULL, when all are taken).
static Uint16 Child(SotnGhostRuntime *r, Uint16 identity)
{
    for (Uint16 i = 1; i < 4; ++i)
        if (!r->slots[i].identity)
        {
            Birth(r, i, &r->slots[0], identity, 0);
            return (Uint16)(i + 4U);
        }
    return 0;
}

bool SotnGhostRuntime_Init(SotnGhostRuntime *runtime, SotnGhostInput in, Uint16 mode,
                           SotnGhostEvents *out)
{
    if (!runtime || !out || mode > 3 || !Stats(in.stats))
        return SDL_SetError("Ghost familiar init requires its actual owner, mode and stats");
    SotnGhostRuntime r = *runtime;
    SotnGhostEvents e = {0};
    if (!r.initialized)
    {
        if (!SotnFamiliarEvents_Init(&r.events, in.event_source))
            return false;
        r.initialized = true;
    }
    if (mode == 3)
    {
        *runtime = r;
        *out = e;
        return true;
    }
    if (!Clut(in.overlay, &r.events, in.event_source, e.clut))
        return false;
    e.clut_upload = true;
    SotnGhostSlot *s = &r.slots[0];
    *s = (SotnGhostSlot){.generation = ++r.generation,
                         .identity = SOTN_GHOST_DEFAULT,
                         .priority = (Uint16)(in.player.priority - 2U),
                         .facing_left = !in.player.facing_left};
    if (mode == 1)
    {
        s->x = Fix(128);
        s->y = Fix(-32);
    }
    else if (in.room_flags & 0x20U)
    {
        s->x = Fix(Side(in) ? 192 : 64);
        s->y = Fix(160);
    }
    else
    {
        s->x = Long((Uint32)in.player.x + (Uint32)Fix(in.player.facing_left ? 18 : -18));
        s->y = Long((Uint32)in.player.y - (Uint32)Fix(32));
    }
    r.stats = in.stats;
    r.destroyed = false;
    *runtime = r;
    *out = e;
    return true;
}

// UpdateEntityVelocityTowardsTarget: turn toward the target and set a distance-scaled velocity.
static bool Velocity(SotnGhostSlot *s, SotnGhostInput in, Sint32 tx, Sint32 ty, Sint32 *out)
{
    const Sint16 dx = Word(tx - Pixel(s->x)), dy = Word(ty - Pixel(s->y));
    Sint16 angle = 0;
    if (!SotnMath_Atan2(in.executable, -(Sint32)dy, dx, &angle))
        return false;
    angle = (Sint16)(angle & 0xfff);
    s->target_angle = SotnFairy_StepAngle(angle, s->target_angle, s->max_angle);
    const Sint16 turn = s->target_angle;
    const Uint32 square = ((Uint32)((Sint32)dx * dx) + (Uint32)((Sint32)dy * dy)) << 12;
    Uint32 root = 0;
    if (square > (Uint32)SDL_MAX_SINT32 || !SotnMath_Sqrt12(in.executable, square, &root))
        return SDL_SetError("Ghost familiar distance exceeds SquareRoot12's original range");
    const Sint32 distance = Word(Long(root) >> 12);
    Sint32 sine = 0, cosine = 0;
    if (!Trig(in.executable, turn, &sine, &cosine))
        return false;
    const Sint64 x = (Sint64)cosine * distance, y = (Sint64)sine * distance;
    switch (s->step)
    {
    case 2:
        s->velocity_x = Long((Uint32)(x * 2));
        s->velocity_y = Long(0U - (Uint32)(y * 2));
        break;
    case 3:
        s->velocity_x = Long((Uint32)(x * 8));
        s->velocity_y = Long(0U - (Uint32)(y * 8));
        break;
    default:
        s->velocity_x = (Sint32)(x >> 2);
        s->velocity_y = (Sint32) - (y >> 2);
        break;
    }
    if (s->velocity_x > 0 && s->velocity_x < 0x4000)
        s->velocity_x = 0x4000;
    else if (s->velocity_x < 0 && s->velocity_x > -0x4000)
        s->velocity_x = -0x4000;
    if (s->velocity_y > 0 && s->velocity_y < 0x4000)
        s->velocity_y = 0x4000;
    else if (s->velocity_y < 0 && s->velocity_y > -0x4000)
        s->velocity_y = -0x4000;
    if (distance > 0x100)
    {
        s->velocity_x = Long((Uint32)(tx - Pixel(s->x)) << 14);
        s->velocity_y = Long((Uint32)(ty - Pixel(s->y)) << 14);
    }
    *out = distance;
    return true;
}
// FindValidTarget: the first unmarked nearby target in front (marking it), else the next marked
// one after the round-robin cursor, else none.
static bool Search(SotnGhostRuntime *r, const SotnGhostSlot *s, SotnGhostInput in, bool bad,
                   SotnGhostEvents *e, Uint16 *out)
{
    if (!in.targets)
        return SDL_SetError("Ghost familiar target search requires the actual stage snapshot");
    bool marked[SOTN_SWORD_TARGETS] = {0};
    Uint16 count = 0;
    const Sint32 x = Pixel(s->x), y = Pixel(s->y);
    for (Uint16 i = 0; i < SOTN_SWORD_TARGETS; ++i)
    {
        const SotnSwordTarget *t = &in.targets[i];
        if (!t->id || !t->hitbox_state || (t->flags & 0x200000U) || t->x < -16 || t->x > 272 ||
            t->y > 240 || t->y < 0 || ((t->hitbox_state & 8U) && !bad) || SDL_abs(x - t->x) >= 49 ||
            SDL_abs(y - t->y) >= 49 || (!s->facing_left && x < t->x) ||
            (s->facing_left && x > t->x) || t->hit_points >= 0x7000)
            continue;
        if (t->flags & 0x80000U)
        {
            marked[i] = true;
            ++count;
        }
        else
        {
            e->mark = (Uint16)(i + 1U);
            *out = (Uint16)(i + 1U);
            return true;
        }
    }
    *out = 0;
    if (count)
    {
        Sint32 index = r->last_target % SOTN_SWORD_TARGETS;
        for (Uint16 i = 0; i < SOTN_SWORD_TARGETS; ++i)
        {
            if (index < 0)
                return SDL_SetError("Ghost familiar target cursor is negative");
            if (marked[index])
            {
                r->last_target = (index + 1) % SOTN_SWORD_TARGETS;
                *out = (Uint16)(index + 1);
                return true;
            }
            index = (index + 1) % SOTN_SWORD_TARGETS;
        }
    }
    return true;
}
// CheckEntityValid (US): hitbox, on screen and 0 < HP < 0x7000; the raw slot keeps no identity.
static bool Valid(SotnGhostInput in, Uint16 target, bool *out)
{
    if (!target || target > SOTN_SWORD_TARGETS || !in.targets)
        return SDL_SetError("Ghost familiar attack lost its original target pointer");
    const SotnSwordTarget *t = &in.targets[target - 1U];
    *out = t->hitbox_state && t->x >= -16 && t->x <= 272 && t->y <= 240 && t->y >= 0 &&
           t->hit_points < 0x7000 && t->hit_points > 0;
    return true;
}
static bool RoomEvents(SotnGhostRuntime *r, SotnGhostInput in)
{
    SotnFamiliarEventPlan plan = {0};
    in.room.occupied = 0;
    for (Uint16 i = 1; i < 4; ++i)
        if (r->slots[i].identity)
            in.room.occupied |= (Uint8)(1U << (i - 1U));
    if (!SotnFamiliarEvents_Tick(&r->events, in.event_source, in.room, &plan))
        return false;
    for (Uint16 i = 0; i < plan.count; ++i)
    {
        const SotnFamiliarEventBirth b = plan.births[i];
        if (b.slot < 5 || b.slot > 7 || r->slots[b.slot - 4U].identity)
            return SDL_SetError("Ghost familiar room event lost its actual empty slot");
        // CreateEventEntity: parent position/priority/facing, KEEP_ALIVE and the parameter.
        Birth(r, (Uint16)(b.slot - 4U), &r->slots[0], b.entity, b.parameter);
    }
    return true;
}
// DRA UpdateAnim(NULL, g_GhostAnimationFrames) for the current entity.
static bool Animate(SotnGhostSlot *s, SotnDataView o)
{
    const Uint8 *p = NULL;
    if (!s->animation || !View(o, s->animation + (Uint32)s->pose * 4U, 4, &p))
        return SDL_SetError("Ghost familiar animation lost its private timeline");
    if (s->pose_timer == 0)
        s->pose_timer = Word(Read16(p));
    else if (s->pose_timer != -1 && --s->pose_timer == 0)
    {
        ++s->pose;
        if (!View(o, s->animation + (Uint32)s->pose * 4U, 4, &p))
            return false;
        const Uint16 duration = Read16(p);
        if (duration == 0)
            s->pose = Read16(p + 2);
        else if (duration == 0xffff)
        {
            --s->pose;
            s->pose_timer = -1;
        }
        else if (duration == 0xfffe)
        {
            const Uint8 *table = NULL;
            if (!View(o, FRAME_TABLE + (Uint32)Read16(p + 2) * 4U, 4, &table))
                return false;
            s->animation = Read32(table);
            s->pose = 0;
        }
        if (duration != 0xffff)
        {
            if (!View(o, s->animation + (Uint32)s->pose * 4U, 4, &p))
                return false;
            s->pose_timer = Word(Read16(p));
        }
    }
    if (!View(o, s->animation + (Uint32)s->pose * 4U, 4, &p))
        return false;
    s->frame = (Sint16)(Read16(p + 2) & 0x1ffU);
    return true;
}
static bool Cloud(SotnGhostRuntime *r, Uint16 index, SotnGhostInput in, SotnGhostEvents *e);
static bool Confused(SotnGhostRuntime *r, Uint16 index, SotnGhostInput in);
// A raw child link: the slot must still hold the expected identity (the NULL link reads PS1 low
// memory in the original, which never holds that identity).
static bool Holds(const SotnGhostRuntime *r, Uint16 link, Uint16 identity)
{
    return link >= 5 && link <= 7 && r->slots[link - 4U].identity == identity;
}
// UpdateServantDefault: follow the player with a bob, close in on targets, attack, or wander
// confused while the player is a bat or in Axe Armor.
static bool Default(SotnGhostRuntime *r, SotnGhostInput in, SotnGhostEvents *e)
{
    SotnGhostSlot *s = &r->slots[0];
    r->stats = in.stats;
    if (r->destroyed)
        s->priority = (Uint16)(in.player.priority - 2U);
    s->hitbox_width = s->hitbox_height = 0;
    Sint32 ability[3] = {0};
    if (!Ability(in.overlay, r->stats.level, ability))
        return false;
    const bool lost = (in.player.status & STATUS_LOST) != 0;
    if (s->step < 2)
    {
        if (in.room_flags & 0x20U)
        {
            switch (Side(in))
            {
            case 0:
                r->follow_x = 0x40;
                break;
            case 1:
                r->follow_x = 0xc0;
                break;
            default:
                r->follow_x = Pixel(s->x) > 0x80 ? 0xc0 : 0x40;
                break;
            }
            r->follow_y = 0xa0;
        }
        else
        {
            r->follow_x = Pixel(in.player.x) + (in.player.facing_left ? 18 : -18);
            r->follow_y = Pixel(in.player.y) - 32;
        }
    }
    Sint32 bob = 0;
    if (!Trig(in.executable, s->bob, &bob, NULL))
        return false;
    r->target_x = Word(r->follow_x);
    r->target_y = Word(r->follow_y + (bob >> 10));
    s->bob = (Sint16)((s->bob + 32) & 0xfff);
    Sint32 distance = 0;
    switch (s->step)
    {
    case 0:
        s->flags = CAMERA_LOCKED | KEEP_ALIVE | RETAINED;
        if (s->animation != FRAMES)
        {
            s->animation = FRAMES;
            s->pose = 0;
            s->pose_timer = 0;
        }
        s->max_angle = 512;
        s->glow = 128;
        s->glow_step = -1;
        ++s->step;
        break;
    case 1: {
        if (lost)
        {
            s->step = 4;
            s->confused_step = 0;
            break;
        }
        const Sint32 x = Pixel(s->x), tx = r->target_x;
        if (in.room_flags & 0x20U)
            s->facing_left = Pixel(in.player.x) >= x;
        else if (in.player.facing_left == s->facing_left)
        {
            if (SDL_abs(tx - x) <= 0 || (s->facing_left && tx < x) || (!s->facing_left && tx > x))
                s->facing_left = !in.player.facing_left;
        }
        else if ((s->facing_left && x - tx > 0x1f) || (!s->facing_left && tx - x > 0x1f))
            s->facing_left = in.player.facing_left;
        if (!Velocity(s, in, r->target_x, r->target_y, &distance))
            return false;
        s->x = Long((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = Long((Uint32)s->y + (Uint32)s->velocity_y);
        if (!in.cutscene)
        {
            if (!Search(r, s, in, ability[2] != 0, e, &s->target))
                return false;
            if (s->target)
                ++s->step;
        }
        break;
    }
    case 2:
    case 3: {
        if (lost)
        {
            s->step = 4;
            s->confused_step = 0;
            break;
        }
        if (in.cutscene)
            s->step = 1;
        bool valid = false;
        if (!Valid(in, s->target, &valid))
            return false;
        if (!valid)
        {
            s->step = 1;
            break;
        }
        r->target_x = in.targets[s->target - 1U].x;
        r->target_y = in.targets[s->target - 1U].y;
        if (s->velocity_x > 0)
            s->facing_left = true;
        if (s->velocity_x < 0)
            s->facing_left = false;
        if (!Velocity(s, in, r->target_x, r->target_y, &distance))
            return false;
        if (s->step == 2)
        {
            if (distance < 8)
            {
                s->frame_counter = 0;
                ++s->step;
            }
        }
        else if (distance < 8)
        {
            s->frame_counter = Word(s->frame_counter + 1);
            if (s->frame_counter == ability[0] - 30)
                s->attack = Child(r, SOTN_GHOST_CLOUD);
            else if (s->frame_counter > ability[0])
            {
                s->frame_counter = 0;
                // GetServantStats(self, spellId, 1): the native owner rolls the spell's attack.
                s->spell = (Uint16)ability[1];
                e->spell = s->spell;
                s->hitbox_width = s->hitbox_height = 8;
            }
        }
        else
        {
            s->frame_counter = 0;
            if (Holds(r, s->attack, SOTN_GHOST_CLOUD))
            {
                r->slots[s->attack - 4U].parameter = 1;
                if (!Cloud(r, (Uint16)(s->attack - 4U), in, e))
                    return false;
            }
        }
        s->x = Long((Uint32)s->x + (Uint32)s->velocity_x);
        s->y = Long((Uint32)s->y + (Uint32)s->velocity_y);
        break;
    }
    case 4:
        if (!lost)
        {
            if (Holds(r, s->confused, SOTN_GHOST_CONFUSED))
            {
                r->slots[s->confused - 4U].parameter = 1;
                if (!Confused(r, (Uint16)(s->confused - 4U), in))
                    return false;
            }
            s->step = 1;
            break;
        }
        if (!Velocity(s, in, r->target_x, r->target_y, &distance))
            return false;
        s->y = Long((Uint32)s->y + (Uint32)s->velocity_y);
        switch (s->confused_step)
        {
        case 0:
            s->frame_counter = 0;
            ++s->confused_step;
            break;
        case 1:
        case 6:
            s->frame_counter = Word(s->frame_counter + 1);
            if (s->frame_counter > 60)
            {
                s->frame_counter = 0;
                ++s->confused_step;
            }
            break;
        case 2:
        case 3:
        case 4:
        case 5:
            s->frame_counter = Word(s->frame_counter + 1);
            if (s->frame_counter == 1)
                s->facing_left = !s->facing_left;
            else if (s->frame_counter > 15)
            {
                s->frame_counter = 0;
                ++s->confused_step;
            }
            break;
        case 7:
            s->frame_counter = Word(s->frame_counter + 1);
            if (s->frame_counter == 1)
                s->confused = Child(r, SOTN_GHOST_CONFUSED);
            else if (s->frame_counter > 60)
            {
                s->frame_counter = 0;
                ++s->step;
            }
            break;
        default:
            break;
        }
        break;
    case 5:
        if (!lost)
        {
            s->step = 1;
            break;
        }
        if (s->velocity_y > -0x18000)
            s->velocity_y = Long((Uint32)s->velocity_y - 0x4000U);
        s->y = Long((Uint32)s->y + (Uint32)s->velocity_y);
        if (Pixel(s->y) < -0x10)
        {
            s->velocity_y = 0;
            s->x = Fix(128);
            s->y = Fix(-16);
            ++s->step;
        }
        break;
    case 6:
        if (!lost)
        {
            s->step = 1;
            break;
        }
        s->x = Fix(128);
        s->y = Fix(-16);
        break;
    default:
        return SDL_SetError("Ghost familiar left its original steps");
    }
    // The pulsing fade between 0x20 and 0x80 (ENTITY_OPACITY).
    s->glow = Word(s->glow + s->glow_step);
    if (!(s->glow > 0x20 && s->glow < 0x80))
        s->glow_step = Word(-(Sint32)s->glow_step);
    s->opacity = (Uint8)s->glow;
    return RoomEvents(r, in) && Animate(s, in.overlay);
}
// UpdateAttackEntities: eight sparkles spiralling into the ghost over thirty updates.
static bool Cloud(SotnGhostRuntime *r, Uint16 index, SotnGhostInput in, SotnGhostEvents *e)
{
    SotnGhostSlot *s = &r->slots[index];
    SotnPlayerEffectQuad *rows = r->rows[index - 1U];
    if (s->parameter)
    {
        Retire(r, index);
        return true;
    }
    if (s->step == 0)
    {
        if (!in.allocated)
        {
            Retire(r, index);
            return true;
        }
        s->flags = KEEP_ALIVE | HAS_PRIMS | RETAINED;
        for (Uint16 i = 0; i < SOTN_GHOST_CLOUD_ROWS; ++i)
        {
            SotnPlayerEffectQuad *q = &rows[i];
            *q = (SotnPlayerEffectQuad){.page = 0x1a,
                                        .palette = 0x143,
                                        .u = {0x78, 0x80, 0x78, 0x80},
                                        .v = {0x30, 0x30, 0x38, 0x38},
                                        .priority = (Uint16)(s->priority + 1U),
                                        .draw_mode = 0x100 | 0x20 | 0x10 | 0x04 | 0x01};
            SDL_memset(q->red, 0x80, sizeof(q->red));
            SDL_memset(q->green, 0x80, sizeof(q->green));
            SDL_memset(q->blue, 0x80, sizeof(q->blue));
        }
        r->cloud_timer = 30;
        r->cloud_angle = 0;
        if (e->sound)
            return SDL_SetError("Ghost familiar callback requested a second sound");
        e->sound = SOTN_GHOST_SOUND;
        s->step = 1;
    }
    if (s->step == 1)
    {
        r->cloud_angle = (r->cloud_angle + 50) & 0xfff;
        if (--r->cloud_timer < 0)
        {
            Retire(r, index);
            return true;
        }
    }
    const SotnGhostSlot *parent = &r->slots[0];
    s->x = SetPixel(s->x, Pixel(parent->x));
    s->y = SetPixel(s->y, Pixel(parent->y));
    for (Uint16 i = 0; i < SOTN_GHOST_CLOUD_ROWS; ++i)
    {
        Sint32 sine = 0, cosine = 0;
        if (!Trig(in.executable, r->cloud_angle + (i << 9), &sine, &cosine))
            return false;
        const Sint32 x = Pixel(s->x) + (Sint32)(((Sint64)cosine * r->cloud_timer) >> 12);
        const Sint32 y = Pixel(s->y) - (Sint32)(((Sint64)sine * r->cloud_timer) >> 12);
        SotnPlayerEffectQuad *q = &rows[i];
        q->x[0] = q->x[2] = Word(x - 4);
        q->x[1] = q->x[3] = Word(x + 4);
        q->y[0] = q->y[1] = Word(y - 4);
        q->y[2] = q->y[3] = Word(y + 4);
    }
    return true;
}
// UpdateConfusedEntities: three question marks appear over the ghost, then retire.
static bool Confused(SotnGhostRuntime *r, Uint16 index, SotnGhostInput in)
{
    SotnGhostSlot *s = &r->slots[index];
    SotnPlayerEffectQuad *rows = r->rows[index - 1U];
    if (s->parameter)
    {
        Retire(r, index);
        return true;
    }
    switch (s->step)
    {
    case 0:
        if (!in.allocated)
        {
            Retire(r, index);
            return true;
        }
        s->flags = CAMERA_LOCKED | KEEP_ALIVE | HAS_PRIMS | RETAINED;
        for (Uint16 i = 0; i < SOTN_GHOST_CONFUSED_ROWS; ++i)
            rows[i] = (SotnPlayerEffectQuad){.page = 0x1e,
                                             .palette = 0x143,
                                             .u = {0x78, 0x80, 0x78, 0x80},
                                             .v = {8, 8, 0x10, 0x10},
                                             .priority = (Uint16)(s->priority + 1U),
                                             .draw_mode = (Uint16)(0x102 | (i ? 8 : 0))};
        r->confused_counter = 0;
        s->step = 1;
        break;
    case 1:
    case 2:
        if (++r->confused_counter > 10)
        {
            rows[s->step].draw_mode &= 0xfff7U;
            r->confused_counter = 0;
            ++s->step;
        }
        break;
    case 3:
        if (++r->confused_counter > 40)
        {
            Retire(r, index);
            return true;
        }
        break;
    default:
        return SDL_SetError("Ghost familiar question marks left their original steps");
    }
    const SotnGhostSlot *parent = &r->slots[0];
    s->x = SetPixel(s->x, Pixel(parent->x));
    s->y = SetPixel(s->y, Pixel(parent->y));
    const Sint32 dx = Pixel(s->x), dy = Pixel(s->y);
    for (Uint16 i = 0; i < SOTN_GHOST_CONFUSED_ROWS; ++i)
    {
        const Uint8 *ox = NULL, *oy = NULL;
        if (!View(in.overlay, OFFSETS_X + i * 2U, 2, &ox) ||
            !View(in.overlay, OFFSETS_Y + i * 2U, 2, &oy))
            return false;
        const Sint32 offset_x = (Sint16)Read16(ox), offset_y = (Sint16)Read16(oy);
        SotnPlayerEffectQuad *q = &rows[i];
        if (!s->facing_left)
        {
            q->x[0] = q->x[2] = Word(dx + offset_x);
            q->x[1] = q->x[3] = Word(dx + offset_x + 8);
        }
        else
        {
            q->x[0] = q->x[2] = Word(dx - (offset_x + 8));
            q->x[1] = q->x[3] = Word(dx - offset_x);
        }
        q->y[0] = q->y[1] = Word(dy + offset_y);
        q->y[2] = q->y[3] = Word(dy + offset_y + 8);
    }
    return true;
}

bool SotnGhostRuntime_UpdateSlot(SotnGhostRuntime *runtime, Uint16 slot, SotnGhostInput in,
                                 SotnGhostEvents *out)
{
    if (!runtime || !out || slot < 4 || slot > 7 || !runtime->initialized || !Stats(in.stats))
        return SDL_SetError("Ghost familiar callback requires its actual owner and slot4..7");
    SotnGhostRuntime r = *runtime;
    SotnGhostEvents e = {0};
    const Uint16 index = (Uint16)(slot - 4U);
    switch (r.slots[index].identity)
    {
    case 0:
        break;
    case SOTN_GHOST_DEFAULT:
        if (index || !Default(&r, in, &e))
            return index ? SDL_SetError("Ghost familiar body left slot4") : false;
        break;
    case SOTN_GHOST_CLOUD:
        if (!index || !Cloud(&r, index, in, &e))
            return index ? false : SDL_SetError("Ghost familiar ring holds slot4");
        break;
    case SOTN_GHOST_CONFUSED:
        if (!index || !Confused(&r, index, in))
            return index ? false : SDL_SetError("Ghost familiar question marks hold slot4");
        break;
    case SOTN_GHOST_DESTROY:
        // DestroyServantEntity: event15 keeps the ghost just behind the player.
        if (r.slots[index].parameter == 15)
            r.destroyed = true;
        Retire(&r, index);
        break;
    default:
        return SDL_SetError("Ghost familiar entity %#x is not integrated", r.slots[index].identity);
    }
    *runtime = r;
    *out = e;
    return true;
}

static void Shift(Uint16 mode, Sint16 camera_x, Sint16 camera_y, Sint32 player_x, Sint32 player_y,
                  bool room, Sint32 *x, Sint32 *y)
{
    *x = *y = 0;
    if (!room && (mode & 2U))
    {
        *x = camera_x;
        *y = camera_y;
    }
    else if (mode & (room ? 0x100U : 0x200U))
    {
        *x = -Pixel(player_x);
        *y = -Pixel(player_y);
    }
}
static bool Rows(const SotnGhostSlot *s, Uint16 *count)
{
    *count = s->identity == SOTN_GHOST_CLOUD && s->step      ? SOTN_GHOST_CLOUD_ROWS
             : s->identity == SOTN_GHOST_CONFUSED && s->step ? SOTN_GHOST_CONFUSED_ROWS
                                                             : 0;
    return *count != 0;
}
bool SotnGhostRuntime_Translate(SotnGhostRuntime *runtime, Sint16 camera_x, Sint16 camera_y,
                                Sint32 player_x, Sint32 player_y, bool room)
{
    if (!runtime)
        return SDL_InvalidParamError("runtime");
    if (!runtime->initialized)
        return true;
    SotnGhostRuntime r = *runtime;
    for (Uint16 i = 0; i < 4; ++i)
    {
        SotnGhostSlot *s = &r.slots[i];
        if (!s->identity)
            continue;
        if (room && !(s->flags & RETAINED))
        {
            Retire(&r, i);
            continue;
        }
        if (!room && (s->flags & CAMERA_LOCKED))
        {
            s->x = Long((Uint32)s->x + (Uint32)(Sint32)camera_x * 65536U);
            s->y = Long((Uint32)s->y + (Uint32)(Sint32)camera_y * 65536U);
        }
        if (room || (s->flags & 0x40000U))
        {
            s->x = Long((Uint32)s->x - (Uint32)player_x);
            s->y = Long((Uint32)s->y - (Uint32)player_y);
        }
        Uint16 count = 0;
        if (!i || !Rows(s, &count))
            continue;
        for (Uint16 k = 0; k < count; ++k)
        {
            SotnPlayerEffectQuad *q = &r.rows[i - 1U][k];
            Sint32 dx = 0, dy = 0;
            Shift(q->draw_mode, camera_x, camera_y, player_x, player_y, room, &dx, &dy);
            for (Uint16 v = 0; v < 4; ++v)
            {
                q->x[v] = Word(q->x[v] + dx);
                q->y[v] = Word(q->y[v] + dy);
            }
        }
    }
    *runtime = r;
    return true;
}
Uint16 SotnGhostRuntime_RowRequest(const SotnGhostRuntime *runtime, Uint16 slot)
{
    if (!runtime || !runtime->initialized || slot < 5 || slot > 7)
        return 0;
    const SotnGhostSlot *s = &runtime->slots[slot - 4U];
    if (s->step || s->parameter)
        return 0;
    return s->identity == SOTN_GHOST_CLOUD      ? SOTN_GHOST_CLOUD_ROWS
           : s->identity == SOTN_GHOST_CONFUSED ? SOTN_GHOST_CONFUSED_ROWS
                                                : 0;
}
bool SotnGhostRuntime_Leases(const SotnGhostRuntime *runtime, SotnGhostLease out[3])
{
    if (!runtime || !out)
        return SDL_SetError("Ghost lease report requires its actual owner and three slots");
    SotnGhostLease leases[3] = {0};
    for (Uint16 i = 1; runtime->initialized && i < 4; ++i)
    {
        Uint16 count = 0;
        if (Rows(&runtime->slots[i], &count))
            leases[i - 1U] = (SotnGhostLease){.generation = runtime->slots[i].generation,
                                              .slot = (Uint16)(i + 4U),
                                              .count = count};
    }
    SDL_memcpy(out, leases, sizeof(leases));
    return true;
}
bool SotnGhostRuntime_Clear(SotnGhostRuntime *runtime)
{
    if (!runtime)
        return SDL_InvalidParamError("runtime");
    SDL_zero(runtime->slots);
    SDL_memset(runtime->rows, 0, sizeof(runtime->rows));
    return true;
}
