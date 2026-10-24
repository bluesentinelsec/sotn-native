// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected activation, centering, and coffin clocks from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, dra/62DEC.c:func_80105428
// and 6DF70.c:func_8010E334. Native slots replace memory-card hardware services.
#include <SDL3/SDL.h>
#include <sotn/save_point.h>

static void Idle(SotnSavePoint *point)
{
    point->phase = SOTN_SAVE_POINT_IDLE;
    point->scale = 192;
    point->scatter = 0;
    point->armed = false;
}

bool SotnSavePoint_Reset(SotnSavePoint *point, Uint8 slots, Uint8 slot, Uint16 occupied)
{
    if (point == NULL || slots == 0 || slots > 15 || slot == 0 || slot > slots ||
        (occupied >> slots) != 0)
        return SDL_SetError("Save point requires bounded native slot selection");
    *point = (SotnSavePoint){.slots = slots,
                             .slot = slot,
                             .occupied = occupied,
                             .scale = 192,
                             .armed = true,
                             .yes = true};
    return true;
}

Uint16 SotnSavePoint_Buttons(const SotnSavePoint *point, SotnPlayerState player, Uint16 buttons)
{
    if (point == NULL || point->phase == SOTN_SAVE_POINT_IDLE ||
        (point->phase == SOTN_SAVE_POINT_EXIT && point->scatter >= 64))
        return buttons;
    if (point->phase == SOTN_SAVE_POINT_CENTER)
    {
        const Sint32 x = player.x / 65536;
        if (x < 127)
            return SOTN_BUTTON_RIGHT;
        if (x > 128)
            return SOTN_BUTTON_LEFT;
    }
    return SOTN_BUTTON_UP;
}

bool SotnSavePoint_Menu(const SotnSavePoint *point)
{
    return point != NULL &&
           (point->phase == SOTN_SAVE_POINT_SLOT || point->phase == SOTN_SAVE_POINT_CONFIRM ||
            point->phase == SOTN_SAVE_POINT_SAVED || point->phase == SOTN_SAVE_POINT_ERROR);
}

bool SotnSavePoint_Tick(SotnSavePoint *point, SotnPlayerState player, Uint16 buttons,
                        SotnSavePointAction *out)
{
    if (point == NULL || out == NULL || point->slots == 0 || point->slots > 15 ||
        point->slot == 0 || point->slot > point->slots || point->phase < SOTN_SAVE_POINT_IDLE ||
        point->phase > SOTN_SAVE_POINT_ERROR)
        return SDL_SetError("Save-point tick requires initialized state and an action output");
    const Uint16 tapped = (Uint16)(buttons & (Uint16)~point->previous_buttons);
    point->previous_buttons = buttons;
    ++point->ticks;
    SotnSavePointAction action = {0};
    const bool confirm = (tapped & (SOTN_BUTTON_JUMP | SOTN_BUTTON_START)) != 0;
    const bool cancel = (tapped & SOTN_BUTTON_CANCEL) != 0;
    const Sint32 x = player.x / 65536;
    switch (point->phase)
    {
    case SOTN_SAVE_POINT_IDLE: {
        point->angle = (Uint16)((point->angle + 6) & 4095);
        Uint32 pulse = point->ticks & 127;
        if (pulse & 64)
            pulse = 127 - pulse;
        point->scale = (Uint16)(pulse >= 8 ? 192 : 160 + pulse * 4);
        if ((point->ticks & 127) == 0)
            action.sound = 0x64d;
        if (!(buttons & SOTN_BUTTON_UP))
            point->armed = true;
        if (point->armed && player.hp != 0 && player.step == SOTN_PLAYER_STAND &&
            player.looking_up && x >= 96 && x <= 160 && (buttons & SOTN_BUTTON_UP))
        {
            point->phase = SOTN_SAVE_POINT_CENTER;
            point->armed = false;
            point->yes = true;
            action.heal = true;
        }
        break;
    }
    case SOTN_SAVE_POINT_CENTER:
        point->angle = (Uint16)((point->angle + 7) & 4095);
        point->scale = (Uint16)SDL_min(192, point->scale + 4);
        if (x >= 127 && x <= 128 && player.looking_up)
        {
            point->phase = SOTN_SAVE_POINT_SLOT;
            action.sound = 0x6ad;
        }
        break;
    case SOTN_SAVE_POINT_SLOT:
        if (tapped & SOTN_BUTTON_UP)
        {
            point->slot = point->slot == 1 ? point->slots : (Uint8)(point->slot - 1);
            action.sound = 0x67b;
        }
        if (tapped & SOTN_BUTTON_DOWN)
        {
            point->slot = point->slot == point->slots ? 1 : (Uint8)(point->slot + 1);
            action.sound = 0x67b;
        }
        if (cancel)
            Idle(point);
        else if (confirm)
        {
            point->phase = SOTN_SAVE_POINT_CONFIRM;
            point->yes = true;
            action.sound = 0x6ad;
        }
        break;
    case SOTN_SAVE_POINT_CONFIRM:
        if (tapped & (SOTN_BUTTON_LEFT | SOTN_BUTTON_RIGHT))
        {
            point->yes = (tapped & SOTN_BUTTON_LEFT) != 0;
            action.sound = 0x6a4;
        }
        if (cancel || (confirm && !point->yes))
            Idle(point);
        else if (confirm)
        {
            point->phase = SOTN_SAVE_POINT_GROW;
            point->speed = 8;
            action.sound = 0x633;
        }
        break;
    case SOTN_SAVE_POINT_GROW:
        point->angle = (Uint16)((point->angle + point->speed) & 4095);
        point->scale = (Uint16)SDL_min(256, point->scale + 4);
        if (point->scale == 256)
            point->phase = SOTN_SAVE_POINT_SPIN;
        break;
    case SOTN_SAVE_POINT_SPIN:
        point->speed = (Uint16)SDL_min(128, point->speed + 1);
        point->angle = (Uint16)((point->angle + point->speed) & 4095);
        if (point->speed == 128)
            point->phase = SOTN_SAVE_POINT_SCATTER;
        break;
    case SOTN_SAVE_POINT_SCATTER:
        point->scatter = (Uint16)SDL_min(127, point->scatter + 2);
        point->angle = (Uint16)((point->angle + point->speed) & 4095);
        if (point->scatter == 127 && point->angle >= 256 && point->angle < 384)
            point->phase = SOTN_SAVE_POINT_COFFIN;
        break;
    case SOTN_SAVE_POINT_COFFIN:
        if (point->speed > 64)
            --point->speed;
        else if ((point->angle & 2047) >= 1024)
            point->speed = (Uint16)SDL_max(16, (2048 - (point->angle & 2047)) / 16);
        point->angle = (Uint16)((point->angle + point->speed) & 4095);
        point->scatter = (Uint16)SDL_max(0, point->scatter - 2);
        if (point->speed == 16 && (point->angle & 2047) >= 2032)
        {
            point->angle = 0;
            point->phase = SOTN_SAVE_POINT_WRITE;
            action.sound = 0x6e0;
            action.write = true;
        }
        break;
    case SOTN_SAVE_POINT_WRITE:
        break;
    case SOTN_SAVE_POINT_ERROR:
        if (cancel)
        {
            point->phase = SOTN_SAVE_POINT_EXIT;
            point->scatter = 0;
            point->speed = 8;
        }
        else if (confirm)
        {
            point->phase = SOTN_SAVE_POINT_WRITE;
            action.write = true;
        }
        break;
    case SOTN_SAVE_POINT_SAVED:
        if (confirm || cancel)
        {
            point->phase = SOTN_SAVE_POINT_EXIT;
            point->scatter = 0;
            point->speed = 8;
        }
        break;
    case SOTN_SAVE_POINT_EXIT:
        point->speed = (Uint16)SDL_min(64, point->speed + 2);
        point->angle = (Uint16)((point->angle + point->speed) & 4095);
        if (++point->scatter >= 127)
            Idle(point);
        break;
    default:
        return SDL_SetError("Unsupported save-point phase");
    }
    *out = action;
    return true;
}

bool SotnSavePoint_Complete(SotnSavePoint *point, bool success)
{
    if (point == NULL || point->slots == 0 || point->slots > 15 || point->slot == 0 ||
        point->slot > point->slots || point->phase != SOTN_SAVE_POINT_WRITE)
        return SDL_SetError("Save completion requires a pending save-point write");
    point->phase = success ? SOTN_SAVE_POINT_SAVED : SOTN_SAVE_POINT_ERROR;
    if (success)
        point->occupied |= (Uint16)(1U << (point->slot - 1));
    return true;
}
