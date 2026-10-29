#ifndef SOTN_STAGE_FREEZE_H
#define SOTN_STAGE_FREEZE_H
#include <sotn/game_data.h>
/** @brief Copied original frozen-update palette/controller fields, not a second actor owner. */
typedef struct SotnStageFreezeActor
{
    Uint32 flags;
    Uint16 step, palette, hit_palette;
    Uint8 palette_effect;
} SotnStageFreezeActor;
/** @brief Resolve the pinned US overlay's palette reference from its exported freeze updater.
 * @param stage Borrowed verified overlay at original80180000, retained while using the view.
 * @param out Receives exactly22 private palette words. @return Success or SDL error preserving out.
 * Validates bounded export, original LUI/ADDIU reference shape and table bounds; executes no code
 * and copies no artwork. Other overlay layouts fail explicitly instead of guessing an address. */
bool SotnStageFreeze_Palettes(SotnDataView stage, SotnDataView *out);
/** @brief Apply original UpdateStageEntities eligibility and hit-palette countdown once.
 * @param actor Copied current fields from an actual live actor with a real behavior.
 * @param palettes Borrowed private22-word palette table. Used only by an eligible nonzero step.
 * @param out Receives whether the owner must invoke the actual behavior and clear hit flags.
 * @return Success or SDL error preserving actor/output. Step-zero initializes regardless of
 * flag10000; initialized actors without that flag retain ALL state. No stun/cooldown, layout,
 * culling, hit detection, movement or simulation-clock work is performed by this service. */
bool SotnStageFreeze_Begin(SotnStageFreezeActor *actor, SotnDataView palettes, bool *out);
#endif
