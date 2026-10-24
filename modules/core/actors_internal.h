#ifndef SOTN_ACTORS_INTERNAL_H
#define SOTN_ACTORS_INTERNAL_H
#include <SDL3/SDL.h>
#include <sotn/actors.h>
#include <sotn/combat.h>
#include <sotn/dialogue.h>
#include <sotn/game_math.h>
#include <sotn/subweapon.h>
#include <sotn/world.h>

enum
{
    ACTOR_PRIMITIVE_CAPACITY = 512,
    ACTOR_PRIMITIVE_LIMIT = 193,
    ACTOR_SPRITE_PART_LIMIT = 32
};
typedef struct ActorPrimitive
{
    SotnActorPrimitive state;
    Sint32 vx, vy;
    Uint32 owner;
    Uint16 timer, phase;
    struct
    {
        Sint32 radius, speed;
        Sint16 x, y, z, scale;
    } reward;
} ActorPrimitive;
typedef struct Actor
{
    SotnActorState state;
    Sint32 vx, vy;
    Uint16 pose, remaining, timer, substep;
    size_t animation_stream;
    Uint8 parallax, cooldown[2 + SOTN_SUBWEAPON_SLOTS];
    Uint16 attack, attack_element, stun, hit_params, hit_flags, life_timer, level, experience;
    Uint32 projectile_generation[SOTN_SUBWEAPON_SLOTS];
    SotnCombatEnemy enemy;
    Sint32 fall_speed, gravity;
    Uint16 collectible;
    Uint16 slot, primitive[ACTOR_PRIMITIVE_LIMIT];
    struct
    {
        Uint16 attack_timer, attack_index, timer, counter;
        bool movement_facing;
        Sint32 initial_x;
        Sint16 angle_delta;
        Uint16 owner_slot;
        Uint32 owner_generation;
    } enemy_state;
    struct
    {
        Sint32 anchor_x;
        Sint16 attack_timer;
        Uint16 timer, decision;
        bool backward;
    } warg;
    struct
    {
        Uint16 palette, timer, timer2, rotation;
        bool underwater, ignore_collision;
    } merman;
    struct
    {
        Sint32 x, y;
        Uint16 timer, phase, scythe;
        bool direction;
    } death;
    struct
    {
        Sint32 base_y;
        Uint16 count, target;
        bool pressed, changed;
    } mechanism;
    struct
    {
        Sint32 target[4];
        Uint16 pending, timer, side;
        bool horizontal;
    } camera;
    struct
    {
        Uint16 maximum, timer, flag, attack_mode;
        Sint16 angle;
        Sint32 speed;
        bool near_death, pickup, ascending;
    } boss;
    struct
    {
        Uint16 groups, cadence;
        Sint32 flash;
    } reward;
    bool dynamic, remove, permanent;
} Actor;

struct SotnActors
{
    SotnDataView dra, nz0, executable;
    Uint16 stage_id, entrance_scimitars;
    SotnCollisionMap map;
    Uint8 *mutable_tiles;
    SotnWorldState fallback_world, *world;
    SotnCamera fallback_camera, *camera;
    SotnPlayerState player;
    SotnActorMotion motion;
    SotnActorControl control;
    Uint16 buttons, puzzle_flags, elevator_target;
    Sint16 puzzle_height[3];
    bool call_elevator, cannon_fired;
    Uint32 boss_flags, game_seconds;
    bool music_playing;
    struct
    {
        SotnDialogueState display;
        size_t script, events;
        Uint32 flags;
        Uint16 wait, speed, event_timer;
        bool active, events_active, voice_playing;
    } death_event;
    SotnRoomActors *spawns;
    Actor slots[SOTN_STAGE_ACTOR_SLOTS];
    ActorPrimitive primitives[ACTOR_PRIMITIVE_CAPACITY];
    Sint32 camera_x, camera_y;
    Uint32 random, combat_random, ticks, effect_serial;
    SotnActorCombat combat;
    bool entered;
    SotnCharacter *character;
    SotnSubweapons *subweapons;
};

static inline Uint16 Read16(const Uint8 *bytes)
{
    return (Uint16)(bytes[0] | (Uint16)bytes[1] << 8);
}

static inline Uint32 Read32(const Uint8 *bytes)
{
    return bytes[0] | (Uint32)bytes[1] << 8 | (Uint32)bytes[2] << 16 | (Uint32)bytes[3] << 24;
}

static inline Sint32 Signed16(const Uint8 *bytes)
{
    const Uint16 value = Read16(bytes);
    return value < 32768 ? value : (Sint32)value - 65536;
}

static inline Sint32 Pixel(Sint32 value)
{
    return value >= 0 ? value / 65536 : (Sint32)(-((-(Sint64)value + 65535) / 65536));
}

static inline Sint32 Wrap16(Sint64 value)
{
    const Uint16 bits = (Uint16)(Uint64)value;
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}

static inline void ShiftY(Actor *actor, Sint64 delta)
{
    const Sint32 high = Wrap16((Sint64)Pixel(actor->state.y) + delta);
    actor->state.y = (Sint32)((Sint64)high * 65536 + ((Uint32)actor->state.y & 65535));
}

static inline bool Slice(SotnDataView image, size_t offset, size_t size, SotnDataView *out)
{
    if (image.bytes == NULL || offset > image.size || size > image.size - offset)
        return SDL_SetError("Laboratory actor table is outside its private image");
    *out = (SotnDataView){image.bytes + offset, size};
    return true;
}

static inline void SetStep(Actor *actor, Uint16 step)
{
    actor->state.step = step;
    actor->substep = actor->pose = actor->remaining = 0;
}

static inline Uint8 Side(const Actor *actor, SotnPlayerState player)
{
    return (Uint8)(Pixel(actor->state.x) > Pixel(player.x));
}

static inline Sint64 Distance(const Actor *actor, SotnPlayerState player)
{
    const Sint64 difference = (Sint64)Pixel(actor->state.x) - Pixel(player.x);
    return difference < 0 ? -difference : difference;
}

/* Shared bounded services for reconstructed stage behaviors. All byte views are borrowed;
 * spawned actors are owned by the service and related through slot/generation, never pointers. */
bool ActorInitialize(SotnActors *actors, Actor *actor, size_t offset);
bool ActorAnimate(SotnActors *actors, Actor *actor, size_t offset, Uint8 *result);
bool ActorQuery(SotnActors *actors, Sint32 x, Sint32 y, SotnCollider *out);
bool ActorSound(SotnActors *actors, Uint16 id);
bool ActorExplosion(SotnActors *actors, const Actor *source, Uint16 kind);
bool ActorGround(SotnActors *actors, Actor *actor, size_t sensors, Uint16 *result);
bool ActorWalk(SotnActors *actors, Actor *actor, size_t sensors, Uint16 *result);
bool ActorField(SotnActors *actors, Actor *actor, size_t sensors, Uint16 count, bool resolve,
                Uint16 *result);
bool ActorSpawn(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters, Uint32 begin,
                Uint32 end, Actor **out);
bool ActorSpawnAt(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters,
                  Uint32 slot, Actor **out);
bool ActorSpittle(SotnActors *actors, Actor *actor, SotnPlayerState player);
bool ActorCrawl(SotnActors *actors, Actor *actor, Uint8 state, Uint8 *out);
bool ActorPrimitives(SotnActors *actors, Actor *actor, Uint16 count);
ActorPrimitive *ActorPrimitiveGet(SotnActors *actors, const Actor *actor, Uint16 index);
void ActorPrimitivesClear(SotnActors *actors, const Actor *actor);
bool ActorStageParts(SotnActors *actors, const Actor *actor, SotnDataView *out);
bool ActorKnight(SotnActors *actors, Actor *actor, SotnPlayerState player);
bool ActorSlogra(SotnActors *actors, Actor *actor);
bool ActorGaibon(SotnActors *actors, Actor *actor);
bool ActorBossReward(SotnActors *actors, Actor *actor);
bool ActorBossPuff(SotnActors *actors, Actor *actor);
bool ActorBossEvent(SotnActors *actors, Actor *actor);
bool ActorBossBox(SotnActors *actors, Actor *actor, size_t boxes, Uint32 count, size_t lookup,
                  Uint32 frames);
bool ActorCamera(SotnActors *actors, Actor *actor);
bool ActorDoor(SotnActors *actors, Actor *actor);
bool ActorCannon(SotnActors *actors, Actor *actor);
bool ActorMechanism(SotnActors *actors, Actor *actor);
bool ActorObjectCollision(SotnActors *actors, const Actor *actor, Uint16 width, Uint16 height,
                          Uint16 flags, Uint16 *result);
bool ActorDisplace(SotnActors *actors, Sint32 dx, Sint32 dy, Uint32 contacts);
bool ActorTile(SotnActors *actors, Uint32 index, Uint16 tile);
bool ActorTilePatch(SotnActors *actors, Uint32 index, Uint32 rows, Uint32 columns, Uint32 stride,
                    size_t offset);
bool ActorSprite(SotnActors *actors, Actor *actor, Uint16 index, Sint32 x, Sint32 y, Uint8 u,
                 Uint8 v, Uint16 width, Uint16 height, Uint16 page, Uint16 palette);
bool ActorSkeleton(SotnActors *actors, Actor *actor, SotnPlayerState player);
bool ActorEntrance(SotnActors *actors, Actor *actor);
bool ActorWarg(SotnActors *actors, Actor *actor);
bool ActorMerman(SotnActors *actors, Actor *actor);
bool ActorDeath(SotnActors *actors, Actor *actor);

/* These services accept only validated Laboratory (12) or first-visit Entrance (65)
 * profiles. Offsets identify private tables; actor IDs always retain their stage meaning. */
static inline size_t ActorTable(const SotnActors *actors, size_t laboratory, size_t entrance)
{
    return actors->stage_id == 65 ? entrance : laboratory;
}

static inline Uint32 ActorRandom(SotnActors *actors)
{
    actors->random = actors->random * 0x01010101U + 1U;
    return actors->random >> 24;
}
static inline Sint32 Signed32(const Uint8 *p)
{
    const Uint32 bits = Read32(p);
    return bits <= SDL_MAX_SINT32 ? (Sint32)bits : (Sint32)((Sint64)bits - 0x100000000LL);
}
static inline Sint32 ActorSin(Sint32 angle)
{
    return (Sint32)SDL_round(SDL_sin(((Uint32)angle & 4095) * (SDL_PI_D / 2048.0)) * 4096.0);
}
static inline Sint32 ActorShift(Sint64 value, Uint32 shift)
{
    const Sint64 divisor = (Sint64)1 << shift;
    return (Sint32)(value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor));
}
#endif
