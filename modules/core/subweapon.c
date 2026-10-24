// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected func_8010EB5C/func_800FE3C4/func_8011A290 and projectile paths in
// dra/6E42C.c, 5D5BC.c, 7879C.c, 84B88.c, 8A0A4.c at sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Definitions remain private runtime data.
#include <sotn/combat.h>
#include <sotn/subweapon.h>

typedef struct Projectile
{
    SotnSubweaponState state;
    Uint16 hit, counter, height;
    Sint32 offset;
} Projectile;
struct SotnSubweapons
{
    SotnDataView dra;
    SotnCharacter *character;
    SotnCollisionMap map;
    Projectile slots[SOTN_SUBWEAPON_SLOTS];
    Uint32 serial, ticks;
    Uint16 sounds[64], sound_count;
    bool warned[10], frozen;
};
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Sint32 Pixel(Sint32 p)
{
    return p >= 0 ? p / 65536 : (Sint32)(-((-(Sint64)p + 65535) / 65536));
}
static bool Sound(SotnSubweapons *s, Uint16 id)
{
    if (s->sound_count >= SDL_arraysize(s->sounds))
        return SDL_SetError("Subweapon sound queue capacity exceeded");
    s->sounds[s->sound_count++] = id;
    return true;
}
static bool Definition(SotnSubweapons *s, Uint16 id, SotnPlayerAttack *attack, Uint16 *cost,
                       Uint16 *chain, Uint16 *animation)
{
    if (id >= 13 || s->dra.bytes == NULL || s->dra.size < 0x4a00 + 13 * 20)
        return SDL_SetError("Subweapon definition is outside the private DRA table");
    const Uint8 *p = s->dra.bytes + 0x4a00 + id * 20;
    const Uint16 power = Read16(p), hearts = Read16(p + 2);
    if (power > 999 || hearts > 9999 || p[6] > SOTN_SUBWEAPON_SLOTS || Read16(p + 18) > 255 ||
        (id != 0 && p[6] == 0))
        return SDL_SetError(
            "Subweapon %u has invalid attack, heart cost, chain, or critical bounds", id);
    *attack = (SotnPlayerAttack){.power = power,
                                 .element = Read16(p + 4),
                                 .invincibility_frames = p[7],
                                 .stun_frames = Read16(p + 8),
                                 .hit_type = Read16(p + 12),
                                 .hit_effect = Read16(p + 14),
                                 .critical_rate = (Uint8)Read16(p + 18),
                                 .active = true};
    *cost = hearts;
    *chain = p[6];
    *animation = p[10];
    return true;
}
static Uint16 Cost(SotnCharacterState profile, Uint16 cost)
{
    const Uint16 broaches =
        (Uint16)((profile.equipment[5] == 77 ? 1 : 0) + (profile.equipment[6] == 77 ? 1 : 0));
    return (Uint16)SDL_max(1, cost / (broaches + 1));
}
static bool InitializeAttack(SotnSubweapons *s, Uint16 id, SotnPlayerAttack *attack)
{
    Uint16 cost = 0, chain = 0, animation = 0;
    SotnCharacterState profile = {0};
    SotnCharacterStats stats = {0};
    if (!Definition(s, id, attack, &cost, &chain, &animation) ||
        !SotnCharacter_Get(s->character, &profile, &stats))
        return false;
    attack->power = (Uint16)(attack->power +
                             (stats.total[2] * 2 + SotnCombat_Random(&profile.random) % 12) / 10);
    return SotnCharacter_Restore(s->character, &profile);
}
bool SotnSubweapons_Create(SotnDataView dra, SotnCharacter *character, SotnCollisionMap map,
                           SotnSubweapons **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (character == NULL || dra.bytes == NULL || dra.size < 0x4a00 + 13 * 20)
        return SDL_SetError("Subweapons require private definitions and an owned character");
    SotnSubweapons *s = SDL_calloc(1, sizeof(*s));
    if (s == NULL)
        return false;
    s->dra = dra;
    s->character = character;
    if (!SotnSubweapons_Enter(s, map))
    {
        SDL_free(s);
        return false;
    }
    *out = s;
    return true;
}
bool SotnSubweapons_Enter(SotnSubweapons *s, SotnCollisionMap map)
{
    SotnCollider col = {0};
    if (s == NULL || !SotnCollision_Query(&map, 0, 0, &col))
        return false;
    s->map = map;
    SDL_memset(s->slots, 0, sizeof(s->slots));
    s->frozen = false;
    s->sound_count = 0;
    return true;
}
bool SotnSubweapons_Use(SotnSubweapons *s, SotnPlayerState player, bool *used, Uint16 *animation)
{
    if (s == NULL || used == NULL || animation == NULL)
        return SDL_SetError("Subweapon use requires a service and action outputs");
    *used = false;
    *animation = 0;
    if (player.x < -256 * 65536 || player.x > 17408 * 65536 || player.y < -256 * 65536 ||
        player.y > 17408 * 65536)
        return SDL_SetError("Subweapon origin exceeds the bounded room margin");
    SotnCharacterState profile = {0};
    if (!SotnCharacter_Get(s->character, &profile, NULL))
        return false;
    const Uint16 id = profile.subweapon;
    if (id == 0 || player.hp == 0 || player.step == SOTN_PLAYER_KILL)
        return true;
    if (id != 1 && id != 2 && id != 3 && id != 6)
    {
        if (!s->warned[id])
        {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_APPLICATION,
                "Subweapon %u is collected but its attack is not integrated in this checkpoint",
                id);
            s->warned[id] = true;
        }
        return true;
    }
    Uint16 cost = 0, chain = 0, anim = 0;
    SotnPlayerAttack attack = {0};
    if (!Definition(s, id, &attack, &cost, &chain, &anim))
        return false;
    cost = Cost(profile, cost);
    if (profile.hearts < cost || (id == 6 && s->frozen))
        return true;
    Uint32 active = 0, slot = SOTN_SUBWEAPON_SLOTS;
    for (Uint32 i = 0; i < SOTN_SUBWEAPON_SLOTS; ++i)
        if (s->slots[i].state.generation != 0)
            active += s->slots[i].state.id == id ? 1U : 0U;
        else if (slot == SOTN_SUBWEAPON_SLOTS)
            slot = i;
    if (active >= chain || slot == SOTN_SUBWEAPON_SLOTS)
        return true;
    if (s->serial == SDL_MAX_UINT32)
        return SDL_SetError("Subweapon generation capacity exhausted");
    profile.hearts = (Uint16)(profile.hearts - cost);
    if (!SotnCharacter_Restore(s->character, &profile) || !InitializeAttack(s, id, &attack))
        return false;
    Projectile p = {.state = {.x = player.x,
                              .y = player.y,
                              .id = id,
                              .phase = 1,
                              .generation = ++s->serial,
                              .attack = attack,
                              .facing_left = player.facing_left}};
    const Sint32 forward = player.facing_left ? -1 : 1;
    if (id == 1)
    {
        if (player.step != SOTN_PLAYER_CROUCH)
            p.state.y -= 9 * 65536;
        p.state.velocity_x = forward * 8 * 65536;
        p.state.attack.width = 4;
        p.state.attack.height = 2;
        p.state.attack.offset_x = 4;
        if (!Sound(s, 0x60c))
            return false;
    }
    else if (id == 2)
    {
        if (player.step != SOTN_PLAYER_CROUCH)
            p.state.y -= 12 * 65536;
        p.state.velocity_x = forward * 2 * 65536;
        p.state.velocity_y = -6 * 65536;
        p.state.angle = player.facing_left ? 0xc00 : 0x400;
        p.state.attack.width = p.state.attack.height = 12;
        if (!Sound(s, 0x60c))
            return false;
    }
    else if (id == 3)
    {
        p.state.y += 8 * 65536;
        p.state.velocity_x = forward * 81920;
        p.state.velocity_y = -204800;
        p.state.attack.width = p.state.attack.height = 4;
    }
    else
    {
        p.state.attack.active = false;
        s->frozen = true;
        if (!Sound(s, 0x6ad))
            return false;
    }
    s->slots[slot] = p;
    *used = true;
    *animation = anim;
    return true;
}
static bool HolyCollision(SotnSubweapons *s, Projectile *p, bool *hit)
{
    SotnCollider first = {0}, above = {0}, wall = {0};
    const Sint32 x = Pixel(p->state.x), y = Pixel(p->state.y);
    if (!SotnCollision_Query(&s->map, x, y, &first) ||
        !SotnCollision_Query(&s->map, x, y - 1 + first.top, &above))
        return false;
    Uint32 flags = first.effects & 0xf801;
    const Uint32 kind = flags & 0x8801;
    *hit = false;
    if (kind == 1 || kind == 0x801)
    {
        flags = above.effects & 0xf001;
        if (!(flags & 1))
        {
            p->state.y += first.top * 65536;
            *hit = true;
        }
        else if ((above.effects & 0x8001) == 0x8001)
        {
            p->state.y += (first.top - 1 + above.top) * 65536;
            *hit = (flags & 1) != 0;
        }
    }
    else if ((flags & 0x8001) == 0x8001)
    {
        p->state.y += first.top * 65536;
        *hit = true;
    }
    p->state.x += p->state.velocity_x;
    if (!SotnCollision_Query(&s->map, Pixel(p->state.x) + (p->state.velocity_x < 0 ? -4 : 4),
                             Pixel(p->state.y) - 7, &wall))
        return false;
    if (wall.effects & SOTN_EFFECT_SIDE)
    {
        p->state.x =
            (Pixel(p->state.x) + (p->state.velocity_x < 0 ? wall.right : wall.left)) * 65536;
        *hit = true;
    }
    return true;
}
static bool Flame(SotnSubweapons *s, Projectile parent)
{
    Uint32 slot = 0;
    while (slot < SOTN_SUBWEAPON_SLOTS && s->slots[slot].state.generation != 0)
        ++slot;
    if (slot == SOTN_SUBWEAPON_SLOTS)
        return true; // Original factory drops children at capacity.
    if (s->serial == SDL_MAX_UINT32)
        return SDL_SetError("Subweapon generation capacity exhausted");
    Projectile p = {.state = {.x = parent.state.x,
                              .y = parent.state.y,
                              .id = 11,
                              .phase = 1,
                              .timer = 80,
                              .generation = ++s->serial,
                              .facing_left = parent.state.facing_left}};
    SotnCharacterState profile = {0};
    if (!SotnCharacter_Get(s->character, &profile, NULL))
        return false;
    for (Uint32 i = 0; i < 3; ++i)
        (void)SotnCombat_Random(&profile.random);
    if (!SotnCharacter_Restore(s->character, &profile) ||
        !InitializeAttack(s, 11, &p.state.attack) ||
        !SotnCharacter_Get(s->character, &profile, NULL))
        return false;
    p.height = (Uint16)((SotnCombat_Random(&profile.random) & 15) + 18);
    if (!SotnCharacter_Restore(s->character, &profile))
        return false;
    p.state.attack.width = 4;
    p.state.visible = true;
    s->slots[slot] = p;
    return true;
}
static bool TickOne(SotnSubweapons *s, Projectile *p, SotnPlayerState player, Sint32 camera_x,
                    Sint32 camera_y)
{
    SotnSubweaponState *q = &p->state;
    if (q->age == 0)
    {
        ++q->age;
        if (q->id != 6)
        {
            q->visible = q->id == 3;
            return true;
        }
        q->x += (player.x + (player.facing_left ? 8 : -8) * 65536 - q->x) / 12;
        q->y += (player.y - 16 * 65536 - q->y) / 4;
        return true;
    }
    if (q->age != SDL_MAX_UINT16)
        ++q->age;
    if (q->id == 1)
    {
        if (q->phase == 1)
        {
            if (p->hit == 1)
            {
                q->phase = 3;
                q->timer = 4;
                q->attack.active = false;
                return true;
            }
            const Sint32 forward = q->velocity_x < 0 ? -1 : 1;
            for (Uint32 i = 0; i < 8; ++i)
            {
                q->x += forward * 65536;
                SotnCollider col = {0};
                if (!SotnCollision_Query(&s->map, Pixel(q->x) + forward * 8, Pixel(q->y), &col))
                    return false;
                if (p->hit == 2 || (col.effects & SOTN_EFFECT_BLOCK))
                {
                    q->phase = 2;
                    q->timer = 64;
                    q->velocity_x = -q->velocity_x / 8;
                    q->velocity_y = -163840;
                    q->attack.active = false;
                    return Sound(s, 0x6a4);
                }
            }
            q->visible = true;
        }
        else if (q->phase == 2)
        {
            if (--q->timer == 0)
            {
                *p = (Projectile){0};
                return true;
            }
            q->x += q->velocity_x;
            q->y += q->velocity_y;
            q->velocity_y += 8192;
            q->angle = (Uint16)((q->angle + (q->facing_left ? 128 : -128)) & 4095);
        }
        else if (--q->timer == 0)
        {
            *p = (Projectile){0};
            return true;
        }
    }
    else if (q->id == 2)
    {
        q->angle = (Uint16)((q->angle + (q->phase == 1 ? (q->facing_left ? 128 : -128)
                                                       : (q->facing_left ? -192 : 192))) &
                            4095);
        if ((q->angle & 1023) == 0 && q->phase == 1 && !Sound(s, 0x60c))
            return false;
        q->velocity_y = SDL_min(8 * 65536, q->velocity_y + (q->phase == 1 ? 17408 : 9216));
        q->x += q->velocity_x;
        q->y += q->velocity_y;
        q->visible = true;
        if (Pixel(q->y) - camera_y > 256)
        {
            *p = (Projectile){0};
            return true;
        }
        if (p->hit == 2 && q->phase == 1)
        {
            q->phase = 2;
            q->velocity_x = -q->velocity_x / 2;
            q->velocity_y = -3 * 65536;
            q->attack.active = false;
        }
    }
    else if (q->id == 3)
    {
        if (q->phase == 1)
        {
            q->y += q->velocity_y;
            if (q->velocity_y < 4 * 65536)
                q->velocity_y += 14336;
            bool hit = false;
            if (!HolyCollision(s, p, &hit))
                return false;
            q->visible = true;
            if (hit)
            {
                q->phase = 2;
                q->timer = 16;
                q->visible = false;
                // The original bottle retains its small contact hitbox while its factory runs.
                if (!Sound(s, 0x69a))
                    return false;
            }
        }
        else if (q->phase == 2)
        {
            if ((q->timer & 3) == 0 && !Flame(s, *p))
                return false;
            if (--q->timer == 0)
            {
                q->phase = 3;
                q->timer = 4;
            }
        }
        else if (--q->timer == 0)
        {
            *p = (Projectile){0};
            return true;
        }
    }
    else if (q->id == 11)
    {
        const Sint32 angle = ((Sint32)q->timer - 16) * 64 + 0xc00;
        // Presentation-only flame waviness is separate from this original height/hit cadence.
        const Sint32 sine = (Sint32)SDL_round(SDL_sin(angle * (SDL_PI_D / 2048.0)) * 4096.0);
        const Sint32 height = (sine >= 0 ? sine / 256 : -((-sine + 255) / 256)) + p->height;
        q->attack.height = (Uint16)(height / 2);
        q->attack.offset_y = (Sint16)(-((height + 1) / 2));
        q->attack.active = (q->timer & 3) == 0;
        q->visual_y = height;
        if (--q->timer < 17)
        {
            *p = (Projectile){0};
            return true;
        }
    }
    else if (q->id == 6)
    {
        q->visible = true;
        switch (q->phase)
        {
        case 1:
            p->offset += 98304;
            if (p->offset >= 26 * 65536)
                q->phase = 2;
            break;
        case 2:
            p->offset -= 65536;
            if (p->offset <= 16 * 65536)
            {
                q->phase = 3;
                q->timer = 5;
                if (!Sound(s, 0x6a1))
                    return false;
            }
            break;
        case 3:
            if (++p->counter > 80)
            {
                p->counter = 0;
                if (!Sound(s, 0x6a1))
                    return false;
                if (q->timer == 0)
                    q->phase = 4;
                else
                    --q->timer;
            }
            break;
        case 4:
            q->x += (q->facing_left ? 6 : -6) * 65536;
            q->y -= p->offset;
            q->timer = 0;
            q->phase = 5;
            break;
        case 5:
            if (++q->timer >= 4)
            {
                q->phase = 6;
                if (!Sound(s, 0x6a4))
                    return false;
            }
            break;
        case 6:
            if (++q->timer > 14)
                q->phase = 7;
            break;
        case 7:
            s->frozen = false;
            *p = (Projectile){0};
            return true;
        default:
            return SDL_SetError("Invalid stopwatch phase");
        }
        if (q->phase < 5)
        {
            q->x += (player.x + (player.facing_left ? 8 : -8) * 65536 - q->x) / 12;
            q->y += (player.y - 16 * 65536 - q->y) / 4;
        }
        q->visual_y = q->phase < 5 ? -Pixel(p->offset) : 0;
    }
    p->hit = 0;
    if (q->id != 6 && q->id != 2)
    {
        const Sint32 x = Pixel(q->x) - camera_x, y = Pixel(q->y) - camera_y;
        if (x < -64 || x > 320 || y < -64 || y > 288)
            *p = (Projectile){0};
    }
    return true;
}
bool SotnSubweapons_Tick(SotnSubweapons *s, SotnPlayerState player, Sint32 camera_x,
                         Sint32 camera_y)
{
    if (s == NULL || camera_x < 0 || camera_x > 16384 || camera_y < 0 || camera_y > 16384)
        return SDL_SetError("Subweapon tick requires bounded room camera coordinates");
    ++s->ticks;
    const Uint32 generation = s->serial;
    for (Uint32 i = 0; i < SOTN_SUBWEAPON_SLOTS; ++i)
        if (s->slots[i].state.generation != 0 && s->slots[i].state.generation <= generation &&
            !TickOne(s, &s->slots[i], player, camera_x, camera_y))
            return false;
    return true;
}
bool SotnSubweapons_Get(const SotnSubweapons *s, Uint32 slot, SotnSubweaponState *out)
{
    if (s == NULL || out == NULL || slot >= SOTN_SUBWEAPON_SLOTS)
        return SDL_SetError("Invalid subweapon slot/output");
    *out = s->slots[slot].state;
    return true;
}
bool SotnSubweapons_Hit(SotnSubweapons *s, Uint32 slot, Uint16 flags)
{
    if (s == NULL || slot >= SOTN_SUBWEAPON_SLOTS || s->slots[slot].state.generation == 0 ||
        flags < 1 || flags > 3)
        return SDL_SetError("Invalid subweapon hit delivery");
    s->slots[slot].hit = flags;
    return true;
}
bool SotnSubweapons_Frozen(const SotnSubweapons *s)
{
    return s != NULL && s->frozen;
}
bool SotnSubweapons_Sounds(SotnSubweapons *s, Uint16 *sounds, size_t capacity, Uint16 *count)
{
    if (s == NULL || sounds == NULL || count == NULL || capacity < s->sound_count)
        return SDL_SetError("Subweapon sound output capacity is insufficient");
    SDL_memcpy(sounds, s->sounds, s->sound_count * sizeof(*sounds));
    *count = s->sound_count;
    s->sound_count = 0;
    return true;
}
void SotnSubweapons_Destroy(SotnSubweapons *s)
{
    SDL_free(s);
}
