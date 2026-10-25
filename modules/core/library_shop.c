// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected st/lib/e_shop.c root/buy/sell states, sotn-decomp 7bc54d8157b6.
// Catalogues, labels, item records and option profiles remain private inputs.
#include "actors_internal.h"
static bool Options(SotnActors *s)
{
    SotnDataView pointer = {0}, list = {0};
    if (!Slice(s->nz0, 0x1340U + (s->world->game_clear ? 2U : 1U) * 4, 4, &pointer))
        return false;
    const Uint32 address = Read32(pointer.bytes);
    if (address < 0x80180000 || !Slice(s->nz0, address - 0x80180000U, 1, &list))
        return SDL_SetError("Library shop option profile has an invalid pointer");
    const Uint16 count = list.bytes[0];
    if (!count || count > 6 || !Slice(s->nz0, address - 0x80180000U + 1, count, &list))
        return SDL_SetError("Library shop option count exceeds the original panel");
    for (Uint16 i = 0; i < count; ++i)
    {
        if (list.bytes[i] > 5)
            return SDL_SetError("Library shop option identity is invalid");
        s->shop.options[i] = list.bytes[i];
    }
    s->shop.count = count;
    return true;
}
static bool Voice(SotnActors *s, Actor *a, Uint16 id)
{
    if (!a->substep)
    {
        if (s->death_event.voice_playing)
            s->combat.voice_stop = true;
        a->substep = 1;
        return false;
    }
    if (s->death_event.voice_playing)
        return false;
    s->combat.voice_change = true;
    s->combat.voice = id;
    a->substep = 0;
    return true;
}
static bool Root(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x824) || !Options(s) || !ActorPrimitives(s, a, 128))
            return false;
        break;
    case 1:
        ++a->state.step;
        break;
    case 2:
        if (s->death_event.flags & 0x200U)
        {
            s->shop.cursor = 0;
            a->librarian.timer = a->pose = 0;
            SetStep(a, 3);
        }
        break;
    case 3:
        ++a->librarian.timer;
        a->pose = (Uint16)(a->pose + 0x40U);
        if (a->librarian.timer == 64)
            SetStep(a, 4);
        break;
    case 4: {
        Uint16 cursor = s->shop.cursor;
        if ((s->shop.repeated & SOTN_BUTTON_DOWN) && cursor + 1U < s->shop.count)
            ++cursor;
        else if ((s->shop.repeated & SOTN_BUTTON_UP) && cursor)
            --cursor;
        if (cursor != s->shop.cursor && !ActorSound(s, 0x67b))
            return false;
        s->shop.cursor = cursor;
        if (s->tapped_buttons & SOTN_BUTTON_JUMP)
        {
            const Uint16 choice = s->shop.options[cursor];
            if (choice == 0 || choice == 5)
            {
                s->shop.sell = choice == 5;
                Actor *child = NULL;
                if (!ActorLibrarySpawnAt(s, a, 0x27, s->shop.sell ? 1 : 0, (Uint32)a->slot + 1,
                                         &child))
                    return false;
                s->death_event.flags |= 0x400U;
                SetStep(a, 5);
            }
            else if (choice == 4)
                SetStep(a, 6);
            else
            {
                // Preserve the choice without pretending its missing service succeeded.
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                            "Library shop option %u is not integrated yet", choice);
                return ActorSound(s, 0x6a5);
            }
            if (!ActorSound(s, 0x6ad))
                return false;
        }
        else if (s->tapped_buttons & SOTN_BUTTON_CANCEL)
        {
            SetStep(a, 6);
            if (!ActorSound(s, 0x6ad))
                return false;
        }
        break;
    }
    case 5:
        if (Voice(s, a, 0x3a9))
            SetStep(a, 9);
        break;
    case 6:
        s->death_event.library_reply = 0;
        if (a->substep < 2)
        {
            if (Voice(s, a, 0x3b2))
            {
                a->timer = 32;
                a->substep = 2;
            }
        }
        else if (!--a->timer)
        {
            s->death_event.flags |= 0x100U;
            SetStep(a, 7);
        }
        break;
    case 7:
        a->pose = 0;
        a->librarian.timer = 64;
        ++a->state.step;
        break;
    case 8:
        --a->librarian.timer;
        a->pose = (Uint16)(a->pose + 0x40U);
        if (!a->librarian.timer)
        {
            s->death_event.flags &= ~0x200U;
            SetStep(a, 2);
        }
        break;
    case 9:
        a->librarian.timer = 0;
        ++a->state.step;
        break;
    case 10:
        if (++a->librarian.timer >= 24)
        {
            s->death_event.flags &= ~0x200U;
            SetStep(a, 11);
        }
        break;
    case 11:
        if (s->death_event.flags & 0x200U)
        {
            a->librarian.timer = 24;
            SetStep(a, 12);
        }
        break;
    case 12:
        if (!--a->librarian.timer)
            SetStep(a, 4);
        break;
    default:
        return SDL_SetError("Library shop root step is invalid");
    }
    return true;
}
static bool Navigate(SotnActors *s, bool sell)
{
    const Uint16 count = s->shop.catalogue.count;
    if (!count)
        return SDL_SetError("Library shop catalogue has no selectable rows");
    const Uint16 max_scroll = count > 7 ? (Uint16)(count - 7U) : 0;
    const Uint16 visible = SDL_min(count, 7);
    Uint16 row = s->shop.item_cursor, scroll = s->shop.scroll;
    const Uint16 repeat = s->shop.repeated;
    if (repeat & SOTN_BUTTON_DOWN)
    {
        if (row + 1U < visible)
            ++row;
        else if (!sell && scroll < max_scroll)
            ++scroll;
    }
    else if (repeat & SOTN_BUTTON_UP)
    {
        if (row)
            --row;
        else if (scroll)
            --scroll;
    }
    else if (!sell && (repeat & 0xa00U)) // Original R1/R2 page navigation.
    {
        if (row + 1U < visible)
            row = (Uint16)(visible - 1U);
        else
            scroll = SDL_min(max_scroll, (Uint16)(scroll + 7U));
    }
    else if (!sell && (repeat & 0x500U)) // Original L1/L2.
    {
        if (row)
            row = 0;
        else
            scroll = scroll > 7 ? (Uint16)(scroll - 7U) : 0;
    }
    bool changed = row != s->shop.item_cursor || scroll != s->shop.scroll;
    s->shop.item_cursor = row;
    s->shop.scroll = scroll;
    const Uint16 index = (Uint16)(row + scroll);
    Uint16 *quantity = &s->shop.quantities[index];
    if ((repeat & SOTN_BUTTON_RIGHT) && *quantity < s->shop.catalogue.offers[index].available)
    {
        ++*quantity;
        changed = true;
    }
    else if ((repeat & SOTN_BUTTON_LEFT) && *quantity > 1)
    {
        --*quantity;
        changed = true;
    }
    return !changed || ActorSound(s, 0x67b);
}
static bool List(SotnActors *s, Actor *a)
{
    switch (a->state.step)
    {
    case 0:
        if (!ActorInitialize(s, a, 0x824) || !ActorPrimitives(s, a, 384))
            return false;
        break;
    case 1:
        if (s->death_event.flags & 0x400U)
        {
            if (!SotnShop_Catalogue(s->nz0, s->character, s->world, s->shop.sell,
                                    &s->shop.catalogue))
                return false;
            SDL_zero(s->shop.quantities);
            for (Uint16 i = 0; i < s->shop.catalogue.count; ++i)
                s->shop.quantities[i] = s->shop.catalogue.offers[i].available ? 1 : 0;
            s->shop.item_cursor = s->shop.scroll = 0;
            a->librarian.timer = a->pose = 0;
            SetStep(a, 2);
        }
        break;
    case 2:
        ++a->librarian.timer;
        a->pose = (Uint16)(a->pose + 0x40U);
        if (a->librarian.timer == 16)
            SetStep(a, s->shop.sell ? 5 : 3);
        break;
    case 3:
    case 5:
        if (!Navigate(s, s->shop.sell))
            return false;
        if (s->tapped_buttons & SOTN_BUTTON_JUMP)
        {
            const Uint16 index = (Uint16)(s->shop.item_cursor + s->shop.scroll);
            SotnShopOffer *offer = &s->shop.catalogue.offers[index];
            const Uint16 quantity = s->shop.quantities[index];
            SotnCharacterState state = {0};
            if (!SotnCharacter_Get(s->character, &state, NULL))
                return false;
            if (!quantity || (!s->shop.sell && state.gold < quantity * offer->price))
                return ActorSound(s, 0x6a5);
            if (!SotnShop_Trade(s->nz0, s->dra, s->character, s->world, s->shop.sell, offer->row,
                                quantity))
                return false;
            offer->available = (Uint16)(offer->available - quantity);
            s->shop.quantities[index] = s->shop.sell || !offer->available ? 0 : 1;
            SetStep(a, s->shop.sell ? 6 : 4);
            if (!ActorSound(s, 0x6ad))
                return false;
        }
        else if (s->tapped_buttons & SOTN_BUTTON_CANCEL)
            SetStep(a, 7);
        break;
    case 4:
    case 6:
        if (Voice(s, a, 0x3ad))
        {
            s->death_event.flags |= 0x800U;
            SetStep(a, s->shop.sell ? 5 : 3);
        }
        break;
    case 7:
        if (!(s->death_event.flags & 0x200U))
        {
            a->pose = 0x400;
            a->librarian.timer = 16;
            s->death_event.flags |= 0x200U;
            ++a->state.step;
        }
        break;
    case 8:
        --a->librarian.timer;
        a->pose = (Uint16)(a->pose - 0x40U);
        if (a->librarian.timer == 1)
            s->death_event.flags &= ~0x400U;
        if (!a->librarian.timer)
            a->remove = true;
        break;
    default:
        return SDL_SetError("Library shop buy/sell step is invalid");
    }
    return true;
}
bool ActorLibraryShop(SotnActors *s, Actor *a)
{
    if (!s || !a || s->stage_id != 2 || !s->character || !s->world ||
        (a->state.id != 0x26 && a->state.id != 0x27))
        return SDL_SetError("Library shop needs its own stage, character and world owners");
    if (a->state.id == 0x26)
    {
        s->shop.repeated = s->repeat_buttons;
        if (!Root(s, a))
            return false;
    }
    else if (a->state.id != 0x27 || !List(s, a))
        return false;
    return ActorLibraryShopGraphics(s, a);
}
