// SPDX-License-Identifier: AGPL-3.0-or-later
// Corresponding private US table addresses for shared reconstructed algorithms.
// Keys identify the Laboratory table layout, not an aliased actor identity.
#include "actors_internal.h"
size_t ActorStageOffset(const SotnActors *s, size_t laboratory)
{
    if (s->stage_id == 25)
    {
        if (laboratory >= 0x1cc4 && laboratory < 0x1e64)
            return laboratory - 0x814;
        switch (laboratory)
        {
        case 0xbc8:
            return 0x454;
        case 0xbd4:
            return 0x460;
        case 0xbe0:
            return 0x46c;
        case 0xbec:
            return 0x478;
        case 0xbf8:
            return 0x484;
        case 0xc04:
            return 0x490;
        case 0xc10:
            return 0x49c;
        case 0xc1c:
            return 0x4a8;
        case 0xd8c:
            return 0x514;
        case 0xdbc:
            return 0x544;
        case 0xdc0:
            return 0x54c;
        case 0xdc4:
            return 0x554;
        case 0x13b0:
            return 0xb78;
        case 0x1960:
            return 0x114c;
        default:
            SDL_SetError("BO5 has no shared private table for offset %zu", laboratory);
            return s->nz0.size;
        }
    }
    if (s->stage_id == 6)
    {
        if (laboratory >= 0x1cc4 && laboratory < 0x1e64)
            return laboratory - 0x504;
        switch (laboratory)
        {
        case 0xbc8:
            return 0x938;
        case 0xbd4:
            return 0x944;
        case 0xbe0:
            return 0x950;
        case 0xbec:
            return 0x95c;
        case 0xbf8:
            return 0x968;
        case 0xc04:
            return 0x974;
        case 0xc10:
            return 0x980;
        case 0xc1c:
            return 0x98c;
        case 0xd8c:
            return 0xadc;
        case 0xdbc:
            return 0xb0c;
        case 0xdc0:
            return 0xb14;
        case 0xdc4:
            return 0xb1c;
        case 0x13b0:
            return 0xec0;
        case 0x1960:
            return 0x145c;
        default:
            SDL_SetError("Chapel has no shared private table for offset %zu", laboratory);
            return s->nz0.size;
        }
    }
    if (s->stage_id == 2)
    {
        if (laboratory >= 0x1cc4 && laboratory < 0x1e64)
            return laboratory + 0x848;
        if (laboratory >= 0x23dc && laboratory < 0x24cc)
            return laboratory + 0x86c;
        switch (laboratory)
        {
        case 0xbc8:
            return 0x800;
        case 0xbd4:
            return 0x80c;
        case 0xbe0:
            return 0x818;
        case 0xbec:
            return 0x824;
        case 0xbf8:
            return 0x830;
        case 0xc04:
            return 0x83c;
        case 0xc10:
            return 0x848;
        case 0xc1c:
            return 0x854;
        case 0xc88:
            return 0x998;
        case 0xc94:
            return 0x9a4;
        case 0xca0:
            return 0x9b0;
        case 0xd8c:
            return 0xac8;
        case 0xdbc:
            return 0xad8;
        case 0xdc0:
            return 0xae0;
        case 0xdc4:
            return 0xae8;
        case 0x13b0:
            return 0x1a90;
        case 0x1960:
            return 0x2068;
        default:
            SDL_SetError("Library has no shared private table for offset %zu", laboratory);
            return s->nz0.size;
        }
    }
    if (s->stage_id == 26)
    {
        if (laboratory >= 0x1cc4 && laboratory < 0x1e64)
            return laboratory - 0xec4;
        switch (laboratory)
        {
        case 0xbc8:
            return 0x3a4;
        case 0xbd4:
            return 0x3b0;
        case 0xbe0:
            return 0x3bc;
        case 0xbec:
            return 0x3c8;
        case 0xbf8:
            return 0x3d4;
        case 0xc10:
            return 0x3ec;
        case 0xc1c:
            return 0x3f8;
        case 0xd8c:
            return 0x49c;
        case 0xdbc:
            return 0x4bc;
        case 0xdc0:
            return 0x4c4;
        case 0xdc4:
            return 0x4cc;
        case 0xe04:
            return 0x54c;
        case 0xe24:
            return 0x56c;
        case 0xe34:
            return 0x57c;
        case 0xe44:
            return 0x58c;
        case 0x1960:
            return 0xa9c;
        default:
            SDL_SetError("BO4 has no shared private table for offset %zu", laboratory);
            return s->nz0.size;
        }
    }
    if (s->stage_id != 0 && s->stage_id != 1)
        return laboratory;
    const bool wall = s->stage_id == 1;
    if (laboratory >= 0x1cc4 && laboratory < 0x1e64)
        return wall ? laboratory + 0x4d0 : laboratory - 0x32c;
    if (laboratory >= 0x23dc && laboratory < 0x24cc)
        return laboratory + (wall ? 0x350 : 0x2a4);
    if (!wall && laboratory >= 0x2180 && laboratory < 0x22d4)
        return laboratory + 0x2f8;
    switch (laboratory)
    {
    case 0xbc8:
        return wall ? 0x938 : 0xa64;
    case 0xbd4:
        return wall ? 0x944 : 0xa70;
    case 0xbe0:
        return wall ? 0x950 : 0xa7c;
    case 0xbec:
        return wall ? 0x95c : 0xa88;
    case 0xbf8:
        return wall ? 0x968 : 0xa94;
    case 0xc10:
        return wall ? 0x9a4 : 0xadc;
    case 0xc1c:
        return wall ? 0x9b0 : 0xae8;
    case 0xc64:
        return wall ? s->nz0.size : 0xbfc;
    case 0xc70:
        return wall ? s->nz0.size : 0xc08;
    case 0xc88:
        return wall ? 0xa58 : 0xc38;
    case 0xc94:
        return wall ? 0xa64 : 0xc44;
    case 0xca0:
        return wall ? 0xa70 : 0xc50;
    case 0xd8c:
        return wall ? 0xc9c : 0xc5c;
    case 0xdbc:
        return wall ? 0xcfc : 0xc8c;
    case 0xdc0:
        return wall ? 0xd04 : 0xc94;
    case 0xdc4:
        return wall ? 0xd0c : 0xc9c;
    case 0xe04:
        return wall ? 0xdd8 : 0xf58;
    case 0xe24:
        return wall ? 0xe00 : 0xf7c;
    case 0xe34:
        return wall ? 0xe2c : 0xf94;
    case 0xe44:
        return wall ? 0xe54 : 0xfa8;
    case 0x13b0:
        return wall ? 0x1a2c : 0x1100;
    case 0x1960:
        return wall ? 0xc84 : 0x1528;
    default:
        SDL_SetError("Stage %u has no shared private table for offset %zu", s->stage_id,
                     laboratory);
        return s->nz0.size;
    }
}
Uint16 ActorStageId(const SotnActors *s, Uint16 laboratory)
{
    if (s->stage_id == 2)
    {
        switch (laboratory)
        {
        case 0x2e:
            return 0x43;
        case 0x2f:
            return 0x44;
        case 0x30:
            return 0x45;
        default:
            return laboratory;
        }
    }
    if (s->stage_id != 0 && s->stage_id != 1)
        return laboratory;
    switch (laboratory)
    {
    case 0x29:
        return s->stage_id == 0 ? 0x47 : 0x20;
    case 0x2a:
        return s->stage_id == 0 ? 0x48 : 0x21;
    case 0x2e:
        return s->stage_id == 0 ? 0x4d : 0x3e;
    case 0x2f:
        return s->stage_id == 0 ? 0x4e : 0x3f;
    case 0x30:
        return s->stage_id == 0 ? 0x4f : 0x40;
    default:
        return laboratory;
    }
}
