// SPDX-License-Identifier: AGPL-3.0-or-later
// servant/tt_002/faerie.c and common servant movement helpers, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Original tables remain private.
#include <sotn/combat.h>
#include <sotn/fairy.h>
#include <sotn/game_math.h>

static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return Read16(p) | (Uint32)Read16(p + 2) << 16;
}
static Sint16 Word(Sint32 value)
{
    const Uint16 bits = (Uint16)(Uint32)value;
    return (Sint16)(bits < 32768 ? bits : (Sint32)bits - 65536);
}
static Sint32 Fixed(Uint32 bits)
{
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 4294967296LL);
}
static Sint32 Shift(Sint32 value, Uint16 bits)
{
    const Sint64 scale = (Sint64)1 << bits;
    return (Sint32)(value >= 0 ? value / scale : -((-(Sint64)value + scale - 1) / scale));
}
static Sint16 Pixel(Sint32 value)
{
    return (Sint16)Shift(value, 16);
}
static bool View(SotnDataView overlay, size_t offset, size_t size, const Uint8 **out)
{
    if (!overlay.bytes || offset > overlay.size || size > overlay.size - offset)
        return SDL_SetError("Fairy requires bounded original private overlay data");
    *out = overlay.bytes + offset;
    return true;
}
static SotnFamiliarAnimationSource Animation(SotnDataView overlay)
{
    return (SotnFamiliarAnimationSource){.overlay = overlay, .table = 0x80172b14, .count = 46};
}
static bool Set(SotnFairy *f, SotnDataView overlay, Uint16 index)
{
    return SotnFamiliarAnimation_Set(&f->animation, Animation(overlay), index, false);
}
bool SotnFairy_Init(SotnFairy *root, SotnFairy *wings, SotnFairyHistory *history, SotnFairyInput in,
                    Uint16 mode, bool *reset_events)
{
    if (!root || !wings || root == wings || !history || !reset_events || mode > 3)
        return SDL_SetError("Fairy initialization requires actual root/wings/history and mode0..3");
    if (mode == 3)
    {
        *reset_events = true;
        return true;
    }
    SotnFairy next = {.mode = SOTN_FAIRY_FOLLOW,
                      .priority = (Uint16)(in.player.priority - 2U),
                      .facing_left = in.player.facing_left};
    SotnFairy child = {.mode = SOTN_FAIRY_WINGS,
                       .priority = (Uint16)(in.player.priority - 3U),
                       .facing_left = in.player.facing_left};
    if (mode == 1)
    {
        next.mode = (in.room_flags & 0x60U) ? SOTN_FAIRY_FOLLOW : SOTN_FAIRY_INTRO;
        next.x = 128 * 65536;
        next.y = -32 * 65536;
    }
    else if (in.room_flags & 32U)
    {
        const Sint32 side = in.room_flags == 34   ? (in.inverted ? 1 : 0)
                            : in.room_flags == 32 ? (in.inverted ? 0 : 1)
                                                  : 2;
        next.x = (side ? 192 : 64) * 65536;
        next.y = 160 * 65536;
    }
    else
    {
        next.x = Fixed((Uint32)in.player.x + (in.player.facing_left ? 24U : (Uint32)-24) * 65536U);
        next.y = Fixed((Uint32)in.player.y - 32U * 65536U);
    }
    *root = next;
    *wings = child;
    history->priority = next.priority;
    history->room_special = 0;
    history->restore_priority = false;
    *reset_events = mode == 1;
    return true;
}
Sint16 SotnFairy_StepAngle(Sint16 current, Sint16 target, Sint16 step)
{
    const Sint32 difference = SDL_abs((Sint32)target - current);
    const Sint16 max_step = Word(difference);
    const Sint32 delta = step > max_step ? difference : step;
    return (Sint16)((Uint32)((target < current) == (max_step < 0x800) ? (Sint32)target + delta
                                                                      : (Sint32)target - delta) &
                    4095U);
}
bool SotnFairy_MovementAllowed(SotnFairyPlayer p, bool standing_history)
{
    if ((p.velocity_y < 0 && !(p.collision & 1U)) || (p.velocity_y > 0 && !(p.collision & 2U)) ||
        (p.velocity_x < 0 && !(p.collision & 8U)) || (p.velocity_x > 0 && !(p.collision & 4U)))
        return true;
    return standing_history &&
           (p.previous_step != p.step || p.step != 0 || p.previous_substep != p.substep ||
            (p.substep != 0 && p.substep != 4));
}
static bool Valid(const SotnFairy *f, const SotnFairyHistory *h, SotnFairyInput in,
                  const Uint32 *random)
{
    if (!f || !h || !random || !in.hp_max || in.hp > in.hp_max || in.level < 1 || in.level > 99 ||
        h->hp_index < 0 || h->hp_index > 4)
        return SDL_SetError("Fairy requires actual entity/history, HP, level and libc RNG");
    return true;
}
static void Mode(SotnFairy *f, Uint16 mode)
{
    f->mode = mode;
    f->phase = 0;
}
static void ResetHp(SotnFairyHistory *h, Uint16 hp)
{
    h->hp_index = h->hp_reset = 0;
    h->cached_hp = hp;
    SDL_memset(h->hp_loss, 0, sizeof(h->hp_loss));
}
static bool Resist(SotnDataView overlay, Uint16 index, const Uint8 **record)
{
    if (index >= 6 || !View(overlay, 0x2494 + index * 8U, 8, record))
        return SDL_SetError("Fairy resist selection lost its original private mapping");
    if (Read16(*record) >= 16 || Read16(*record + 2) >= SOTN_HAND_ITEMS)
        return SDL_SetError("Fairy resist mapping exceeds actual buff or bag ownership");
    return true;
}
static bool Select(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng)
{
    const Uint8 *profile = NULL;
    if (!View(in.overlay, 0x2408 + (in.level / 10U) * 14U, 14, &profile))
        return false;
    const Sint32 elements = (Sint32)(in.player.damage_effects & 0xfa00U);
    if (elements)
    {
        if ((h->elements & elements) == elements)
        {
            h->resist_clear = 0;
            h->resist_delay = Fixed((Uint32)h->resist_delay + 1U);
            h->elements &= elements;
        }
        else
        {
            h->resist_delay = 0;
            h->elements |= elements;
        }
    }
    else
    {
        h->resist_clear = Fixed((Uint32)h->resist_clear + 1U);
        if (h->resist_clear > 3600)
            h->elements = h->resist_delay = h->resist_clear = 0;
    }
    if (h->cached_hp > in.hp)
    {
        h->hp_loss[h->hp_index] = Fixed((Uint32)h->cached_hp - in.hp);
        h->hp_index = h->hp_index == 4 ? 0 : (Sint16)(h->hp_index + 1);
        h->cached_hp = in.hp;
        h->hp_reset = 0;
    }
    else
    {
        h->hp_reset = Word(h->hp_reset + 1);
        if (h->hp_reset > 120)
            ResetHp(h, in.hp);
    }
    if ((in.player.status & 0x40000U) && SotnCombat_Random(rng) % 100U <= Read16(profile + 2))
    {
        Mode(f, SOTN_FAIRY_APPLE);
        return true;
    }
    if (f->timer < 0)
        return true;
    if (f->timer)
    {
        --f->timer;
        return true;
    }
    f->timer = Word(Read16(profile));
    if (f->mode == SOTN_FAIRY_HAMMER)
        return true;
    if (in.player.step == 11 && !SotnFairy_MovementAllowed(in.player, false) &&
        in.hand_count[147] && SotnCombat_Random(rng) % 100U <= Read16(profile + 4))
    {
        f->hammer = 0;
        Mode(f, SOTN_FAIRY_HAMMER);
        return true;
    }
    if (f->mode == SOTN_FAIRY_UNCURSE)
        return true;
    if (in.player.status & 0x8000U)
    {
        if (in.hand_count[145] && SotnCombat_Random(rng) % 100U <= Read16(profile + 6))
        {
            f->uncurse_luck = 0;
            Mode(f, SOTN_FAIRY_UNCURSE);
            return true;
        }
        if (!f->uncurse_luck)
        {
            f->uncurse_luck = 1;
            Mode(f, SOTN_FAIRY_UNCURSE);
            return true;
        }
    }
    if (f->mode == SOTN_FAIRY_ANTIVENOM)
        return true;
    if (in.player.status & 0x4000U)
    {
        if (in.hand_count[144] && SotnCombat_Random(rng) % 100U <= Read16(profile + 8))
        {
            f->antivenom_luck = 0;
            Mode(f, SOTN_FAIRY_ANTIVENOM);
            return true;
        }
        if (!f->antivenom_luck)
        {
            f->antivenom_luck = 1;
            Mode(f, SOTN_FAIRY_ANTIVENOM);
            return true;
        }
    }
    if (h->resist_delay >= 10)
    {
        const Uint16 elements_priority[6] = {0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x200};
        Uint16 index = 0;
        while (index < 6 && !(h->elements & elements_priority[index]))
            ++index;
        const Uint8 *record = NULL;
        if (!Resist(in.overlay, index, &record))
            return false;
        if (!in.buffs[Read16(record)] && in.hand_count[Read16(record + 2)] &&
            SotnCombat_Random(rng) % 100U <= Read16(profile + 10))
        {
            Mode(f, SOTN_FAIRY_RESIST);
            f->parameter = index;
            h->resist_delay = h->resist_clear = 0;
            return true;
        }
    }
    if (f->mode == SOTN_FAIRY_POTION)
        return true;
    Uint32 loss_bits = 0;
    for (Uint16 i = 0; i < 5; ++i)
        loss_bits += (Uint32)h->hp_loss[i];
    const Sint32 loss = Fixed(loss_bits);
    Uint16 urgency = 0;
    if (loss >= in.hp_max / 2 || in.hp <= in.hp_max / 10)
        urgency = in.hp_max < 100 ? 1 : 2;
    if (in.hp <= in.hp_max / 4)
    {
        if (loss >= in.hp_max / 8)
            urgency = 2;
        else if (loss >= in.hp_max / 16)
            urgency = 1;
    }
    if (!urgency)
        return true;
    if (in.hand_count[159] || in.hand_count[160])
    {
        if (SotnCombat_Random(rng) % 100U > Read16(profile + 12))
            return true;
    }
    else if (f->potion_luck)
        return true;
    f->potion_luck = 1; // Original sets this even on successful medicine selection.
    Mode(f, SOTN_FAIRY_POTION);
    f->parameter = (Uint16)(urgency - 1U);
    ResetHp(h, in.hp);
    return true;
}
bool SotnFairy_SelectAbility(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *random)
{
    if (!Valid(f, h, in, random))
        return false;
    SotnFairy next = *f;
    SotnFairyHistory history = *h;
    Uint32 rng = *random;
    if (!Select(&next, &history, in, &rng))
        return false;
    *f = next;
    *h = history;
    *random = rng;
    return true;
}
static bool Initialize(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng)
{
    if (!f->ability)
    {
        if (f->mode == SOTN_FAIRY_FOLLOW || f->mode == SOTN_FAIRY_INTRO)
        {
            f->flags = 0x0c020000;
            if (!Set(f, in.overlay, 14))
                return false;
            f->wave_angle = (Sint16)(SotnCombat_Random(rng) % 4096U);
            f->target_angle = 0;
            f->distance = 8;
            f->max_angle = 32;
            ++f->phase;
        }
        else if (f->mode == SOTN_FAIRY_WINGS)
        {
            f->flags = 0x0c020000;
            ++f->phase;
        }
    }
    else if (f->mode == SOTN_FAIRY_WINGS)
    {
        f->flags = 0x0c020000;
        ++f->phase;
    }
    else if (f->mode >= SOTN_FAIRY_FOLLOW && f->mode <= SOTN_FAIRY_HINT &&
             f->mode != SOTN_FAIRY_INTRO)
    {
        if (f->mode == SOTN_FAIRY_FOLLOW)
            f->timer = 120;
        f->flags = 0x0c020000;
        if (!Set(f, in.overlay, 14))
            return false;
        ++f->phase;
    }
    f->ability = Word(f->mode);
    h->room_special = 0;
    return true;
}
static bool SelectAnimation(SotnFairy *f, SotnDataView overlay)
{
    const Sint32 ax = f->velocity_x >= 0 ? f->velocity_x : Fixed(0U - (Uint32)f->velocity_x);
    const Sint32 ay = f->velocity_y >= 0 ? f->velocity_y : Fixed(0U - (Uint32)f->velocity_y);
    Uint16 index = 0;
    if (ay > ax)
    {
        if (ay < 32768 && (f->animation_flag == 1 || f->animation_flag == 2))
        {
            index = f->animation_flag == 1 ? 41 : 14;
            f->animation_flag = 0;
        }
        else if (ay > 65536)
        {
            f->animation_flag = 2;
            index = f->velocity_y >= 0 ? 11 : 12;
        }
    }
    else
    {
        if (ax > 36864 && f->animation_flag == 0)
        {
            f->animation_flag = 1;
            index = 15;
        }
        else if (((ax > 36864 || ax < 24576) && f->animation_flag == 2) ||
                 (ax < 24576 && f->animation_flag == 1))
        {
            index = f->animation_flag == 1 ? 41 : 14;
            f->animation_flag = 0;
        }
        if (ax > 32768)
            f->facing_left = f->velocity_x >= 0;
    }
    return !index || Set(f, overlay, index);
}
static bool Distance(SotnFairy f, SotnDataView executable, Sint32 x, Sint32 y, Uint32 *distance)
{
    const Uint32 dx = (Uint32)x - (Uint32)(Sint32)Pixel(f.x);
    const Uint32 dy = (Uint32)y - (Uint32)(Sint32)Pixel(f.y);
    Uint32 root = 0;
    if (!SotnMath_Sqrt12(executable, (dx * dx + dy * dy) << 12, &root))
        return false;
    *distance = root >> 12;
    return true;
}
static bool Wave(SotnFairy *f, SotnDataView executable, Sint32 *x, Sint32 *y)
{
    SotnMathTrig half = {0}, full = {0};
    const Sint16 angle = f->wave_angle;
    f->wave_angle = (Sint16)((Uint16)(angle + 16) & 4095U);
    if (!SotnMath_Trig(executable, angle / 2, &half) || !SotnMath_Trig(executable, angle, &full))
        return false;
    *x = Fixed((Uint32)*x + (Uint32)Shift(half.cosine * f->distance, 12));
    *y = Fixed((Uint32)*y - (Uint32)Shift(full.sine * (f->distance / 2), 12));
    return true;
}
static bool Steer(SotnFairy *f, SotnFairyInput in, Sint32 x, Sint32 y, Sint16 angle_step,
                  Sint32 scale)
{
    Sint16 angle = 0;
    SotnMathTrig trig = {0};
    if (!SotnMath_Atan2Vector(in.executable, -Word(Fixed((Uint32)y - (Uint32)(Sint32)Pixel(f->y))),
                              Word(Fixed((Uint32)x - (Uint32)(Sint32)Pixel(f->x))), &angle))
        return false;
    f->target_angle =
        SotnFairy_StepAngle((Sint16)((Uint16)angle & 4095U), f->target_angle, angle_step);
    if (!SotnMath_Trig(in.executable, f->target_angle, &trig))
        return false;
    f->velocity_x = trig.cosine * scale;
    f->velocity_y = -trig.sine * scale;
    return true;
}
static void Move(SotnFairy *f)
{
    f->x = Fixed((Uint32)f->x + (Uint32)f->velocity_x);
    f->y = Fixed((Uint32)f->y + (Uint32)f->velocity_y);
}
static bool Follow(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng,
                   SotnFairyEvents *e, bool *early)
{
    if (h->restore_priority)
        h->priority = f->priority = (Uint16)(in.player.priority - 2U);
    Sint32 x = Pixel(in.player.x) + (in.player.facing_left ? 24 : -24);
    Sint32 y = Pixel(in.player.y) - 32;
    if (in.room_flags & 32U)
    {
        const Sint32 side = in.room_flags == 34   ? (in.inverted ? 1 : 0)
                            : in.room_flags == 32 ? (in.inverted ? 0 : 1)
                                                  : 2;
        x = side == 0 ? 64 : side == 1 ? 192 : Pixel(f->x) > 128 ? 192 : 64;
        y = 160;
    }
    else if (!Wave(f, in.executable, &x, &y))
        return false;
    if (!f->phase)
    {
        if (!Initialize(f, h, in, rng) || !Set(f, in.overlay, 14))
            return false;
        f->animation_flag = 0;
        h->cached_hp = in.hp;
        h->hp_index = h->hp_reset = f->idle = 0;
    }
    else if (f->phase == 1)
    {
        Uint32 distance = 0;
        if (!Distance(*f, in.executable, x, y, &distance) || !Steer(f, in, x, y, f->max_angle,
                                                                    distance < 40    ? 8
                                                                    : distance < 60  ? 16
                                                                    : distance < 100 ? 32
                                                                                     : 64))
            return false;
        f->max_angle = distance < 40 ? 32 : distance < 60 ? 64 : distance < 100 ? 96 : 128;
        if (distance >= 256)
        {
            f->velocity_x = Fixed(((Uint32)x - (Uint32)(Sint32)Pixel(f->x)) << 14);
            f->velocity_y = Fixed(((Uint32)y - (Uint32)(Sint32)Pixel(f->y)) << 14);
        }
        if (!SelectAnimation(f, in.overlay))
            return false;
        Move(f);
        if (!in.cutscene && !SotnFairy_MovementAllowed(in.player, true) && !in.enemies &&
            !(in.room_flags & 32U))
            f->idle = Word(f->idle + 1);
        else
            f->idle = 0;
        if (f->idle > 1800)
        {
            Mode(f, SOTN_FAIRY_SHOULDER);
            *early = true;
            return true;
        }
        if (h->room_special >= 2)
        {
            Mode(f, SOTN_FAIRY_HINT);
            f->parameter = (Uint16)((Uint32)h->room_special - 2U);
        }
    }
    e->process_room_events = true;
    return in.cutscene || Select(f, h, in, rng);
}
static bool Sound(SotnFairyEvents *e, SotnDataView overlay, Uint16 index)
{
    const Uint8 *p = NULL;
    if (index >= 6 || e->sound_count >= 2 || !View(overlay, 0x2bcc + index * 4U, 4, &p) ||
        Read32(p) > SDL_MAX_UINT16)
        return SDL_SetError("Fairy sound lost its bounded original VAB mapping");
    e->sounds[e->sound_count++] = (Uint16)Read32(p);
    return true;
}
static bool RandomSound(SotnFairyEvents *e, SotnDataView overlay, Uint32 *rng)
{
    const Uint16 roll = SotnCombat_Random(rng) % 256U;
    const Uint8 *p = NULL;
    // This original table has four threshold/sound pairs, ending with255.
    for (Uint16 i = 0; i < 4; ++i)
    {
        if (!View(overlay, 0x2be4 + i * 8U, 8, &p))
            return false;
        if (roll <= Read32(p))
        {
            if (Read32(p + 4) > SDL_MAX_UINT16 || e->sound_count >= 2)
                break;
            e->sounds[e->sound_count++] = (Uint16)Read32(p + 4);
            return true;
        }
    }
    return SDL_SetError("Fairy randomized sound table has no bounded matching threshold");
}
bool SotnFamiliarSfx_Tick(SotnFamiliarSfx *sfx, SotnFamiliarSfxInput in, Uint16 *command,
                          Sint32 *step)
{
    if (!sfx || !command || !step || sfx->removed || sfx->phase > 8 ||
        (sfx->phase == 2 && !sfx->parameter))
        return SDL_SetError(
            "Familiar narrative sound requires its actual live event and CD inputs");
    SotnFamiliarSfx next = *sfx;
    Uint16 request = 0;
    switch (next.phase)
    {
    case 0:
        next.flags = 0x04020000;
        if (in.cutscene)
            next.removed = true;
        else if (in.cd_queue_empty)
        {
            request = 0x10;
            ++next.phase;
        }
        break;
    case 1:
        if (in.command_acknowledged)
            ++next.phase;
        break;
    case 2:
        request = next.parameter;
        ++next.phase;
        break;
    case 3:
        if (in.xa_playing)
            ++next.phase;
        break;
    case 4:
        if (!in.xa_playing)
            ++next.phase;
        break;
    case 5:
        if (in.room_flags & 0x60U)
            next.removed = true;
        else if (in.cd_queue_empty)
        {
            request = 0x11;
            ++next.phase;
        }
        break;
    case 6:
        next.removed = in.command_acknowledged;
        break;
    case 7:
        request = 0xa;
        next.phase = 4;
        break;
    case 8:
        next.removed = true;
        break;
    }
    *sfx = next;
    *command = request;
    *step = next.removed ? 99 : next.phase;
    return true;
}
bool SotnFairy_RoomEvent(SotnFairyHistory *history, Uint16 parameter, bool sound_present,
                         Uint16 *sound_phase)
{
    if (!history || (sound_present && !sound_phase))
        return SDL_SetError("Fairy room event requires actual history and occupied sound cursor");
    if (parameter <= 10)
        history->room_special = (Sint32)parameter + 1;
    else if (parameter == 15)
        history->restore_priority = true;
    if (!parameter && sound_present && *sound_phase < 5)
        *sound_phase = 8;
    return true;
}
static bool PrivateEvent(SotnDataView overlay, Uint32 address, const Uint8 **event)
{
    if (address < 0x80170000U)
        return SDL_SetError("Fairy dialogue lost its original private event address");
    return View(overlay, address - 0x80170000U, 6, event);
}
static bool Dialogue(SotnFairy *f, SotnFairyInput in, SotnFairyEvents *e)
{
    f->facing_left = Pixel(in.player.x) >= Pixel(f->x);
    const Uint8 *event = NULL;
    if (!PrivateEvent(in.overlay, f->sound_event, &event))
        return false;
    if (f->sound_flag < 0)
    {
        if (in.sound_step > 4)
        {
            if (!Set(f, in.overlay, Read16(event + 2)))
                return false;
            ++f->phase;
        }
    }
    else
    {
        if (in.sound_step == 4 || in.sound_step >= 99)
            f->sound_flag = Word(f->sound_flag - 1);
        if (f->sound_flag < 0)
        {
            if (!Set(f, in.overlay, Read16(event + 2)))
                return false;
            if (Read16(event + 4) && !in.sound_present)
            {
                e->event_mode = 0xde;
                e->event_parameter = Read16(event + 4);
            }
            const Uint32 next = f->sound_event + 6U;
            if (next < f->sound_event || !PrivateEvent(in.overlay, next, &event))
                return false;
            f->sound_event = next;
            f->sound_flag = Word(Read16(event));
        }
    }
    return true;
}
static bool GentleFollow(SotnFairy *f, SotnFairyInput in, Sint32 x, Sint32 y, Uint32 *distance)
{
    if (!Wave(f, in.executable, &x, &y) || !Distance(*f, in.executable, x, y, distance) ||
        !Steer(f, in, x, y, f->max_angle,
               *distance < 60    ? 8
               : *distance < 100 ? 16
                                 : 32))
        return false;
    f->max_angle = *distance < 60 ? 64 : *distance < 100 ? 96 : 128;
    Move(f);
    // Callers that test arrival do so after movement, against this same wavy target.
    return Distance(*f, in.executable, x, y, distance);
}
static bool Intro(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng,
                  SotnFairyEvents *e, bool *early)
{
    if (h->restore_priority)
        h->priority = f->priority = (Uint16)(in.player.priority - 2U);
    Uint32 distance = 0;
    if (!GentleFollow(f, in, Pixel(in.player.x) + (in.player.facing_left ? -24 : 24),
                      Pixel(in.player.y) - 32, &distance))
        return false;
    switch (f->phase)
    {
    case 0:
        if (!Initialize(f, h, in, rng))
            return false;
        f->timer = -1;
        break;
    case 1:
        if (!SelectAnimation(f, in.overlay))
            return false;
        if (SotnFairy_MovementAllowed(in.player, true) || in.enemies || h->room_special == 1 ||
            in.cutscene || in.global_flash)
        {
            if (!Set(f, in.overlay, 14))
                return false;
            Mode(f, SOTN_FAIRY_FOLLOW);
            *early = true;
            return true;
        }
        if (distance < 32)
        {
            f->facing_left = in.player.facing_left;
            ++f->phase;
        }
        break;
    case 2: {
        const Uint16 roll = SotnCombat_Random(rng) % 256U;
        const size_t table = in.load_count == 1 ? 0x2c3c : 0x2c64;
        const Uint8 *record = NULL, *event = NULL;
        Uint16 index = 0;
        for (; index < 2; ++index)
        {
            if (!View(in.overlay, table + index * 8U, 8, &record))
                return false;
            if (roll <= Read32(record))
                break;
        }
        if (index == 2 || !PrivateEvent(in.overlay, Read32(record + 4), &event))
            return SDL_SetError("Fairy introduction lost its bounded private random event");
        f->sound_event = Read32(record + 4);
        f->sound_flag = Word(Read16(event));
        e->pause_write = true;
        ++f->phase;
        break;
    }
    case 3:
        if (!Dialogue(f, in, e))
            return false;
        break;
    case 4:
        if (in.sound_step == 99)
            ++f->phase;
        break;
    case 5:
        if (!Set(f, in.overlay, 14))
            return false;
        e->pause_write = e->pause_allowed = true;
        Mode(f, SOTN_FAIRY_FOLLOW);
        break;
    default:
        return SDL_SetError("Fairy introduction has an unsupported original phase");
    }
    e->process_room_events = true;
    return Select(f, h, in, rng);
}
static bool Hint(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng,
                 SotnFairyEvents *e)
{
    const Uint8 *record = NULL;
    if (f->parameter >= 10 || !View(in.overlay, 0x2cb0 + f->parameter * 12U, 12, &record))
        return SDL_SetError("Fairy hint lost its bounded private room mapping");
    f->tilemap_x = (Uint32)in.tilemap_x;
    f->tilemap_y = (Uint32)in.tilemap_y;
    if (h->restore_priority)
        h->priority = f->priority = (Uint16)(in.player.priority - 2U);
    Sint32 x = Pixel(in.player.x) + (in.player.facing_left ? -24 : 24);
    Sint32 y = Pixel(in.player.y) - 32;
    if (Read32(record) != SDL_MAX_UINT32 && f->phase)
    {
        x = Fixed((Uint32)h->hint_x - f->tilemap_x);
        y = Fixed((Uint32)h->hint_y - f->tilemap_y);
    }
    Uint32 distance = 0;
    if (!GentleFollow(f, in, x, y, &distance))
        return false;
    switch (f->phase)
    {
    case 0:
        if (!Initialize(f, h, in, rng))
            return false;
        h->hint_x = Fixed(Read32(record) + f->tilemap_x);
        h->hint_y = Fixed(Read32(record + 4) + f->tilemap_y);
        f->timer = -1;
        break;
    case 1:
        if (!SelectAnimation(f, in.overlay))
            return false;
        if (distance < 32)
        {
            f->facing_left = in.player.facing_left;
            ++f->phase;
        }
        break;
    case 2: {
        const Uint8 *event = NULL;
        f->sound_event = Read32(record + 8);
        if (!PrivateEvent(in.overlay, f->sound_event, &event))
            return false;
        f->sound_flag = Word(Read16(event));
        e->pause_write = true;
        ++f->phase;
        break;
    }
    case 3:
        if (!Dialogue(f, in, e))
            return false;
        break;
    case 4:
        if (!Set(f, in.overlay, 14))
            return false;
        e->pause_write = e->pause_allowed = true;
        Mode(f, SOTN_FAIRY_FOLLOW);
        break;
    default:
        return SDL_SetError("Fairy hint has an unsupported original phase");
    }
    return true;
}
static bool Shoulder(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng,
                     SotnFairyEvents *e, bool *early)
{
    if (h->restore_priority)
        h->priority = f->priority = (Uint16)(in.player.priority - 2U);
    if (SotnFairy_MovementAllowed(in.player, true))
    {
        if (f->phase < 2)
        {
            Mode(f, SOTN_FAIRY_FOLLOW);
            *early = true;
            return true;
        }
        if (f->phase < 9)
            f->phase = 9;
    }
    const Sint32 offset =
        !in.player.substep ? (in.player.facing_left ? 6 : -6) : (in.player.facing_left ? -16 : 16);
    const Sint32 x = Pixel(in.player.x) + offset;
    const Sint32 y = Pixel(in.player.y) - (in.player.substep ? 8 : 12);
    switch (f->phase)
    {
    case 0:
        if (!Initialize(f, h, in, rng) || !Set(f, in.overlay, 14))
            return false;
        break;
    case 1: {
        Uint32 distance = 0;
        if (!Steer(f, in, x, y, 384, 8) || !SelectAnimation(f, in.overlay))
            return false;
        Move(f);
        if (!Distance(*f, in.executable, x, y, &distance))
            return false;
        if (distance < 2)
        {
            f->facing_left = in.player.substep ? !in.player.facing_left : in.player.facing_left;
            if (!Set(f, in.overlay, 24))
                return false;
            f->frames = 0;
            f->flags = (f->flags | 0x40000U) & ~0x08000000U;
            // The US Fairy is not the alternate Yousei, so it never enters song phases2..4.
            f->phase = 5;
        }
        break;
    }
    case 5:
        f->frames = Word(f->frames + 1);
        if (f->frames > 1280)
        {
            if (!Set(f, in.overlay, 24))
                return false;
            f->frames = f->song = 0;
            ++f->phase;
        }
        break;
    case 6:
        f->frames = (Sint16)((SotnCombat_Random(rng) % 2048U) + 1024U);
        ++f->phase;
        SDL_FALLTHROUGH;
    case 7:
        f->frames = Word(f->frames - 1);
        if (f->frames < 0)
        {
            f->frames = (Sint16)((SotnCombat_Random(rng) % 128U) + 128U);
            if (!Set(f, in.overlay, 25))
                return false;
            ++f->phase;
        }
        break;
    case 8:
        f->frames = Word(f->frames - 1);
        if (f->frames < 0)
        {
            if (!Set(f, in.overlay, 24))
                return false;
            f->phase = 6;
        }
        break;
    case 9: {
        e->release_song = f->song != 0;
        f->flags = (f->flags & ~0x40000U) | 0x08000000U;
        if (!Set(f, in.overlay, 27))
            return false;
        f->velocity_x = f->facing_left ? -16384 : 16384;
        f->velocity_y = 65536;
        const Uint16 roll = SotnCombat_Random(rng) % 256U;
        const Uint8 *record = NULL;
        Uint16 index = 0;
        for (; index < 4; ++index)
        {
            if (!View(in.overlay, 0x2c04 + index * 8U, 8, &record))
                return false;
            if (roll <= Read32(record))
                break;
        }
        if (index == 4 || Read32(record + 4) > SDL_MAX_UINT16)
            return SDL_SetError("Fairy shoulder lost its bounded private grunt mapping");
        e->sounds[e->sound_count++] = (Uint16)Read32(record + 4);
        ++f->phase;
        break;
    }
    case 10:
        Move(f);
        f->velocity_y = Fixed((Uint32)f->velocity_y - 2048U);
        if (h->shoulder_status == -1)
        {
            Mode(f, SOTN_FAIRY_FOLLOW);
            *early = true;
            return true;
        }
        break;
    default:
        return SDL_SetError("US Fairy shoulder has an unsupported alternate-familiar phase");
    }
    e->process_room_events = true;
    return Select(f, h, in, rng);
}
static bool Approach(SotnFairy *f, SotnFairyInput in, Sint32 x, Sint32 y)
{
    Uint32 distance = 0;
    if (!Steer(f, in, x, y, 384, 32) || !SelectAnimation(f, in.overlay))
        return false;
    Move(f);
    if (!Distance(*f, in.executable, x, y, &distance))
        return false;
    if (distance < 2)
        ++f->phase;
    return true;
}
static bool Apple(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng,
                  SotnFairyEvents *e, bool *early)
{
    switch (f->phase)
    {
    case 0:
        if (!Initialize(f, h, in, rng))
            return false;
        f->frames = 0;
        break;
    case 1:
        f->frames = Word(f->frames + 1);
        if (f->frames <= 24)
            break;
        ++f->phase;
        SDL_FALLTHROUGH;
    case 2:
        if (!in.hand_count[146])
        {
            f->phase = 90;
            break;
        }
        ++f->phase;
        SDL_FALLTHROUGH;
    case 3:
        if (!in.scene)
        {
            e->consume = 146;
            e->scene_write = true;
            e->scene = 0xfff;
            ++f->phase;
        }
        break;
    case 4: {
        Sint32 x = Pixel(in.player.x), y = Pixel(in.player.y) - 32;
        if (!Wave(f, in.executable, &x, &y) || !Approach(f, in, x, y))
            return false;
        break;
    }
    case 5:
        f->facing_left = in.player.facing_left;
        if (!Set(f, in.overlay, 19))
            return false;
        ++f->phase;
        break;
    case 6:
        if (f->animation.pose == 10)
        {
            if (!Sound(e, in.overlay, 2))
                return false;
            ++f->phase;
        }
        break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
        f->frames = Word(f->frames + 1);
        if (f->frames <= (f->phase == 9 || f->phase == 10 ? 90 : 60))
            break;
        if (f->phase == 7 || f->phase == 8)
            e->event_mode = f->phase == 7 ? 0xdf : 0xdd;
        else if (f->phase == 9)
            e->clear_shared = e->restore_resources = e->clear_flash_if29 = true;
        else if (f->phase == 10)
            e->scene_write = true;
        else
        {
            Mode(f, SOTN_FAIRY_FOLLOW);
            *early = true;
            return true;
        }
        // The last phase deliberately retains the91-frame counter on release.
        if (f->phase != 10)
            f->frames = 0;
        ++f->phase;
        break;
    case 90:
        if (!Set(f, in.overlay, 32) || !Sound(e, in.overlay, 4))
            return false;
        ++f->phase;
        break;
    case 91:
        break;
    default:
        return SDL_SetError("Fairy Life Apple has an unsupported original phase");
    }
    return true;
}
static bool ItemMap(SotnFairy *f, SotnFairyInput in, Uint16 *item, Uint16 *profile, Uint16 *event)
{
    const Uint8 *p = NULL;
    if (f->mode == SOTN_FAIRY_RESIST)
    {
        if (!Resist(in.overlay, f->parameter, &p))
            return false;
        *item = Read16(p + 2);
        *profile = Read16(p + 4);
        *event = Read16(p + 6);
    }
    else if (f->mode == SOTN_FAIRY_POTION)
    {
        if (f->parameter >= 2 || !View(in.overlay, 0x24c4 + f->parameter * 4U, 4, &p))
            return SDL_SetError("Fairy potion lost its private original mapping");
        *item = Read16(p);
        *profile = Read16(p + 2);
        *event = 2;
    }
    else
    {
        *item = f->mode == SOTN_FAIRY_HAMMER ? 147 : f->mode == SOTN_FAIRY_UNCURSE ? 145 : 144;
        *profile = f->mode == SOTN_FAIRY_HAMMER ? 3 : f->mode == SOTN_FAIRY_UNCURSE ? 1 : 0;
        *event = f->mode == SOTN_FAIRY_HAMMER ? 1 : f->mode == SOTN_FAIRY_UNCURSE ? 4 : 3;
    }
    if (*item >= SOTN_HAND_ITEMS || *profile > 255)
        return SDL_SetError("Fairy item mapping exceeds actual inventory or factory parameters");
    return true;
}
static bool Item(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *rng,
                 SotnFairyEvents *e, bool *early)
{
    Uint16 item = 0, profile = 0, event = 0;
    if (!ItemMap(f, in, &item, &profile, &event))
        return false;
    const bool hammer = f->mode == SOTN_FAIRY_HAMMER;
    switch (f->phase)
    {
    case 0:
        if (!Initialize(f, h, in, rng) || !Set(f, in.overlay, 14))
            return false;
        break;
    case 1:
        if (!Approach(f, in, Pixel(in.player.x) + (in.player.facing_left ? -24 : 24),
                      Pixel(in.player.y) - 24))
            return false;
        break;
    case 2:
        if (hammer || f->mode == SOTN_FAIRY_RESIST)
            f->facing_left = in.player.facing_left;
        if (!in.hand_count[item] && f->mode == SOTN_FAIRY_POTION)
        {
            f->parameter = f->parameter ? 0 : 1;
            if (!ItemMap(f, in, &item, &profile, &event))
                return false;
        }
        if (!in.hand_count[item])
        {
            if (!Set(f, in.overlay, hammer || f->mode == SOTN_FAIRY_RESIST ? 16 : 20))
                return false;
            f->phase = hammer ? 4 : 5;
            if (hammer)
            {
                if (!Sound(e, in.overlay, 3))
                    return false;
                f->frames = 0;
            }
            break;
        }
        if (!hammer && in.medicine_active)
        {
            Mode(f, SOTN_FAIRY_FOLLOW);
            *early = true;
            return true;
        }
        if (!Set(f, in.overlay, hammer ? 23 : 18) || (hammer && !RandomSound(e, in.overlay, rng)))
            return false;
        ++f->phase;
        break;
    case 3:
        if (!hammer)
            f->facing_left = !in.player.facing_left;
        if (f->animation.pose == (hammer ? 5 : 11))
        {
            if (hammer || f->mode == SOTN_FAIRY_RESIST)
            {
                if (!hammer && !RandomSound(e, in.overlay, rng))
                    return false;
            }
            else if (!Sound(e, in.overlay, f->mode == SOTN_FAIRY_POTION ? 1 : 0))
                return false;
            e->consume = item;
            e->factory = ((Uint32)profile << 16) | 55U;
            e->event_mode = 0xdf;
            e->event_parameter = event;
            if (hammer)
            {
                e->sounds[e->sound_count++] = 0x675;
                e->shake = true;
            }
            f->frames = 0;
            ++f->phase;
        }
        break;
    case 4:
    case 6:
        f->frames = Word(f->frames + 1);
        if (f->frames > 60)
        {
            Mode(f, SOTN_FAIRY_FOLLOW);
            *early = true;
            return true;
        }
        break;
    case 5:
        if (hammer)
            return SDL_SetError("Fairy Hammer has no original phase5");
        f->facing_left = in.player.facing_left;
        if (f->animation.pose == 32)
        {
            if (!Sound(e, in.overlay, 3))
                return false;
            f->frames = 0;
            ++f->phase;
        }
        break;
    default:
        return SDL_SetError("Fairy item has an unsupported original phase");
    }
    return Select(f, h, in, rng);
}
bool SotnFairy_Tick(SotnFairy *f, SotnFairyHistory *h, SotnFairyInput in, Uint32 *random,
                    SotnFairyEvents *out)
{
    if (!out || !Valid(f, h, in, random))
        return SDL_SetError("Fairy callback requires actual cursors, RNG and output");
    SotnFairy next = *f;
    SotnFairyHistory history = *h;
    SotnFairyEvents events = {0};
    Uint32 rng = *random;
    bool early = false;
    if (next.mode == SOTN_FAIRY_FOLLOW)
    {
        if (!Follow(&next, &history, in, &rng, &events, &early))
            return false;
    }
    else if (next.mode == SOTN_FAIRY_APPLE)
    {
        if (!Apple(&next, &history, in, &rng, &events, &early))
            return false;
    }
    else if (next.mode >= SOTN_FAIRY_HAMMER && next.mode <= SOTN_FAIRY_POTION)
    {
        if (!Item(&next, &history, in, &rng, &events, &early))
            return false;
    }
    else if (next.mode == SOTN_FAIRY_INTRO)
    {
        if (!Intro(&next, &history, in, &rng, &events, &early))
            return false;
    }
    else if (next.mode == SOTN_FAIRY_SHOULDER)
    {
        if (!Shoulder(&next, &history, in, &rng, &events, &early))
            return false;
    }
    else if (next.mode == SOTN_FAIRY_HINT)
    {
        if (!Hint(&next, &history, in, &rng, &events))
            return false;
    }
    else
        return SDL_SetError("Fairy callback requires a supported actual root mode");
    if (!early && !SotnFamiliarAnimation_Tick(
                      &next.animation, Animation(in.overlay),
                      f->mode == SOTN_FAIRY_SHOULDER ? &history.shoulder_status : NULL))
        return false;
    *f = next;
    *h = history;
    *random = rng;
    *out = events;
    return true;
}
bool SotnFairy_Wings(SotnFairy *wings, const SotnFairy *parent, SotnFairyHistory *history,
                     SotnDataView overlay)
{
    if (!wings || !parent || !history || wings == parent || wings->mode != SOTN_FAIRY_WINGS)
        return SDL_SetError("Fairy wings require their actual child and root");
    SotnFairy next = *wings;
    SotnFairyHistory h = *history;
    if (!next.phase)
    {
        next.flags = 0x0c020000;
        h.room_special = 0;
        next.phase = 2;
    }
    next.x = parent->x;
    next.y = parent->y;
    next.facing_left = parent->facing_left;
    const Uint8 *table = NULL, *record = NULL;
    if (!View(overlay, 0x2b14, 46 * 4, &table))
        return false;
    Uint16 index = 6;
    while (index < 46 && Read32(table + index * 4U) != parent->animation.address)
        ++index;
    if (index == 46 || !View(overlay, 0x2368 + (index - 6U) * 4U, 4, &record))
        return SDL_SetError("Fairy wings lost their actual private parent animation");
    const Sint16 animation = Word(Read16(record));
    if (!Set(&next, overlay, (Uint16)SDL_abs((Sint32)animation)) ||
        !SotnFamiliarAnimation_Tick(&next.animation, Animation(overlay), NULL))
        return false;
    next.priority = (Uint16)(h.priority + (Read16(record + 2) ? -1 : 1));
    *wings = next;
    *history = h;
    return true;
}
