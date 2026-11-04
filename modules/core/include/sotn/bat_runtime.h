#ifndef SOTN_BAT_RUNTIME_H
#define SOTN_BAT_RUNTIME_H

#include <sotn/bat_echo.h>
#include <sotn/bat_fireball.h>
#include <sotn/bat_trail.h>
#include <sotn/dark_hit.h>
#include <sotn/fire_hit.h>
#include <sotn/food.h>
#include <sotn/food_popup.h>
#include <sotn/hand_weapon.h>
#include <sotn/healing_number.h>
#include <sotn/hellfire_beam.h>
#include <sotn/hellfire_effects.h>
#include <sotn/hellfire_fireball.h>
#include <sotn/holy_hit.h>
#include <sotn/ice_hit.h>
#include <sotn/immunity_text.h>
#include <sotn/karma_coin.h>
#include <sotn/magic_missile.h>
#include <sotn/neutron.h>
#include <sotn/pentagram.h>
#include <sotn/player.h>
#include <sotn/player_circle.h>
#include <sotn/player_outline.h>
#include <sotn/player_particles.h>
#include <sotn/player_smoke.h>
#include <sotn/primitive_slots.h>
#include <sotn/rising_heart.h>
#include <sotn/sire.h>
#include <sotn/spell_target.h>
#include <sotn/spirit_spell.h>
#include <sotn/star_weapon.h>
#include <sotn/stone_dissolve.h>
#include <sotn/sword_warp.h>
#include <sotn/talisman_effect.h>
#include <sotn/teleport_effect.h>
#include <sotn/thrown_weapon.h>
#include <sotn/thunder_hit.h>
#include <sotn/weapon_secondary.h>

enum
{
    SOTN_BAT_RUNTIME_SLOTS = 48,
    SOTN_BAT_RUNTIME_FACTORIES = 8
};

/** @brief Actual stage target view before this frame's stage update; no ownership transfers. */
typedef struct SotnSpiritTargetSnapshot
{
    SotnSpellTarget slots[SOTN_SPELL_TARGETS];
    Uint32 generations[SOTN_SPELL_TARGETS];
} SotnSpiritTargetSnapshot;
/** @brief One completed portal capture intent, fulfilled by the native ordered-frame owner. */
typedef struct SotnSpiritCapture
{
    Uint32 generation;
    Uint16 slot;
    Sint16 x, y;
} SotnSpiritCapture;

/** @brief One actual selected DRA child, indexed by original slot minus sixteen. */
typedef struct SotnBatRuntimeChild
{
    SotnPlayerFactorySource source;
    Uint32 generation, bat_generation;
    Uint32 particle_parent; /**< Actual fire entity generation for profile4 birth. */
    Uint16 particle_parent_slot;
    Uint16 kind, parameter;
    Uint8 group;
    bool initialized, food, neutron, sire, sire_child, pentagram, magic, thrown, star,
        thrown_particle_parent, karma, hand_weapon;
    bool weapon_secondary; /**< Blueprint56 E1/F1 func_ptr_80170004 of a resident program. */
    bool weapon_warp;      /**< Weapon53 kind135 slot16 E2/F2 warp afterimage (blueprint60). */
    union {
        struct
        {
            SotnHandWeapon motion;
            SotnPlayerAttack attack;
            Uint16 item, family;
        } hand;
        struct
        {
            SotnWeaponSecondary motion;
            SotnPlayerAttack attack;
            Uint16 family, item; /**< item: raw parent equip id copied at birth. */
            Uint16 parent_slot;  /**< Raw parent slot16..31; never generation-checked. */
        } secondary;
        SotnWeaponWarp warp;
        struct
        {
            SotnKarmaCoin motion;
            SotnPlayerAttack attack;
            Uint16 parent_slot, item;
        } karma;
        struct
        {
            SotnStarWeapon motion;
            SotnThrownAnimation animation;
            SotnPlayerAttack attack;
            Uint16 parent_slot, animation_offset;
        } star;
        struct
        {
            SotnThrownWeapon motion;
            SotnThrownAnimation animation;
            SotnPlayerAttack attack;
            Uint16 item, parent_slot;
        } thrown;
        SotnHellfireBeam hellfire_beam;
        SotnHellfireEffect hellfire_effect;
        struct
        {
            SotnHellfireTrail motion;
            Uint16 pose, frame;
            Sint16 remaining;
        } hellfire_trail;
        struct
        {
            SotnHellfireFireball motion;
            SotnPlayerAttack attack;
            Uint16 pose, frame;
            Sint16 remaining;
            Uint8 hit_flags;
        } hellfire_fire;
        SotnPlayerBlink blink;
        SotnTalismanEffect talisman;
        SotnBatTrail trail;
        SotnPlayerAttack attack;
        struct
        {
            Uint16 bank, palette, texture, frame;
            Uint8 opacity, draw_flags;
        } axe_shadow;
        SotnPlayerParticles particles;
        SotnPlayerSmoke smoke;
        SotnPlayerOutline outline;
        struct
        {
            SotnSwordWarp image;
            SotnSwordStrike strike;
            SotnPlayerAttack attack;
            Uint32 parent_generation;
        } sword;
        SotnRisingHeart heart;
        SotnHealingNumber healing_number;
        SotnImmunityText immunity_text;
        SotnHolyHit holy;
        SotnDarkHit dark;
        SotnFireHit fire_hit;
        SotnThunderHit thunder;
        SotnIceHit ice;
        SotnStoneDissolve dissolve;
        SotnTeleportEffect teleport;
        SotnPlayerCircle circle;
        SotnMedicineEffect medicine;
        SotnFood food;
        struct
        {
            SotnNeutron motion;
            SotnNeutronAnimation animation;
            SotnPlayerAttack attack;
        } neutron;
        struct
        {
            union {
                SotnSireParent parent;
                SotnSireChild child;
            };
            SotnSireAnimation animation;
            SotnPlayerAttack attack;
            Uint16 parent_slot; /**< Original raw entity slot, deliberately no generation gate. */
        } sire;
        struct
        {
            SotnPentagram motion;
            SotnPlayerAttack attack;
            Uint16 item;
        } pentagram;
        struct
        {
            SotnMagicMissile motion;
            SotnMagicMissileAnimation animation;
            SotnPlayerAttack attack;
            Uint16 parent_slot; /**< Actual raw17..31 parent slot, without a generation gate. */
            Uint8 timeline, hit_flags;
        } magic;
        SotnSpiritPortal portal;
        struct
        {
            SotnSpirit motion;
            SotnPlayerAttack attack;
            SotnPlayerEffectQuad quads[8];
            Uint16 pose, remaining, frame, target_slot;
            bool target_bound;
        } spirit;
        struct
        {
            SotnBatFireball motion;
            SotnPlayerAttack attack;
            Uint16 pose, remaining, frame;
            Uint8 hit_flags;
        } fire;
    } state;
    SotnPlayerEffectQuad trail_quad;
    Uint8 draw_count;
    /** Actual row indices, including particle sentinels (weapon50's sparkles lease eighty). */
    Uint16 primitives[SOTN_BAT_CHILD_ROWS];
    Uint8 primitive_count;
} SotnBatRuntimeChild;

/** @brief Owned selected Bat/Wolf factories and children sharing original DRA slot indices.
 * Zero-initialize for a fresh stage. The owner supplies other actual occupied slots;
 * this service never overrides their availability or invents visual readiness. */
typedef struct SotnBatRuntime
{
    struct
    {
        SotnPlayerFactory cursor;
        Uint32 bat_generation, sword_parent;
        Uint32 fire_parent;
        Uint16 fire_parent_slot, sire_parent_slot, magic_parent_slot, thrown_parent_slot,
            star_parent_slot, karma_parent_slot;
        bool sire_factory, magic_factory, thrown_factory, thrown_particles, star_factory,
            karma_factory;
        bool secondary_factory;       /**< Weapon WFACTORY(0x38) from the actual slot16 program. */
        Uint16 secondary_parent_slot; /**< Raw ext.weapon.parent: slot16 or a secondary. */
    } factories[SOTN_BAT_RUNTIME_FACTORIES];
    SotnBatRuntimeChild children[SOTN_BAT_RUNTIME_SLOTS];
    SotnPlayerBlinkShared shared;
    SotnPentagramShared pentagram[2]; /**< Original weapon17 BSS, one actual program per hand. */
    SotnKarmaShared karma[2]; /**< Actual initialized per-hand weapon20 writable data/BSS. */
    SotnHandWeaponProgram hand_program[2]; /**< Original weapon37/52 writable program state. */
    Uint32 star_fire_counter[2];           /**< Actual per-hand weapon15 mutable program BSS. */
    Uint32 thrown_fire_counter[2]; /**< Actual weapon16 BSS; reset only with program replacement. */
    Uint32 dark_blend_phase;       /**< Actual DRA global, retained through session replacement. */
    SotnHolyHitShared holy;        /**< Actual shared entity47 points, not one copy per child. */
    SotnTeleportShared teleport;   /**< Actual shared entity65 points, never a new particle pool. */
    Uint16 teleport_palette;
    Uint8 teleport_warp_mode;
    bool teleport_acknowledged;
    SotnMedicineEffectEvents medicine; /**< This callback's resources, committed after real rows. */
    SotnFoodEvents food_events[15];
    Uint8 food_event_count;
    Uint16 player_hitbox_state; /**< Actual post-player body word; Toadstool clears it. */
    SotnFoodTextPlan food_text;
    SotnPlayerParticleVectors particle_vectors; /**< Actual full-player-init velocity tables. */
    Uint8 screen_flash_ticks, screen_red, screen_green, screen_blue;
    bool screen_flash; /**< Original one-update opaque GPU tile, separate from retained rows. */
    Uint32 common_generation;  /**< Original global player eight-row lease, not a factory child. */
    Uint16 afterimage_rows[6]; /**< Hidden metadata rows; the final two rows live in wolf. */
    SotnWolfRuntime wolf;
    SotnWolfControllerFrame wolf_frame;
    SotnWolfRuntimeEvents wolf_events;
    SotnPlayerAttack
        wolf_attacks[2]; /**< Actual front18/head20 attacks, sharing enemy group three. */
    Uint8 wolf_mask;
    bool wolf_bound;  /**< This stage owner includes actual Wolf entities/common environment. */
    SotnBatEcho echo; /**< Single original slot-47 wave; not multiplied across the child union. */
    Uint16 echo_primitives[SOTN_BAT_ECHO_PRIMITIVES];
    Uint32 echo_generation;
    Uint32 serial;
    Uint16 wing_mask;
    Uint64 fire_mask, spirit_mask, hellfire_mask, sword_mask, axe_mask;
    Uint64 neutron_mask, sire_mask, pentagram_mask, magic_mask, thrown_mask, star_mask, karma_mask;
    Uint64 secondary_mask; /**< Active weapon func_ptr_80170004 attacks by actual slot. */
    bool hand_attack;      /**< Actual shared slot16 ordinary-hand attack. */
    /** Weapon51 glow g_Clut words written this pass, in callback order (the session commits
     * them to its palette store after a successful pass). */
    Uint8 clut_count;
    struct
    {
        Uint16 palette, color, value;
    } clut[4];
    Uint8
        hand_shield; /**< Original retained unk48 word, updated by actual callbacks/replacement. */
    bool magic_clear_shield; /**< Conditional hand-effect shield retirement, committed by its owner.
                              */
    Uint16 hellfire_signal, hellfire_palette;
    bool hellfire_upload; /**< One-update private-gradient upload intent, not completed pixels. */
    Uint64
        target_marks[2]; /**< New original flag80000 writes, committed with real row ownership. */
    Uint32 target_generations[SOTN_SPELL_TARGETS];
    Uint16 target_identities[SOTN_SPELL_TARGETS], target_cursor;
    SotnSpiritCapture captures[SOTN_BAT_RUNTIME_SLOTS];
    Uint16 capture_count, freeze;
    bool freeze_changed; /**< Actual child request this tick, never capture/factory readiness. */
    Uint16 sounds[48], sound_count;
    Uint16 sound_volumes[48];
    Uint64 sound_volume_mask; /**< Selected ordered PlaySfxVolPan requests; pan remains zero. */
    Uint8 wolf_shakes[32], wolf_shake_count; /**< Includes up to24 actual Iron-ball impact calls. */
    Uint8 use_ticks;       /**< Original timer10: selected effect births and Karma tails. */
    Uint8 spell_use_ticks; /**< Original timer12: selected weapon17/18/19 births and Karma heads. */
    Uint16 dark_minimum;   /**< Original timer11 floor from weapon54 specials; zero if absent. */
    Uint8 groups[11];
    bool soul_attack; /**< Actual initialized slot47/child53 global spell-five attack. */
    bool echo_attack;
    bool dissolve_frame_load; /**< Actual entity32 ANIM_FRAME_LOAD request, never body hiding. */
    bool afterimage_reset;    /**< Actual BlinkWhite reset emitted after player presentation. */
} SotnBatRuntime;

/** @brief Reserve actual ordinary bp48 or unarmed bp42/43 factories. */
bool SotnBatRuntime_RequestHand(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                Uint16 item, bool *allocated);
/** @brief Validate actual shared ordinary slot16 identity, properties and optional tiles. */
bool SotnBatRuntime_HandValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Validate an actual blueprint56 weapon secondary in slots17..31, its resident program,
 * mandatory rows, copied raw-parent group and properties. */
bool SotnBatRuntime_SecondaryValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Validate the actual slot16 weapon53 warp afterimage (rowless, groupless). */
bool SotnBatRuntime_WarpValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Apply actual func_8010FAF4/func_8010E3E0 slot retirement to a working row plan.
 * The caller must commit the real stage pool before granting another slot16 owner. */
bool SotnBatRuntime_RetireHand(SotnBatRuntime *, bool shield_only);

/** @brief Reserve actual weapon20 item24 root50 after its normal chain/curse gates. */
bool SotnBatRuntime_RequestKarma(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                 Uint16 item, bool *allocated);
/** @brief Validate actual weapon20 root/child/row/raw-parent ownership. */
bool SotnBatRuntime_KarmaValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Retire actual weapon20 room owners without ticking or resetting resident program data. */
bool SotnBatRuntime_PruneKarma(SotnBatRuntime *);

/** @brief Run the selected entity33 subset of original room-entry cleanup.
 * Other shared children retain their existing owners; no callbacks or RNG advance. */
bool SotnBatRuntime_PruneIce(SotnBatRuntime *runtime);
/** @brief Run the entity32 subset of actual room-entry cleanup without callbacks or RNG. */
bool SotnBatRuntime_PruneDissolve(SotnBatRuntime *runtime);
/** @brief Validate the actual entity32 lifecycle and thirty-six-row ownership. */
bool SotnBatRuntime_DissolveValid(const SotnBatRuntimeChild *child);
/** @brief Validate actual teleport child generation, parameter and36-row lifecycle. */
bool SotnBatRuntime_TeleportValid(const SotnBatRuntimeChild *child);
/** @brief Validate actual entity38 profile, lifecycle and thirty-two row ownership. */
bool SotnBatRuntime_CircleValid(const SotnBatRuntimeChild *child);
/** @brief Validate actual rowless entity39, profile and signed cursor. */
bool SotnBatRuntime_MedicineValid(const SotnBatRuntimeChild *child);
/** @brief Prune actual cold38/39 children; initialized20000 medicine/circle owners retain rows. */
bool SotnBatRuntime_PruneMedicines(SotnBatRuntime *runtime);
/** @brief Retire weapon21's actual room-local children without running callbacks. */
bool SotnBatRuntime_PruneFood(SotnBatRuntime *runtime);
/** @brief Original InitWeapon retirement after actual menu equipment changes.
 * Mask bits0/1 select changed hands; both special entity rangesE8..ED/F8..FD also retire. */
bool SotnBatRuntime_PruneWeapons(SotnBatRuntime *runtime, Uint8 changed_hands);
/** @brief Original actual factory-slot cleanup by retained20000 flags, before new room allocation.
 * This owns every shared factory record, independently of the remaining child cleanup gates. */
bool SotnBatRuntime_PruneRoomFactories(SotnBatRuntime *runtime);
/** @brief Original room-entry retirement of entity65 and its nonretained factory121 records.
 * Commit the resulting row plan to the actual stage pool before any new allocation. */
bool SotnBatRuntime_PruneTeleport(SotnBatRuntime *runtime);
/** @brief Reserve an actual factory8..15 for original teleport121/high0..5.
 * Exhaustion returns allocated=false without replacing unrelated owners. */
bool SotnBatRuntime_RequestTeleport(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                    Uint32 packed, bool *allocated);
/** @brief Reserve actual55/high profile/hand for a successful ordinary medicine request.
 * Actual eight-root exhaustion returns allocated=false. Pending roots are not live39 actors. */
bool SotnBatRuntime_RequestMedicine(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                    Uint32 packed, Uint16 item, bool *allocated);
/** @brief Reserve actual50/weapon21 food in the same eight roots and17..31 children.
 * This does not consume inventory or enable ordinary controls. */
bool SotnBatRuntime_RequestFood(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                Uint32 packed, Uint16 item, bool *allocated);
/** @brief Reserve actual50 for owned71..74, or original57/profile1 curse feedback. */
bool SotnBatRuntime_RequestThrowing(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                    Uint16 item, bool *allocated);
/** @brief Validate actual weapon17 parent, item and twenty-five-row lifecycle. */
bool SotnBatRuntime_PentagramValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Retire room-local weapon17 parents/rows without callbacks or changing program BSS. */
bool SotnBatRuntime_PrunePentagram(SotnBatRuntime *);
/** @brief Validate actual weapon22 bow/arrow role, raw parent and callback ownership. */
bool SotnBatRuntime_MagicMissileValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Retire actual weapon22 room-local children without callbacks, RNG or another pool. */
bool SotnBatRuntime_PruneMagicMissile(SotnBatRuntime *);
/** @brief Actual weapon16 role/item/row/parent validation, shared by update and stage commit. */
/** @brief Validate actual weapon15 role, raw parent, rows, attack and animation lifetime. */
bool SotnBatRuntime_StarValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Retire actual weapon15 room-local children without callbacks or a BSS tick. */
bool SotnBatRuntime_PruneStar(SotnBatRuntime *);
/** @brief Reserve actual weapon15 root50/68 in the existing eight factory records. */
bool SotnBatRuntime_RequestStar(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                Uint16 item, bool *allocated);
bool SotnBatRuntime_ThrownValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Retire real room-local weapon16 children/rows without advancing callbacks or BSS. */
bool SotnBatRuntime_PruneThrown(SotnBatRuntime *);
/** @brief Reserve original50/62/63/68 roots for the five actual owned weapon16 items. */
bool SotnBatRuntime_RequestThrown(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                  Uint16 item, bool *allocated);
/** @brief Reserve actual weapon22/item25 root50 in the existing eight factory slots.
 * Inventory and player-success commitment belong to the bound native hand owner. */
bool SotnBatRuntime_RequestMagicMissile(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                        bool *allocated);
/** @brief Reserve actual50 weapon19/Neutron71, or its original57 curse outline. */
bool SotnBatRuntime_RequestNeutron(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                   Uint32 packed, Uint16 item, bool *allocated);
/** @brief Reserve actual50/item72 in the existing eight roots. Exhaustion spends nothing. */
bool SotnBatRuntime_RequestSire(SotnBatRuntime *, SotnPlayerFactorySource, Uint32 packed,
                                Uint16 item, bool *allocated);
/** @brief Validate actual weapon18 parent/burst roles, slots and animation cursors. */
bool SotnBatRuntime_SireValid(const SotnBatRuntimeChild *, Uint16 index);
/** @brief Retire room-local weapon18 parents and bursts without ticking owners. */
bool SotnBatRuntime_PruneSire(SotnBatRuntime *);
/** @brief Validate a real weapon19 child and its single actual GT4 lease. */
bool SotnBatRuntime_NeutronValid(const SotnBatRuntimeChild *child, Uint16 index);
/** @brief Retire room-local weapon19 children without callbacks or extra clocks. */
bool SotnBatRuntime_PruneNeutron(SotnBatRuntime *runtime);
/** @brief Validate a real weapon21 primary or inventory child, independently of weapon43. */
bool SotnBatRuntime_FoodChildValid(const SotnBatRuntimeChild *child, Uint16 index);
/** @brief Reserve the original familiar's actual55/profile0..19 with signed arg2 zero.
 * Source must be the actual familiar's captured room-space origin/facing, not the player.
 * Uses the same8..15 roots and later32..63 medicine actors as ordinary hand use. Exhaustion
 * returns allocated=false; item spending still belongs to the successful familiar callback.
 * Does not permit hand input for Hammer/Life Apple or enable any familiar by itself. */
bool SotnBatRuntime_RequestFamiliarMedicine(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                            Uint32 packed, bool *allocated);
/** @brief Inspect actual entity39 slots32..63 only, without treating roots as actors. */
bool SotnBatRuntime_MedicinePresent(const SotnBatRuntime *runtime);
/** @brief Commit actual post-stage shoves to player-locked holy/ice/circle/medicine entities only.
 * Packets, shared points, rows, RNG and controllers hold. Room entry has its own rebase. */
bool SotnBatRuntime_PlayerShove(SotnBatRuntime *runtime, Sint32 x, Sint32 y);

/** @brief Actual owned context and room/camera translation at this update point. */
typedef struct SotnBatRuntimeInput
{
    SotnDataView dra, executable, weapons;
    Uint8
        weapon_family[2]; /**< Actual resident program identity, retained after last consumption. */
    bool weapon_bound[2]; /**< False only for component callers without a native program owner. */
    const SotnFoodTextPlan *food_text;
    SotnCollisionMap food_collision; /**< Actual current room collision for weapon21/16. */
    SotnPlayerState player;
    SotnPlayerImage image;
    SotnPlayerImage talisman_image;    /**< Actual private normal frame9F. */
    SotnWolfCaptureBounds wolf_bounds; /**< External bounds only when Wolf is not bound here. */
    SotnWolfCollisionQuery wolf_query;
    void *wolf_context;
    bool wolf_owned; /**< Bind the actual Wolf body to this shared entity update order. */
    Uint64 other_occupied;
    SotnPrimitiveSlots primitive_slots; /**< Snapshot of actual stage/player primitive occupancy. */
    Sint32 camera_x, camera_y;
    Sint32 wolf_command_camera_x, wolf_command_camera_y; /**< Raw controller capture camera;
                                                            zero for owned room-space exports. */
    Sint16 room_shift_x, room_shift_y, screen_shift_x, screen_shift_y;
    bool entering;
    Uint32 game_timer; /**< Actual wrapping game clock for Hellfire, never a local child age. */
    bool hellfire_up;  /**< Actual current Up input, sampled when the beam requests projectiles. */
    Uint16 freeze;     /**< Original request snapshotted before ALL actual factory/child updates. */
    const SotnSpiritTargetSnapshot
        *spirit_targets;     /**< Actual preceding stage view; required by spirits. */
    bool force_echo_enabled; /**< Found-and-enabled world snapshot, sampled on real Echo birth. */
    SotnDeathImage *dissolve_image; /**< Actual shared DRA5120-byte scratch, not a child pool. */
    SotnStoneDissolveCapture dissolve_capture;
    SotnStoneDissolveUpload dissolve_upload;
    void *dissolve_context;
    Uint32 engine_timer;      /**< Actual g_Timer for entity32, distinct from gameplay clock. */
    bool holy_symbol_enabled; /**< Actual relic snapshot for selected water-pain Blink holding. */
} SotnBatRuntimeInput;

/** @brief Reserve the original global player's eight rows before stage actors advance.
 * @param runtime Fresh zeroed stage owner. @param slots Actual stage primitive occupancy.
 * @return Success or SDL error without mutation, including exhausted capacity.
 * Reserves six hidden metadata rows and the same two common environment rows used by Wolf.
 * Commit through SotnActors_CommitBatPrimitives before another owner allocates. These rows
 * persist through body retirement and room changes; they confer no effect readiness or
 * afterimage display/texture ownership. Full stage reset requires a fresh stage owner. */
bool SotnBatRuntime_InitCommon(SotnBatRuntime *runtime, SotnPrimitiveSlots slots);

/** @brief Current selected occupied child/factory slots for shared allocation.
 * @param runtime Borrowed owner. @return Actual nonzero owners, or zero for NULL. */
Uint64 SotnBatRuntime_Occupied(const SotnBatRuntime *runtime);
/** @brief Whether an actual factory slot 8..15 is free in this selected owner. */
bool SotnBatRuntime_Available(const SotnBatRuntime *runtime);
/** @brief Perform original Wolf entry's forced replacement of selected slots17..30.
 * @param runtime Actual shared factory/child owner. @return Success or SDL error.
 * Retires only those original slots, retaining groups' enemy cooldowns in their combat owner.
 * Commit this retirement to the stage pool before reserving new Wolf root/environment rows. */
bool SotnBatRuntime_ReplaceWolfSlots(SotnBatRuntime *runtime);
/** @brief Find the actual current transformation child or pending real factory.
 * @param runtime Borrowed owner. @param generation Current nonzero Bat identity.
 * @param effect Receives borrowed child or NULL. @param pending Receives borrowed factory or NULL.
 * @return Success or SDL error for duplicate ownership/invalid outputs. Views last until next Tick.
 */
bool SotnBatRuntime_Context(const SotnBatRuntime *runtime, Uint32 generation,
                            const SotnPlayerBlink **effect, const SotnPlayerFactory **pending);
/** @brief Run actual selected factory requests/children once in original slot order.
 * @param runtime Owned stage lifetime. @param character Borrowed owner of spell stats/RNG.
 * @param input Actual player/artwork/context. Entering only rebases and never advances time.
 * @return Success or SDL error. Malformed inputs preserve runtime and character progress.
 * Supports selected Bat morphs, Wing/Fire children, Echo, spirit61/portal63 and optional Wolf.
 * Bound Wolf runs at slots17..30; later flashes read its current capture bounds. The owning
 * player must commit wolf_frame once after success. Body MP charges commit here, not again
 * through wolf_events. Selected Wolf spell requests initialize real wolf_attacks here;
 * the native combat owner applies them. Selected Wolf puffs/particles use real factories
 * and primitive rows; the native scene must draw their retained state and consume shakes.
 * Actual entity31 outlines retain full parameters, share one row per child, and propagate
 * real afterimage resets. Normal spell/MP-refill/curse request producers remain separate.
 * The owner must commit the planned rows to the actual primitive pool before advancing
 * other owners; no pool storage is mutated by this transactional update.
 * Spirits require an actual pre-stage target snapshot. Commit target_marks atomically against
 * its identity/generation, then acknowledge them before another non-Tick row commit. Retained
 * targets follow original raw slots. Consume captures/freeze_changed once through real engine
 * owners; an intent never establishes capture readiness. Input freeze is fixed for all slots.
 * Portal row exhaustion fails transactionally rather than hanging in its original freeze. No
 * native spell controls or scene freeze are enabled by this service alone. */
bool SotnBatRuntime_Tick(SotnBatRuntime *runtime, SotnCharacter *character,
                         SotnBatRuntimeInput input);
/** @brief Borrow one child's current draw packets, valid until the next runtime update.
 * @param runtime Borrowed owner. @param index Original slot minus sixteen, 0..47.
 * @param quads Receives a borrowed contiguous view or NULL for no drawing. @param count 0..8.
 * @return Success or SDL error for invalid inputs. No allocation or simulation advances. */
bool SotnBatRuntime_Visual(const SotnBatRuntime *runtime, Uint16 index,
                           const SotnPlayerEffectQuad **quads, Uint16 *count);

/** @brief Deliver actual enemy contact to one generation-owned fireball.
 * @param runtime Borrowed owner. @param index Original slot minus sixteen.
 * @param generation Observed child identity. @param flags Original contact word, 1..3.
 * @return Success or SDL error without mutation on stale/non-fire ownership. */
bool SotnBatRuntime_Hit(SotnBatRuntime *runtime, Uint16 index, Uint32 generation, Uint8 flags);

/** @brief Reserve an actual shared factory for root33/circle37 or original finishing flashes.
 * @param runtime Actual shared owner. @param source Current player source.
 * @param packed Original request. @param allocated Receives actual reservation outcome.
 * @return Success or SDL error preserving runtime/output. Exhaustion returns success/false;
 * no child/readiness is manufactured. The ordinary shared update advances the reserved factory.
 * Call once at the original player boundary; this does not enable native Hellfire controls. */
bool SotnBatRuntime_RequestHellfire(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                    Uint32 packed, bool *allocated);
/** @brief Reserve a real sword123 root or optional outline factory before player commit.
 * @param runtime Actual shared owner. @param source Captured player/root origin.
 * @param packed Original left/right root54 or outline61 profile20/21 request.
 * @param argument Actual virtual equipment197 for root, zero for outline.
 * @param allocated Actual reservation outcome. @return Success or SDL error, atomically.
 * Exhaustion never invents children. The native player caller must undo reservations on failure. */
bool SotnBatRuntime_RequestSword(SotnBatRuntime *runtime, SotnPlayerFactorySource source,
                                 Uint32 packed, Sint16 argument, bool *allocated);
/** @brief Validate actual weapon43 root/strike role and lifecycle before row or stage commit. */
bool SotnBatRuntime_SwordChildValid(const SotnBatRuntimeChild *child, Uint16 index);
#endif
