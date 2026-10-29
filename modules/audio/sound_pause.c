// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected dra/sound.c:PauseSfxScripts/UnpauseSfxScripts/RestoreSfxScriptData,
// sotn-decomp 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. No sound tables are embedded.
#include "sound_internal.h"
void SoundPauseScripts(SotnSound *sound)
{
    for (Uint16 i = 0; i < 4; ++i)
    {
        SoundScript *script = &sound->scripts[i];
        if (!script->id || script->mode == 5)
            continue;
        sound->script_backup[i] = *script;
        sound->script_backup[i].allocation_id = 0;
        sound->script_backup[i].priority = 0;
        *script = (SoundScript){0};
    }
    sound->scripts_paused = true;
}
static void Restore(SotnSound *sound, Uint16 source, Uint16 destination)
{
    const Uint16 allocation = sound->scripts[destination].allocation_id;
    const Uint8 priority = sound->scripts[destination].priority;
    sound->scripts[destination] = sound->script_backup[source];
    // Original Restore copies no allocation identity/priority, even when a prior restore
    // left a nonzero cursor here. Later copied slots can overwrite that same free channel.
    sound->scripts[destination].allocation_id = allocation;
    sound->scripts[destination].priority = priority;
}
void SoundResumeScripts(SotnSound *sound)
{
    for (Uint16 i = 0; i < 3; ++i)
    {
        if (!sound->script_backup[i].id)
            continue;
        for (Uint16 j = 0; j < 3; ++j)
            if (!sound->scripts[j].allocation_id)
            {
                Restore(sound, i, j);
                break;
            }
        sound->script_backup[i].id = 0;
    }
    if (sound->script_backup[3].id && !sound->scripts[3].allocation_id)
    {
        Restore(sound, 3, 3);
        sound->script_backup[3].id = 0;
    }
    sound->scripts_paused = false;
}
