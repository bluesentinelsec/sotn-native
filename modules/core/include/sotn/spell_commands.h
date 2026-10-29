#ifndef SOTN_SPELL_COMMANDS_H
#define SOTN_SPELL_COMMANDS_H

#include <sotn/game_data.h>

enum
{
    SOTN_SPELL_COMMAND_DARK = 0,
    SOTN_SPELL_COMMAND_SUMMON = 1,
    SOTN_SPELL_COMMAND_HELLFIRE = 2,
    SOTN_SPELL_COMMAND_TETRA = 3,
    SOTN_SPELL_COMMAND_SOUL = 5,
    SOTN_SPELL_COMMAND_SWORD = 7,
    SOTN_SPELL_COMMAND_COUNT = 8
};

/** @brief Original signed-word command globals, retained across simulation ticks only. */
typedef struct SotnSpellCommand
{
    Sint16 phase, timer;
    bool facing_left;
} SotnSpellCommand;

/** @brief Six selected ordinary spell commands indexed by original spell identity.
 * Zero-initialize on full character initialization. Unused indices4/6 remain zero. */
typedef struct SotnSpellCommands
{
    SotnSpellCommand spells[SOTN_SPELL_COMMAND_COUNT];
} SotnSpellCommands;

/** @brief Current original input/state and actual native spell-owner availability. */
typedef struct SotnSpellCommandFrame
{
    Uint32 pressed, tapped; /**< Full US action words; exact comparisons must retain other bits. */
    Uint16 step, attack_mode, mp;
    Uint8 available; /**< Original spell-id bits; set only for fully owned native spell paths. */
    bool facing_left, cramped, sword_active;
} SotnSpellCommandFrame;

/** @brief One original cast request and its copied resource result, never effect readiness. */
typedef struct SotnSpellCommandEvents
{
    Uint16 spell, mp_spent;
    bool cast, sword_notify;
} SotnSpellCommandEvents;

/** @brief Advance six ordinary commands in original relative priority, at most one cast.
 * @param commands Owned transient input globals. @param dra Borrowed private spell definitions.
 * @param frame Copied current input/state and MP; successful requests debit this snapshot once.
 * @param out Receives current events; cast is false when no spell request succeeds.
 * @return Success or SDL error preserving every output/state on malformed input/private data.
 * The owner must first resolve earlier Gravity Boots/weapon commands, then invoke this once
 * if no earlier command handled the frame. The owner must fulfill actual player/effect/combat
 * allocation, commit MP/learning once, and retain pending ownership. Availability defaults to
 * zero; this component enables no controls, creates no effects and writes no saves. */
bool SotnSpellCommands_Tick(SotnSpellCommands *commands, SotnDataView dra,
                            SotnSpellCommandFrame *frame, SotnSpellCommandEvents *out);

#endif
