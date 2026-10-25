#ifndef SOTN_ACTORS_INTERNAL_H
#define SOTN_ACTORS_INTERNAL_H
#include <SDL3/SDL.h>
#include <sotn/actors.h>
#include <sotn/combat.h>
#include <sotn/dialogue.h>
#include <sotn/dopple_ai.h>
#include <sotn/dopple_body.h>
#include <sotn/game_math.h>
#include <sotn/relic.h>
#include <sotn/shop.h>
#include <sotn/stage_profile.h>
#include <sotn/subweapon.h>
#include <sotn/world.h>

enum
{
    ACTOR_PRIMITIVE_CAPACITY = SOTN_ACTOR_PRIMITIVES_MAX,
    ACTOR_PRIMITIVE_LIMIT = SOTN_ACTOR_PRIMITIVES_PER_ACTOR_MAX,
    ACTOR_SPRITE_PART_LIMIT = 32,
    ACTOR_FOREIGN_ATTACK_GROUPS = 4,
    ACTOR_CHAPEL_BALL_LINKS = 15
};
typedef struct ActorPrimitive
{
    SotnActorPrimitive state;
    Sint32 vx, vy;
    Sint16 corner_angle;
    Uint32 owner;
    Uint16 timer, phase;
    struct
    {
        Uint32 generation;
        Uint8 branches;
        bool used, smoke, damage;
    } armor_wave;
    struct
    {
        Sint32 radius, speed;
        Sint16 x, y, z, scale;
    } reward;
    struct
    {
        Sint16 angle, width, height;
        Sint32 tail_y;
        Uint8 brightness, pose, remaining;
        bool ended;
    } blood;
    struct
    {
        Uint16 fraction_x, fraction_y;
        Sint16 angle, turn;
        Uint8 phase, wait, hops, color_index;
        bool used, side;
    } demon;
} ActorPrimitive;
typedef struct Actor
{
    SotnActorState state;
    Sint32 vx, vy;
    struct
    {
        Uint32 x[3], y[3]; /**< Original wrapped 16.16 cloud phases. */
    } clouds;
    Uint16 pose, remaining, timer, substep;
    size_t animation_stream;
    Uint8 parallax, cooldown[2 + SOTN_SUBWEAPON_SLOTS + ACTOR_FOREIGN_ATTACK_GROUPS], damage_effect;
    Uint16 attack, attack_element, stun, hit_params, hit_flags, life_timer, level, experience;
    Uint32 projectile_generation[SOTN_SUBWEAPON_SLOTS];
    SotnCombatEnemy enemy;
    Sint32 fall_speed, gravity;
    Uint16 collectible;
    Uint16 slot, primitive[ACTOR_PRIMITIVE_LIMIT];
    struct
    {
        Uint16 attack_timer, attack_index, timer, counter, counter2;
        bool flag;
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
        Uint32 child_generation;
    } chapel;
    struct
    {
        Sint32 reference_y;
        Uint16 hover_timer;
    } crow;
    struct
    {
        Uint16 timer, interval, lunge_timer;
        bool movement_facing;
    } halberd;
    struct
    {
        Uint16 timer;
        bool previous_left, movement_left;
    } corner;
    struct
    {
        Uint16 timer, interval;
        bool bottom_dead, bounced;
    } pillar;
    struct
    {
        Uint16 attack_step, frames, attack_timer, cycle_timer, trail, base_attack;
        Sint16 turn, sine_phase;
        Sint32 reference_y;
        Uint8 brightness, pattern, random, next_step;
        bool attacking;
    } hunt;
    struct
    {
        Sint32 target_x, target_y;
        Sint16 radius, spin, spin_target;
        Uint16 timer, moves, count;
        bool has_weapons;
    } spectral;
    struct
    {
        Sint32 distance, velocity, target;
        Uint32 phase, ring_timer;
    } bell;
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
        Uint16 angle, timer;
        bool spawned, thrown;
    } table;
    struct
    {
        Sint32 phase, base, amplitude;
        Uint16 timer, seed_timer, recoil, sway, radius;
        Uint16 spawn_id, total, per_tick, delay, interval, spawned, mode, pool, parameter;
        bool hit, increment, drop_if_full;
    } rose;
    struct
    {
        Sint32 acceleration, previous_x, previous_y, previous_vx, previous_vy;
        Sint16 angle;
        Uint16 index, phase, cycles, reaction, hits, last_hit, active_foot, walk_count, tip;
    } diplo;
    struct
    {
        Uint8 timer, delay, fade, phase, brightness, u;
        Uint16 scale, palette;
        bool smashed, pulse;
    } sword;
    struct
    {
        Sint16 delay;
        Uint16 size;
        Uint8 ticks, brightness;
        bool armored, backward;
    } blue;
    struct
    {
        Uint16 timer, width, height, pair;
        Uint32 pair_generation;
        Uint8 brightness, phase, depth;
        Sint16 rotation, spread;
        bool movement_facing, did_shield;
    } armor;
    struct
    {
        Sint32 velocity, force;
        Sint16 angle, delay, damping, depth;
    } shelf;
    struct
    {
        Sint16 angle;
    } stair;
    struct
    {
        Uint8 current, target, wait, side;
        Sint16 sway;
        Sint16 upper[2], lower[2];
        bool called, moving, sounded;
    } outer_lift;
    struct
    {
        Uint8 timer, patrol, index, side;
        bool movement_facing, alternate;
    } spear;
    struct
    {
        Sint32 x, y;
    } archer;
    struct
    {
        Uint16 barrel_slot, counter, bounces;
        Uint32 barrel_generation;
        Sint32 speed;
    } ape;
    struct
    {
        Sint32 origin, line_start, line_end, line_y;
        Uint16 neighbor;
        Uint32 generation;
        bool signal, repeat;
    } musket;
    struct
    {
        Sint32 hand;
        Uint16 timer, bell, bell_wait, state, previous;
        bool stopped;
    } clock;
    struct
    {
        Uint16 groups, cadence;
        Sint32 flash;
    } reward;
    struct
    {
        Uint16 blueprint, params, amount, per_cycle, kind, origin, delay, cycle, spawned;
        bool noncritical, increment;
    } dopple_factory;
    struct
    {
        Sint16 brightness, sway;
        Uint16 angle, speed, wave, hold;
    } blink;
    struct
    {
        Sint32 anchor_x, anchor_y, offset, speed;
        Uint32 phases[16];
        Sint16 angles[8];
        Uint16 timer, phase, growth;
        bool triggered;
    } arena;
    struct
    {
        Uint32 partner_generation;
        Uint16 timer, hover;
        Sint32 camera_x, camera_y;
        bool separated, hovering;
    } flying;
    struct
    {
        Uint16 timer, beam_index, turn_ticks;
        Sint16 angle, delta;
        bool charged, direction;
    } dhuron;
    struct
    {
        Uint16 timer, bob_timer, scale;
        Sint16 bob_angle, leaves_x, leaves_y, stalk_x, stalk_y;
        Uint32 child_generation;
        bool spawned, leaves_grown, stalk_grown;
    } plant;
    struct
    {
        Sint32 radius_x, radius_y;
        Sint16 phase_x, phase_y, excited, counter, squash, speed, limit, scale;
    } ecto;
    struct
    {
        Sint16 angle[3], delta[3], opening, depth, direction, center_x, center_y;
        Sint32 magnitude;
        Uint16 timer, wait, priority;
        Uint32 child_generation;
        bool command;
    } book;
    struct
    {
        Uint16 timer, saved_hp, child_slot;
        Uint32 child_generation;
        Uint8 effect_step, effect_timer;
        Sint16 width, height, scale, angle;
        Uint8 brightness;
        bool attack_active;
    } demon;
    struct
    {
        Uint16 palette, timer;
    } summon;
    struct
    {
        Uint16 counter;
    } mudman;
    struct
    {
        Uint16 timer, total_hits, consecutive_hits;
    } librarian;
    bool dynamic, remove, permanent;
} Actor;

struct SotnActors
{
    SotnDataView dra, nz0, executable;
    Uint16 stage_id, entrance_scimitars;
    SotnCollisionMap map, background;
    SotnDataView tile_pages, tile_graphics, tile_palettes;
    Uint8 *mutable_tiles, *background_tiles;
    SotnWorldState fallback_world, *world;
    SotnCamera fallback_camera, *camera;
    SotnPlayerState player;
    SotnRoomWindow window;
    Sint32 frame_start_x;
    bool player_initialized;
    SotnActorMotion motion;
    SotnPaletteBlend palette_blend;
    SotnActorControl control;
    Uint16 buttons, previous_buttons, tapped_buttons, repeat_buttons, puzzle_flags, elevator_target;
    Uint8 button_repeat_timer[16];
    Sint16 puzzle_height[3];
    bool call_elevator, cannon_fired, armor_lord_killed;
    Uint32 boss_flags, game_seconds;
    Uint8 game_frame;
    Uint32 service_ticks; /**< Full frame phase with original unsigned 32-bit wrap. */
    bool clock_statues[2];
    bool music_playing;
    struct
    {
        SotnDialogueState display;
        size_t script, events;
        Uint32 flags;
        Uint16 wait, speed, event_timer;
        bool active, events_active, voice_playing, cd_busy, library_met;
        Uint8 library_reply;
    } death_event;
    SotnRoomActors *spawns;
    Actor slots[SOTN_STAGE_ACTOR_SLOTS];
    struct
    {
        Uint32 generation;
        Uint16 slot;
        Uint8 group;
    } chapel_ball[ACTOR_CHAPEL_BALL_LINKS];
    ActorPrimitive primitives[ACTOR_PRIMITIVE_CAPACITY];
    Sint32 camera_x, camera_y;
    Uint32 random, combat_random, ticks, effect_serial;
    SotnActorCombat combat;
    bool entered;
    struct
    {
        SotnDoppleBody body;
        SotnDoppleAI ai;
        bool active, blink_lock;
        Sint32 anchor_x, anchor_y;
        Uint32 blink_angles[16];
        Sint32 debris_x[32], debris_y[32];
        Sint16 previous_hp;
        Uint16 hitbox_state, shake;
        Uint8 death_profile[4];
    } dopple;
    /** Original LIB transient minion signal; not ordinary save state. */
    struct
    {
        SotnShopCatalogue catalogue;
        Uint16 quantities[SOTN_SHOP_STOCK], options[6], count, cursor, item_cursor, scroll;
        Uint16 repeated;
        bool sell;
    } shop;
    Uint32 library_enemy_flags;
    bool library_arena_active, library_camera_unlocked;
    Uint16 backdrop_color;
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
/** @brief Queue original screen-distance attenuation/pan without owning voices.
 * @param actors Borrowed stage/audio-request owner. @param actor Borrowed sound source.
 * @param sound Original sound identifier. @return Success, including inaudible sources,
 * or false with an SDL error for invalid owners or a full request queue. */
bool ActorPositionalSound(SotnActors *actors, const Actor *actor, Uint16 sound);
/** @brief Advance selected original Chapel actors using borrowed private stage tables.
 * @param actors Borrowed stage-6 owner. @param actor Borrowed owned actor slot.
 * @return Success, or false with an SDL error for invalid state/data. Unsupported
 * actor kinds are explicitly marked; this does not grant connected-stage support. */
bool ActorChapel(SotnActors *actors, Actor *actor);
/** @brief Advance an owned Chapel lamp/debris slot from private profiles.
 * @param actors Borrowed Chapel/world owner. @param actor Borrowed lamp or debris.
 * @return Success or SDL error for invalid owners, state or private tables. */
bool ActorChapelBreakable(SotnActors *actors, Actor *actor);
/** @brief Advance selected Chapel shortcut/statue or gargoyle terrain actors.
 * @param actors Borrowed Chapel/world/room owner. @param actor Borrowed supported slot.
 * @return Success or SDL error for invalid state/data, lost children or occupied reservations. */
bool ActorChapelMechanism(SotnActors *actors, Actor *actor);
/** @brief Advance selected Chapel wall geometry or screen-space staircase shadow.
 * @param actors Borrowed Chapel owner. @param actor Borrowed scenery slot.
 * @return Success or SDL error; exhausted optional primitives retire the actor. */
bool ActorChapelScenery(SotnActors *actors, Actor *actor);
/** @brief Advance original Chapel Crow/Raven flight, frame hitboxes and deaths.
 * @param actors Borrowed stage-6 owner. @param actor Borrowed Crow/Raven slot.
 * @return Success or SDL error for invalid identity/state/private profile or motion. */
bool ActorChapelCrow(SotnActors *actors, Actor *actor);
/** @brief Advance selected original Chapel Bone Halberd body, weapon or debris.
 * @param actors Borrowed Chapel owner. @param actor Borrowed supported slot.
 * @return Success or SDL error for invalid state/data/ownership or bounded motion. */
bool ActorChapelHalberd(SotnActors *actors, Actor *actor);
/** @brief Advance selected original Chapel stair guard, linked weapon and death pieces.
 * @param actors Borrowed stage-6 owner. @param actor Borrowed supported slot.
 * @return Success or SDL error for invalid private data, state, ownership or motion. */
bool ActorChapelCornerGuard(SotnActors *actors, Actor *actor);
/** @brief Advance Chapel stacked skulls, original fire breath and delayed death pieces.
 * @param actors Borrowed stage owner. @param actor Borrowed supported slot.
 * @return Success or SDL error; private images and room collision remain borrowed. */
bool ActorChapelPillar(SotnActors *actors, Actor *actor);
/** @brief Advance original Chapel Hunting Girl body or owned blade child.
 * @param actors Borrowed entered Chapel owner. @param actor Borrowed active slot.
 * @return Success or SDL error; private profiles remain borrowed and children owned by generation.
 */
bool ActorChapelHunting(SotnActors *actors, Actor *actor);
/** @brief Advance Chapel Spectral Sword, its owned aura or one of sixteen weapons.
 * @param actors Borrowed stage-6 owner. @param actor Borrowed active supported slot.
 * @return Success or SDL error for invalid private data, ownership, state or motion. */
bool ActorChapelSpectral(SotnActors *actors, Actor *actor);
/** @brief Advance original Chapel spike tiles, owned damage or independent debris/dust.
 * @param actors Borrowed stage-6 owner with mutable room tiles and optional character.
 * @param actor Borrowed supported slot; original tables remain borrowed.
 * @return Success or SDL error for invalid data, ownership, state or motion. */
bool ActorChapelSpikes(SotnActors *actors, Actor *actor);
/** @brief Advance an original Chapel tower-exit threshold and transient room window.
 * @param actors Borrowed entered stage-6 owner and private initialization records.
 * @param actor Borrowed raw-0x16 slot; physical player/map coordinates remain stable.
 * @return Success or SDL error for invalid owner, variant, state or bounded window. */
bool ActorChapelExit(SotnActors *actors, Actor *actor);
/** @brief Advance original Chapel cloud planes or initialize the repeating sky-land strip.
 * @param actors Borrowed stage-6 owner and private models, profiles and UV/grid records.
 * @param actor Borrowed raw-0x1f/0x20 slot; primitives remain generation-owned.
 * @return Success or SDL error for malformed data/state or a nonterminating cloud row. */
bool ActorChapelSky(SotnActors *actors, Actor *actor);
/** @brief Advance the original Chapel stained-glass/light or background actor.
 * @param actors Borrowed stage-6 owner, private geometry/poses/colors and math tables.
 * @param actor Borrowed raw-0x17/0x18 slot; bounded primitives remain generation-owned.
 * @return Success or SDL error for invalid private records, state or primitive capacity. */
bool ActorChapelGlass(SotnActors *actors, Actor *actor);
/** @brief Advance original Chapel bell swing/geometry/interaction or its two-bell spawner.
 * @param actors Borrowed stage-6 simulation and private tables.
 * @param actor Borrowed supported slot; primitives stay generation-owned.
 * @return Success or SDL error for invalid data, state, motion or child reservations. */
bool ActorChapelBell(SotnActors *actors, Actor *actor);
/** @brief Borrowed Hunting Girl graphics/motion helpers; require its 32 owned primitives.
 * @note Tips use whole room coordinates; spirit/attack completion is copied to done.
 * Profiles/streams stay private, and invalid bounds/ownership return SDL errors. */
bool ActorHuntingTip(SotnActors *actors, const Actor *actor, Sint32 *x, Sint32 *y);
bool ActorHuntingSpirit(SotnActors *actors, Actor *actor, Uint16 profile, Uint16 phase, bool *done);
bool ActorHuntingDrawAttack(SotnActors *actors, Actor *actor, size_t pattern, bool *done);
bool ActorHuntingTrails(SotnActors *actors, Actor *actor);
/** @brief Advance original Chapel spike-ball release, collision and rendering.
 * @param actors Borrowed stage-6 owner. @param actor Borrowed owned ball slot.
 * @return Success or SDL error; exhausted optional primitives retire the ball. */
bool ActorChapelSpikeBall(SotnActors *actors, Actor *actor);
/** @brief Copy one live linked player-side attack and its original cooldown group.
 * @param actors Borrowed stage owner; retires stale links. @param index Link 0..14.
 * @param out Borrowed player snapshot, attack/position overwritten for active links.
 * @param group Receives original group 0..3 when active. @param active Receives availability.
 * @return Success or SDL error; does not publish or own any private data. */
bool ActorChapelBallAttack(SotnActors *actors, Uint16 index, SotnPlayerState *out, Uint32 *group,
                           bool *active);
/** @brief Advance the selected non-inverted Bat using its own stage tables.
 * @param actors Borrowed Entrance or Chapel owner. @param actor Borrowed owned slot.
 * @return Success, or false with an SDL error for invalid state/private data. */
bool ActorBat(SotnActors *actors, Actor *actor);
bool ActorSoundVolumePan(SotnActors *actors, Uint16 id, Uint16 volume, Sint8 pan);
bool ActorExplosion(SotnActors *actors, const Actor *source, Uint16 kind);
bool ActorExplosionInPool(SotnActors *actors, const Actor *source, Uint16 kind, Uint32 begin,
                          Uint32 end, Uint16 priority);
bool ActorGround(SotnActors *actors, Actor *actor, size_t sensors, Uint16 *result);
bool ActorWalk(SotnActors *actors, Actor *actor, size_t sensors, Uint16 *result);
bool ActorField(SotnActors *actors, Actor *actor, size_t sensors, Uint16 count, bool resolve,
                Uint16 *result);
bool ActorSpawn(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters, Uint32 begin,
                Uint32 end, Actor **out);
bool ActorSpawnAt(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters,
                  Uint32 slot, Actor **out);
/** @brief Recreate a fixed child, replacing only a matching nonplaced owned survivor.
 * @param actors Borrowed stage owner. @param source Active parent with nonzero generation.
 * @param id Original child identity. @param parameters Original child parameters.
 * @param slot Bounded destination distinct from the parent. @param out Receives borrowed child.
 * @return Success or SDL error, protecting original placements and unrelated owners. */
bool ActorOwnedSpawnAt(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters,
                       Uint32 slot, Actor **out);
bool ActorSpittle(SotnActors *actors, Actor *actor, SotnPlayerState player);
bool ActorCrawl(SotnActors *actors, Actor *actor, Uint8 state, Uint8 *out);
bool ActorPrimitives(SotnActors *actors, Actor *actor, Uint16 count);
/** @brief Original func_800EDB58 consecutive allocation; no partial pool ownership on failure. */
bool ActorPrimitivesContiguous(SotnActors *actors, Actor *actor, Uint16 count);
ActorPrimitive *ActorPrimitiveGet(SotnActors *actors, const Actor *actor, Uint16 index);
void ActorPrimitivesClear(SotnActors *actors, const Actor *actor);
bool ActorStageParts(SotnActors *actors, const Actor *actor, SotnDataView *out);
bool ActorKnight(SotnActors *actors, Actor *actor, SotnPlayerState player);
/** Advance shared green/blue sprite-piece death physics. Reports the original pool fallback. */
bool ActorKnightShatter(SotnActors *actors, Actor *actor, bool *done);
bool ActorIntenseExplosion(SotnActors *actors, Actor *actor);
bool ActorEntranceStairway(SotnActors *actors, Actor *actor);
bool ActorEntranceRock(SotnActors *actors, Actor *actor);
bool ActorOuterElevator(SotnActors *actors, Actor *actor);
bool ActorOuterLiftDoors(SotnActors *actors, Actor *actor);
bool ActorOuterLiftSwitch(SotnActors *actors, Actor *actor);
bool ActorOuterLiftRope(SotnActors *actors, Actor *actor);
bool ActorBlueKnight(SotnActors *actors, Actor *actor);
bool ActorBlueDeath(SotnActors *actors, Actor *actor, bool *done);
bool ActorSkeletonApe(SotnActors *actors, Actor *actor);
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
bool ActorBloodyZombie(SotnActors *actors, Actor *actor, SotnPlayerState player);
bool ActorMerman(SotnActors *actors, Actor *actor);
bool ActorDeath(SotnActors *actors, Actor *actor);
bool ActorDialogue(SotnActors *actors, Actor *actor);
bool ActorMaria(SotnActors *actors, Actor *actor);

/* These services accept only validated Laboratory (12) or first-visit Entrance (65)
 * or revisited Entrance (7) profiles. Offsets identify private tables; actor IDs always retain
 * their stage meaning. */
size_t ActorStageOffset(const SotnActors *actors, size_t laboratory);
Uint16 ActorStageId(const SotnActors *actors, Uint16 laboratory);
bool ActorDoppleEntry(SotnActors *actors, Actor *actor);
bool ActorDoppleTrigger(SotnActors *actors, Uint32 slot);
bool ActorDoppleLight(SotnActors *actors, Actor *actor);
bool ActorDoppleShock(SotnActors *actors, Actor *actor);
bool ActorDoppleEncounter(SotnActors *actors, Actor *actor);
bool ActorDoppleHitEffect(SotnActors *actors, Uint16 element, Uint8 *out);
bool ActorDoppleSmoke(SotnActors *actors, Actor *actor);
bool ActorDoppleParticles(SotnActors *actors, Actor *actor);
bool ActorDoppleTrail(SotnActors *actors, Actor *actor);
bool ActorDoppleWingHitbox(SotnActors *actors, Actor *actor);
bool ActorDoppleFactoryRequest(SotnActors *actors, const Actor *source,
                               const SotnDoppleRequest *request);
bool ActorDoppleFactory(SotnActors *actors, Actor *actor);
bool ActorDoppleUnarmed(SotnActors *actors, Actor *actor);
bool ActorDoppleKnife(SotnActors *actors, Actor *actor);
bool ActorDoppleBlink(SotnActors *actors, Actor *actor);
bool ActorMedusa(SotnActors *actors, Actor *actor);
bool ActorArmorLord(SotnActors *actors, Actor *actor);
bool ActorArmorWave(SotnActors *actors, Actor *actor);
bool ActorArmorDeath(SotnActors *actors, Actor *actor);
bool ActorSwordLord(SotnActors *actors, Actor *actor);
bool ActorOuterEffect(SotnActors *actors, Actor *actor);
bool ActorOuterDust(SotnActors *actors, const Actor *actor, Uint16 count, Uint16 kind, Sint16 x,
                    Sint16 y, Uint16 index, Sint16 gap);
bool ActorSpearGuard(SotnActors *actors, Actor *actor);
bool ActorArcher(SotnActors *actors, Actor *actor);
bool ActorMusket(SotnActors *actors, Actor *actor);
bool ActorClock(SotnActors *actors, Actor *actor);
/** @brief Replace only a matching owned LIB child on parent respawn, as the original factory does.
 * @note Rejects unrelated occupied slots and original placements; clears old primitive ownership.
 */
bool ActorLibrarySpawnAt(SotnActors *actors, const Actor *source, Uint16 id, Uint16 parameters,
                         Uint32 slot, Actor **out);
/** @brief Advance original Library minion fade; zero sensors skips grounding.
 * @param actors Borrowed entered Library owner. @param actor Borrowed summoned actor.
 * @param sensors Optional private sixteen-byte ground sensors offset.
 * @return Success or SDL error; no ownership transfers. */
bool ActorLibrarySummon(SotnActors *actors, Actor *actor, size_t sensors);
/** @brief Advance Library Mudman, attack child or independently falling death fragment. */
bool ActorLibraryMudman(SotnActors *actors, Actor *actor);
/** @brief Advance selected original spit/column; owners/data must be entered Library state. */
bool ActorLibraryDemonSpit(SotnActors *actors, Actor *actor);
/** @brief Advance original bolt and bounded sixty-packet branching line trail. */
bool ActorLibraryDemonBolt(SotnActors *actors, Actor *actor);
/** @brief Project private Library book geometry and copy its hitbox anchor. */
bool ActorLibraryBookGeometry(SotnActors *actors, Actor *actor);
/** @brief Initialize the twelve owned book faces from private UV pointers. */
bool ActorLibraryBookFaces(SotnActors *actors, Actor *actor);
/** @brief Advance bounded fixed-point book/debris motion, failing before overflow. */
bool ActorLibraryBookMove(Actor *actor);
/** @brief Advance original Library book body, shared beam or page debris. */
bool ActorLibraryBook(SotnActors *actors, Actor *actor);
/** @brief Advance selected owned beam or independent page fragment. */
bool ActorLibraryBookBeam(SotnActors *actors, Actor *actor);
/** @brief Advance the original rotating Library secret bookcase and saved discovery. */
bool ActorLibraryShelf(SotnActors *actors, Actor *actor);
/** @brief Copy projected, lit and recursively subdivided original shelf faces. */
bool ActorLibraryShelfGeometry(SotnActors *actors, Actor *actor);
/** @brief Advance the original Library arena manager or owned sealing books.
 * @param actors Borrowed entered Library owner with writable progress.
 * @param actor Borrowed manager or sealing-book state.
 * @return Success or SDL error; optional exhausted reward allocation waits for another tick.
 * Native factories retain ownership; no pointers transfer to the caller. */
bool ActorLibraryEvent(SotnActors *actors, Actor *actor);
/** @brief Project and present the selected private Library parallax actors. */
bool ActorLibraryBackground(SotnActors *actors, Actor *actor);
/** @brief Decode private Library/Chapel foreground/background decoration records.
 * @param actors Borrowed stage owner. @param actor Borrowed supported decoration.
 * @return Success or SDL error for invalid identity/index/data; retains borrowed lifetimes. */
bool ActorRoomDecoration(SotnActors *actors, Actor *actor);
/** @brief Advance original per-button repeat timers after tapped/pressed input is installed. */
void ActorRepeatButtons(SotnActors *actors);
/** @brief Advance the selected original Library root/buy/sell menu actors. */
bool ActorLibraryShop(SotnActors *actors, Actor *actor);
/** @brief Build owned screen-space shop panels/text from private data. */
bool ActorLibraryShopGraphics(SotnActors *actors, Actor *actor);
/** @brief Advance selected original Librarian trigger, control, chair or dialogue.
 * @param actors Borrowed initialized Library owner with writable progress and frame services.
 * @param actor Borrowed supported NPC actor. @return Success or SDL error.
 * Factories retain ownership; optional exhausted gift allocation preserves its unclaimed flag.
 * Normal Alucard is supported; transformed/high-jump player adapters remain separate work. */
bool ActorLibraryLibrarian(SotnActors *actors, Actor *actor);
/** @brief Queue the original positional LIB sound; owners remain borrowed. */
bool ActorLibrarySound(SotnActors *actors, const Actor *actor, Uint16 sound);
/** @brief Query a bounded private sensor list; write out only on success. */
bool ActorLibraryBlocked(SotnActors *actors, const Actor *actor, size_t offset, bool facing,
                         bool *out);
/** @brief Advance the generation-owned original Dhuron charging effect. */
bool ActorLibraryDhuronCharge(SotnActors *actors, Actor *actor);
/** @brief Advance original LIB Ectoplasm, its owned emitter and independent afterimages. */
bool ActorLibraryEctoplasm(SotnActors *actors, Actor *actor);
/** @brief Read the original private LIB plant pose hitbox into a borrowed actor. */
bool ActorLibraryPlantBox(SotnActors *actors, Actor *actor);
/** @brief Advance original growing LIB Corpseweed and its generation-owned root link. */
bool ActorLibraryCorpseweed(SotnActors *actors, Actor *actor);
/** @brief Advance original LIB plant projectile, collision and splat effect. */
bool ActorLibraryPlantProjectile(SotnActors *actors, Actor *actor);
/** @brief Dispatch original LIB Thornweed/Corpseweed/projectile identities. */
bool ActorLibraryPlant(SotnActors *actors, Actor *actor);
/** @brief Advance original placed LIB Dhuron, attack, charge and fragment actors. */
bool ActorLibraryDhuron(SotnActors *actors, Actor *actor);
/** @brief Advance original placed LIB Flea Armor and its generation-owned attack child. */
bool ActorLibraryFleaArmor(SotnActors *actors, Actor *actor);
/** @brief Advance the selected original LIB paired body and generation-owned torso. */
bool ActorLibraryFlying(SotnActors *actors, Actor *actor);
/** @brief Advance original LIB blood children and their owned native primitives. */
bool ActorLibraryBlood(SotnActors *actors, Actor *actor);
/** @brief Advance one selected LIB breakable/helper using borrowed initialized owners. */
bool ActorLibraryBreakable(SotnActors *actors, Actor *actor);
/** @brief Dispatch selected LIB actors; unsupported families remain explicitly marked. */
bool ActorLibrary(SotnActors *actors, Actor *actor);
bool ActorEarlyCastle(SotnActors *actors, Actor *actor);
bool ActorFlea(SotnActors *actors, Actor *actor);
Actor *ActorDiploOwner(SotnActors *actors, const Actor *actor);
bool ActorDiplo(SotnActors *actors, Actor *actor);
bool ActorDiploTail(SotnActors *actors, Actor *actor, Actor *root);
bool ActorDiploFireball(SotnActors *actors, Actor *actor);
/** @brief Advance shared Marble/Chapel Skelerang bodies and owned children.
 * @param actors Borrowed stage owner. @param actor Borrowed supported original slot.
 * @return Success or SDL error for invalid identities/tables/state or conflicting children.
 * Private animation/sensor/math views remain borrowed for the owner's lifetime. */
bool ActorSkelerang(SotnActors *actors, Actor *actor);
bool ActorOuija(SotnActors *actors, Actor *actor);
bool ActorRose(SotnActors *actors, Actor *actor);
bool ActorRoseSeed(SotnActors *actors, Actor *actor);
bool ActorRoseSpawn(SotnActors *actors, const Actor *actor, Uint16 parameter);
bool ActorWideAnimation(SotnActors *actors, Actor *actor, size_t offset, bool *ended);
bool ActorSlinger(SotnActors *actors, Actor *actor);
bool ActorReplaceExplosion(SotnActors *actors, Actor *actor, Uint16 kind);
bool ActorExplosionTick(SotnActors *actors, Actor *actor);

static inline size_t ActorTable(const SotnActors *actors, size_t laboratory, size_t entrance,
                                size_t revisited)
{
    return actors->stage_id == 65  ? entrance
           : actors->stage_id == 7 ? revisited
                                   : ActorStageOffset(actors, laboratory);
}

/* NO3 and NP3 keep their original raw entity IDs in state and placements. This selects
 * the corresponding family only when a shared behavior creates an actor. */
static inline Uint16 EntranceActorId(const SotnActors *s, Uint16 first_visit)
{
    if (s->stage_id != 7)
        return first_visit;
    if (first_visit >= 0x3a && first_visit <= 0x48)
        return (Uint16)(first_visit - 8);
    if (first_visit == 0x4c || first_visit == 0x4d)
        return (Uint16)(first_visit - 10);
    return first_visit == 0x62 ? 0x4d : first_visit;
}
static inline size_t EntranceTable(const SotnActors *s, size_t first_visit, size_t revisited)
{
    return s->stage_id == 7 ? revisited : first_visit;
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
