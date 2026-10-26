// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected Start/equipment slot/selection rules from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5: dra/menu.c.
#include <SDL3/SDL.h>
#include <sotn/inventory_menu.h>
#include <sotn/relic.h>
static bool Rows(SotnInventoryMenu *menu, SotnCharacter *c)
{
    SotnCharacterState state = {0};
    if (!SotnCharacter_Get(c, &state, NULL))
        return false;
    const bool body = menu->slot >= 2;
    const Uint16 empty = menu->slot < 2    ? 0
                         : menu->slot == 2 ? 26
                         : menu->slot == 3 ? 0
                         : menu->slot == 4 ? 48
                                           : 57;
    menu->count = 1;
    menu->rows[0] = empty;
    const Uint8 *order = body ? state.body_order : state.hand_order,
                *counts = body ? state.body_count : state.hand_count;
    const Uint32 count = body ? SOTN_BODY_ITEMS : SOTN_HAND_ITEMS;
    for (Uint32 i = 0; i < count; ++i)
    {
        const Uint16 id = order[i];
        if (counts[id] == 0 || id == empty)
            continue;
        /* Type filtering uses the character's private metadata through its query API. */
        SotnItem item = {0};
        if (!SotnCharacter_Item(c, body, id, &item))
            return false;
        const Uint16 type = menu->slot < 2 ? 0 : menu->slot < 5 ? (Uint16)(menu->slot - 2) : 3;
        if (!body || item.type == type)
            menu->rows[menu->count++] = id;
    }
    menu->item_cursor = 0;
    return true;
}
bool SotnInventoryMenu_Tick(SotnInventoryMenu *m, SotnCharacter *c, Uint16 buttons, Uint16 *sound,
                            bool *changed, SotnWorldState *world, SotnDataView dra)
{
    if (m == NULL || c == NULL || sound == NULL || changed == NULL || world == NULL ||
        m->phase < SOTN_MENU_CLOSED || m->phase > SOTN_MENU_RELICS || m->cursor >= 5 ||
        m->slot >= 7 || m->count > 170 || m->relic_cursor >= 28)
        return SDL_SetError("Inventory menu requires initialized bounded cursors and outputs");
    *sound = 0;
    *changed = false;
    Uint16 tapped = (Uint16)(buttons & (Uint16)~m->previous);
    const Uint16 directions =
        buttons & (SOTN_BUTTON_UP | SOTN_BUTTON_DOWN | SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT);
    if (directions != 0 && directions == (m->previous & (SOTN_BUTTON_UP | SOTN_BUTTON_DOWN |
                                                         SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT)))
    {
        if (++m->repeat >= 24 && (m->repeat % 4) == 0)
            tapped |= directions;
    }
    else
        m->repeat = 0;
    m->previous = buttons;
    ++m->ticks;
    if (tapped & SOTN_BUTTON_START)
    {
        m->phase = m->phase == SOTN_MENU_CLOSED ? SOTN_MENU_MAIN : SOTN_MENU_CLOSED;
        *sound = 0x6ad;
        return true;
    }
    if (m->phase == SOTN_MENU_CLOSED)
        return true;
    if (tapped & SOTN_BUTTON_CANCEL)
    {
        m->phase = m->phase == SOTN_MENU_ITEMS ? SOTN_MENU_EQUIPMENT
                   : m->phase == SOTN_MENU_EQUIPMENT || m->phase == SOTN_MENU_RELICS
                       ? SOTN_MENU_MAIN
                       : SOTN_MENU_CLOSED;
        m->message[0] = '\0';
        *sound = 0x6a4;
        return true;
    }
    if (m->phase == SOTN_MENU_MAIN)
    {
        if (tapped & SOTN_BUTTON_UP)
        {
            m->cursor = m->cursor == 0 ? 4 : (Uint16)(m->cursor - 1);
            *sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_DOWN)
        {
            m->cursor = (Uint16)((m->cursor + 1) % 5);
            *sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_JUMP)
        {
            if (m->cursor == 0 || m->cursor == 2)
            {
                m->phase = m->cursor == 0 ? SOTN_MENU_EQUIPMENT : SOTN_MENU_RELICS;
                m->message[0] = '\0';
                *sound = 0x6ad;
            }
            else
            {
                SDL_strlcpy(m->message, "This menu is not integrated yet.", sizeof(m->message));
                *sound = 0x6a5;
            }
        }
    }
    else if (m->phase == SOTN_MENU_EQUIPMENT)
    {
        if (tapped & SOTN_BUTTON_UP)
        {
            m->slot = m->slot == 0 ? 6 : (Uint8)(m->slot - 1);
            *sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_DOWN)
        {
            m->slot = (Uint8)((m->slot + 1) % 7);
            *sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_JUMP)
        {
            if (!Rows(m, c))
                return false;
            m->phase = SOTN_MENU_ITEMS;
            *sound = 0x6ad;
        }
    }
    else if (m->phase == SOTN_MENU_RELICS)
    {
        const Uint8 previous = m->relic_cursor;
        if ((tapped & SOTN_BUTTON_UP) && m->relic_cursor >= 2)
            m->relic_cursor -= 2;
        if ((tapped & SOTN_BUTTON_DOWN) && m->relic_cursor < 26)
            m->relic_cursor += 2;
        if (tapped & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT))
            m->relic_cursor ^= 1;
        if (m->relic_cursor != previous)
        {
            m->message[0] = '\0';
            *sound = 0x67b;
        }
        const Uint16 id = m->relic_cursor < 23 ? m->relic_cursor : (Uint16)(m->relic_cursor + 2);
        if ((tapped & SOTN_BUTTON_JUMP) && (world->relics[id] & 1))
        {
            if (SotnRelic_Toggle(world, dra, id))
            {
                *changed = true;
                *sound = 0x6ad;
                m->message[0] = '\0';
            }
            else
            {
                SDL_strlcpy(m->message, SDL_GetError(), sizeof(m->message));
                *sound = 0x6a5;
            }
        }
    }
    else
    {
        if (m->count == 0 || m->item_cursor >= m->count)
            return SDL_SetError("Equipment selection has no valid inventory row");
        if (tapped & SOTN_BUTTON_UP)
        {
            m->item_cursor =
                m->item_cursor == 0 ? (Uint16)(m->count - 1) : (Uint16)(m->item_cursor - 1);
            m->message[0] = '\0';
            *sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_DOWN)
        {
            m->item_cursor = (Uint16)((m->item_cursor + 1) % m->count);
            m->message[0] = '\0';
            *sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_JUMP)
        {
            if (SotnCharacter_Equip(c, m->slot, m->rows[m->item_cursor]))
            {
                m->phase = SOTN_MENU_EQUIPMENT;
                *changed = true;
                *sound = 0x6ad;
                m->message[0] = '\0';
            }
            else
            {
                SDL_strlcpy(m->message, SDL_GetError(), sizeof(m->message));
                *sound = 0x6a5;
            }
        }
    }
    return true;
}
