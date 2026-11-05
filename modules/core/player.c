// SPDX-License-Identifier: AGPL-3.0-or-later
// Adapted normal-form movement paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5:
// src/dra/6DF70.c, 6E42C.c, 6FDF8.c, 71830.c, 692E8.c,
// and selected unarmed paths in 7879C.c and alu_anim.c.
#include "player_internal.h"
#include <sotn/combat.h>
#include <sotn/hellfire_cast.h>
#include <sotn/subweapon.h>

enum
{
    FIX_ONE = 65536,
    WALK_SPEED = 98304,
    GRAVITY = 11264,
    JUMP_SPEED = -319488
};

_Static_assert(sizeof(PlayerFixed) == 4,
               "Player coordinates require an exact 32-bit representation");

static bool ValidateMap(SotnCollisionMap map)
{
    if (map.width == 0 || map.height == 0 || map.width > 1024 || map.height > 1024 ||
        map.tiles.bytes == NULL || map.types.bytes == NULL || map.types.size == 0 ||
        map.width > (map.tiles.size / 2) / map.height)
        return SDL_SetError("Player requires a bounded original room collision map");
    const size_t count = (size_t)map.width * map.height;
    for (size_t i = 0; i < count; ++i)
    {
        const Uint16 id =
            (Uint16)(map.tiles.bytes[i * 2] | (Uint16)map.tiles.bytes[i * 2 + 1] << 8);
        if (id >= map.types.size)
            return SDL_SetError("Player room references a missing collision definition");
    }
    return true;
}

static bool PositionValid(SotnCollisionMap map, Sint64 x, Sint64 y)
{
    const Sint64 min = -256LL * FIX_ONE;
    const Sint64 max_x = ((Sint64)map.width * 16 + 256) * FIX_ONE;
    const Sint64 max_y = ((Sint64)map.height * 16 + 256) * FIX_ONE;
    return x >= min && x <= max_x && y >= min && y <= max_y;
}

static void SetStep(SotnPlayer *player, SotnPlayerStep step)
{
    player->step = step;
    player->substep = 0;
}

static void Animate(SotnPlayer *player, Uint16 id)
{
    if (!SotnAnimation_Set(&player->animation, id))
        player->failed = true;
}

static void SetSpeed(SotnPlayer *player, Sint32 speed)
{
    player->vx = player->facing_left ? -speed : speed;
}

static void Decelerate(SotnPlayer *player, Sint32 amount)
{
    if (player->vx < 0)
        player->vx = SDL_min(0, player->vx + amount);
    else
        player->vx = SDL_max(0, player->vx - amount);
}

static int Direction(SotnPlayer *player)
{
    if (player->movement_flags & 2)
        return 0;
    if (player->buttons & SOTN_BUTTON_RIGHT)
    {
        if (player->facing_left)
        {
            player->facing_left = false;
            player->turned = true;
            return -1;
        }
        return 1;
    }
    if (player->buttons & SOTN_BUTTON_LEFT)
    {
        if (!player->facing_left)
        {
            player->facing_left = true;
            player->turned = true;
            return -1;
        }
        return 1;
    }
    return 0;
}

void SotnPlayer_FormDirection(SotnPlayer *player)
{
    (void)Direction(player);
}

static void Land(SotnPlayer *player, Sint32 speed)
{
    const bool raised = player->shield_cursor.phase == 1 || player->shield_cursor.phase == 2;
    const bool ledge = !raised && (player->contact & 0x20) != 0;
    player->vx = speed;
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_STAND);
    if (raised)
        player->substep = 2;
    Uint16 animation = ledge ? 3 : 4;
    if (player->previous_step == SOTN_PLAYER_WALK)
    {
        if (player->animation.id == 9)
        {
            player->animation.id = ledge ? 11 : 10;
            return;
        }
        animation = player->animation.id == 7 ? (ledge ? 5 : 4) : (ledge ? 15 : 14);
    }
    else if (player->previous_step == SOTN_PLAYER_UNMORPH_BAT ||
             ((player->previous_step == SOTN_PLAYER_JUMP ||
               player->previous_step == SOTN_PLAYER_FALL) &&
              (speed > 163840 || speed < -163840)))
        animation = ledge ? 15 : 14;
    else if (player->previous_step == SOTN_PLAYER_JUMP || player->previous_step == SOTN_PLAYER_FALL)
        animation = ledge ? 30 : 29;
    Animate(player, animation);
}

static void MovementSmoke(SotnPlayer *player, Uint32 blueprint)
{
    if (player->movement_effects.smoke)
    {
        player->failed = true;
        SDL_SetError("Ordinary movement exceeded its selected one-update factory request");
        return;
    }
    player->movement_effects.smoke = true;
    player->movement_effects.smoke_blueprint = blueprint;
    player->movement_effects.smoke_source =
        (SotnPlayerFactorySource){.x = player->x.value,
                                  .y = player->y.value,
                                  .priority = 148,
                                  .facing_left = player->facing_left};
}
static void Walk(SotnPlayer *player, bool force)
{
    const bool ledge = (player->contact & 0x20) != 0;
    SetSpeed(player, WALK_SPEED);
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_WALK);
    if (!force)
        MovementSmoke(player, 0x50001);
    if (force)
    {
        if (player->animation.id != 13)
            Animate(player, 13);
    }
    else
        Animate(player, 7);
    if (player->turned)
        player->animation.id = 9;
    if (player->animation.id == 7 && ledge)
        player->animation.pose = 1;
    if (player->previous_step == SOTN_PLAYER_CROUCH)
        player->animation.pose = 4;
}

static void Crouch(SotnPlayer *player, unsigned int kind, Sint32 speed)
{
    player->vx = speed;
    player->vy = 0;
    SetStep(player, SOTN_PLAYER_CROUCH);
    switch (kind)
    {
    case 0:
        player->substep = 0;
        Animate(player, 17);
        break;
    case 1:
        player->substep = 4;
        Animate(player, 21);
        break;
    case 2:
        player->substep = 1;
        Animate(player, 16);
        break;
    default:
        player->substep = 3;
        Animate(player, 35);
        break;
    }
}

static void Fall(SotnPlayer *player)
{
    SetStep(player, SOTN_PLAYER_FALL);
    if (player->previous_step != SOTN_PLAYER_WALK)
        Animate(player, 28);
    player->vx = 0;
    player->vy = 2 * FIX_ONE;
    player->timers[6] = 8;
    player->timers[5] = (player->previous_contact & 1) ? 8 : 0;
    player->movement_flags = 16;
}

static void Jump(SotnPlayer *player)
{
    if (player->tight_space)
    {
        Fall(player);
        return;
    }
    if (Direction(player))
    {
        Animate(player, 26);
        SetSpeed(player, WALK_SPEED);
        player->movement_flags = 0;
    }
    else
    {
        Animate(player, 22);
        player->vx = 0;
        player->movement_flags = 4;
    }
    player->vy = JUMP_SPEED;
    SetStep(player, SOTN_PLAYER_JUMP);
    if (player->previous_step == SOTN_PLAYER_WALK)
        player->movement_flags |= 0x10;
    player->movement_flags &= ~1U;
}

void SotnPlayer_FormReturn(SotnPlayer *p, bool cramped)
{
    if (cramped)
    {
        if (p->tight_space == 1 || p->tight_space == 2)
        {
            Animate(p, 0xc7);
            p->vx = p->vy = 0;
            SetStep(p, SOTN_PLAYER_STUCK);
            p->sound = 0x6f2;
        }
        else
            Crouch(p, 0, 0);
        return;
    }
    Jump(p);
    if (!p->tight_space)
        p->movement_flags |= 1U;
    p->y.parts.high = (Sint16)(p->y.parts.high - 3);
    if (!(p->contact & 0x8000))
        p->vy = -FIX_ONE;
    p->movement_flags |= 0x100U;
    p->mist_lockout = 24;
}

void SotnPlayer_WolfReturn(SotnPlayer *p, bool cramped)
{
    if (cramped)
    {
        SotnPlayer_FormReturn(p, true);
        return;
    }
    Jump(p);
    if (!p->tight_space)
        p->movement_flags |= 1U;
    if (!(p->contact & 0x8000))
        p->vy = -FIX_ONE;
    p->movement_flags |= 0x100U;
    p->mist_lockout = 24;
}

void SotnPlayer_GravityLaunch(SotnPlayer *p)
{
    if (Direction(p))
        SetSpeed(p, 3 * FIX_ONE);
    else
        p->vx = 0;
    p->movement_flags = p->step == SOTN_PLAYER_JUMP ? p->movement_flags | 1U : 0;
    p->gravity_spawn = true;
    p->gravity_origin_x = p->x.value;
    p->gravity_origin_y = p->y.value;
    SetStep(p, SOTN_PLAYER_HIGH_JUMP);
    p->vy = -12 * FIX_ONE;
    Animate(p, 33);
    p->gravity.timer = 0;
}

static bool AttackView(SotnPlayer *player, Uint64 address, size_t size, const Uint8 **out)
{
    if (player->dra.bytes == NULL || address < 0x800a0000 || address > SDL_MAX_UINT32 ||
        (address & 1) != 0)
        return SDL_SetError("Unarmed attack references an invalid private DRA address");
    const size_t offset = address - 0x800a0000;
    if (offset > player->dra.size || size > player->dra.size - offset)
        return SDL_SetError("Unarmed attack references data outside private DRA");
    *out = player->dra.bytes + offset;
    return true;
}

static Uint16 Attack16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static Uint32 Attack32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static void ClearAttack(SotnPlayer *player)
{
    player->subweapon_cast = false;
    player->curse_failure = false;
    player->attack_mode = 0;
    player->attack = (SotnPlayerAttack){0};
}

static SotnAxeArmorFrame AxeFrame(const SotnPlayer *p)
{
    return (SotnAxeArmorFrame){.animation = p->animation,
                               .velocity_x = p->vx,
                               .velocity_y = p->vy,
                               .movement_flags = p->movement_flags,
                               .step = (Uint16)p->step,
                               .substep = p->substep,
                               .draw_flags = p->axe_armor.draw_flags,
                               .rotation = p->axe_armor.rotation,
                               .pivot_y = p->draw_pivot_y,
                               .opacity = p->axe_armor.opacity,
                               .facing_left = p->facing_left,
                               .knock_right = p->axe_armor.knock_right,
                               .turned = p->turned};
}
static void ApplyAxeFrame(SotnPlayer *p, const SotnAxeArmorFrame *f)
{
    p->animation = f->animation;
    p->vx = f->velocity_x;
    p->vy = f->velocity_y;
    p->step = (SotnPlayerStep)f->step;
    p->substep = f->substep;
    p->axe_armor.draw_flags = (Uint8)f->draw_flags;
    p->axe_armor.rotation = f->rotation;
    p->axe_armor.opacity = f->opacity;
    p->draw_pivot_y = f->pivot_y;
    p->facing_left = f->facing_left;
    p->turned = f->turned;
}
static bool AxePrepare(SotnPlayer *p, bool bootstrap)
{
    if (!p->axe_context || SotnPlayerStep_IsDead(p->step))
        return true;
    if (p->armor != 25)
        p->axe_overlay_owned = false;
    else if (!p->axe_overlay_owned)
    {
        // menu.c:LoadWeaponPrg reloads only on a family change. Preserve words through
        // same-family hand changes and scripted reentry; a new overlay uses actual file words.
        SotnEquipment equipment = {0};
        SotnDataView overlay = {0};
        if (!SotnEquipment_ReadAxeArmor(p->dra, &equipment) ||
            !SotnWeapon_Overlay(p->weapons, equipment, &overlay))
            return false;
        const Uint32 recovery = Attack32(overlay.bytes + 0x13b8);
        SDL_memcpy(&p->axe_armor.cursor.recovery, &recovery, sizeof(recovery));
        p->axe_armor.cursor.angle = Attack32(overlay.bytes + 0x13bc);
        p->axe_overlay_owned = true;
    }
    if (p->axe_armor.active == (p->armor == 25))
        return true;
    ClearAttack(p);
    p->shield = (SotnPlayerAttack){0};
    p->shield_cursor = (SotnShield){0};
    p->movement_flags = 0;
    p->axe_armor.source = (SotnPlayerFactorySource){
        .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left};
    if (p->armor == 25)
    {
        // The overlay enters40/42 before09328 tests HighJump. Retain its upside-down
        // pose attributes and facing; ordinary form cancellation would flip a second time.
        if (p->step == SOTN_PLAYER_HIGH_JUMP && p->gravity.upside_down)
        {
            p->axe_armor.rotation = 2048;
        }
        p->gravity.upside_down = false;
        SotnPlayer_BatCancel(p);
        SotnPlayer_WolfCancel(p);
        if (p->mist.active)
        {
            p->mist.active = p->mist.restoring = false;
            p->mist.interrupted = true;
        }
        p->dark_ticks = 0;
        p->axe_armor.active = p->axe_armor.entered = true;
        SotnAxeArmorFrame f = AxeFrame(p);
        if (!SotnAxeArmor_Enter(&f, (Uint32)p->contact))
            return false;
        ApplyAxeFrame(p, &f);
        p->animation_bank = 16;
        p->palette = 0x110;
        if (!SotnPlayerAfterimage_Reset(&p->afterimage, true, 10, &p->afterimage_timer15))
            return false;
        if (!bootstrap)
        {
            p->sound = 0x6f2;
            SotnPlayer_HoldInput(p, 32);
        }
    }
    else
    {
        p->axe_armor.active = false;
        p->axe_armor.exited = true;
        p->axe_armor.draw_flags = 0;
        p->axe_armor.rotation = 0;
        p->draw_pivot_x = p->draw_pivot_y = 0;
        p->animation_bank = 1;
        p->palette = 0x8100;
        if (p->contact & 1)
            Land(p, 0);
        else
            Fall(p);
    }
    return !p->failed;
}
static void AxeTick(SotnPlayer *p)
{
    const bool overlay_callback = p->step != SOTN_PLAYER_AXE_SCRIPT;
    SotnAxeArmorFrame f = AxeFrame(p);
    SotnAxeArmorEvents events = {0};
    if (!p->axe_armor.active ||
        !SotnAxeArmor_Tick(&p->axe_armor.cursor, &f,
                           (SotnAxeArmorInput){.executable = p->script_executable,
                                               .contacts = (Uint32)p->contact,
                                               .clock = p->damage_clock,
                                               .pressed = p->buttons,
                                               .tapped = p->tapped,
                                               .simulated_mode = p->simulated_pad.mode,
                                               .tight_space = p->tight_space != 0},
                           &events))
    {
        p->failed = true;
        return;
    }
    ApplyAxeFrame(p, &f);
    if (overlay_callback)
    {
        p->animation_bank = 16;
        p->palette = 0x110;
    }
    p->axe_armor.source = (SotnPlayerFactorySource){
        .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left};
    for (Uint16 i = 0; i < events.factory_count; ++i)
    {
        if (p->axe_armor.events.factory_count >= SDL_arraysize(p->axe_armor.events.factories))
        {
            SDL_SetError("Axe Armor exceeded its actual per-update factory requests");
            p->failed = true;
            return;
        }
        p->axe_armor.events.factories[p->axe_armor.events.factory_count++] = events.factories[i];
    }
    // The ordinary player audio owner consumes sounds; shared events retain factories only.
    for (Uint16 i = 0; i < events.sound_count; ++i)
    {
        if (!p->sound)
            p->sound = events.sounds[i];
        else
            p->extra_sound = events.sounds[i];
    }
}

static void Backdash(SotnPlayer *player)
{
    SetStep(player, SOTN_PLAYER_STAND);
    player->substep = 3;
    SetSpeed(player, -229376);
    player->vy = 0;
    Animate(player, 0xdb);
    MovementSmoke(player, 0);
    ClearAttack(player);
    player->sound = 0x617;
}

// A both-buttons combo spell's factory takes its hand bits at <<14 (6E42C.c var_s7): the plain
// DRA factory entity with the E0/F0 child offset.
static bool ReserveHandAt(SotnPlayer *player, SotnEquipment equipment, Uint32 hand, bool combo)
{
    if (!player->hand_factory)
        return true;
    SotnPlayerState captured = {0};
    bool allocated = false;
    const Uint32 packed = equipment.id ? equipment.blueprint | ((hand + 1U) << (combo ? 14 : 12)) |
                                             ((Uint32)(equipment.variant + hand * 128U) << 16)
                                       : (42U + hand) | (hand << 23);
    if (!SotnPlayer_GetState(player, &captured) ||
        !player->hand_factory(player->hand_context,
                              (SotnPlayerFactorySource){.x = player->x.value,
                                                        .y = player->y.value,
                                                        .priority = captured.priority,
                                                        .facing_left = player->facing_left},
                              packed, equipment.id, &allocated))
    {
        player->failed = true;
        return false;
    }
    return allocated;
}
static bool ReserveHand(SotnPlayer *player, SotnEquipment equipment, Uint32 hand)
{
    return ReserveHandAt(player, equipment, hand, false);
}

// 6E42C.c block_70: the shared curse and Red Rust failure animation/lock/voice.
static void HandFailure(SotnPlayer *player)
{
    ClearAttack(player);
    player->curse_failure = true;
    player->attack_mode = 0x8012;
    player->attack_lock = 255;
    player->substep = 0x51;
    player->sound = 0x6e7;
    if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK)
    {
        player->step = SOTN_PLAYER_STAND;
        Animate(player, 0xb5);
    }
    else if (player->step == SOTN_PLAYER_CROUCH)
        Animate(player, 0xb6);
    else
    {
        player->step = SOTN_PLAYER_JUMP;
        Animate(player, 0xb7);
    }
}

// 6E42C.c cases129/130/132: the food/throwing factory, successful-player cost commit, casting
// grunt (throws only) and the item animation. Returns false when the factory is exhausted.
static bool StartThrow(SotnPlayer *player, SotnEquipment equipment, Uint32 hand)
{
    const bool throwing = (equipment.kind == 129 || equipment.kind == 130);
    const SotnPlayerMedicineFactory factory =
        throwing ? player->throwing_factory : player->food_factory;
    void *context = throwing ? player->throwing_context : player->food_context;
    SotnPlayerState captured = {0};
    if (!SotnPlayer_GetState(player, &captured))
    {
        player->failed = true;
        return true;
    }
    bool allocated = false;
    const Uint32 packed = (throwing ? equipment.blueprint : 50U) | ((hand + 1U) << 12) |
                          ((Uint32)(equipment.variant + hand * 128U) << 16);
    if (!factory(context,
                 (SotnPlayerFactorySource){.x = player->x.value,
                                           .y = player->y.value,
                                           .priority = captured.priority,
                                           .facing_left = player->facing_left},
                 packed, equipment.id, &allocated))
    {
        player->failed = true;
        return true;
    }
    if (!allocated)
        return false;
    const SotnPlayerMedicineUse use = {.hand = (Uint16)hand,
                                       .item = equipment.id,
                                       .used = true,
                                       .keep = throwing && !equipment.consumable};
    if (throwing)
    {
        Uint16 random = 0;
        if (!player->throwing_random(context, &random) || !player->dra.bytes ||
            player->dra.size < 0xcf68)
        {
            player->failed = true;
            return true;
        }
        const Uint16 voice = random % 8U;
        if (voice < 4)
        {
            const Uint8 *p = player->dra.bytes + 0xcf60 + voice * 2U;
            player->throwing_grunt = (Uint16)(p[0] | (Uint16)p[1] << 8);
        }
        player->throwing = use;
    }
    else
        player->food = use;
    if (player->substep < 0x40)
    {
        Uint16 variant = (player->contact & 0x20) != 0;
        if (player->step == SOTN_PLAYER_CROUCH)
        {
            variant = 2;
            if (!player->tight_space && player->substep == 2)
            {
                variant = (player->contact & 0x20) != 0;
                SetStep(player, SOTN_PLAYER_STAND);
            }
        }
        if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_CROUCH)
            Animate(player, (Uint16)(equipment.animation + variant));
    }
    return true;
}

// Selected empty-hand paths in 6E42C.c, 71830.c, and EntityUnarmedAttack.
static bool StartAttack(SotnPlayer *player)
{
    Uint16 pressed = player->tapped & (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND);
    if (player->shield_cursor.phase != 1 && player->shield_cursor.phase != 2)
    {
        if (SotnEquipment_IsShield(player->equipment[0]) &&
            (player->buttons & SOTN_BUTTON_LEFT_HAND))
            pressed |= SOTN_BUTTON_LEFT_HAND;
        if (SotnEquipment_IsShield(player->equipment[1]) &&
            (player->buttons & SOTN_BUTTON_RIGHT_HAND))
            pressed |= SOTN_BUTTON_RIGHT_HAND;
    }
    if (pressed == 0)
        return false;
    if (player->subweapons != NULL && (player->buttons & SOTN_BUTTON_UP) &&
        (player->tapped & (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND)) &&
        player->subweapon_ticks == 0)
    {
        bool used = false;
        Uint16 animation = 0;
        SotnPlayerState state = {0};
        if (!SotnPlayer_GetState(player, &state) ||
            !SotnSubweapons_Use(player->subweapons, state, &used, &animation))
        {
            player->failed = true;
            return true;
        }
        if (used)
        {
            player->subweapon_ticks = 4;
            if (player->step == SOTN_PLAYER_STAND && player->attack_mode == 0)
            {
                Animate(player, (Uint16)(animation + ((player->contact & 0x20) ? 1 : 0)));
                player->subweapon_cast = true;
                player->subweapon_animation = player->animation.id;
                player->substep = 1;
            }
            return true;
        }
    }
    if (player->attack_mode & 0x8000)
        return false;
    Uint32 hand = pressed == SOTN_BUTTON_RIGHT_HAND ? 1U : 0U;
    SotnEquipment equipment = {0};
    // 6E42C.c both buttons: an equipped combo (CheckWeaponCombo's D_8013AEE4) casts its spell
    // with the combo's hand when its chain limit and MP allow; otherwise the left hand attacks.
    bool combo = false;
    const Uint16 combo_item = (Uint16)(player->combo & 0xffffU);
    if (pressed == (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND) && player->hand_factory &&
        !player->tight_space && combo_item && SotnPlayer_ComboSupported(combo_item))
    {
        SotnEquipment special = {0};
        Uint16 cost = 0;
        if (!SotnEquipment_Read(player->dra, combo_item, &special) ||
            !SotnEquipment_ReadCost(player->dra, combo_item, &cost))
        {
            player->failed = true;
            return true;
        }
        if (((special.chain_limit & 0x80) || player->combo_count < special.chain_limit) &&
            player->mp >= cost)
        {
            combo = true;
            hand = (player->combo & 0x80000000U) ? 1U : 0U;
            equipment = special;
        }
    }
    if (!combo && !SotnEquipment_Read(player->dra, player->equipment[hand], &equipment))
    {
        player->failed = true;
        return true;
    }
    if (equipment.kind == 132 &&
        (!player->food_factory || !player->food_consume || player->food_count[hand] >= 2))
        return false;
    if ((equipment.kind == 129 || equipment.kind == 130) &&
        (!player->throwing_factory || !player->throwing_consume || !player->throwing_random ||
         !equipment.chain_limit || player->throwing_count[hand] >= equipment.chain_limit))
        return false;
    if (equipment.id == 166)
    {
        if (!player->teleport_factory || !player->library_card_use ||
            (player->step != SOTN_PLAYER_STAND && player->step != SOTN_PLAYER_WALK &&
             player->step != SOTN_PLAYER_CROUCH))
            return false;
        SetStep(player, SOTN_PLAYER_TELEPORT);
        Animate(player, 0xf4);
        player->teleport.used = true;
        player->teleport.hand = (Uint16)hand;
        return true;
    }
    if (equipment.id >= 144 && equipment.id <= 162)
    {
        if (equipment.id == 146 || equipment.id == 147)
            return false;
        if (!player->medicine_factory || !player->medicine_consume || player->medicine_present)
            return false;
        bool allocated = false;
        SotnPlayerState captured = {0};
        if (!SotnPlayer_GetState(player, &captured))
        {
            player->failed = true;
            return true;
        }
        const SotnPlayerFactorySource source = {.x = player->x.value,
                                                .y = player->y.value,
                                                .priority = captured.priority,
                                                .facing_left = player->facing_left};
        const Uint32 packed = 55U | ((Uint32)(equipment.variant + hand * 128U) << 16);
        if (!player->medicine_factory(player->medicine_context, source, packed, equipment.id,
                                      &allocated))
        {
            player->failed = true;
            return true;
        }
        if (!allocated)
            return false;
        player->medicine =
            (SotnPlayerMedicineUse){.hand = (Uint16)hand, .item = equipment.id, .used = true};
        if (player->substep < 0x40 && player->step == SOTN_PLAYER_STAND)
            Animate(player, (Uint16)(equipment.animation + ((player->contact & 0x20) != 0)));
        return true;
    }
    if (player->curse_ticks && equipment.id != 0)
    {
        if ((equipment.kind == 132 && player->food_factory) ||
            ((equipment.kind == 129 || equipment.kind == 130) && player->throwing_factory) ||
            player->hand_factory)
        {
            const SotnPlayerMedicineFactory factory =
                (equipment.kind == 129 || equipment.kind == 130) ? player->throwing_factory
                : equipment.kind == 132                          ? player->food_factory
                                                                 : player->hand_factory;
            void *context = (equipment.kind == 129 || equipment.kind == 130)
                                ? player->throwing_context
                            : equipment.kind == 132 ? player->food_context
                                                    : player->hand_context;
            bool allocated = false;
            SotnPlayerState captured = {0};
            if (!SotnPlayer_GetState(player, &captured) ||
                !factory(context,
                         (SotnPlayerFactorySource){.x = player->x.value,
                                                   .y = player->y.value,
                                                   .priority = captured.priority,
                                                   .facing_left = player->facing_left},
                         0x10039, 0, &allocated))
            {
                player->failed = true;
                return true;
            }
        }
        HandFailure(player);
        return true;
    }
    if (equipment.kind == 132 || equipment.kind == 129 || equipment.kind == 130)
        return StartThrow(player, equipment, hand);
    if (equipment.kind == 128)
    {
        if (!ReserveHand(player, equipment, hand))
            return player->failed;
        player->shield_button = hand == 0 ? SOTN_BUTTON_LEFT_HAND : SOTN_BUTTON_RIGHT_HAND;
        if (player->step == SOTN_PLAYER_STAND)
        {
            player->substep = 2;
            Animate(player, (Uint16)(0x63 + ((player->contact & 0x20) ? 1 : 0)));
        }
        else if (player->step == SOTN_PLAYER_CROUCH)
        {
            player->substep = 0;
            Animate(player, 0x65);
        }
        return true;
    }
    if (equipment.id == 123 && player->sword_factory && player->sword_command.phase == 255 &&
        (player->contact & 1) && !player->tight_space)
    {
        SotnEquipment special = {0};
        Uint16 cost = 0;
        if (!SotnEquipment_ReadSwordSpecial(player->dra, equipment.id, &special, &cost))
        {
            player->failed = true;
            return true;
        }
        if (player->mp >= cost)
        {
            const SotnPlayerFactorySource source = {.x = player->x.value,
                                                    .y = player->y.value,
                                                    .priority = 148,
                                                    .facing_left = player->facing_left};
            bool allocated = false;
            if (!player->sword_factory(player->sword_context, source, hand ? 0x802036U : 0x1036U,
                                       197, &allocated))
            {
                player->failed = true;
                return true;
            }
            if (!allocated)
                return false;
            player->mp = (Uint16)(player->mp - cost);
            player->mp_spent = cost;
            player->weapon_startup = 4;
            player->vx = player->vy = 0;
            SetStep(player, SOTN_PLAYER_SWORD_WARP);
            Animate(player, special.animation);
            player->sound = 0x6f0;
            if (!player->sword_factory(player->sword_context, source, 0x14003dU, 0, &allocated))
                player->failed = true;
            return true;
        }
    }
    // 6E42C.c Combat knife: unk17 == specialMove. A lingering ordinary knife in slot16 makes
    // this swing the special; a lingering special makes it the ordinary knife. Either skips the
    // QCF/back-forward checks; with no knife in slot16 the ordinary checks follow.
    bool knife_resolved = combo;
    if (!combo && player->hand_factory && equipment.back_forward &&
        equipment.back_forward == equipment.special_move)
    {
        if (player->slot16_item == equipment.id)
        {
            SotnEquipment special = {0};
            if (!SotnEquipment_Read(player->dra, equipment.back_forward, &special) ||
                !(special.chain_limit & 0x80))
            {
                if (special.id && !(special.chain_limit & 0x80))
                    SDL_SetError("Combat knife special requires its actual chain owner");
                player->failed = true;
                return true;
            }
            equipment = special;
            knife_resolved = true;
        }
        else if (player->slot16_item == equipment.back_forward)
            knife_resolved = true;
    }
    // 6E42C.c block_32: a completed grounded QCF selects the equipped specialMove when its
    // chain limit and MP allow; otherwise block_38c's back-forward check follows.
    bool special_selected = false;
    // Sword of Dawn (0x11) additionally refuses ground that also sets vram flag0x40.
    if (!knife_resolved && player->hand_factory && equipment.special_move && equipment.id != 123 &&
        player->sword_command.phase == 255 && (player->contact & 1) &&
        (equipment.id != 17 || (player->contact & 0x41) == 1))
    {
        SotnEquipment special = {0};
        Uint16 cost = 0;
        if (!SotnEquipment_Read(player->dra, equipment.special_move, &special) ||
            !SotnEquipment_ReadCost(player->dra, equipment.special_move, &cost))
        {
            player->failed = true;
            return true;
        }
        // CheckChainLimit: unlimited specials pass (the active-attack case returned above);
        // limited ones (Moon rod's172) count their live children of this hand.
        const bool chained =
            (special.chain_limit & 0x80) || player->special_count[hand] < special.chain_limit;
        if (chained && player->mp >= cost)
        {
            equipment = special;
            special_selected = true;
        }
    }
    // A kind129 special (Shotel's176) then takes case129's thrown-item path: its own factory,
    // the casting grunt and the throw animation; the equipped item is never consumed.
    if (special_selected && (equipment.kind == 129 || equipment.kind == 130))
    {
        if (!player->throwing_factory || !player->throwing_random)
        {
            SDL_SetError("Thrown weapon special requires its actual throwing owner");
            player->failed = true;
            return true;
        }
        return StartThrow(player, equipment, hand);
    }
    // 6E42C.c block_38c: a completed back-forward command selects the equipped unk17 item.
    // Chain limit 0x80 allows it whenever no attack is active, as already checked above.
    if (!combo && !knife_resolved && !special_selected && player->hand_factory &&
        equipment.back_forward && player->back_forward.phase == 255)
    {
        SotnEquipment special = {0};
        if (!SotnEquipment_Read(player->dra, equipment.back_forward, &special) ||
            !(special.chain_limit & 0x80))
        {
            if (!(special.chain_limit & 0x80))
                SDL_SetError("Back-forward special requires its actual chain owner");
            player->failed = true;
            return true;
        }
        equipment = special;
    }
    if (!ReserveHandAt(player, equipment, hand, combo))
        return player->failed;
    // 6E42C.c case12 (Shotel): while a thrown specialMove lives with either hand's params
    // (CheckChainLimit for both hands), the swing becomes a punch. Shotel's own factory already
    // exists; the empty-hand factory follows (its result is ignored) and item0 supplies the
    // animation, kind and lock.
    if (player->hand_factory && equipment.kind == 12)
    {
        SotnEquipment special = {0};
        if (!SotnEquipment_Read(player->dra, equipment.special_move, &special))
        {
            player->failed = true;
            return true;
        }
        if (player->special_count[hand] >= special.chain_limit ||
            player->special_cross[hand] >= special.chain_limit)
        {
            SotnEquipment empty = {0};
            if (!SotnEquipment_Read(player->dra, 0, &empty))
            {
                player->failed = true;
                return true;
            }
            (void)ReserveHand(player, empty, hand);
            if (player->failed)
                return true;
            equipment = empty;
        }
    }
    // HasEnoughMp(mpUsage, 1) after the actual factory, for whichever item was selected: a
    // back-forward special (weapon51's182..185) runs without MP and then costs nothing.
    if (player->hand_factory && equipment.id)
    {
        Uint16 cost = 0;
        if (!SotnEquipment_ReadCost(player->dra, equipment.id, &cost))
        {
            player->failed = true;
            return true;
        }
        if (cost && player->mp >= cost)
        {
            player->mp = (Uint16)(player->mp - cost);
            player->mp_spent = (Uint16)(player->mp_spent + cost);
        }
    }
    // Estoc in a cramped space keeps its factory and MP but starts no attack.
    if (player->hand_factory && equipment.id == 95 && player->tight_space)
        return false;
    // Muramasa (kind29) first runs GetEquipProperties: one critical rand, then its attack
    // (halved by poison). At11 or more it is an ordinary sword; below, it shares Red Rust's roll.
    bool roll_failure = player->hand_factory && equipment.kind == 13;
    if (player->hand_factory && equipment.kind == 29)
    {
        Uint16 critical = 0;
        if (!player->hand_random(player->hand_context, &critical))
        {
            player->failed = true;
            return true;
        }
        roll_failure =
            (Uint16)(player->attributes.attack[hand] >> (player->poison_ticks ? 1 : 0)) < 11;
    }
    // Red Rust (kind13) rolls once after its actual hand factory; rand()&7 == 0 fails.
    if (roll_failure)
    {
        Uint16 roll = 0;
        if (!player->hand_random(player->hand_context, &roll))
        {
            player->failed = true;
            return true;
        }
        if (!(roll & 7U))
        {
            HandFailure(player);
            return true;
        }
    }
    // Kinds8/9/10/25 enter block_98 directly and skip the block_1000 casting grunt; kinds28
    // (pose0 hold10) and26 (lunge hold40, vertical stop) instead stand still and play
    // SFX_VO_ALU_ATTACK_B.
    // 6E42C.c case135 (weapon53 warps): SFX_VO_ALU_ATTACK_C, timer9, then func_8010ED54 enters
    // Player_SwordWarp with the item's animation and the profile20 outline; no block_98 setup.
    if (player->hand_factory && equipment.kind == 135)
    {
        player->sound = 0x6f0;
        player->weapon_startup = 4;
        player->vx = player->vy = 0;
        SetStep(player, SOTN_PLAYER_SWORD_WARP);
        Animate(player, equipment.animation);
        bool outlined = false;
        if (!player->sword_factory ||
            !player->sword_factory(player->sword_context,
                                   (SotnPlayerFactorySource){.x = player->x.value,
                                                             .y = player->y.value,
                                                             .priority = 148,
                                                             .facing_left = player->facing_left},
                                   0x14003dU, 0, &outlined))
            player->failed = true;
        return true;
    }
    // Kinds19..22 (brands) hold pose0 like kind28 but for forty updates.
    const bool hold = equipment.kind == 28 || equipment.kind == 26 ||
                      (equipment.kind >= 19 && equipment.kind <= 22);
    const bool grunt = equipment.kind != 8 && equipment.kind != 9 && equipment.kind != 10 &&
                       equipment.kind != 25 && equipment.kind != 24 && equipment.kind != 23 &&
                       equipment.kind != 27 && !hold;
    // Kind23 (weapon51 specials) stands, turns to the input and lunges at speed5 (halved by
    // block_98); kind27 (Estoc) lunges at speed4 halved, staying airborne when off the ground.
    if (player->hand_factory && (equipment.kind == 23 || equipment.kind == 27))
    {
        (void)Direction(player);
        const Sint32 speed = equipment.kind == 23 ? 5 * FIX_ONE : 4 * FIX_ONE;
        player->vx = player->facing_left ? -speed : speed;
        player->sound = 0x6ef;
        if (equipment.kind == 27)
        {
            player->vx = player->vx >= 0 ? player->vx / 2 : -((-player->vx + 1) / 2);
            if (player->contact & 1)
            {
                player->step = SOTN_PLAYER_STAND;
                player->vy = 0;
            }
            else
                player->step = SOTN_PLAYER_JUMP;
        }
        else
        {
            player->step = SOTN_PLAYER_STAND;
            player->vy = 0;
        }
    }
    // Kind24 (Iron Fist/Tulkas punch rush) keeps an airborne player in Player_Jump and stops both
    // velocities for a0x80-update D_80139824 hold.
    if (player->hand_factory && equipment.kind == 24)
    {
        player->step = player->step == SOTN_PLAYER_FALL || player->step == SOTN_PLAYER_JUMP
                           ? SOTN_PLAYER_JUMP
                           : SOTN_PLAYER_STAND;
        player->attack_hold = 0x80;
        player->vx = player->vy = 0;
        player->sound = 0x6ef;
    }
    if (player->hand_factory && hold)
    {
        player->step = SOTN_PLAYER_STAND;
        player->attack_hold = equipment.kind == 28 ? 10 : 40;
        if (equipment.kind == 26)
            player->vy = 0;
        player->vx = 0;
        player->sound = 0x6ef;
    }
    if (player->hand_factory && grunt)
    {
        Uint16 random = 0;
        if (!player->hand_random(player->hand_context, &random) || !player->dra.bytes ||
            player->dra.size < 0xcf68)
        {
            player->failed = true;
            return true;
        }
        const Uint16 voice = random % 8U;
        if (voice < 4)
            player->hand_grunt = Attack16(player->dra.bytes + 0xcf60 + voice * 2U);
        player->hand_used = true;
    }
    player->subweapon_cast = false;
    player->attack_equipment = equipment;
    Sint32 critical = equipment.critical_rate;
    if (equipment.id != 0 && !player->hand_factory)
    {
        const Uint16 value = (Uint16)(player->attributes.total[3] * 2 +
                                      (SotnCombat_Random(&player->damage_random) & 15));
        Sint32 root = 0;
        while ((root + 1) * (root + 1) <= value)
            ++root;
        critical += root - 5;
    }
    player->attack_critical = (Uint8)SDL_clamp(critical, 0, 255);
    player->attack_power =
        (Uint16)(player->attributes.attack[hand] >> (player->poison_ticks ? 1 : 0));
    // 6E42C.c case5 (Moon rod special172): grounded steps stop, take the ledge variant and
    // substep0x45; an airborne start keeps its state. Both keep the casting grunt above.
    if (player->hand_factory && equipment.kind == 5)
    {
        if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK ||
            player->step == SOTN_PLAYER_CROUCH)
        {
            Animate(player, (Uint16)(equipment.animation + ((player->contact & 0x20) ? 1 : 0)));
            player->attack_mode = (Uint16)(0x8001 + equipment.kind);
            player->attack_lock = equipment.lock;
            player->step = SOTN_PLAYER_STAND;
            player->substep = (Uint16)(0x40 + equipment.kind);
            player->vx = 0;
            player->attack_button =
                pressed == SOTN_BUTTON_RIGHT_HAND ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND;
        }
        return true;
    }
    Uint16 variant = (player->contact & 0x20) ? 1 : 0;
    if (player->hand_factory && equipment.kind == 27)
    {
        // Kind27 bypasses block_98: it keeps its own step and the ledge variant.
    }
    else if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK)
    {
        player->vx = player->vx >= 0 ? player->vx / 2 : -((-player->vx + 1) / 2);
        player->step = SOTN_PLAYER_STAND;
    }
    else if (player->step == SOTN_PLAYER_CROUCH)
    {
        variant = (player->buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)) ? 3 : 2;
        if (player->tight_space == 0 && player->substep == 2)
        {
            variant = (player->contact & 0x20) ? 1 : 0;
            player->step = SOTN_PLAYER_STAND;
        }
    }
    else
    {
        variant = player->vy > 0 ? 5 : 4;
        if (player->vy > 0 && (player->buttons & SOTN_BUTTON_DOWN) &&
            (equipment.kind != 0 || (player->buttons & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT))))
            variant = 6;
    }
    Animate(player, (Uint16)(equipment.animation + variant));
    player->substep = (Uint16)(0x40 + equipment.kind);
    player->attack_mode = (Uint16)(0x8001 + equipment.kind);
    player->attack_lock = equipment.lock;
    player->weapon_startup = 4;
    player->attack_button =
        pressed == SOTN_BUTTON_RIGHT_HAND ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND;
    return true;
}

static bool TickShield(SotnPlayer *player)
{
    if (player->hand_factory)
    {
        player->shield = (SotnPlayerAttack){0};
        return true;
    }
    Uint16 button = player->shield_button;
    if (button == 0)
        button = player->equipment[0] == 16 ? SOTN_BUTTON_LEFT_HAND : SOTN_BUTTON_RIGHT_HAND;
    const Uint32 hand = button == SOTN_BUTTON_RIGHT_HAND ? 1U : 0U;
    const bool disabled =
        player->equipment[hand] != 16 || player->curse_ticks != 0 || player->curse_failure ||
        SotnPlayerStep_IsDead(player->step) || player->step == SOTN_PLAYER_HIT ||
        player->step == SOTN_PLAYER_STONE || player->step == SOTN_PLAYER_MIST ||
        player->step == SOTN_PLAYER_UNMORPH_MIST || player->step == SOTN_PLAYER_WOLF ||
        player->step == SOTN_PLAYER_UNMORPH_WOLF || player->step == SOTN_PLAYER_BAT ||
        player->step == SOTN_PLAYER_UNMORPH_BAT || player->step == SOTN_PLAYER_STUCK ||
        player->step == SOTN_PLAYER_SCRIPT_REACTION || player->step == SOTN_PLAYER_BOSS_GRAB ||
        player->step == SOTN_PLAYER_SWORD_WARP || player->step == SOTN_PLAYER_TELEPORT ||
        player->step == SOTN_PLAYER_DARK || player->step == SOTN_PLAYER_SUMMON ||
        player->step == SOTN_PLAYER_TETRA || player->step == SOTN_PLAYER_HELLFIRE ||
        player->step == SOTN_PLAYER_SOUL;
    const bool starting = player->shield_cursor.phase == 0;
    if (!SotnShield_Tick(player->weapons, player->animation.id, player->animation.pose,
                         player->step == SOTN_PLAYER_CROUCH, (player->buttons & button) != 0,
                         disabled, &player->shield_cursor))
        return false;
    if (starting && player->shield_cursor.phase)
        player->shield_power = player->poison_ticks ? 0 : 1;
    const SotnWeaponFrame f = player->shield_cursor.frame;
    player->shield = (SotnPlayerAttack){.offset_x = f.x,
                                        .offset_y = f.y,
                                        .width = f.width,
                                        .height = f.height,
                                        .sprite = f.sprite,
                                        .active = f.width != 0 && f.height != 0,
                                        .equipment = player->shield_cursor.phase ? 16 : 0,
                                        .right_hand = hand != 0,
                                        .hit_type = 4,
                                        .hit_effect = 1,
                                        .invincibility_frames = 6,
                                        .power = player->shield_power};
    return true;
}

static bool ReadAttack(SotnPlayer *player)
{
    player->attack = (SotnPlayerAttack){0};
    if (player->hand_factory)
        return true;
    if (player->subweapon_cast || player->curse_failure)
        return true;
    if (player->attack_mode != 0 && player->attack_equipment.id != 0)
    {
        SotnWeaponFrame weapon = {0};
        const SotnEquipment equipment = player->attack_equipment;
        if (!SotnWeapon_ReadFrame(player->weapons, equipment, player->animation.id,
                                  player->animation.pose, player->animation.remaining, &weapon))
            return false;
        player->attack =
            (SotnPlayerAttack){.offset_x = weapon.x,
                               .offset_y = weapon.y,
                               .width = weapon.width,
                               .height = weapon.height,
                               .element = equipment.element,
                               .hit_type = equipment.hit_type,
                               .hit_effect = equipment.hit_effect,
                               .stun_frames = equipment.stun,
                               .invincibility_frames = equipment.invincibility,
                               .critical_rate = player->attack_critical,
                               .active = weapon.width != 0 && weapon.height != 0,
                               .right_hand = player->attack_button == SOTN_BUTTON_RIGHT_HAND,
                               .sound = weapon.sound != 0,
                               .sound_id = weapon.sound,
                               .power = player->attack_power,
                               .equipment = equipment.id,
                               .sprite = weapon.sprite};
        if (equipment.id == 18 && player->animation.id < equipment.animation + 2 &&
            player->animation.pose == 6)
            player->attack.hit_type = 4;
        return true;
    }
    if (player->attack_mode == 0 || player->animation.id < 0x24 || player->animation.id > 0x2a ||
        player->animation.remaining < 0)
        return true;
    const Uint8 *descriptor = NULL, *pointer = NULL, *frame = NULL, *box = NULL, *equipment = NULL;
    if (!AttackView(player, 0x800ad53c, 16, &descriptor) ||
        !AttackView(player, (Uint64)Attack32(descriptor) + (player->animation.id - 0x24U) * 4, 4,
                    &pointer))
        return false;
    if (player->animation.pose >= 32 ||
        !AttackView(player, (Uint64)Attack32(pointer) + (Uint32)player->animation.pose * 2, 2,
                    &frame))
        return SDL_SetError("Unarmed attack exceeds its bounded frame stream");
    const Uint16 packed = Attack16(frame);
    if (packed == 0xffff)
        return true;
    const Uint32 property = (packed >> 9) & 127;
    if (property >= 12 ||
        !AttackView(player, (Uint64)Attack32(descriptor + 4) + property * 4, 4, &box) ||
        !AttackView(player, 0x800a00c0, 4, &pointer) ||
        !AttackView(player, Attack32(pointer), 50, &equipment))
        return SDL_SetError("Unarmed hitbox or equipment data is invalid");
    player->attack.offset_x = (Sint16)(box[0] < 128 ? box[0] : (Sint32)box[0] - 256);
    player->attack.offset_y = (Sint16)(box[1] < 128 ? box[1] : (Sint32)box[1] - 256);
    player->attack.width = box[2];
    player->attack.height = box[3];
    player->attack.power = player->attack_power;
    player->attack.sound_id = 0x60c;
    player->attack.element = Attack16(equipment + 12);
    player->attack.hit_type = Attack16(equipment + 40);
    player->attack.hit_effect = Attack16(equipment + 42);
    player->attack.stun_frames = Attack16(equipment + 38);
    player->attack.invincibility_frames = equipment[26];
    player->attack.critical_rate = (Uint8)SDL_min(255, Attack16(equipment + 48));
    player->attack.right_hand = player->attack_button == SOTN_BUTTON_RIGHT_HAND;
    player->attack.active = box[2] != 0 && box[3] != 0;
    player->attack.sound =
        player->animation.remaining == 1 && player->animation.pose == descriptor[13];
    return true;
}

static bool Transitions(SotnPlayer *player, Uint32 flags)
{
    if ((flags & 8) && player->attack_mode == 0)
        (void)Direction(player);
    Sint32 gravity = (flags & 0x8000) ? GRAVITY : 0;
    if (flags & 0x10000)
        gravity = player->vy < 24576 && player->vy > -8192 && !(player->movement_flags & 0x20) &&
                          (player->buttons & SOTN_BUTTON_JUMP)
                      ? 2252
                      : GRAVITY;
    if (flags & 0x200)
        gravity = player->vy < 24576 && player->vy > -8192 ? 5632 : GRAVITY;
    if (player->water.feet > 40)
        gravity /= 4;
    const Uint32 velocity = (Uint32)player->vy + (Uint32)gravity;
    SDL_memcpy(&player->vy, &velocity, sizeof(velocity));
    player->vy = SDL_min(7 * FIX_ONE, player->vy);
    if ((flags & 0x80) && (player->contact & 2) && player->vy < -FIX_ONE)
        player->vy = -FIX_ONE;
    if (player->vy >= 0 && (flags & 1) && (player->contact & 1))
    {
        const bool attacking = player->attack_mode != 0;
        const bool sliding =
            !attacking && (player->substep == 0x70 || player->step == SOTN_PLAYER_JUMP);
        const bool hard =
            player->vy > 450560 || (!player->attack_mode && !(player->movement_flags & 0x10) &&
                                    (player->vx > 2 * FIX_ONE || player->vx < -2 * FIX_ONE));
        if (player->vy > 450560)
            Crouch(player, sliding ? 3U : 1U, sliding ? player->vx / 2 : 0);
        else if (player->movement_flags & 0x10)
            Walk(player, true);
        else if (!attacking && (player->vx > 2 * FIX_ONE || player->vx < -2 * FIX_ONE))
            Land(player, player->vx);
        else
            Land(player, 0);
        ClearAttack(player);
        player->sound = hard ? 0x647 : 0x64c;
        if (hard)
            MovementSmoke(player, 0);
        return true;
    }
    if (player->vy >= 0 && (flags & 0x20000) && (player->contact & 1))
    {
        Crouch(player, 3, player->vx);
        player->sound = 0x647;
        MovementSmoke(player, 0);
        return true;
    }
    if ((flags & 4) && !(player->contact & 1))
    {
        Fall(player);
        return true;
    }
    if ((flags & 0x1000) && StartAttack(player))
        return true;
    if ((flags & 0x10) && !(player->attack_mode & 0x8000) && (player->tapped & SOTN_BUTTON_JUMP))
    {
        Jump(player);
        return true;
    }
    if ((flags & 0x20) && !(player->attack_mode & 0x8000) &&
        (player->relics[SOTN_RELIC_LEAP] & 2) && (player->tapped & SOTN_BUTTON_JUMP) &&
        !(player->movement_flags & 1))
    {
        // 6E42C.c:func_8010E940 retains Jump/Fall and attack state, unlike an ordinary jump.
        player->movement_flags |= 0x21;
        Animate(player, 32);
        player->substep = 0;
        player->vy = player->tight_space ? 0 : -278528;
        return true;
    }
    if ((flags & 0x2000) && !(player->attack_mode & 0x8000) && (player->buttons & SOTN_BUTTON_DOWN))
    {
        Crouch(player, 2, 0);
        return true;
    }
    if ((flags & 0x40000) && !(player->attack_mode & 0x8000) && player->animation.id != 0xdb &&
        ((player->attack_mode & 0x7fff ? player->buttons : player->tapped) & SOTN_BUTTON_CANCEL))
    {
        Backdash(player);
        return true;
    }
    return false;
}

// 71830.c handles substeps0x40/0x59 (kind0 empty hand, fists and kind25) separately from
// weapon kinds: shorter crouch window, fixed0x24 stand-up and the pose7 combo continuation.
static bool UnarmedKind(const SotnPlayer *player)
{
    return player->substep == 0x40 || player->substep == 0x59;
}

static void StandTick(SotnPlayer *player)
{
    if (player->subweapon_cast && player->animation.id != player->subweapon_animation)
        player->subweapon_cast = false;
    // 71830.c: the kind23/27 lunges (substeps0x57/0x5B) glide with the 1/16 deceleration and
    // without the landing transition until pose5.
    const bool lunge = player->attack_mode != 0 &&
                       (player->substep == 0x57 || player->substep == 0x5b) &&
                       player->animation.pose < 5;
    if (Transitions(player, lunge ? 0x43018 : 0x4301c))
        return;
    Decelerate(player, lunge ? 4096 : 8192);
    SotnPlayer_WaterBreath(player, false,
                           player->shield_cursor.phase == 1 || player->shield_cursor.phase == 2);
    Uint32 flags = 3;
    if (player->substep == 1)
    {
        flags = 1;
        if (player->chair_kind)
        {
            Animate(player, player->chair_kind <= 2 ? 0xf2 : 0xf3);
            player->facing_left = (player->chair_kind & 1U) == 0;
            player->substep = 4;
            player->chair_sleep = 0x1800;
        }
        else if (!(player->buttons & SOTN_BUTTON_UP))
            flags = 5;
    }
    else if (player->substep == 2)
    {
        flags = 3;
        if (player->animation.id != 10 && player->animation.id != 11 &&
            player->animation.id != 14 && player->animation.id != 15)
        {
            if (player->animation.id != 0x63)
                Animate(player, 0x63);
            if (player->shield_cursor.phase != 1 && player->shield_cursor.phase != 2)
                flags = 5;
        }
    }
    else if (player->substep == 3)
    {
        flags = 0;
        if (player->animation.pose >= 4 || player->animation.id == 14)
            flags = 1;
        if (player->animation.pose >= 7 || player->animation.remaining < 0)
            flags = 7;
    }
    else if (player->substep == 4)
    {
        if (player->animation.id == 0xf2 && player->chair_sleep && --player->chair_sleep == 0)
        {
            player->sleep_event = true;
            player->chair_sleep = 0x200;
        }
        flags = 1;
    }
    else if (player->attack_mode != 0 && player->substep == 0x45)
    {
        // 71830.c stand substep0x45 (kind5): no movement window, standing up at the END.
        SotnPlayer_AfterimageReset(player);
        flags = 0;
        if (player->animation.remaining < 0)
            flags = 0xf;
    }
    else if (player->attack_mode != 0 && (player->substep == 0x57 || player->substep == 0x5b))
    {
        // 71830.c stand substeps0x57/0x5B: grounded dust FACTORY(69,1) on odd game-timer
        // updates while the pose is six short of the lock; leaving the ground at pose4's last
        // update switches to animation0x6C pose5 in Player_Jump.
        SotnPlayer_AfterimageReset(player);
        flags = 0;
        if (player->animation.pose < player->attack_lock - 6 && (player->contact & 1) &&
            (player->mist_game_timer & 1U))
            MovementSmoke(player, 0x10045);
        if (player->animation.pose >= player->attack_lock)
        {
            player->attack_mode &= 0x7fff;
            flags = 0x19;
        }
        if (player->animation.remaining < 0)
            flags = 0xf;
        if (player->animation.pose == 4 && player->animation.remaining == 1 &&
            !(player->contact & 1))
        {
            player->animation.pose = 5;
            player->animation.id = 0x6c;
            player->step = SOTN_PLAYER_JUMP;
            player->movement_flags = 1;
        }
    }
    else if (player->attack_mode != 0 && player->substep == 0x58)
    {
        // 71830.c stand substep0x58: the punch rush rewinds pose4 to pose2 while D_80139824
        // lasts; pose6, a Down tap or the backward direction resumes substep0x40.
        SotnPlayer_AfterimageReset(player);
        if (player->attack_hold > 0)
            --player->attack_hold;
        if (player->animation.pose == 4 && player->animation.remaining == 1 && player->attack_hold)
            player->animation.pose = 2;
        if (player->animation.pose == 6 || (player->tapped & SOTN_BUTTON_DOWN) ||
            (player->buttons & (player->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT)))
            player->substep = 0x40;
        flags = 10;
    }
    else if (player->attack_mode != 0 && player->substep == 0x5a)
    {
        // 71830.c substep0x5A: the lunge holds pose6 while D_80139824 lasts; pose10, a Down/Up
        // tap or the backward direction resumes the item's ordinary substep0x50.
        SotnPlayer_AfterimageReset(player);
        if (player->attack_hold > 0)
            --player->attack_hold;
        if (player->animation.pose == 6 && player->animation.remaining == 1 && player->attack_hold)
            player->animation.remaining = 2;
        if (player->animation.pose == 10 ||
            (player->tapped & (SOTN_BUTTON_DOWN | SOTN_BUTTON_UP)) ||
            (player->buttons & (player->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT)))
            player->substep = 0x50;
        flags = 0;
    }
    else if (player->attack_mode != 0 &&
             (player->substep == 0x5c || (player->substep >= 0x53 && player->substep <= 0x56)))
    {
        // 71830.c substeps0x53..0x56/0x5C hold pose0 on D_80139824, with no crouch window.
        SotnPlayer_AfterimageReset(player);
        flags = 0;
        if (player->animation.pose == 0)
            player->animation.remaining = --player->attack_hold == 0 ? 1 : 2;
        if (player->animation.pose >= player->attack_lock)
        {
            player->attack_mode &= 0x7fff;
            flags = 0x1b;
        }
        if (player->animation.remaining < 0)
            flags = 0xf;
    }
    else if (player->attack_mode != 0 && player->substep >= 0x40)
    {
        SotnPlayer_AfterimageReset(player);
        flags = 0;
        if ((player->substep == 0x47 || player->substep == 0x49) && player->animation.pose == 6 &&
            (player->buttons & player->attack_button))
            player->animation.remaining = 2;
        if (player->curse_failure)
        {
            if (player->animation.remaining < 0)
                flags = 0xf;
        }
        else if (player->animation.pose < player->attack_lock)
        {
            if (player->animation.pose < (UnarmedKind(player) ? 2 : 3))
            {
                (void)Direction(player);
                if (player->buttons & SOTN_BUTTON_DOWN)
                {
                    player->step = SOTN_PLAYER_CROUCH;
                    player->animation.id = (Uint16)(player->attack_equipment.animation + 2);
                }
            }
        }
        else
        {
            player->attack_mode &= 0x7fff;
            flags = UnarmedKind(player) ? 0xb : 0x1b;
            if (UnarmedKind(player) && player->animation.pose == 7)
            {
                if (player->buttons & player->attack_button)
                    player->animation.remaining = 2;
                else
                    flags = 0xf;
            }
            else if (!UnarmedKind(player) && player->animation.remaining < 0)
                flags = 0xf;
        }
    }
    bool changed = false;
    if (flags & 4)
    {
        Land(player, 0);
        changed = true;
    }
    if ((flags & 2) && (player->buttons & SOTN_BUTTON_UP) && player->shield_cursor.phase != 1 &&
        player->shield_cursor.phase != 2)
    {
        Animate(player, (player->contact & 0x20) ? 1 : 0);
        player->substep = 1;
        changed = true;
        if (player->chair_kind)
        {
            Animate(player, player->chair_kind <= 2 ? 0xf2 : 0xf3);
            player->facing_left = (player->chair_kind & 1U) == 0;
            player->substep = 4;
            player->chair_sleep = 0x1800;
        }
    }
    if ((flags & 1) && Direction(player))
    {
        Walk(player, false);
        changed = true;
    }
    if (changed && (flags & 8))
        ClearAttack(player);
}

static void JumpTick(SotnPlayer *player)
{
    // 71830.c PlayerStepJump substep0x58 hovers: no deceleration, rise clamp or transitions
    // (gravity/landing) until pose4 or a Down/Up tap resumes substep0x40.
    if (player->attack_mode != 0 && player->substep == 0x58)
    {
        SotnPlayer_AfterimageReset(player);
        if (player->attack_hold > 0)
            --player->attack_hold;
        if (player->animation.pose == 3 && player->animation.remaining == 1 && player->attack_hold)
            player->animation.pose = 1;
        if (player->animation.pose == 4 || (player->tapped & (SOTN_BUTTON_DOWN | SOTN_BUTTON_UP)))
            player->substep = 0x40;
        if (player->movement_flags & 0x100)
            player->vx = 0;
        return;
    }
    Decelerate(player, 4096);
    if (player->vy < -FIX_ONE)
    {
        if (!(player->movement_flags & 0x40) && !(player->buttons & SOTN_BUTTON_JUMP))
            player->vy = -FIX_ONE;
        if (player->contact & 2)
        {
            player->vy = -16384;
            player->movement_flags |= 0x20;
        }
    }
    if (Transitions(player, 0x11029))
        return;
    if (player->substep == 0x70)
    {
        if (player->movement_flags & 0x80)
        {
            Jump(player);
            if (!(player->buttons & SOTN_BUTTON_JUMP))
                player->vy = -278528;
            player->movement_flags |= 0x40;
        }
        if ((player->substep > 1 || (player->movement_flags & 1)) &&
            (player->movement_flags & 0x100))
            player->vx = 0;
        return;
    }
    if (player->attack_mode != 0 && (player->substep == 0x57 || player->substep == 0x5b))
    {
        // 71830.c jump substeps0x57/0x5B decelerate again; animation0x6C ends into the ordinary
        // fall/jump, leaving the back-forward recognizer in its0xFE hold for sixteen updates.
        SotnPlayer_AfterimageReset(player);
        Decelerate(player, 4096);
        if (player->animation.id == 0x6c)
        {
            if (player->animation.remaining < 0)
            {
                player->substep = player->vy > FIX_ONE ? 1 : 0;
                Animate(player, player->vy > FIX_ONE ? 31 : 27);
                ClearAttack(player);
                player->movement_flags = 1;
                player->back_forward.phase = 0xfe;
                player->back_forward.timer = 0x10;
            }
        }
        else if (player->animation.pose == 4 && player->animation.remaining == 1)
        {
            player->animation.pose = 5;
            player->animation.id = 0x6c;
        }
        if ((player->substep > 1 || (player->movement_flags & 1)) &&
            (player->movement_flags & 0x100))
            player->vx = 0;
        return;
    }
    if (player->attack_mode != 0 && player->substep >= 0x40)
    {
        SotnPlayer_AfterimageReset(player);
        if (player->buttons & SOTN_BUTTON_LEFT)
            player->vx = -WALK_SPEED;
        if (player->buttons & SOTN_BUTTON_RIGHT)
            player->vx = WALK_SPEED;
        if (player->animation.remaining < 0)
        {
            player->substep = player->vy > FIX_ONE ? 1 : 0;
            Animate(player, player->vy > FIX_ONE ? 31 : 27);
            ClearAttack(player);
        }
        if ((player->substep > 1 || (player->movement_flags & 1)) &&
            (player->movement_flags & 0x100))
            player->vx = 0;
        return;
    }
    const int direction = Direction(player);
    if (direction != 0)
    {
        if (player->substep == 0 && (player->animation.id == 22 || player->animation.id == 25))
            Animate(player, 24);
        SetSpeed(player, WALK_SPEED);
    }
    else if (player->substep == 0 && (player->animation.id == 26 || player->animation.id == 24))
        Animate(player, 25);
    if (direction <= 0)
        player->movement_flags &= ~0x10U;
    if (player->substep == 0 && player->vy > 0)
    {
        if (player->animation.id != 27)
            Animate(player, 27);
        player->substep = 1;
    }
    if (player->substep <= 1 && !(player->movement_flags & 1))
        return;
    if (player->substep <= 1 && (player->buttons & SOTN_BUTTON_DOWN) &&
        (player->tapped & SOTN_BUTTON_JUMP))
    {
        Animate(player, 34);
        player->substep = 0x70;
        player->movement_flags &= ~0x80U;
        player->vy = 6 * FIX_ONE;
        if (direction != 0)
            SetSpeed(player, 294912);
        player->sound = 0x6f0;
    }
    if (player->movement_flags & 0x100)
        player->vx = 0;
}

static void CrouchTick(SotnPlayer *player)
{
    if ((player->tapped & SOTN_BUTTON_JUMP) && !(player->attack_mode & 0x8000))
    {
        if (player->buttons & SOTN_BUTTON_DOWN)
        {
            for (int i = 0; i < 4; ++i)
                if (player->floor[i].effects & SOTN_EFFECT_SOLID_FROM_ABOVE)
                {
                    player->timers[7] = 8;
                    return;
                }
        }
        if (!player->tight_space)
        {
            Jump(player);
            return;
        }
    }
    if (Transitions(player, 0x100c))
        return;
    Decelerate(player, 8192);
    const bool raised = player->shield_cursor.phase == 1 || player->shield_cursor.phase == 2;
    if (raised && player->animation.id == 17)
    {
        player->animation.id = 0x65;
        player->animation.remaining = 2;
        player->animation.pose = 3;
    }
    else if (!raised && player->animation.id == 0x65)
        Animate(player, 17);
    Uint32 flags = 0;
    switch (player->substep)
    {
    case 0:
        SotnPlayer_WaterBreath(
            player, true, player->shield_cursor.phase == 1 || player->shield_cursor.phase == 2);
        flags = 6;
        break;
    case 1:
        if (!(player->buttons & SOTN_BUTTON_DOWN))
        {
            flags = 1;
            if (player->animation.pose >= 4 || player->animation.id == 0x65)
            {
                Animate(player, 19);
                player->substep = 2;
                player->animation.pose = player->tight_space == 1   ? 0
                                         : player->tight_space == 2 ? 1
                                                                    : 2;
                return;
            }
        }
        else if (player->animation.id == 0x65)
            player->substep = 0;
        else if (player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 3:
    case 4:
        SotnPlayer_AfterimageReset(player);
        if (player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 5:
        SotnPlayer_AfterimageReset(player);
        if (player->tight_space == player->animation.pose || player->animation.remaining < 0)
            flags = 0x20;
        break;
    case 2:
        flags = 1;
        if (player->buttons & SOTN_BUTTON_DOWN)
        {
            Crouch(player, 0, player->vx);
            break;
        }
        if (player->tight_space)
        {
            SotnPlayer_AfterimageReset(player);
            const Uint16 limit = (Uint16)(player->tight_space - 1);
            if (player->animation.pose >= limit)
            {
                player->animation.pose = limit;
                player->animation.remaining = 3;
            }
        }
        else if (player->animation.remaining < 0)
            Land(player, 0);
        break;
    default:
        if (player->attack_mode == 0 || player->substep < 0x40)
        {
            player->failed = true;
            SDL_SetError("Unsupported Alucard crouch substate");
            return;
        }
        SotnPlayer_AfterimageReset(player);
        if ((player->substep == 0x47 || player->substep == 0x49) && player->animation.pose == 6 &&
            (player->buttons & player->attack_button))
            player->animation.remaining = 2;
        if (player->curse_failure)
        {
            if (player->animation.remaining < 0)
                flags = 0x2e;
        }
        else if (player->animation.pose < player->attack_lock)
        {
            if (player->animation.pose < (UnarmedKind(player) ? 2 : 3))
            {
                (void)Direction(player);
                if (!(player->buttons & SOTN_BUTTON_DOWN) && player->tight_space == 0)
                {
                    player->step = SOTN_PLAYER_STAND;
                    player->animation.id =
                        (Uint16)(player->attack_equipment.animation +
                                 (!UnarmedKind(player) && (player->contact & 0x20) ? 1 : 0));
                }
            }
        }
        else
        {
            player->attack_mode &= 0x7fff;
            flags = 0xe;
            if (UnarmedKind(player) && player->animation.pose == 7)
            {
                if (player->buttons & player->attack_button)
                    player->animation.remaining = 2;
                else
                    flags = 0x2e;
            }
            else if (!UnarmedKind(player) && player->animation.remaining < 0)
                flags = 0x2e;
        }
        break;
    }
    bool changed = false;
    if (flags & 0x20)
    {
        Crouch(player, 0, 0);
        changed = true;
    }
    if ((flags & 2) && player->turned)
    {
        Animate(player, 20);
        player->substep = 0;
        changed = true;
    }
    if ((flags & 4) && !(player->buttons & SOTN_BUTTON_DOWN))
    {
        Animate(player, 19);
        player->substep = 2;
        changed = true;
    }
    if ((flags & 1) && Direction(player))
    {
        if (player->tight_space == 0 || player->tight_space >= 3)
        {
            Walk(player, false);
            changed = true;
        }
        else if (player->tapped & (SOTN_BUTTON_RIGHT | SOTN_BUTTON_LEFT))
        {
            player->substep = 5;
            Animate(player, 246);
            SetSpeed(player, 49152);
            changed = true;
        }
    }
    if (changed && (flags & 8))
        ClearAttack(player);
}

static void PhysicalDeath(SotnPlayer *player, bool right, Uint16 effects)
{
    if (player->axe_armor.active)
    {
        player->axe_armor.active = false;
        player->axe_armor.exited = true;
        player->axe_armor.draw_flags = 0;
        player->axe_armor.rotation = 0;
        player->animation_bank = 1;
        player->palette = 0x8100;
    }
    player->step = SOTN_PLAYER_KILL;
    player->substep = 1;
    player->death = (SotnPlayerDeath){.phase = 1, .red = 128, .blue = 128};
    player->draw_pivot_x = player->draw_pivot_y = 0;
    player->vx = right ? 81920 : -81920;
    player->vy = -212992;
    player->sound = 0x6f6;
    Animate(player, 0xc0);
    if (player->death_capture && player->death_decay && player->death_context)
    {
        if (player->death_serial == SDL_MAX_UINT32)
        {
            player->failed = true;
            SDL_SetError("Physical death exhausted its actual capture generation");
            return;
        }
        player->animation_bank = 1;
        player->form_rotated = false;
        player->bat.rotation = 0;
        player->death.native = player->death.started = player->death.color_blend = true;
        player->death.generation = ++player->death_serial;
        player->death.source = (SotnPlayerFactorySource){.x = player->x.value,
                                                         .y = player->y.value,
                                                         .priority = 148,
                                                         .facing_left = player->facing_left};
        player->death.variant = effects & 0x8000U   ? 1
                                : effects & 0x4000U ? 2
                                : effects & 0x2000U ? 3
                                                    : 0;
        player->death.blend = player->death.variant == 3 ? 0x30 : 0;
        if (player->tight_space)
            player->vy = 0;
        Decelerate(player, 1024);
        if (player->mist.active)
        {
            player->mist.active = player->mist.restoring = false;
            player->mist.interrupted = true;
        }
    }
}

static void HurtFactory(SotnPlayer *p, Uint32 blueprint, Sint16 x, Sint16 y)
{
    if (p->hurt.count >= SDL_arraysize(p->hurt.factories))
    {
        p->failed = true;
        SDL_SetError("Hurt controller exceeded its original per-update factory calls");
        return;
    }
    SotnPlayerFactorySource source = {.priority = 148, .facing_left = p->facing_left};
    const Uint32 bx = (Uint32)p->x.value + (Uint32)(Sint32)x * 65536U;
    const Uint32 by = (Uint32)p->y.value + (Uint32)(Sint32)y * 65536U;
    SDL_memcpy(&source.x, &bx, sizeof(bx));
    SDL_memcpy(&source.y, &by, sizeof(by));
    p->hurt.factories[p->hurt.count].source = source;
    p->hurt.factories[p->hurt.count++].blueprint = blueprint;
}
static void HurtShake(SotnPlayer *p, Uint8 kind)
{
    p->hurt.shake = kind;
    p->hurt.shake_requested = true;
}
bool SotnPlayer_StoneLanding(SotnPlayer *p)
{
    if (!Transitions(p, 0x20280U))
        return false;
    if (!p->movement_effects.smoke || p->hurt.count >= SDL_arraysize(p->hurt.factories))
    {
        p->failed = true;
        SDL_SetError("Stone landing lost its original preceding smoke factory");
        return false;
    }
    p->hurt.factories[p->hurt.count].source = p->movement_effects.smoke_source;
    p->hurt.factories[p->hurt.count++].blueprint = p->movement_effects.smoke_blueprint;
    p->movement_effects.smoke = false;
    return true;
}
void SotnPlayer_ReactionFactory(SotnPlayer *p, Uint32 blueprint)
{
    HurtFactory(p, blueprint, 0, 0);
}
void SotnPlayer_ReactionShake(SotnPlayer *p, Uint8 kind)
{
    HurtShake(p, kind);
}
void SotnPlayer_ReactionSound(SotnPlayer *p, Uint16 sound)
{
    if (!p->sound)
        p->sound = sound;
    else if (!p->extra_sound)
        p->extra_sound = sound;
    else if (!p->third_sound)
        p->third_sound = sound;
    else
    {
        p->failed = true;
        SDL_SetError("Reaction exceeded its actual three ordered sound calls");
    }
}
void SotnPlayer_StonePhysicalDeath(SotnPlayer *p, bool right)
{
    p->stone = (SotnPlayerStone){0};
    if (!p->death_capture)
        p->palette = 0x8100;
    PhysicalDeath(p, right, 0);
}
static void HurtCeiling(SotnPlayer *p, bool sound)
{
    HurtFactory(p, 0x10004U, p->facing_left ? -3 : 3, -22);
    if (sound)
    {
        HurtShake(p, 3);
        p->sound = 0x644;
    }
}
static Uint16 HurtAnimation(SotnPlayer *p)
{
    const Uint16 value = SotnCombat_Random(&p->damage_random);
    Animate(p, (Uint16)(0x2eU + value % 3U));
    return (Uint16)(value % 16U);
}
static void KindTwoDamage(SotnPlayer *p, Sint32 power, bool grab)
{
    const Sint32 requested =
        (power - p->attributes.defense) * ((p->timer_status & 0x4000U) ? 2 : 1);
    const bool dodged = p->talismans && (Uint32)p->attributes.total[3] * p->talismans >=
                                            (SotnCombat_Random(&p->damage_random) & 511U);
    const Uint16 amount = (Uint16)(dodged ? requested : SDL_max(1, requested));
    Uint8 response = dodged ? 2 : requested <= 0 && p->attributes.defense >= 100 ? 1 : 3;
    if (!dodged)
        p->hp = amount >= p->hp ? 0 : (Uint16)(p->hp - amount);
    if (!p->hp)
        response = 4;
    p->damage_number = (SotnPlayerDamageNumber){.source = {.x = p->x.value,
                                                           .y = p->y.value,
                                                           .priority = 148,
                                                           .facing_left = p->facing_left},
                                                .amount = amount,
                                                .response = response,
                                                .applied = true,
                                                .boss_grab = grab,
                                                .secondary_impact = !grab};
    if (!dodged && p->hp)
    {
        p->hud_hp_hold = true;
        if (p->cape == 54 && p->hearts < p->hearts_max)
            p->heart_gain = (SotnPlayerHeartGain){
                .source = p->damage_number.source, .amount = amount, .requested = true};
        if (p->armor == 22 && p->stat_buffs)
            p->defense_buff_minimum = 512;
    }
}
static bool HurtSecondary(SotnPlayer *p)
{
    // The original helper accepts s16 before dividing, even though the retained word is u16.
    const Sint32 retained =
        p->hurt.damage_taken < 32768 ? p->hurt.damage_taken : (Sint32)p->hurt.damage_taken - 65536;
    KindTwoDamage(p, retained / 2, false);
    // This caller explicitly requests the HUD hold even for a dodge or lethal result.
    p->hud_hp_hold = true;
    if (!p->hp)
    {
        const Uint16 impact_sound = p->sound;
        PhysicalDeath(p, p->hurt.knock_right, 0);
        p->extra_sound = p->sound;
        p->sound = impact_sound;
        return true;
    }
    p->hit_effect_ticks = 4;
    p->damage_palette = 0x8166;
    const Uint16 index = (Uint16)((SotnCombat_Random(&p->damage_random) & 1U) + 3U);
    const Uint8 *sound = NULL;
    if (!AttackView(p, 0x800acf84U + index * 2U, 2, &sound))
        p->failed = true;
    else
        p->extra_sound = Attack16(sound);
    return false;
}
void SotnPlayer_HurtTick(SotnPlayer *p)
{
    SotnPlayer_Protect(p, 12);
    const Uint16 previous = p->substep;
    switch (p->substep)
    {
    case 1:
        if (Transitions(p, 0x20280))
            return;
        if (p->animation.remaining < 0)
        {
            Animate(p, 0x1c);
            p->facing_left = !p->facing_left;
        }
        break;
    case 2:
    case 15: {
        const bool slope_exit = (p->previous_contact & 0x8000) && !(p->contact & 0x8000);
        if (!slope_exit)
        {
            if ((p->contact & 0x8000) && !(p->damage_clock & 1U))
                HurtFactory(p, 0xa0045U, 0, 0);
            if (!(p->contact & 14))
                break;
            if (p->contact & 2)
            {
                HurtCeiling(p, true);
                p->vx /= 2;
                p->vy = 0;
                p->impact_ticks = 24;
                p->substep = 5;
                if (previous == 15)
                    (void)HurtSecondary(p);
                break;
            }
        }
        bool side = false;
        for (Uint16 i = 2; i < 14; ++i)
            if (i != 7 && i != 8 && (p->wall[i].effects & SOTN_EFFECT_SIDE))
                side = true;
        if (slope_exit || (!side && p->stage_id != 24 && p->stage_id != 56 && p->stage_id != 18))
        {
            p->vy = -4 * FIX_ONE;
            p->vx = p->hurt.knock_right ? 81920 : -81920;
            p->animation.id = 0x2f; // Original direct assignment retains pose and duration here.
            // The original tests nonzero speed after assigning it, not its sign.
            HurtFactory(p, 0x90004U, p->vx ? 3 : -3, 21);
            p->sound = 0x644;
            HurtShake(p, 2);
            p->substep = 1;
            if (previous == 15)
                (void)HurtSecondary(p);
            break;
        }
        if (previous == 15)
        {
            p->impact_ticks = 8;
            Animate(p, 0x3f);
            p->sound = 0x644;
            p->vy = -163840;
            HurtShake(p, 2);
            p->substep = 3;
            HurtFactory(p, 0x8001fU, 0, 0);
            (void)HurtSecondary(p);
            break;
        }
        p->vx /= 2;
        p->vy = 0;
        p->impact_ticks = 48;
        p->sound = 0x647;
        p->form_rotated = true;
        p->bat.rotation = 0x400;
        p->draw_pivot_x = 16;
        p->draw_pivot_y = 4;
        Animate(p, 0x2c);
        p->substep = 14;
        HurtFactory(p, 0x8001fU, 0, 0);
        break;
    }
    case 3:
        if (!p->impact_ticks)
        {
            SetSpeed(p, 49152);
            (void)Transitions(p, 0x20280);
        }
        break;
    case 4:
        break;
    case 5:
        Decelerate(p, 8192);
        if (p->impact_ticks)
        {
            if ((p->contact & 2) && !(p->damage_clock & 3U))
                HurtCeiling(p, false);
            break;
        }
        if ((p->contact & 12) && !(p->contact & 0xff03))
        {
            const Uint32 bits = (Uint32)p->vy + 6144U;
            SDL_memcpy(&p->vy, &bits, sizeof(bits));
            p->vy = SDL_min(7 * FIX_ONE, p->vy);
            if (!(p->damage_clock & 3U))
                HurtFactory(p, 0x4000cU, 0, 0);
            break;
        }
        p->substep = 1;
        (void)HurtAnimation(p);
        p->animation.remaining = 0;
        p->animation.pose = 0;
        break;
    case 14:
        p->form_rotated = true;
        if (!p->impact_ticks || p->impact_ticks >= 32768)
        {
            p->form_rotated = false;
            p->bat.rotation = 0x800;
            p->vx = -(p->vx / 2);
            p->vy = 0;
            Animate(p, 0x1c);
            p->substep = 1;
            p->step = SOTN_PLAYER_JUMP;
            p->movement_flags = 0;
        }
        break;
    case 6:
    case 7:
        Decelerate(p, 8192);
        if (!(p->contact & 1))
            Fall(p);
        if (p->animation.remaining < 0)
        {
            if (p->substep == 6)
                Land(p, 0);
            else
                Crouch(p, 0, p->vx);
        }
        break;
    default:
        p->failed = true;
        SDL_SetError("Hurt controller lost its original supported substate");
        break;
    }
}

static void SwordWarpTick(SotnPlayer *p)
{
    if (!p->substep)
    {
        if (!p->sword_weapon_present)
        {
            p->sword_recovery = 16;
            bool allocated = false;
            if (!p->sword_factory ||
                !p->sword_factory(p->sword_context,
                                  (SotnPlayerFactorySource){.x = p->x.value,
                                                            .y = p->y.value,
                                                            .priority = 148,
                                                            .facing_left = p->facing_left},
                                  0x15003dU, 0, &allocated))
                p->failed = true;
            ++p->substep;
        }
    }
    else
    {
        const Uint32 bits = (Uint32)p->sword_recovery - 1U;
        SDL_memcpy(&p->sword_recovery, &bits, sizeof(bits));
        if (!p->sword_recovery)
        {
            p->palette = 0x8100;
            Land(p, 0);
        }
    }
}

static void TeleportTick(SotnPlayer *p)
{
    p->vx = p->vy = 0;
    p->simulated_pad.buttons = 0;
    p->simulated_pad.remaining = 4;
    if (p->substep > 5 || !p->teleport_factory)
    {
        p->failed = true;
        SDL_SetError("Teleport requires its actual original controller/factory owner");
        return;
    }
    if (p->substep & 1U)
    {
        if (p->animation.remaining < 0)
            Land(p, 0);
        if (p->teleport.acknowledged)
            Land(p, 0);
        return;
    }
    if (p->substep && (p->stage_id == 11 || p->stage_id == 43))
    {
        const Sint32 world_x = p->teleport_origin_x + p->x.parts.high;
        const Sint32 delta = SDL_abs(world_x) - (p->stage_id == 11 ? 8000 : 8384);
        // Two original tests use the same pre-update global X, not abs(x - target).
        p->x.parts.high = (Sint16)((Uint16)p->x.parts.high + (delta > 0 ? -1 : delta < 0));
    }
    if (p->animation.pose == 8 && p->animation.remaining == 1)
    {
        bool allocated = false;
        if (!p->teleport_factory(p->teleport_context,
                                 (SotnPlayerFactorySource){.x = p->x.value,
                                                           .y = p->y.value,
                                                           .priority = 148,
                                                           .facing_left = p->facing_left},
                                 121U | (Uint32)p->substep << 16, &allocated))
            p->failed = true;
        if (!allocated)
            p->animation.remaining = 2;
    }
    if (p->animation.remaining < 0)
        Land(p, 0);
}

static void ScriptReactionTick(SotnPlayer *p)
{
    if (p->substep == 0)
    {
        p->fairy_rescue.blend = 0;
        ClearAttack(p);
        p->movement_flags = 0;
        p->stone = (SotnPlayerStone){0};
        p->gravity.upside_down = false;
        p->animation_bank = 1;
        if (p->axe_armor.active)
        {
            p->axe_armor.active = false;
            p->axe_armor.exited = true;
            p->axe_armor.draw_flags = 0;
        }
        p->form_rotated = false;
        p->damage_palette = 0x8166;
        p->timers[2] = 6;
        p->vx = p->vy = 0;
        Animate(p, 0x33);
        MovementSmoke(p, 0);
        p->script_reaction = (SotnPlayerScriptReaction){
            .started = true,
            .source = {
                .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left}};
        if (p->mist.active)
        {
            p->mist.active = p->mist.restoring = false;
            p->mist.interrupted = true;
        }
        ++p->substep;
    }
    else if (p->substep == 1 && p->animation.remaining < 0)
    {
        p->sound = 0x6f2;
        Land(p, 0);
    }
}

// dra/71830.c:PlayerStepBossGrab. BO5 clears GRAB_NONE rather than using the thrown release.
static void BossGrabTick(SotnPlayer *player)
{
    if (player->substep == 0)
    {
        player->fairy_rescue.blend = 0;
        ClearAttack(player);
        player->movement_flags = 0;
        player->stone = (SotnPlayerStone){0};
        if (player->axe_armor.active)
        {
            player->axe_armor.active = false;
            player->axe_armor.exited = true;
            player->axe_armor.draw_flags = 0;
        }
        player->animation_bank = 1;
        player->animation.pose = 0;
        player->animation.remaining = 0;
        player->bat.rotation = 0;
        player->form_rotated = false;
        if (player->grab.reaction == SOTN_GRAB_REACTION_NORMAL)
        {
            Animate(player, 0x37);
            player->timers[2] = 8;
            player->damage_palette = 0x8166;
            player->sound = 0x6eb;
        }
        player->vx = player->vy = 0;
        player->substep = 1;
        player->grab.state = SOTN_GRAB_HELD;
    }
    else if (player->grab.state == SOTN_GRAB_DAMAGE)
    {
        // This caller always displays the original result, without contact guard/dodge effects.
        KindTwoDamage(player, player->grab.damage, true);
        if (!player->hp)
        {
            const bool right =
                SotnPlayer_KnockRight(player, player->grab.source_x, player->grab.source_velocity);
            PhysicalDeath(player, right, 0);
            return;
        }
        player->timers[2] = 4;
        player->damage_palette =
            player->grab.reaction == SOTN_GRAB_REACTION_STONE ? 0x8161 : 0x8166;
        if (player->grab.reaction == SOTN_GRAB_REACTION_NORMAL)
            player->sound = 0x6e7;
        player->grab.state = SOTN_GRAB_HELD;
    }
    else if (player->grab.state == SOTN_GRAB_RELEASE)
    {
        player->step = SOTN_PLAYER_HIT;
        player->substep = 15;
        player->grab.state = SOTN_GRAB_NONE;
        Animate(player, 0x2e);
        player->hurt.damage_taken = player->grab.damage;
        player->sound = 0x6ed;
    }
    else if (player->grab.state == SOTN_GRAB_NONE)
    {
        player->step = SOTN_PLAYER_HIT;
        player->substep = 1;
    }
    SotnPlayer_Protect(player, 12);
}

// Selected physical death; the selected stone completion uses its own clocks.
static bool FairyInterrupt(SotnPlayer *p)
{
    if (!p->fairy_rescue.bound || p->fairy_rescue.scene != 0xfff || !p->substep)
        return false;
    p->fairy_rescue.rotation = p->water_death.active
                                   ? (Sint16)(p->water_death.motion.rotation < 32768U
                                                  ? p->water_death.motion.rotation
                                                  : (Sint32)p->water_death.motion.rotation - 65536)
                                   : p->death.rotation;
    p->fairy_rescue.restored = false;
    p->fairy_rescue.blend =
        (Uint8)(p->water_death.active ? p->water_death.motion.blend : p->death.blend);
    SetStep(p, SOTN_PLAYER_DEATH_HOLD);
    p->death.phase = 0;
    p->vx = p->vy = 0;
    // Both original entry assignments (DRAW_COLORS and ENTITY_ROTATE) are numeric4.
    p->form_rotated = true;
    if (p->water_death.active)
    {
        p->water_death.motion.step = SOTN_PLAYER_DEATH_HOLD;
        p->water_death.motion.phase = 0;
        p->water_death.motion.velocity_x = p->water_death.motion.velocity_y = 0;
    }
    return true;
}
static void DeathTick(SotnPlayer *player)
{
    if (FairyInterrupt(player))
        return;
    if (player->stone.active)
    {
        SotnPlayer_StoneDeathTick(player);
        return;
    }
    if (player->death.native)
    {
        SotnPlayer_PhysicalDeathTick(player);
        return;
    }
    if (player->death.phase == 1)
    {
        if (player->death.red < 248)
            ++player->death.red;
        if (player->death.blue > 8)
            --player->death.blue;
        player->vy += 5632;
        if (player->vy > 16384)
            player->vy = 4096;
        if (player->animation.remaining < 0)
        {
            player->death.phase = 2;
            player->death.timer = 64;
        }
    }
    else if (player->death.phase == 2)
    {
        for (Uint32 i = 0; i < 4; ++i)
        {
            const Uint16 cell = player->death.cursor;
            player->death.mask[cell / 8] |= (Uint8)(1U << (cell & 7));
            player->death.cursor = (Uint16)((cell + 35) & 255);
        }
        if (--player->death.timer == 0)
        {
            player->vy = 0;
            player->death.phase = 128;
        }
    }
    Decelerate(player, 1024);
    if (player->animation.pose >= 15)
    {
        if (player->animation.pose == 22 && player->animation.remaining == 1)
            player->death.rotation -= 256;
        player->death.rotation = (Sint16)SDL_max(-640, player->death.rotation - 6);
    }
}

bool SotnPlayerStep_IsDead(SotnPlayerStep step)
{
    return step == SOTN_PLAYER_KILL || step == SOTN_PLAYER_KILL_WATER ||
           step == SOTN_PLAYER_DEATH_HOLD;
}
bool SotnPlayer_SetFairyScene(SotnPlayer *p, Uint16 scene)
{
    if (!p || (scene != 0 && scene != 0xfff) ||
        (scene == 0xfff && (!SotnPlayerStep_IsDead(p->step) || !p->death_capture ||
                            !p->death_decay || !p->death_context)) ||
        (scene == 0 && p->step == SOTN_PLAYER_DEATH_HOLD && !p->fairy_rescue.restored))
        return SDL_SetError("Fairy scene requires its actual dead/capture/resource owners");
    p->fairy_rescue.bound = true;
    p->fairy_rescue.scene = scene;
    return true;
}
bool SotnPlayer_CommitFairyResources(SotnPlayer *p, Uint16 hp_max, Uint16 mp_max)
{
    if (!p || !p->fairy_rescue.bound || p->fairy_rescue.scene != 0xfff ||
        p->step != SOTN_PLAYER_DEATH_HOLD || p->fairy_rescue.restored || !hp_max ||
        hp_max != p->hp_max || mp_max > 9999)
        return SDL_SetError("Fairy resource commit requires its actual FFF/state17 episode");
    p->hp = hp_max;
    p->mp = mp_max;
    p->fairy_rescue.restored = true;
    return true;
}
static void FairyHoldTick(SotnPlayer *p)
{
    if (!p->fairy_rescue.bound || (p->fairy_rescue.scene != 0 && p->fairy_rescue.scene != 0xfff))
    {
        p->failed = true;
        SDL_SetError("Player17 lost its actual Fairy scene owner");
        return;
    }
    p->form_rotated = true;
    p->vx = p->vy = 0;
    p->animation.remaining = 4;
    if (!p->fairy_rescue.scene)
    {
        if (!p->fairy_rescue.restored || !p->hp)
        {
            p->failed = true;
            SDL_SetError("Fairy recovery requires committed actual resource restoration");
            return;
        }
        if (p->contact & 1)
            Land(p, 0);
        else
            Fall(p);
        p->fairy_rescue.source = (SotnPlayerFactorySource){
            .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left};
        p->fairy_rescue.recovered = true;
        // Retire native death presentation only at actual normal recovery. The separate
        // capture serial/scratch survive; this frame retains the original entity rotation.
        SDL_zero(p->death);
        SDL_zero(p->water_death);
        p->stone.active = false;
        p->body_color_blend = false;
    }
}
bool SotnPlayer_SetDeathCapture(SotnPlayer *p, SotnPlayerDeathCapture capture,
                                SotnPlayerDeathDecay decay, void *context)
{
    if (!p || !capture || !decay || !context ||
        ((p->water_death.active || p->death.native) &&
         (p->death_capture != capture || p->death_decay != decay || p->death_context != context)))
        return SDL_SetError("Player death requires real capture, decay/upload and context owners");
    p->death_capture = capture;
    p->death_decay = decay;
    p->death_context = context;
    return true;
}
bool SotnPlayer_SetStoneContext(SotnPlayer *p, SotnPlayerDissolvePresent present, void *context)
{
    if (!p || !present || !context)
        return SDL_SetError("Stone death requires its actual shared child query and context");
    p->dissolve_present = present;
    p->dissolve_context = context;
    return true;
}
bool SotnPlayer_StartWaterDeath(SotnPlayer *p, SotnWaterDamageEvents damage)
{
    if (!p || damage.response != 6 || damage.hp || damage.damage != 1 || p->hp != 1 ||
        p->invincibility || p->healing.pending || p->grab.state != SOTN_GRAB_NONE ||
        p->script_hold || p->step == SOTN_PLAYER_SCRIPT || p->step == SOTN_PLAYER_STONE ||
        p->step == SOTN_PLAYER_BOSS_GRAB || p->mist.active || SotnPlayerStep_IsDead(p->step) ||
        !p->death_capture || !p->death_decay || !p->death_context ||
        p->water_death.generation == SDL_MAX_UINT32 || p->death_serial == SDL_MAX_UINT32)
        return SDL_SetError("Water death requires an actual unprotected lethal result and owners");
    const Uint32 generation = SDL_max(p->water_death.generation, p->death_serial) + 1U;
    p->death_serial = generation;
    p->water_death =
        (SotnPlayerWaterDeath){.generation = generation, .active = true, .motion = {.step = 13}};
    p->death = (SotnPlayerDeath){0};
    p->hp = 0;
    SetStep(p, SOTN_PLAYER_KILL_WATER);
    return true;
}
static void WaterDeathTick(SotnPlayer *p)
{
    if (!p->water_death.active || !p->water_death.generation || !p->death_capture ||
        !p->death_decay || !p->death_context)
    {
        SDL_SetError("Water death lost its actual controller/capture ownership");
        p->failed = true;
        return;
    }
    if (FairyInterrupt(p))
        return;
    SotnWaterDeath motion = p->water_death.motion;
    motion.velocity_x = p->vx;
    motion.velocity_y = p->vy;
    motion.step = (Uint16)p->step;
    motion.phase = p->substep;
    if (p->step == SOTN_PLAYER_KILL_WATER)
    {
        SotnWaterDeathEvents events = {0};
        if (!SotnWaterDeath_Tick(&motion,
                                 (SotnWaterDeathInput){.remaining = (Sint16)p->animation.remaining,
                                                       .tight_space = p->tight_space != 0},
                                 &events))
        {
            p->failed = true;
            return;
        }
        for (Uint16 i = 0; i < events.count; ++i)
        {
            const SotnWaterDeathAction a = events.actions[i];
            if (a.kind == SOTN_WATER_DEATH_SOUND)
                p->sound = (Uint16)a.value;
            else if (a.kind == SOTN_WATER_DEATH_RESET_NORMAL)
            {
                p->fairy_rescue.blend = 0;
                if (p->axe_armor.active)
                {
                    p->axe_armor.active = false;
                    p->axe_armor.exited = true;
                    p->axe_armor.draw_flags = 0;
                    p->axe_armor.rotation = 0;
                    p->animation_bank = 1;
                    p->palette = 0x8100;
                }
                SotnPlayer_GravityCancel(p);
                SotnPlayer_BatCancel(p);
                SotnPlayer_WolfCancel(p);
                p->animation.pose = 0;
                p->animation.remaining = 0;
                p->movement_flags = 0;
                ClearAttack(p);
            }
            else if (a.kind == SOTN_WATER_DEATH_ANIMATION)
                Animate(p, (Uint16)a.value);
            else if (a.kind == SOTN_WATER_DEATH_GLOBAL_EFFECT)
            {
                p->water_death.started = true;
                p->water_death.source = (SotnPlayerFactorySource){.x = p->x.value,
                                                                  .y = p->y.value,
                                                                  .priority = 148,
                                                                  .facing_left = p->facing_left};
            }
            else if (a.kind == SOTN_WATER_DEATH_CAPTURE)
            {
                if (!p->death_capture(p->death_context, p->water_death.generation))
                {
                    p->failed = true;
                    return;
                }
                p->water_death.captured = true;
            }
            // Birth factory intents are consumed together, in original order, by the shared
            // native owner after this player update commits. No child readiness is inferred.
        }
    }
    else if (p->step == SOTN_PLAYER_KILL && p->substep == 2)
    {
        bool finished = false;
        if (!p->water_death.captured ||
            !p->death_decay(p->death_context, p->water_death.generation, &finished))
        {
            if (!p->water_death.captured)
                SDL_SetError("Common water death cannot advance before its actual capture");
            p->failed = true;
            return;
        }
        motion.draw_flags = 4;
        if (finished)
        {
            motion.velocity_y = 0;
            motion.color_blend = false;
            motion.phase = 128;
            p->death.phase = 128;
        }
        p->vx = motion.velocity_x;
        Decelerate(p, 1024);
        motion.velocity_x = p->vx;
        if (p->animation.pose >= 15)
        {
            Sint32 rotation =
                motion.rotation < 32768 ? motion.rotation : (Sint32)motion.rotation - 65536;
            if (p->animation.pose == 22 && p->animation.remaining == 1)
                rotation -= 256;
            const Uint16 bits = (Uint16)(Uint32)(rotation - 6);
            rotation = bits < 32768 ? bits : (Sint32)bits - 65536;
            motion.rotation = (Uint16)(Sint16)SDL_max(-640, rotation);
        }
    }
    else if (p->step != SOTN_PLAYER_KILL || p->substep != 128)
    {
        SDL_SetError("Water death entered an unsupported common/fade-recovery context");
        p->failed = true;
        return;
    }
    p->water_death.motion = motion;
    p->step = (SotnPlayerStep)motion.step;
    p->substep = motion.phase;
    p->vx = motion.velocity_x;
    p->vy = motion.velocity_y;
    p->form_rotated = (motion.draw_flags & 4U) != 0;
}
static void NormalSpellInput(SotnPlayer *p)
{
    if ((!p->dark_context && !p->spirit_context && !p->hellfire_factory && !p->soul_context) ||
        p->mp_spent)
        return;
    SotnSpellCommandFrame frame = {
        .pressed = p->buttons,
        .tapped = p->tapped,
        .step = p->step,
        .attack_mode = p->attack_mode,
        .mp = p->mp,
        .available = (Uint8)((p->dark_context ? 1U : 0U) | (p->spirit_context ? 0xaU : 0U) |
                             (p->hellfire_factory ? 4U : 0U) | (p->soul_context ? 0x20U : 0U)),
        .facing_left = p->facing_left,
        .cramped = p->tight_space != 0};
    SotnSpellCommandEvents events = {0};
    if (!SotnSpellCommands_Tick(&p->spell_commands, p->dra, &frame, &events))
    {
        p->failed = true;
        return;
    }
    if (!events.cast)
        return;
    p->mp = frame.mp;
    p->mp_spent = events.mp_spent;
    p->vx = p->vy = 0;
    SetStep(p, events.spell == 1   ? SOTN_PLAYER_SUMMON
               : events.spell == 3 ? SOTN_PLAYER_TETRA
               : events.spell == 2 ? SOTN_PLAYER_HELLFIRE
               : events.spell == 5 ? SOTN_PLAYER_SOUL
                                   : SOTN_PLAYER_DARK);
    p->shield_cursor = (SotnShield){0};
    p->shield = (SotnPlayerAttack){0};
    p->hand_retire = SDL_max(p->hand_retire, 1);
    if (events.spell == 5)
    {
        Animate(p, 0xda);
        p->sound = 0x6f4;
        p->spell_use_ticks = 4;
        p->soul = (SotnPlayerSoul){.source = {.x = p->x.value,
                                              .y = p->y.value,
                                              .priority = 148,
                                              .facing_left = p->facing_left},
                                   .started = true};
        return;
    }
    if (events.spell == 2)
    {
        p->hellfire_started = true;
        SotnPlayer_Protect(p, 16);
        return;
    }
    if (events.spell == 1 || events.spell == 3)
    {
        Animate(p, events.spell == 1 ? 0xf0 : 0xf1);
        p->sound = 0x6f1;
        p->spell_use_ticks = 4;
        p->spirit = (SotnPlayerSpirit){.source = {.x = p->x.value,
                                                  .y = p->y.value,
                                                  .priority = 148,
                                                  .facing_left = p->facing_left},
                                       .spell = events.spell,
                                       .blueprint = events.spell == 1 ? 117U : 0x10075U,
                                       .started = true};
        return;
    }
    Animate(p, 0xba);
    p->sound = 0x6f3;
    p->extra_sound = 0x67d;
    p->dark_ticks = (Uint16)(1024U + p->attributes.total[2] * 4U);
    p->dark = (PlayerDarkControl){
        .started = true,
        .source = {
            .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left}};
}
bool SotnPlayer_SetSoulContext(SotnPlayer *p, bool enabled)
{
    if (!p || (!enabled && p->step == SOTN_PLAYER_SOUL) ||
        (enabled && (!p->dra.bytes || p->dra.size < 0x101b8 + 0xdbU * 4U)))
        return SDL_SetError("Soul Steal requires actual native factory/attack/orb/healing owners "
                            "and private metadata");
    p->soul_context = enabled;
    return true;
}
bool SotnPlayer_SetDarkContext(SotnPlayer *p, bool enabled)
{
    if (!p || (enabled && (!p->dra.bytes || p->dra.size < 0xcfb4 + 12U * 4U)))
        return SDL_SetError("Dark spell requires its actual native effects/palette/healing owner "
                            "and private metadata");
    p->dark_context = enabled;
    return true;
}

static bool HellfireFeedbackValid(Uint16 signal, Uint16 palette)
{
    return (signal <= 3 || signal == 0xffffU) &&
           (palette == 0 || palette == 0x8100 || palette == 0x810d);
}
bool SotnPlayer_SetHellfireContext(SotnPlayer *p, SotnPlayerHellfireFactory factory, void *context,
                                   Sint32 camera_x, Uint16 signal, Uint16 palette)
{
    if (!p || (factory != NULL) != (context != NULL) || camera_x < 0 || camera_x > 16384 ||
        !HellfireFeedbackValid(signal, palette) ||
        (!factory && (p->step == SOTN_PLAYER_HELLFIRE || signal || palette)))
        return SDL_SetError("Hellfire requires its actual shared owner, camera and child feedback");
    p->hellfire_factory = factory;
    p->hellfire_context = context;
    p->hellfire_camera_x = camera_x;
    p->hellfire_signal = signal;
    p->hellfire_palette = palette;
    return true;
}
bool SotnPlayer_CommitHellfire(SotnPlayer *p, Uint16 signal, Uint16 palette)
{
    if (!p || !HellfireFeedbackValid(signal, palette) ||
        (!p->hellfire_factory && (signal || palette)))
        return SDL_SetError("Hellfire feedback requires its actual committed native child owner");
    p->hellfire_signal = signal;
    p->hellfire_palette = palette;
    return true;
}
static void HellfireTick(SotnPlayer *p)
{
    if (!p->hellfire_factory)
    {
        p->failed = true;
        SDL_SetError("Hellfire casting has no actual shared factory owner");
        return;
    }
    SotnPlayerFactorySource source = {
        .x = p->x.value, .y = p->y.value, .priority = 148, .facing_left = p->facing_left};
    bool root_created = false;
    if (!p->substep && !p->hellfire_factory(p->hellfire_context, source, 33, &root_created))
    {
        p->failed = true;
        return;
    }
    const bool raised = p->shield_cursor.phase != 0 && p->shield_cursor.phase < 3 &&
                        (p->buttons & p->shield_button);
    SotnHellfireCast cast = {
        .velocity_x = p->vx, .phase = p->substep, .facing_left = p->facing_left};
    SotnHellfireCastEvents events = {0};
    if (!SotnHellfireCast_Tick(
            &cast,
            (SotnHellfireCastInput){.buttons = p->buttons,
                                    .movement_flags = p->movement_flags,
                                    .floor_center = (Uint16)p->floor[1].effects,
                                    .floor_right = (Uint16)p->floor[2].effects,
                                    .floor_left = (Uint16)p->floor[3].effects,
                                    .signal = p->hellfire_signal,
                                    .pose = p->animation.pose,
                                    .remaining = (Sint16)p->animation.remaining,
                                    .normal_phase = raised ? 2 : 0,
                                    .x = (Sint16)(p->x.parts.high - p->hellfire_camera_x),
                                    .root_created = root_created},
            &events))
    {
        p->failed = true;
        return;
    }
    p->facing_left = cast.facing_left;
    source.facing_left = p->facing_left;
    p->turned |= events.turned;
    for (Uint16 i = 0; i < events.count; ++i)
    {
        const SotnHellfireCastAction action = events.actions[i];
        if (action.kind == SOTN_HELLFIRE_FACTORY)
        {
            if (action.value == 33)
                continue;
            bool allocated = false;
            if (!p->hellfire_factory(p->hellfire_context, source, action.value, &allocated))
                p->failed = true;
        }
        else if (action.kind == SOTN_HELLFIRE_NORMAL)
            Land(p, 0);
        else if (action.kind == SOTN_HELLFIRE_ANIMATION)
            Animate(p, action.value);
        else if (action.kind == SOTN_HELLFIRE_USE_TIMER)
            p->spell_use_ticks = action.value;
        else if (action.kind == SOTN_HELLFIRE_RESTORE_FLASHES)
        {
            const Uint32 factories[2] = {p->curse_ticks ? 0x17002cU : 0U,
                                         p->poison_ticks ? 0x16002cU : 0U};
            for (Uint16 j = 0; j < 2; ++j)
            {
                bool allocated = false;
                if (factories[j] &&
                    !p->hellfire_factory(p->hellfire_context, source, factories[j], &allocated))
                    p->failed = true;
            }
        }
        if (p->failed)
            return;
    }
    p->substep = cast.phase;
    p->vx = cast.velocity_x;
}
bool SotnPlayer_Tick(SotnPlayer *player, Uint16 buttons)
{
    const Uint16 supported =
        SOTN_BUTTON_UP | SOTN_BUTTON_RIGHT | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT |
        SOTN_BUTTON_JUMP | SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND |
        (player && player->mist_context ? SOTN_BUTTON_MIST : 0) |
        (player && player->bat_context ? SOTN_BUTTON_BAT : 0) |
        (player && player->wolf_context ? SOTN_BUTTON_WOLF : 0) | SOTN_BUTTON_CANCEL;
    if (player == NULL || (buttons & (Uint16)~supported) != 0)
        return SDL_SetError("Player input contains unsupported normal-form button bits");
    SotnPlayer next = *player;
    if (next.hammer_timer)
    {
        const Uint32 bits = (Uint32)next.hammer_timer - 1U;
        SDL_memcpy(&next.hammer_timer, &bits, sizeof(bits));
    }
    next.frame_contacts = (Uint32)next.contact;
    next.received_damage_effects = 0;
    next.timer_status = SotnPlayer_Status(player);
    bool force_reaction = false, force_hold = false;
    next.failed = false;
    next.turned = false;
    next.sound = 0;
    next.extra_sound = 0;
    next.third_sound = 0;
    next.mp_spent = 0;
    next.dark.started = false;
    next.spirit.started = false;
    next.hellfire_started = false;
    next.soul.started = next.soul.attack = false;
    next.water_death.started = false;
    next.death.started = false;
    next.fairy_rescue.recovered = false;
    const bool alive = !SotnPlayerStep_IsDead(next.step);
    const bool teleporting = next.step == SOTN_PLAYER_TELEPORT;
    next.teleport.used = false;
    next.medicine.used = false;
    next.food.used = false;
    next.throwing.used = false;
    next.throwing_grunt = 0;
    next.hand_retire = 0;
    next.hand_grunt = 0;
    next.hand_used = false;
    next.water_pain = (SotnPlayerWaterPain){0};
    next.defense_buff_minimum = 0;
    next.hud_hp_hold = false;
    next.heart_gain = (SotnPlayerHeartGain){0};
    next.absorption = (SotnPlayerAbsorption){0};
    next.immunity = (SotnPlayerImmunity){0};
    next.guard = (SotnPlayerGuard){0};
    next.damage_number = (SotnPlayerDamageNumber){0};
    next.dodge = (SotnPlayerDodge){0};
    next.script_reaction.started = false;
    next.axe_armor.events = (SotnAxeArmorEvents){0};
    next.axe_armor.draw_flags = 0; // EntityAlucard resets the live entity flags before dispatch.
    next.axe_armor.entered = next.axe_armor.exited = false;
    next.mist.interrupted = false;
    if (!AxePrepare(&next, false))
        return false;
    next.healing.applied = next.healing.effects = false;
    next.healing.applied_kind = 0;
    next.healing.applied_amount = 0;
    // dra/692E8.c func_8010A3F0 precedes healing for living, non-teleporting players: two-hand
    // Muramasa (both programs weapon56) holds Dark Metamorphosis' absorption timer at0x20 and
    // counts an update with blood contacts; the contact counter (g_Player.unk10) then clears.
    next.muramasa_blood = false;
    if (alive && !teleporting)
    {
        if (next.equipment[0] == 141 && next.equipment[1] == 141)
        {
            next.dark_ticks = 0x20;
            next.muramasa_blood = next.healing.contacts != 0;
        }
        next.healing.contacts = 0;
    }
    if (next.healing.pending && alive && !teleporting)
    {
        if (next.healing.pending_kind != 1 && next.healing.pending_kind != 2)
            return SDL_SetError("Deferred healing lost its actual original kind");
        next.hp = (Uint16)SDL_min(next.hp_max, (Uint32)next.hp + next.healing.pending);
        next.healing.applied_kind = next.healing.pending_kind;
        next.healing.applied_amount = next.healing.pending;
        next.healing.pending_kind = 0;
        next.healing.pending = 0;
        next.healing.applied = true;
        next.hud_hp_hold = true;
        next.healing.effects = next.step != SOTN_PLAYER_STONE;
        next.healing.source = (SotnPlayerFactorySource){
            .x = next.x.value, .y = next.y.value, .priority = 148, .facing_left = next.facing_left};
    }
    next.gravity.ceiling_impact = false;
    next.hurt.count = 0;
    next.hurt.shake_requested = false;
    next.hurt.started = false;
    next.bat_commands = (SotnBatCommandEvents){0};
    next.wolf.commands = (SotnWolfControllerEvents){0};
    next.form_rotated = false;
    next.sleep_event = false;
    next.movement_effects = (SotnPlayerMovementEffects){0};
    if (alive && !teleporting && !SotnPlayer_TimersTick(&next))
        return false;
    if (!SotnPlayer_AfterimageBegin(&next))
        return false;
    if (alive && !teleporting)
    {
        if (next.legacy_script_hold)
            next.simulated_pad.remaining = 1;
        const bool simulated = next.simulated_pad.remaining != 0;
        if (simulated)
        {
            --next.simulated_pad.remaining;
            buttons = next.simulated_pad.buttons;
            force_reaction =
                next.simulated_pad.mode == 1 && next.step != SOTN_PLAYER_SCRIPT_REACTION;
            force_hold = next.simulated_pad.mode == 2;
        }
        if (buttons & (Uint16)~supported)
            return SDL_SetError("Simulated input references an unavailable player control");
        if (next.bell_inside)
            buttons &= (Uint16) ~(SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND);
        next.bell_inside = false;
        if (!simulated && next.simulated_input)
            next.previous_buttons = buttons;
        next.simulated_input = simulated;
        next.buttons = buttons;
        next.held = next.previous_buttons;
        next.tapped = (Uint16)(buttons & (Uint16)~next.previous_buttons);
        next.previous_buttons = buttons;
    }
    if (!SotnPlayerStep_IsDead(next.step))
        next.tight_space = SotnPlayer_TightSpace(&next);
    if (next.failed)
        return false;
    next.previous_step = next.step;
    if (alive && !teleporting && (force_hold || force_reaction))
    {
        if (force_hold)
            SotnPlayer_GravityCancel(&next);
        SotnPlayer_BatCancel(&next);
        SotnPlayer_WolfCancel(&next);
        // PAD_SIM mode 2 changes step without selecting a new animation (692E8.c/alu_anim.c).
        SetStep(&next, force_reaction          ? SOTN_PLAYER_SCRIPT_REACTION
                       : next.axe_armor.active ? SOTN_PLAYER_AXE_SCRIPT
                                               : SOTN_PLAYER_SCRIPT);
        SotnPlayer_Protect(&next, 4);
        if (force_hold && next.legacy_script_hold)
            ClearAttack(&next);
    }
    if (alive && !teleporting && next.grab.state == SOTN_GRAB_START)
    {
        SotnPlayer_GravityCancel(&next);
        SotnPlayer_BatCancel(&next);
        SotnPlayer_WolfCancel(&next);
        SetStep(&next, SOTN_PLAYER_BOSS_GRAB);
    }
    bool reaction_born = false;
    if (!teleporting && !SotnPlayer_ContactTick(&next, &reaction_born))
        return false;
    if (teleporting)
        next.queued_contact = (SotnPlayerContact){0}; // Original post-animation hitParams clear.
    if (!SotnPlayerStep_IsDead(next.step) && !teleporting)
    {
        SotnPlayer_GravityInput(&next);
        if (next.sword_factory || next.hand_factory)
            SotnWeaponCommand_Tick(&next.sword_command, next.buttons, next.facing_left,
                                   next.tight_space != 0);
        if (next.hand_factory)
            SotnWeaponBackForward_Tick(&next.back_forward, next.buttons, next.facing_left,
                                       next.tight_space != 0);
        NormalSpellInput(&next);
        SotnPlayer_MistInput(&next);
        SotnPlayer_BatInput(&next);
        SotnPlayer_WolfInput(&next);
    }
    else
    {
        next.bat.started = next.bat.return_started = false;
        next.wolf.started = next.wolf.body_started = next.wolf.finish_flash =
            next.wolf.return_started = false;
    }
    next.control_step = next.step;
    next.control_substep = next.substep;
    if (!reaction_born)
        switch (next.step)
        {
        case SOTN_PLAYER_AXE_STAND:
        case SOTN_PLAYER_AXE_WALK:
        case SOTN_PLAYER_AXE_JUMP:
        case SOTN_PLAYER_AXE_HIT:
        case SOTN_PLAYER_AXE_SCRIPT:
            AxeTick(&next);
            break;
        case SOTN_PLAYER_SWORD_WARP:
            SwordWarpTick(&next);
            break;
        case SOTN_PLAYER_TELEPORT:
            TeleportTick(&next);
            break;
        case SOTN_PLAYER_WOLF:
            SotnPlayer_WolfTick(&next);
            break;
        case SOTN_PLAYER_UNMORPH_WOLF:
            SotnPlayer_UnmorphWolfTick(&next);
            break;
        case SOTN_PLAYER_BAT:
            SotnPlayer_BatTick(&next);
            break;
        case SOTN_PLAYER_UNMORPH_BAT:
            SotnPlayer_UnmorphBatTick(&next);
            break;
        case SOTN_PLAYER_MIST:
            SotnPlayer_MistTick(&next);
            break;
        case SOTN_PLAYER_UNMORPH_MIST:
            SotnPlayer_UnmorphMistTick(&next);
            break;
        case SOTN_PLAYER_STUCK:
            SotnPlayer_StuckTick(&next);
            break;
        case SOTN_PLAYER_BOSS_GRAB:
            BossGrabTick(&next);
            break;
        case SOTN_PLAYER_HELLFIRE:
            HellfireTick(&next);
            break;
        case SOTN_PLAYER_SUMMON:
        case SOTN_PLAYER_TETRA:
            if (next.animation.remaining < 0)
                Crouch(&next, 0, 0);
            break;
        case SOTN_PLAYER_SOUL:
            if (next.animation.pose == 7 && next.animation.remaining == 1)
            {
                next.soul.attack = true;
                next.soul.source = (SotnPlayerFactorySource){.x = next.x.value,
                                                             .y = next.y.value,
                                                             .priority = 148,
                                                             .facing_left = next.facing_left};
                next.sound = 0x67d;
            }
            if (next.animation.remaining < 0)
                Land(&next, 0);
            break;
        case SOTN_PLAYER_DARK:
            if (next.animation.remaining < 0)
                Land(&next, 0);
            break;
        case SOTN_PLAYER_SCRIPT:
            next.vx = next.vy = 0;
            if (!next.script_hold)
                Land(&next, 0);
            break;
        case SOTN_PLAYER_SCRIPT_REACTION:
            ScriptReactionTick(&next);
            break;
        case SOTN_PLAYER_STAND:
            StandTick(&next);
            break;
        case SOTN_PLAYER_WALK:
            if (!Transitions(&next, 0x4301c))
            {
                SetSpeed(&next, WALK_SPEED);
                if (Direction(&next) == 0)
                    Land(&next, 0);
            }
            break;
        case SOTN_PLAYER_JUMP:
            JumpTick(&next);
            break;
        case SOTN_PLAYER_HIGH_JUMP:
            SotnPlayer_GravityTick(&next);
            break;
        case SOTN_PLAYER_CROUCH:
            CrouchTick(&next);
            break;
        case SOTN_PLAYER_FALL:
            if (next.timers[5] && (next.tapped & SOTN_BUTTON_JUMP))
            {
                Jump(&next);
            }
            else if (!Transitions(&next, 0x9029))
            {
                Decelerate(&next, 4096);
                if (Direction(&next))
                    SetSpeed(&next, 49152);
            }
            break;
        case SOTN_PLAYER_STONE:
            SotnPlayer_StoneTick(&next);
            break;
        case SOTN_PLAYER_KILL:
            if (next.water_death.active)
                WaterDeathTick(&next);
            else
                DeathTick(&next);
            break;
        case SOTN_PLAYER_KILL_WATER:
            WaterDeathTick(&next);
            break;
        case SOTN_PLAYER_DEATH_HOLD:
            FairyHoldTick(&next);
            break;
        case SOTN_PLAYER_HIT:
            SotnPlayer_HurtTick(&next);
            break;
        default:
            return SDL_SetError("Unsupported Alucard movement state");
        }
    if (next.bat.active || next.mist.active || next.wolf.active || next.axe_armor.active ||
        next.axe_armor.exited)
        next.fairy_rescue.blend = 0; // Actual form reset/initialization owns BLEND_NO.
    next.status =
        (next.status & ~7U) |
        ((next.step == SOTN_PLAYER_MIST || next.step == SOTN_PLAYER_UNMORPH_MIST) ? 2U : 0U) |
        ((next.step == SOTN_PLAYER_BAT || next.step == SOTN_PLAYER_UNMORPH_BAT) ? 1U : 0U) |
        (next.step == SOTN_PLAYER_WOLF ? 4U : 0U);
    if (next.step == SOTN_PLAYER_AXE_HIT)
        SotnPlayer_Protect(&next, 20);
    if (next.axe_armor.active)
        next.animation_bank = 16;
    if (next.step == SOTN_PLAYER_DARK || next.step == SOTN_PLAYER_SUMMON ||
        next.step == SOTN_PLAYER_TETRA || next.step == SOTN_PLAYER_HELLFIRE ||
        next.step == SOTN_PLAYER_SOUL)
        SotnPlayer_Protect(&next, 16);
    if (SotnPlayerStep_IsDead(next.step) || next.step == SOTN_PLAYER_SCRIPT_REACTION ||
        next.step == SOTN_PLAYER_SCRIPT || next.step == SOTN_PLAYER_TELEPORT)
        SotnPlayer_Protect(&next, 12);
    if ((next.status & 2U) || next.step == SOTN_PLAYER_UNMORPH_BAT ||
        next.step == SOTN_PLAYER_UNMORPH_WOLF ||
        (next.step == SOTN_PLAYER_WOLF && SDL_abs(next.vx) > 3 * FIX_ONE) ||
        (next.step == SOTN_PLAYER_BAT && next.substep == 3))
        SotnPlayer_Protect(&next, 4);
    SotnPlayer_ReactionRecovery(&next);
    if (next.step == SOTN_PLAYER_SWORD_WARP)
    {
        next.palette = 0x810d;
        SotnPlayer_Protect(&next, 12);
    }
    if (next.failed)
        return false;
    const bool animated =
        next.wolf.active
            ? SotnAnimation_TickWolf(next.dra, next.form_pose,
                                     next.wolf.controller.cadence.durations, &next.animation)
        : next.bat.active || next.mist.active
            ? SotnAnimation_TickPlayerForm(next.dra, next.form_pose, &next.animation)
            : SotnAnimation_Tick(next.dra, &next.animation);
    if (!animated || !ReadAttack(&next) || !TickShield(&next) || !SotnPlayer_KickTick(&next))
        return false;
    if (next.death.native && next.animation.remaining < 0)
        next.animation.sprite |= 0x8000U;
    if (!SotnPlayer_AfterimageCapture(&next))
        return false;
    const bool water_drag = SotnPlayer_WaterDrag(&next);
    if (water_drag)
    {
        next.vx = SotnPlayer_WaterScale(next.vx, false);
        next.vy = SotnPlayer_WaterScale(next.vy, false);
    }
    if (SotnPlayerStep_IsDead(next.step))
    {
        const Uint32 x = (Uint32)next.x.value + (Uint32)next.vx,
                     y = (Uint32)next.y.value + (Uint32)next.vy;
        SDL_memcpy(&next.x.value, &x, sizeof(x));
        SDL_memcpy(&next.y.value, &y, sizeof(y));
    }
    else if (next.step != SOTN_PLAYER_BOSS_GRAB)
        SotnPlayer_Collide(&next);
    if (water_drag)
    {
        next.vx = SotnPlayer_WaterScale(next.vx, true);
        next.vy = SotnPlayer_WaterScale(next.vy, true);
    }
    if (!(SotnPlayer_Status(&next) & 0x50U))
        next.collision_invincibility = 0;
    if (next.failed)
        return false;
    if (!SotnPlayer_AfterimagePresent(&next))
        return false;
    if (!PositionValid(next.map, next.x.value, next.y.value))
        return SDL_SetError("Player left the bounded room region without a supported transition");
    if (!SotnPlayer_SleepTick(&next))
        return false;
    SotnPlayer_GravityBeams(&next);
    next.chair_kind = 0;
    ++next.frame_number;
    if (next.damage_context)
    {
        next.damage_last_clock = next.damage_clock;
        next.damage_ticked = true;
    }
    next.damage_pending = false;
    if (next.teleport.used && !next.library_card_use(next.teleport_context, next.teleport.hand,
                                                     &next.equipment[next.teleport.hand]))
        return false;
    if (next.medicine.used &&
        !next.medicine_consume(next.medicine_context, next.medicine.hand, next.medicine.item,
                               &next.equipment[next.medicine.hand]))
        return false;
    if (next.food.used && !next.food_consume(next.food_context, next.food.hand, next.food.item,
                                             &next.equipment[next.food.hand]))
        return false;
    if (next.throwing.used && !next.throwing.keep &&
        !next.throwing_consume(next.throwing_context, next.throwing.hand, next.throwing.item,
                               &next.equipment[next.throwing.hand]))
        return false;
    *player = next;
    return true;
}

bool SotnPlayer_Create(SotnDataView dra, SotnCollisionMap map, Sint32 x, Sint32 y, SotnPlayer **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (!ValidateMap(map))
        return false;
    const Sint64 start_x = (Sint64)x * FIX_ONE, start_y = (Sint64)y * FIX_ONE;
    if (!PositionValid(map, start_x, start_y))
        return SDL_SetError("Player development start is outside its bounded room");
    SotnPlayer *player = SDL_calloc(1, sizeof(*player));
    if (player == NULL)
        return false;
    player->dra = dra;
    (void)SotnPlayerAfterimage_Init(&player->afterimage);
    player->hp = player->hp_max = 70;
    player->mp = 20;
    player->mist_lockout = 16;
    player->animation_bank = 1;
    player->palette = 0x8100;
    player->attributes =
        (SotnCharacterStats){.total = {6, 6, 6, 6}, .attack = {6, 6}, .unarmed_attack = 6};
    player->map = map;
    player->x.value = (Sint32)start_x;
    player->y.value = (Sint32)start_y;
    player->contact = player->previous_contact = 1;
    Land(player, 0);
    if (!SotnAnimation_Tick(dra, &player->animation))
    {
        SotnPlayer_Close(player);
        return false;
    }
    player->animation.remaining = 0;
    *out = player;
    return true;
}

bool SotnPlayer_SetAxeContext(SotnPlayer *player, SotnDataView weapons, bool bootstrap)
{
    SotnEquipment equipment = {0};
    SotnDataView overlay = {0};
    if (!player || !player->script_executable.bytes ||
        !SotnEquipment_ReadAxeArmor(player->dra, &equipment) ||
        !SotnWeapon_Overlay(weapons, equipment, &overlay))
        return SDL_SetError(
            "Axe Armor context requires actual weapon45 metadata/artwork/SDK owners");
    SotnPlayer next = *player;
    next.axe_context = true;
    next.weapons = weapons;
    const bool was_active = next.axe_armor.active;
    if (bootstrap && (!AxePrepare(&next, true) || (!was_active && next.axe_armor.active &&
                                                   !SotnAnimation_Tick(next.dra, &next.animation))))
        return false;
    *player = next;
    return true;
}

bool SotnPlayer_SetSwordContext(SotnPlayer *p, SotnPlayerSwordFactory factory, void *context,
                                bool weapon_present)
{
    if (!p || !factory || !context)
        return SDL_SetError("Sword control requires its actual shared factory and slot owner");
    p->sword_factory = factory;
    p->sword_context = context;
    p->sword_weapon_present = weapon_present;
    return true;
}

bool SotnPlayer_SetHandContext(SotnPlayer *p, SotnPlayerMedicineFactory factory,
                               SotnPlayerThrowingRandom random, void *context)
{
    if (!p || !factory || !random || !context)
        return SDL_SetError("Ordinary hands require actual shared factory ownership");
    p->hand_factory = factory;
    p->hand_random = random;
    p->hand_context = context;
    return true;
}
bool SotnPlayer_CommitHandShield(SotnPlayer *p, Uint16 phase, Uint16 hand)
{
    if (!p || !p->hand_factory || phase > 4 || hand > 1)
        return SDL_SetError("Shield status requires its actual ordinary hand callback");
    p->shield_cursor = (SotnShield){.phase = phase};
    p->shield_button = hand ? SOTN_BUTTON_RIGHT_HAND : SOTN_BUTTON_LEFT_HAND;
    p->shield = (SotnPlayerAttack){0};
    p->hand_retire = 0;
    return true;
}

bool SotnPlayer_SetTeleportContext(SotnPlayer *p, SotnPlayerTeleportFactory factory,
                                   SotnPlayerLibraryCardUse use, void *context, Sint32 origin_x)
{
    if (!p || !factory || !use || !context || origin_x < 0 || origin_x > 16384)
        return SDL_SetError("Teleport requires real factory, equipped-card and room owners");
    p->teleport_factory = factory;
    p->library_card_use = use;
    p->teleport_context = context;
    p->teleport_origin_x = origin_x;
    return true;
}
bool SotnPlayer_BeginTeleportArrival(SotnPlayer *p, Uint16 substep)
{
    if (!p || !p->teleport_factory || (substep != 1 && substep != 3 && substep != 5))
        return SDL_SetError("Teleport arrival requires its actual incoming native owner");
    SotnPlayer next = *p;
    SetStep(&next, SOTN_PLAYER_TELEPORT);
    next.substep = substep;
    next.teleport = (SotnPlayerTeleport){0};
    Animate(&next, 0xf5);
    if (next.failed)
        return false;
    *p = next;
    return true;
}
bool SotnPlayer_CommitTeleport(SotnPlayer *p, Uint16 palette, bool acknowledged)
{
    if (!p || (palette && palette != 0x810d))
        return SDL_SetError("Teleport commit requires its actual child65 palette/ACK");
    if (palette)
        p->palette = palette;
    p->teleport.acknowledged |= acknowledged;
    return true;
}

bool SotnPlayer_SetEquipment(SotnPlayer *player, SotnDataView weapons, Uint16 left, Uint16 right)
{
    if (player == NULL || player->attack_mode != 0)
        return SDL_SetError("Equipment can change only when the player has no active attack");
    if ((left == 125 || right == 125 || SotnEquipment_IsBasicShield(left) ||
         SotnEquipment_IsBasicShield(right) || SotnEquipment_IsBasicFist(left) ||
         SotnEquipment_IsBasicFist(right) || SotnEquipment_IsPlainWeapon(left) ||
         SotnEquipment_IsPlainWeapon(right)) &&
        !player->hand_factory)
        return SDL_SetError("This equipment requires actual shared hand/property ownership");
    if ((SotnEquipment_IsTwoHand(left) || SotnEquipment_IsTwoHand(right)) && left != right)
        return SDL_SetError("Two-hand equipment requires its actual paired hand slots");
    const Uint16 ids[2] = {left, right};
    Uint16 power[2] = {0};
    for (Uint32 i = 0; i < 2; ++i)
    {
        SotnEquipment equipment = {0};
        SotnDataView overlay = {0};
        if (!SotnEquipment_Read(player->dra, ids[i], &equipment) ||
            (ids[i] != 0 && !(equipment.kind == 131 && equipment.family == 255) &&
             !SotnWeapon_Overlay(weapons, equipment, &overlay)))
            return false;
        const Uint16 strength = player->attributes.total[0];
        power[i] =
            equipment.id == 16 ||
                    (SotnEquipment_IsBasicShield(equipment.id) && equipment.attack == 1) ||
                    equipment.kind == 131 || equipment.kind == 132
                ? 0
                : (Uint16)SDL_clamp(equipment.attack +
                                        (equipment.attack <= strength ? strength : strength / 2),
                                    0, 999);
    }
    player->attributes.attack[0] = power[0];
    player->attributes.attack[1] = power[1];
    player->weapons = weapons;
    // menu.c CheckWeaponCombo runs with each weapon load.
    Uint32 combo = 0;
    if (!SotnEquipment_Combo(player->dra, left, right, &combo))
        return false;
    player->combo = combo;
    player->equipment[0] = left;
    player->equipment[1] = right;
    player->shield_cursor = (SotnShield){0};
    player->shield = (SotnPlayerAttack){0};
    player->shield_button = 0;
    return true;
}

bool SotnPlayer_ChangeRoom(SotnPlayer *player, SotnCollisionMap map, Sint32 offset_x,
                           Sint32 offset_y)
{
    if (player == NULL)
        return SDL_InvalidParamError("player");
    if (!ValidateMap(map))
        return false;
    if (player->hurt.count > SDL_arraysize(player->hurt.factories))
        return SDL_SetError("Room rebase lost its bounded hurt factory list");
    SotnPlayerHurt hurt = player->hurt;
    for (Uint16 i = 0; i < hurt.count; ++i)
    {
        const Sint64 x = (Sint64)hurt.factories[i].source.x + (Sint64)offset_x * FIX_ONE;
        const Sint64 y = (Sint64)hurt.factories[i].source.y + (Sint64)offset_y * FIX_ONE;
        if (x < SDL_MIN_SINT32 || x > SDL_MAX_SINT32 || y < SDL_MIN_SINT32 || y > SDL_MAX_SINT32)
            return SDL_SetError("Room rebase exceeds a captured hurt factory origin");
        hurt.factories[i].source.x = (Sint32)x;
        hurt.factories[i].source.y = (Sint32)y;
    }
    const Sint64 x = player->x.value + (Sint64)offset_x * FIX_ONE;
    const Sint64 y = player->y.value + (Sint64)offset_y * FIX_ONE;
    if (!PositionValid(map, x, y))
        return SDL_SetError("Player room rebase is outside the new bounded room");
    const Sint64 script_x = (Sint64)player->script_reaction.source.x + (Sint64)offset_x * FIX_ONE,
                 script_y = (Sint64)player->script_reaction.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->script_reaction.started &&
        (script_x < SDL_MIN_SINT32 || script_x > SDL_MAX_SINT32 || script_y < SDL_MIN_SINT32 ||
         script_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured scripted-reaction source");
    SotnPlayerAfterimage afterimage = player->afterimage;
    if (player->afterimage_generation &&
        !SotnPlayerAfterimage_Rebase(&afterimage, offset_x, offset_y))
        return false;
    if (player->wolf.commands.count > SDL_arraysize(player->wolf.commands.requests))
        return SDL_SetError("Wolf room transfer lost its bounded command list");
    const Sint64 impact_x = (Sint64)player->gravity.impact_source.x + (Sint64)offset_x * FIX_ONE,
                 impact_y = (Sint64)player->gravity.impact_source.y + (Sint64)offset_y * FIX_ONE;
    if (player->gravity.ceiling_impact && (impact_x < SDL_MIN_SINT32 || impact_x > SDL_MAX_SINT32 ||
                                           impact_y < SDL_MIN_SINT32 || impact_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured Gravity Boots impact origin");
    const Sint64 smoke_x =
                     (Sint64)player->movement_effects.smoke_source.x + (Sint64)offset_x * FIX_ONE,
                 smoke_y =
                     (Sint64)player->movement_effects.smoke_source.y + (Sint64)offset_y * FIX_ONE;
    if (player->movement_effects.smoke && (smoke_x < SDL_MIN_SINT32 || smoke_x > SDL_MAX_SINT32 ||
                                           smoke_y < SDL_MIN_SINT32 || smoke_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured ordinary movement source");
    const Sint64 bubble_x =
                     (Sint64)player->movement_effects.bubble_source.x + (Sint64)offset_x * FIX_ONE,
                 bubble_y =
                     (Sint64)player->movement_effects.bubble_source.y + (Sint64)offset_y * FIX_ONE;
    if (player->movement_effects.water_bubble &&
        (bubble_x < SDL_MIN_SINT32 || bubble_x > SDL_MAX_SINT32 || bubble_y < SDL_MIN_SINT32 ||
         bubble_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured normal water bubble source");

    const Sint64 dark_x = (Sint64)player->dark.source.x + (Sint64)offset_x * FIX_ONE,
                 dark_y = (Sint64)player->dark.source.y + (Sint64)offset_y * FIX_ONE;
    const Sint64 soul_x = (Sint64)player->soul.source.x + (Sint64)offset_x * FIX_ONE,
                 soul_y = (Sint64)player->soul.source.y + (Sint64)offset_y * FIX_ONE;
    if ((player->soul.started || player->soul.attack) &&
        (soul_x < SDL_MIN_SINT32 || soul_x > SDL_MAX_SINT32 || soul_y < SDL_MIN_SINT32 ||
         soul_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured Soul Steal source");
    const Sint64 spirit_x = (Sint64)player->spirit.source.x + (Sint64)offset_x * FIX_ONE,
                 spirit_y = (Sint64)player->spirit.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->spirit.started && (spirit_x < SDL_MIN_SINT32 || spirit_x > SDL_MAX_SINT32 ||
                                   spirit_y < SDL_MIN_SINT32 || spirit_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured spirit spell source");
    if (player->dark.started && (dark_x < SDL_MIN_SINT32 || dark_x > SDL_MAX_SINT32 ||
                                 dark_y < SDL_MIN_SINT32 || dark_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured Dark spell source");
    const Sint64 heal_x = (Sint64)player->healing.source.x + (Sint64)offset_x * FIX_ONE,
                 heal_y = (Sint64)player->healing.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->healing.applied && (heal_x < SDL_MIN_SINT32 || heal_x > SDL_MAX_SINT32 ||
                                    heal_y < SDL_MIN_SINT32 || heal_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured blood-healing source");
    const Sint64 absorb_x = (Sint64)player->absorption.source.x + (Sint64)offset_x * FIX_ONE,
                 absorb_y = (Sint64)player->absorption.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->absorption.applied && (absorb_x < SDL_MIN_SINT32 || absorb_x > SDL_MAX_SINT32 ||
                                       absorb_y < SDL_MIN_SINT32 || absorb_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured contact-absorption source");
    const Sint64 dodge_x = (Sint64)player->dodge.source.x + (Sint64)offset_x * FIX_ONE,
                 dodge_y = (Sint64)player->dodge.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->dodge.applied && (dodge_x < SDL_MIN_SINT32 || dodge_x > SDL_MAX_SINT32 ||
                                  dodge_y < SDL_MIN_SINT32 || dodge_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured dodge source");
    const Sint64 axe_x = (Sint64)player->axe_armor.source.x + (Sint64)offset_x * FIX_ONE,
                 axe_y = (Sint64)player->axe_armor.source.y + (Sint64)offset_y * FIX_ONE;
    const bool axe_request = player->axe_armor.events.factory_count || player->axe_armor.entered ||
                             player->axe_armor.exited;
    if (axe_request && (axe_x < SDL_MIN_SINT32 || axe_x > SDL_MAX_SINT32 ||
                        axe_y < SDL_MIN_SINT32 || axe_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the actual Axe Armor factory source");
    const Sint64 number_x = (Sint64)player->damage_number.source.x + (Sint64)offset_x * FIX_ONE,
                 number_y = (Sint64)player->damage_number.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->damage_number.applied && (number_x < SDL_MIN_SINT32 || number_x > SDL_MAX_SINT32 ||
                                          number_y < SDL_MIN_SINT32 || number_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured contact-number source");
    const Sint64 guard_x = (Sint64)player->guard.source.x + (Sint64)offset_x * FIX_ONE,
                 guard_y = (Sint64)player->guard.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->guard.applied && (guard_x < SDL_MIN_SINT32 || guard_x > SDL_MAX_SINT32 ||
                                  guard_y < SDL_MIN_SINT32 || guard_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured guard source");
    const Sint64 immune_x = (Sint64)player->immunity.source.x + (Sint64)offset_x * FIX_ONE,
                 immune_y = (Sint64)player->immunity.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->immunity.applied && (immune_x < SDL_MIN_SINT32 || immune_x > SDL_MAX_SINT32 ||
                                     immune_y < SDL_MIN_SINT32 || immune_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured contact-immunity source");
    const Sint64 heart_x = (Sint64)player->heart_gain.source.x + (Sint64)offset_x * FIX_ONE,
                 heart_y = (Sint64)player->heart_gain.source.y + (Sint64)offset_y * FIX_ONE;
    if (player->heart_gain.requested && (heart_x < SDL_MIN_SINT32 || heart_x > SDL_MAX_SINT32 ||
                                         heart_y < SDL_MIN_SINT32 || heart_y > SDL_MAX_SINT32))
        return SDL_SetError("Room rebase exceeds the captured Blood Cloak source");
    for (Uint16 i = 0; i < player->wolf.commands.count; ++i)
    {
        const Sint64 source_x = (Sint64)player->wolf.commands.requests[i].source.x +
                                (Sint64)offset_x * FIX_ONE,
                     source_y = (Sint64)player->wolf.commands.requests[i].source.y +
                                (Sint64)offset_y * FIX_ONE;
        if (source_x < SDL_MIN_SINT32 || source_x > SDL_MAX_SINT32 || source_y < SDL_MIN_SINT32 ||
            source_y > SDL_MAX_SINT32)
            return SDL_SetError("Wolf room transfer exceeds a captured command origin");
    }
    for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
    {
        const SotnPlayerGravityBeam b = player->gravity.beams[i];
        if (b.active && ((Sint64)b.x + (Sint64)offset_x * FIX_ONE < SDL_MIN_SINT32 ||
                         (Sint64)b.x + (Sint64)offset_x * FIX_ONE > SDL_MAX_SINT32 ||
                         (Sint64)b.y + (Sint64)offset_y * FIX_ONE < SDL_MIN_SINT32 ||
                         (Sint64)b.y + (Sint64)offset_y * FIX_ONE > SDL_MAX_SINT32))
            return SDL_SetError("Room rebase exceeds the owned Gravity Boots beam coordinates");
    }
    for (Uint32 i = 0; i < SOTN_PLAYER_GRAVITY_BEAMS; ++i)
        if (player->gravity.beams[i].active)
        {
            player->gravity.beams[i].x =
                (Sint32)((Sint64)player->gravity.beams[i].x + (Sint64)offset_x * FIX_ONE);
            player->gravity.beams[i].y =
                (Sint32)((Sint64)player->gravity.beams[i].y + (Sint64)offset_y * FIX_ONE);
        }
    player->map = map;
    player->afterimage = afterimage;
    player->water = (SotnPlayerWater){0};
    if (player->soul.started || player->soul.attack)
    {
        player->soul.source.x = (Sint32)soul_x;
        player->soul.source.y = (Sint32)soul_y;
    }
    if (player->dark.started)
    {
        player->dark.source.x = (Sint32)dark_x;
        player->dark.source.y = (Sint32)dark_y;
    }
    if (player->healing.applied)
    {
        player->healing.source.x = (Sint32)heal_x;
        player->healing.source.y = (Sint32)heal_y;
    }
    if (player->dodge.applied)
    {
        player->dodge.source.x = (Sint32)dodge_x;
        player->dodge.source.y = (Sint32)dodge_y;
    }
    if (axe_request)
    {
        player->axe_armor.source.x = (Sint32)axe_x;
        player->axe_armor.source.y = (Sint32)axe_y;
    }
    if (player->damage_number.applied)
    {
        player->damage_number.source.x = (Sint32)number_x;
        player->damage_number.source.y = (Sint32)number_y;
    }
    if (player->guard.applied)
    {
        player->guard.source.x = (Sint32)guard_x;
        player->guard.source.y = (Sint32)guard_y;
    }
    if (player->immunity.applied)
    {
        player->immunity.source.x = (Sint32)immune_x;
        player->immunity.source.y = (Sint32)immune_y;
    }
    if (player->absorption.applied)
    {
        player->absorption.source.x = (Sint32)absorb_x;
        player->absorption.source.y = (Sint32)absorb_y;
    }
    if (player->heart_gain.requested)
    {
        player->heart_gain.source.x = (Sint32)heart_x;
        player->heart_gain.source.y = (Sint32)heart_y;
    }
    if (player->movement_effects.smoke)
    {
        player->movement_effects.smoke_source.x = (Sint32)smoke_x;
        player->movement_effects.smoke_source.y = (Sint32)smoke_y;
    }
    if (player->movement_effects.water_bubble)
    {
        player->movement_effects.bubble_source.x = (Sint32)bubble_x;
        player->movement_effects.bubble_source.y = (Sint32)bubble_y;
    }
    if (player->gravity.ceiling_impact)
    {
        player->gravity.impact_source.x = (Sint32)impact_x;
        player->gravity.impact_source.y = (Sint32)impact_y;
    }
    for (Uint16 i = 0; i < player->wolf.commands.count; ++i)
    {
        player->wolf.commands.requests[i].source.x =
            (Sint32)((Sint64)player->wolf.commands.requests[i].source.x +
                     (Sint64)offset_x * FIX_ONE);
        player->wolf.commands.requests[i].source.y =
            (Sint32)((Sint64)player->wolf.commands.requests[i].source.y +
                     (Sint64)offset_y * FIX_ONE);
    }
    if (player->spirit.started)
    {
        player->spirit.source.x = (Sint32)spirit_x;
        player->spirit.source.y = (Sint32)spirit_y;
    }
    player->hurt = hurt;
    player->x.value = (Sint32)x;
    player->y.value = (Sint32)y;
    if (player->script_reaction.started)
    {
        player->script_reaction.source.x = (Sint32)script_x;
        player->script_reaction.source.y = (Sint32)script_y;
    }
    player->sleep = (SotnPlayerSleep){0};
    return true;
}

Uint32 SotnPlayer_Status(const SotnPlayer *player)
{
    Uint32 status = 0;
    if (player->bell_inside)
        status |= 0x100008U;
    if (player->curse_ticks)
        status |= 0x28108000U;
    if (player->poison_ticks)
        status |= 0x28104000U;
    if (player->water.feet)
        status |= 0x08020000U;
    if (player->step == SOTN_PLAYER_STAND || player->step == SOTN_PLAYER_WALK)
    {
        status |= 0x10000000U;
        if (player->step == SOTN_PLAYER_WALK)
            status |= 0x4000000U;
        if (player->substep == 4)
            status |= 0x8100000U;
    }
    if (player->step == SOTN_PLAYER_CROUCH)
        status |= 0x10000000U | (player->substep != 2 ? 0x20U : 0U);
    if (player->step == SOTN_PLAYER_FALL || player->step == SOTN_PLAYER_JUMP)
        status |= 0x10002000U;
    if (player->step == SOTN_PLAYER_HIT || player->step == SOTN_PLAYER_SCRIPT_REACTION)
        status |= 0x38110000U;
    if (SotnPlayerStep_IsDead(player->step))
        status |=
            0x18150000U |
            (player->step != SOTN_PLAYER_DEATH_HOLD && player->substep == 0x80 ? 0x80000U : 0U);
    if (player->step == SOTN_PLAYER_STONE)
        status |= 0x38110080U;
    if (player->step == SOTN_PLAYER_BOSS_GRAB)
        status |= 0x38110040U;
    if (player->step == SOTN_PLAYER_DARK || player->step == SOTN_PLAYER_HELLFIRE ||
        player->step == SOTN_PLAYER_SOUL)
        status |= 0x38000200U;
    if (player->step == SOTN_PLAYER_SUMMON || player->step == SOTN_PLAYER_TETRA)
        status |= 0x38000220U;
    if (player->spell_use_ticks)
        status |= 0x1000U;
    if (player->dark_ticks)
        status |= 0x2000000U;
    if (player->step == SOTN_PLAYER_SCRIPT)
        status |= 0x38110000U;
    if (player->axe_armor.active)
        status |= 0x29100000U;
    if (player->step == SOTN_PLAYER_HIGH_JUMP)
        status |= 0x38000000U;
    if (player->step == SOTN_PLAYER_SWORD_WARP || player->step == SOTN_PLAYER_TELEPORT)
        status |= 0x18100000U;
    if (player->step == SOTN_PLAYER_BAT)
        status |= 0x28100001U;
    if (player->step == SOTN_PLAYER_UNMORPH_BAT)
        status |= 0x28500001U;
    if (player->step == SOTN_PLAYER_MIST)
        status |= 0x28100002U;
    if (player->step == SOTN_PLAYER_UNMORPH_MIST)
        status |= 0x28900002U;
    if (player->step == SOTN_PLAYER_WOLF)
    {
        status |= 0x2c100204U;
        if (!player->substep || player->substep == 8)
            status |= 0x40000000U;
        if (player->substep == 3)
            status |= 0x20U;
    }
    if (player->step == SOTN_PLAYER_UNMORPH_WOLF)
        status |= 0x68100000U;
    if (player->step == SOTN_PLAYER_STUCK)
        status |= 0x18100030U;
    if (SotnPlayer_Protected(player))
        status |= 0x100U;
    if (player->weapon_startup)
        status |= 0x400U;
    if (player->subweapon_ticks)
        status |= 0x800U;
    return status;
}

bool SotnPlayer_GetState(const SotnPlayer *player, SotnPlayerState *out)
{
    if (player == NULL || out == NULL)
        return SDL_SetError("Player state requires a simulation and output");
    const Uint32 status = SotnPlayer_Status(player);
    *out = (SotnPlayerState){
        .water = player->water,
        .medicine = player->medicine,
        .food = player->food,
        .throwing = player->throwing,
        .throwing_grunt = player->throwing_grunt,
        .hammer_timer = player->hammer_timer,
        .water_breath_timer = player->water_breath_timer,
        .x = player->x.value,
        .y = player->y.value,
        .velocity_x = player->vx,
        .velocity_y = player->vy,
        .contacts = (Uint32)player->contact,
        .status = status,
        .received_damage_effects = player->received_damage_effects,
        .control_step = player->control_step,
        .control_substep = player->control_substep,
        .animation = player->animation,
        .simulated_pad = player->simulated_pad,
        .teleport = player->teleport,
        .script_reaction = player->script_reaction,
        .palette = player->palette,
        .body_grey = {player->body_grey[0], player->body_grey[1], player->body_grey[2],
                      player->body_grey[3]},
        .body_color_blend = player->body_color_blend,
        .step = player->step,
        .substep = player->substep,
        .sleep_event = player->sleep_event,
        .sleep = player->sleep,
        .facing_left = player->facing_left,
        .hellfire_started = player->hellfire_started,
        .hellfire_hidden = player->hellfire_palette == 0x810d,
        .hellfire_signal = player->hellfire_signal,
        .buttons = player->buttons,
        .tapped = player->tapped,
        .attack = player->attack,
        .attack_mode = player->attack_mode,
        .hand_owned = player->hand_factory != NULL,
        .hand_retire = player->hand_retire,
        .hand_grunt = player->hand_grunt,
        .hand_used = player->hand_used,
        .hp = player->hp,
        .hp_max = player->hp_max,
        .invulnerable = SotnPlayer_Protected(player) || player->step == SOTN_PLAYER_HIT ||
                        SotnPlayerStep_IsDead(player->step) || player->grab.state != SOTN_GRAB_NONE,
        .sound = player->sound,
        .death = player->death,
        .water_death = player->water_death,
        .fairy_rescue = player->fairy_rescue,
        .water_pain = player->water_pain,
        .water_pain_timer = player->water_damage.pain_timer,
        .defense_buff_minimum = player->defense_buff_minimum,
        .hud_hp_hold = player->hud_hp_hold,
        .heart_gain = player->heart_gain,
        .previous_contacts = player->frame_contacts,
        .absorption = player->absorption,
        .immunity = player->immunity,
        .guard = player->guard,
        .damage_number = player->damage_number,
        .dodge = player->dodge,
        .axe_armor = player->axe_armor,
        .looking_up = player->step == SOTN_PLAYER_STAND && player->substep == 1,
        .can_pause = player->attack_mode == 0 && !SotnPlayerStep_IsDead(player->step) &&
                     player->step != SOTN_PLAYER_HIT && player->step != SOTN_PLAYER_STONE &&
                     player->step != SOTN_PLAYER_SCRIPT && player->step != SOTN_PLAYER_AXE_SCRIPT &&
                     player->grab.state == SOTN_GRAB_NONE &&
                     player->step != SOTN_PLAYER_SCRIPT_REACTION &&
                     player->step != SOTN_PLAYER_BOSS_GRAB,
        .random = player->damage_random,
        .solid_width = (Uint16)SDL_max(0, player->wall_sensors[0].x),
        .solid_height = (Uint16)SDL_max(0, player->floor_sensors[1].y - 1),
        .shield = player->shield,
        .subweapon_starting = player->subweapon_ticks != 0,
        .weapon_starting = player->weapon_startup != 0,
        .stone = player->stone,
        .curse_ticks = player->curse_ticks,
        .poison_ticks = player->poison_ticks,
        .extra_sound = player->extra_sound,
        .third_sound = player->third_sound,
        .grab = player->grab,
        .priority = player->wolf.active ? player->wolf.priority : 148,
        .kick = player->kick,
        .gravity = player->gravity,
        .hurt = player->hurt,
        .mp = player->mp,
        .mp_spent = player->mp_spent,
        .muramasa_blood = player->muramasa_blood,
        .mist = player->mist,
        .bat = player->bat,
        .wolf = player->wolf,
        .afterimage = player->afterimage,
        .afterimage_generation = player->afterimage_generation,
        .movement_effects = player->movement_effects,
        .healing = player->healing,
        .dark = {.source = player->dark.source,
                 .started = player->dark.started,
                 .ticks = player->dark_ticks},
        .spirit = player->spirit,
        .soul = player->soul,
        .spell_use_ticks = player->spell_use_ticks,
        .bat_commands = player->bat_commands,
        .animation_bank = player->animation_bank,
        .rotated = player->form_rotated,
        .draw_pivot_x = player->draw_pivot_x,
        .draw_pivot_y = player->draw_pivot_y};
    SDL_memcpy(out->player_timers, player->timers, sizeof(out->player_timers));
    out->smoke_environment =
        (SotnPlayerSmokeEnvironment){.x = player->x.value,
                                     .y = player->y.value,
                                     .velocity_x = player->vx,
                                     .wall_owned = player->wall_snapshot_owned};
    for (Uint16 i = 0; i < SDL_arraysize(player->wall); ++i)
    {
        out->smoke_environment.wall[i].effects = player->wall[i].effects;
        out->smoke_environment.wall[i].x = player->wall_sensors[i].x;
        out->smoke_environment.wall[i].y = player->wall_sensors[i].y;
    }
    if (player->wolf.active)
    {
        out->wolf.controller.x =
            (Sint32)((Sint64)player->x.value - (Sint64)player->wolf_camera_x * 65536);
        out->wolf.controller.y =
            (Sint32)((Sint64)player->y.value - (Sint64)player->wolf_camera_y * 65536);
        out->wolf.controller.motion.animation = player->animation;
        out->wolf.controller.motion.velocity_x = player->vx;
        out->wolf.controller.motion.velocity_y = player->vy;
        out->wolf.controller.motion.contacts = (Uint32)player->contact;
        out->wolf.controller.motion.substep = player->substep;
        out->wolf.controller.motion.facing_left = player->facing_left;
        out->wolf.controller.mp = player->mp;
        out->wolf.controller.priority = player->wolf.priority;
    }
    return true;
}

bool SotnPlayer_SetArrivalFacing(SotnPlayer *player, bool facing_left)
{
    if (player == NULL)
        return SDL_InvalidParamError("player");
    player->facing_left = facing_left;
    return true;
}

bool SotnPlayer_KnockRight(const SotnPlayer *player, Sint32 source_x, Sint32 source_velocity)
{
    // func_80113F7C narrows the center difference to s16 before abs/sign tests.
    const Sint32 offset =
        player->facing_left ? -player->animation.hitbox_x : player->animation.hitbox_x;
    const Uint16 bits = (Uint16)((Uint32)player->x.parts.high + (Uint32)offset - (Uint32)source_x);
    const Sint32 delta = bits < 32768 ? bits : (Sint32)bits - 65536;
    return SDL_abs(delta) < 16 && source_velocity ? source_velocity > 0 : delta >= 0;
}

static bool Damage(SotnPlayer *player, Uint16 amount, Sint32 source_x, Sint32 source_velocity,
                   bool zero_stone, Uint16 kind, Uint16 effects)
{
    if (player == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Player damage requires a bounded physical attack and source");
    if ((!amount && !zero_stone) || player->hp == 0 || SotnPlayer_Protected(player) ||
        player->step == SOTN_PLAYER_HIT || player->grab.state != SOTN_GRAB_NONE ||
        player->step == SOTN_PLAYER_BOSS_GRAB)
        return true;
    if (player->step == SOTN_PLAYER_STONE)
        return SotnPlayer_Petrify(player, amount, source_x, source_velocity);
    SotnPlayer next = *player;
    // Hurt's original normal reset clears rotation without the transformation helper's flip.
    next.fairy_rescue.blend = 0;
    next.gravity.upside_down = false;
    SotnPlayer_BatCancel(&next);
    SotnPlayer_WolfCancel(&next);
    next.hp = (Uint16)(amount >= next.hp ? 0 : next.hp - amount);
    const bool direction = SotnPlayer_KnockRight(&next, source_x, source_velocity);
    const bool right = kind == 17 ? true : kind == 18 ? false : direction;
    ClearAttack(&next);
    next.movement_flags = 0;
    next.invincibility = 12;
    if (next.hp == 0)
    {
        PhysicalDeath(&next, direction, effects);
        if (next.failed ||
            (!next.contact_dispatch && !SotnAnimation_Tick(next.dra, &next.animation)))
            return false;
        *player = next;
        return true;
    }
    if (next.axe_armor.active)
    {
        next.axe_armor.knock_right = right;
        next.axe_armor.events.factories[0] = 85;
        next.axe_armor.events.factory_count = 1;
        next.vx = right ? 81920 : -81920;
        next.vy = -3 * FIX_ONE;
        SetStep(&next, SOTN_PLAYER_AXE_HIT);
        Animate(&next, 0xd1);
        next.sound = 0x6ed;
        next.invincibility = 20;
        AxeTick(&next);
        if (next.tight_space)
            next.vy = 0;
        if (next.failed ||
            (!next.contact_dispatch && !SotnAnimation_Tick(next.dra, &next.animation)))
            return false;
        *player = next;
        return true;
    }
    if (kind < 2)
        kind = (Uint32)amount * 2U >= next.hp_max ? 4 : (Uint32)amount * 50U >= next.hp_max ? 3 : 2;
    if (kind != 2 && kind != 3 && kind != 4 && kind != 5 && kind != 7 && kind != 16 && kind != 17 &&
        kind != 18)
        return SDL_SetError("Player hurt requires a supported original resolved damage kind");
    next.hurt.knock_right = right;
    next.hurt.started = true;
    if (next.curse_ticks)
        HurtFactory(&next, 0x17002cU, 0, 0);
    if (next.poison_ticks)
        HurtFactory(&next, 0x16002cU, 0, 0);
    Uint16 pain = 0;
    const SotnPlayerStep previous = next.step;
    if (kind == 4 || kind == 5)
    {
        const Uint32 y = (Uint32)next.y.value - 1U;
        SDL_memcpy(&next.y.value, &y, sizeof(y));
        next.vy = -32768;
        next.vx = right ? 8 * FIX_ONE : -8 * FIX_ONE;
        next.substep = kind == 5 ? 15 : 2;
        Animate(&next, 0x2e);
        next.facing_left = right;
        if (kind == 5)
            next.hurt.damage_taken = amount;
        else
            pain = (Uint16)((SotnCombat_Random(&next.damage_random) & 1U) + 1U);
    }
    else
    {
        const bool strong = kind != 2;
        pain = (Uint16)((SotnCombat_Random(&next.damage_random) & 1U) + (strong ? 3U : 5U));
        if ((kind == 2 && (previous == SOTN_PLAYER_STAND || previous == SOTN_PLAYER_WALK)) ||
            (kind < 16 && (previous == SOTN_PLAYER_CROUCH || previous == SOTN_PLAYER_STUCK)))
        {
            next.vx = right ? 163840 : -163840;
            next.vy = 0;
            if (previous == SOTN_PLAYER_CROUCH || previous == SOTN_PLAYER_STUCK)
            {
                next.substep = 7;
                Animate(&next, 0x23);
                HurtFactory(&next, 0, 0, 0);
            }
            else
            {
                next.substep = 6;
                Animate(&next, (Uint16)(0x31U + (SotnCombat_Random(&next.damage_random) & 1U) +
                                        (right != next.facing_left ? 2U : 0U)));
                HurtFactory(&next, 0x60000U, 0, 0);
            }
        }
        else
        {
            next.vx = right ? (strong ? 109226 : 81920) : -(strong ? 109226 : 81920);
            next.vy = strong ? ((previous == SOTN_PLAYER_JUMP || previous == SOTN_PLAYER_FALL)
                                    ? -3 * FIX_ONE
                                    : -4 * FIX_ONE)
                             : -2 * FIX_ONE;
            next.substep = 1;
            const Uint16 choice = HurtAnimation(&next);
            if (strong && !choice)
                Animate(&next, 0x40);
        }
    }
    const Uint8 *sound = NULL;
    if (!AttackView(&next, 0x800acf84 + pain * 2U, 2, &sound))
        return false;
    next.sound = Attack16(sound);
    if (next.tight_space)
        next.vy = 0;
    next.step = SOTN_PLAYER_HIT;
    next.hit_effect_ticks = 6;
    next.damage_palette = 0x8166;
    if (next.failed || (!next.contact_dispatch && !SotnAnimation_Tick(next.dra, &next.animation)))
        return false;
    *player = next;
    return true;
}

bool SotnPlayer_Damage(SotnPlayer *player, Uint16 amount, Sint32 source_x, Sint32 source_velocity)
{
    return Damage(player, amount, source_x, source_velocity, false, 1, 0);
}
bool SotnPlayer_ContactDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                              Sint32 source_velocity, Uint16 kind, Uint16 effects)
{
    if (!player || kind > 31)
        return SDL_SetError("Contact damage requires its actual resolved kind and player");
    const Uint16 before = player->hp;
    SotnPlayer next = *player;
    if (!Damage(&next, amount, source_x, source_velocity, false, kind, effects))
        return false;
    if (next.hp && next.hp < before && !next.axe_armor.active)
    {
        if (!(effects & 0xfbc0U))
            HurtFactory(&next, 0x58002cU, 0, 0);
        if (effects & 0x100U)
        {
            next.curse_ticks = (Uint16)SDL_max(64, 1024 - (Sint32)next.attributes.total[1] * 4);
            next.damage_palette = 0x8165;
            HurtFactory(&next, 0x2002fU, 0, 0);
            HurtFactory(&next, 0x17002cU, 0, 0);
            next.extra_sound = 0x6f2;
        }
        else if (effects & 0x80U)
        {
            next.poison_ticks = (Uint16)SDL_max(256, 4095 - (Sint32)next.attributes.total[1] * 16);
            next.damage_palette = 0x8164;
            HurtFactory(&next, 0x16002cU, 0, 0);
            HurtFactory(&next, 0x1002fU, 0, 0);
            next.extra_sound = 0x6f2;
        }
        else if (effects & 0x8000U)
        {
            next.extra_sound = 0x690;
            HurtFactory(&next, 0x45002cU, 0, 0);
            HurtFactory(&next, 0x10011U, 0, 0);
            HurtFactory(&next, 18, 0, 0);
            next.damage_palette = 0x8160;
            next.hit_effect_ticks = 16;
        }
        else if (effects & 0x4000U)
        {
            HurtFactory(&next, 45, 0, 0);
            HurtFactory(&next, 0x46002cU, 0, 0);
            next.damage_palette = 0x8102;
            next.hit_effect_ticks = 24;
        }
        else if (effects & 0x2000U)
        {
            HurtFactory(&next, 46, 0, 0);
            next.damage_palette = 0x8169;
            next.hit_effect_ticks = 12;
            next.animation.id = 0x2e;
        }
        else
        {
            if (effects & 0x1000U)
            {
                HurtFactory(&next, 119, 0, 0);
                HurtFactory(&next, 0x68002cU, 0, 0);
                next.damage_palette = 0x8168;
                next.hit_effect_ticks = 8;
            }
            if (effects & 0x800U)
            {
                HurtFactory(&next, 113, 0, 0);
                HurtFactory(&next, 0x62002cU, 0, 0);
                next.damage_palette = 0x8164;
                next.hit_effect_ticks = 16;
            }
            if (effects & 0x40U)
            {
                next.damage_palette = 0x8166;
                next.hit_effect_ticks = 16;
                next.extra_sound = 0x62e;
                HurtFactory(&next, 0x42002cU, 0, 0);
                HurtFactory(&next, 0x50014U, 0, 0);
            }
        }
    }
    if (next.failed)
        return false;
    *player = next;
    return true;
}
bool SotnPlayer_AxeContactDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                                 Sint32 source_velocity, Uint16 kind, bool zero_stone,
                                 Uint16 effects)
{
    if (!player || !player->axe_armor.active || kind > 31)
        return SDL_SetError("Axe contact requires its actual equipped player and damage kind");
    return Damage(player, amount, source_x, source_velocity, zero_stone, kind, effects);
}

bool SotnPlayer_Restore(SotnPlayer *player, Uint16 hp, Uint32 random)
{
    if (player == NULL || hp == 0 || hp > player->hp_max || SotnPlayerStep_IsDead(player->step) ||
        player->attack_mode != 0)
        return SDL_SetError("Health restore requires a living, non-attacking player and health "
                            "within the current maximum");
    player->hp = hp;
    player->damage_random = random;
    return true;
}

bool SotnPlayer_SetCharacter(SotnPlayer *player, const SotnCharacterState *state,
                             SotnCharacterStats stats)
{
    if (player == NULL || state == NULL || state->hp_max == 0 || state->hp_max > 9999 ||
        state->hp > state->hp_max || state->mp > state->mp_max || state->mp_max > 9999 ||
        state->hearts_max > 9999 || state->hearts > state->hearts_max ||
        (state->equipment[4] == 54 && !state->hearts_max) || stats.attack[0] > 999 ||
        stats.attack[1] > 999 || stats.unarmed_attack > 999 || SotnPlayerStep_IsDead(player->step))
        return SDL_SetError(
            "Player character update requires bounded living progress and attributes");
    player->hp = state->hp;
    player->hp_max = state->hp_max;
    player->mp = state->mp;
    player->attributes = stats;
    player->head = state->equipment[2];
    player->armor = state->equipment[3];
    player->cape = state->equipment[4];
    player->talismans = (Uint16)((state->equipment[5] == 83) + (state->equipment[6] == 83));
    player->hearts = state->hearts;
    player->hearts_max = state->hearts_max;
    return true;
}

void SotnPlayer_Close(SotnPlayer *player)
{
    SDL_free(player);
}

bool SotnPlayer_SetSubweapons(SotnPlayer *player, SotnSubweapons *service)
{
    if (player == NULL)
        return SDL_InvalidParamError("player");
    player->subweapons = service;
    return true;
}

bool SotnPlayer_SetRelics(SotnPlayer *player, const SotnWorldState *world)
{
    if (player == NULL || world == NULL)
        return SDL_SetError("Player relic update requires a simulation and persistent relics");
    for (Uint32 i = 0; i < SOTN_RELICS; ++i)
        if (world->relics[i] > 3 || world->relics[i] == 2)
            return SDL_SetError("Player relic %u has invalid found/active bits", i);
    SDL_memcpy(player->relics, world->relics, sizeof(player->relics));
    return true;
}

bool SotnPlayer_Displace(SotnPlayer *p, Sint32 dx, Sint32 dy, Uint32 contacts)
{
    if (p == NULL || dx < -64 * 65536 || dx > 64 * 65536 || dy < -64 * 65536 || dy > 64 * 65536 ||
        (contacts & ~0x4fU) != 0)
        return SDL_SetError("Stage-object displacement exceeds the bounded physical contacts");
    const Sint64 x = (Sint64)p->x.value + dx, y = (Sint64)p->y.value + dy;
    if (!PositionValid(p->map, x, y))
        return SDL_SetError("Stage object pushed Alucard outside the room margin");
    p->x.value = (Sint32)x;
    p->y.value = (Sint32)y;
    p->contact |= (Sint32)contacts;
    return true;
}

bool SotnPlayer_StageStop(SotnPlayer *p)
{
    if (p == NULL)
        return SDL_InvalidParamError("player");
    if (!SotnPlayerStep_IsDead(p->step))
        p->vx = p->vy = 0;
    return true;
}

bool SotnPlayer_StageStopHorizontal(SotnPlayer *p)
{
    if (p == NULL)
        return SDL_InvalidParamError("player");
    p->vx = 0;
    return true;
}

bool SotnPlayer_StageGuard(SotnPlayer *p, bool halt_vertical)
{
    if (p == NULL)
        return SDL_InvalidParamError("player");
    if (SotnPlayerStep_IsDead(p->step))
        return true;
    if (halt_vertical)
        p->vy = 0;
    SotnPlayer_Protect(p, 32);
    return true;
}

bool SotnPlayer_BellInside(SotnPlayer *player, bool inside)
{
    if (!player)
        return SDL_InvalidParamError("player");
    player->bell_inside = inside;
    return true;
}

// Selected status timer and hit ordering from dra/{5D5BC,71830,692E8,6E42C}.c,
// pinned reconstruction. Native adapters retain the transient timer across stage arrivals.
bool SotnPlayer_RestoreCurse(SotnPlayer *player, Uint16 ticks)
{
    if (player == NULL || ticks > 1024 || SotnPlayerStep_IsDead(player->step))
        return SDL_SetError("Curse timer restore requires a living player and 0..1024 ticks");
    player->curse_ticks = ticks;
    return true;
}
bool SotnPlayer_CurseDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                            Sint32 source_velocity)
{
    if (player == NULL || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Curse contact requires bounded damage and source motion");
    if (!amount || !player->hp || SotnPlayer_Protected(player) || player->step == SOTN_PLAYER_HIT ||
        SotnPlayerStep_IsDead(player->step) || player->step == SOTN_PLAYER_BOSS_GRAB ||
        player->grab.state != SOTN_GRAB_NONE)
        return true;
    if (player->step == SOTN_PLAYER_STONE)
        return SotnPlayer_Damage(player, amount, source_x, source_velocity);
    SotnPlayer next = *player;
    if (!SotnPlayer_Damage(&next, amount, source_x, source_velocity))
        return false;
    if (next.hp)
    {
        next.curse_ticks = (Uint16)SDL_max(64, 1024 - (Sint32)next.attributes.total[1] * 4);
        next.extra_sound = 0x6f2;
    }
    *player = next;
    return true;
}

bool SotnPlayer_SetScriptHold(SotnPlayer *player, bool hold)
{
    if (player == NULL ||
        (hold && (SotnPlayerStep_IsDead(player->step) || player->step == SOTN_PLAYER_BOSS_GRAB ||
                  player->grab.state != SOTN_GRAB_NONE)))
        return SDL_SetError("Script hold requires a living normal-form player owner");
    if (!SotnPlayer_SetSimulatedPad(
            player, (SotnPlayerSimulatedPad){.mode = hold ? 2 : 0, .remaining = hold ? 1 : 0}))
        return false;
    player->legacy_script_hold = hold;
    return true;
}

bool SotnPlayer_StageAlignX(SotnPlayer *p, Sint32 x)
{
    if (!p)
        return SDL_InvalidParamError("player");
    const Sint64 target = (Sint64)x * FIX_ONE + p->x.parts.low;
    if (!PositionValid(p->map, target, p->y.value))
        return SDL_SetError("Script horizontal alignment exceeds the bounded player room");
    p->x.value = (Sint32)target;
    return true;
}
bool SotnPlayer_StageAlignY(SotnPlayer *p, Sint32 y)
{
    if (!p)
        return SDL_InvalidParamError("player");
    const Sint64 target = (Sint64)y * FIX_ONE + p->y.parts.low;
    if (!PositionValid(p->map, p->x.value, target))
        return SDL_SetError("Boss vertical alignment exceeds the bounded player room");
    p->y.value = (Sint32)target;
    return true;
}

bool SotnPlayer_RequestGrab(SotnPlayer *p, SotnPlayerGrab request)
{
    if (!p || request.damage > 9999 || request.source_x < -512 || request.source_x > 16896 ||
        request.source_velocity < -16 * FIX_ONE || request.source_velocity > 16 * FIX_ONE ||
        (request.state != SOTN_GRAB_NONE && request.state != SOTN_GRAB_START &&
         request.state != SOTN_GRAB_DAMAGE && request.state != SOTN_GRAB_RELEASE) ||
        (request.reaction != SOTN_GRAB_REACTION_NORMAL &&
         request.reaction != SOTN_GRAB_REACTION_STONE) ||
        request.release_velocity_x < -16 * FIX_ONE || request.release_velocity_x > 16 * FIX_ONE ||
        request.release_velocity_y < -16 * FIX_ONE || request.release_velocity_y > 16 * FIX_ONE ||
        (request.state != SOTN_GRAB_RELEASE &&
         (request.release_velocity_x || request.release_velocity_y)))
        return SDL_SetError("Boss grab requires a supported request and bounded damage/source");
    if (request.state == SOTN_GRAB_NONE)
    {
        p->grab.state = SOTN_GRAB_NONE;
        return true;
    }
    if (!p->hp || SotnPlayerStep_IsDead(p->step) || p->script_hold ||
        p->step == SOTN_PLAYER_SCRIPT || p->step == SOTN_PLAYER_STONE)
        return SDL_SetError("Boss grab requires a living normal-form player outside a script");
    if ((request.state == SOTN_GRAB_START &&
         (p->grab.state != SOTN_GRAB_NONE || p->step == SOTN_PLAYER_BOSS_GRAB)) ||
        ((request.state == SOTN_GRAB_DAMAGE || request.state == SOTN_GRAB_RELEASE) &&
         (p->step != SOTN_PLAYER_BOSS_GRAB || p->grab.state != SOTN_GRAB_HELD)))
        return SDL_SetError("Boss grab request does not match the player's acknowledgement");
    if (request.state != SOTN_GRAB_START)
        request.reaction = p->grab.reaction;
    if (request.state == SOTN_GRAB_RELEASE)
    {
        p->vx = request.release_velocity_x;
        p->vy = request.release_velocity_y;
    }
    p->grab = request;
    return true;
}
bool SotnPlayer_SetChairKind(SotnPlayer *player, Uint16 kind)
{
    if (player == NULL || kind > 4)
        return SDL_SetError("Chair hint requires a player and original pose kind 0..4");
    player->chair_kind = kind;
    return true;
}

// dra/71830.c applies poison after the normal surviving hit; curse takes precedence in scene.
bool SotnPlayer_RestorePoison(SotnPlayer *player, Uint16 ticks)
{
    if (!player || ticks > 4095 || SotnPlayerStep_IsDead(player->step))
        return SDL_SetError("Poison timer restore requires a living player and 0..4095 ticks");
    player->poison_ticks = ticks;
    return true;
}
bool SotnPlayer_PoisonDamage(SotnPlayer *player, Uint16 amount, Sint32 source_x,
                             Sint32 source_velocity)
{
    if (!player || amount > 9999 || source_x < -512 || source_x > 16896 ||
        source_velocity < -16 * FIX_ONE || source_velocity > 16 * FIX_ONE)
        return SDL_SetError("Poison contact requires bounded damage and source motion");
    if (!amount || !player->hp || SotnPlayer_Protected(player) || player->step == SOTN_PLAYER_HIT ||
        SotnPlayerStep_IsDead(player->step) || player->step == SOTN_PLAYER_BOSS_GRAB ||
        player->grab.state != SOTN_GRAB_NONE)
        return true;
    if (player->step == SOTN_PLAYER_STONE)
        return SotnPlayer_Damage(player, amount, source_x, source_velocity);
    SotnPlayer next = *player;
    if (!SotnPlayer_Damage(&next, amount, source_x, source_velocity))
        return false;
    if (next.hp)
    {
        next.poison_ticks = (Uint16)SDL_max(256, 4095 - (Sint32)next.attributes.total[1] * 16);
        next.extra_sound = 0x6f2;
    }
    *player = next;
    return true;
}

bool SotnPlayer_SetUseTimer(SotnPlayer *p, Uint8 ticks)
{
    if (!p || ticks > 32)
        return SDL_SetError("Player use timer requires its owner and bounded ticks");
    p->subweapon_ticks = ticks;
    return true;
}

bool SotnPlayer_SetWeaponStartup(SotnPlayer *p, Uint8 ticks)
{
    if (!p || ticks > 32)
        return SDL_SetError("Player weapon-startup timer requires its owner and bounded ticks");
    p->weapon_startup = ticks;
    return true;
}

bool SotnPlayer_SetSpellUseTimer(SotnPlayer *p, Uint8 ticks)
{
    if (!p || ticks > 32)
        return SDL_SetError("Player spell-use timer requires its owner and bounded ticks");
    p->spell_use_ticks = ticks;
    return true;
}

bool SotnPlayer_SetSlot16Item(SotnPlayer *p, Uint16 item)
{
    if (!p || item >= 217)
        return SDL_SetError("Slot16 item requires its owner and a bounded original item");
    p->slot16_item = item;
    return true;
}

bool SotnPlayer_RaiseDarkTimer(SotnPlayer *p, Uint16 minimum)
{
    if (!p || !minimum || minimum > 0x38)
        return SDL_SetError("Dark timer floor requires its owner and the original bounded value");
    if (p->dark_ticks < minimum)
        p->dark_ticks = minimum;
    return true;
}

bool SotnPlayer_SeedInput(SotnPlayer *p, Uint16 buttons)
{
    const Uint16 valid = 0xf9feU;
    if (!p || (buttons & (Uint16)~valid))
        return SDL_SetError("Player input handoff requires actual supported physical button words");
    p->buttons = p->previous_buttons = p->held = buttons;
    p->tapped = 0;
    return true;
}

bool SotnPlayer_ClearShield(SotnPlayer *p)
{
    if (!p || p->shield_cursor.phase > (p->hand_factory ? 4 : 3))
        return SDL_SetError("Shield retirement requires its actual ordinary weapon owner");
    if (p->shield_cursor.phase)
    {
        p->shield = (SotnPlayerAttack){0};
        p->shield_cursor = (SotnShield){0};
    }
    return true;
}

bool SotnPlayer_QueueBloodHealing(SotnPlayer *p, Uint16 amount, Uint16 contacts)
{
    if (!p || (amount != 0 && amount != 8 && amount != 16) || !contacts || contacts > 32)
        return SDL_SetError("Blood healing requires an actual original amount and contact count");
    if (!p->healing.pending)
    {
        p->healing.pending = amount;
        if (amount)
            p->healing.pending_kind = 1;
    }
    p->healing.contacts += contacts;
    return true;
}
bool SotnPlayer_QueueMailHealing(SotnPlayer *p)
{
    if (!p)
        return SDL_InvalidParamError("player");
    p->healing.pending = 1;
    p->healing.pending_kind = 2;
    return true;
}

bool SotnPlayer_SetSpiritContext(SotnPlayer *p, bool enabled)
{
    if (!p || (enabled && (!p->dra.bytes || p->dra.size < 0x101b8U + 0xf2U * 4U)))
        return SDL_SetError(
            "Spirit commands require actual native effect/capture/freeze/combat owners");
    p->spirit_context = enabled;
    return true;
}
bool SotnPlayer_TickSpellFreeze(SotnPlayer *p, Uint16 buttons, bool animate)
{
    const Uint16 valid = 0xf9feU;
    if (!p || (buttons & (Uint16)~valid))
        return SDL_SetError("Frozen player animation requires its owner and supported US buttons");
    SotnPlayer next = *p;
    next.previous_buttons = buttons;
    next.sound = next.extra_sound = next.third_sound = next.mp_spent = 0;
    next.dark.started = next.spirit.started = false;
    next.hellfire_started = false;
    next.soul.started = next.soul.attack = false;
    next.water_death.started = false;
    next.death.started = false;
    next.teleport.used = false;
    next.medicine.used = false;
    next.food.used = false;
    next.throwing.used = false;
    next.throwing_grunt = 0;
    next.water_pain = (SotnPlayerWaterPain){0};
    next.defense_buff_minimum = 0;
    next.hud_hp_hold = false;
    next.heart_gain = (SotnPlayerHeartGain){0};
    next.absorption = (SotnPlayerAbsorption){0};
    next.immunity = (SotnPlayerImmunity){0};
    next.guard = (SotnPlayerGuard){0};
    next.damage_number = (SotnPlayerDamageNumber){0};
    next.dodge = (SotnPlayerDodge){0};
    next.axe_armor.events = (SotnAxeArmorEvents){0};
    next.axe_armor.entered = next.axe_armor.exited = false;
    next.script_reaction.started = false;
    next.mist.interrupted = false;
    next.healing.applied = next.healing.effects = false;
    next.healing.applied_kind = 0;
    next.healing.applied_amount = 0;
    next.gravity.ceiling_impact = false;
    next.hurt.count = 0;
    next.hurt.shake_requested = false;
    next.hurt.started = false;
    next.bat_commands = (SotnBatCommandEvents){0};
    next.wolf.commands = (SotnWolfControllerEvents){0};
    next.sleep_event = false;
    next.movement_effects = (SotnPlayerMovementEffects){0};
    const Uint16 id = next.animation.id;
    if (animate && (id == 0xba || id == 0xbb || id == 0xbe || id == 0xf0 || id == 0xf1) &&
        !SotnAnimation_Tick(next.dra, &next.animation))
        return false;
    *p = next;
    return true;
}
bool SotnPlayer_EndSpellFreeze(SotnPlayer *p)
{
    if (!p)
        return SDL_InvalidParamError("player");
    SotnPlayer_Protect(p, 48);
    return true;
}
