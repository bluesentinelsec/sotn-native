#include "input.h"
#include <sotn/player.h>

bool LaboratoryInput_Init(LaboratoryInput *input)
{
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD))
        return false;
    input->initialized = true;
    int count = 0;
    SDL_JoystickID *ids = SDL_GetGamepads(&count);
    if (ids == NULL)
        return false;
    if (count > 0)
    {
        input->pad = SDL_OpenGamepad(ids[0]);
        if (input->pad == NULL)
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                        "Cannot open gamepad; keyboard remains available: %s", SDL_GetError());
    }
    SDL_free(ids);
    return true;
}

bool LaboratoryInput_Event(LaboratoryInput *input, const SDL_Event *event)
{
    const bool unsupported_key = event->type == SDL_EVENT_KEY_DOWN && !event->key.repeat &&
                                 event->key.scancode == SDL_SCANCODE_A;
    const bool unsupported_pad = event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN &&
                                 input->pad != NULL &&
                                 event->gbutton.which == SDL_GetGamepadID(input->pad) &&
                                 event->gbutton.button == SDL_GAMEPAD_BUTTON_NORTH;
    if (!input->menu_open && !input->warned_unsupported && (unsupported_key || unsupported_pad))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                    "Ability controls are not implemented in this combat checkpoint; "
                    "input ignored. Z/D or gamepad X/B attack; X/Space or gamepad A jumps.");
        input->warned_unsupported = true;
    }
    if (event->type == SDL_EVENT_GAMEPAD_REMOVED && input->pad != NULL &&
        SDL_GetGamepadID(input->pad) == event->gdevice.which)
    {
        SDL_CloseGamepad(input->pad);
        input->pad = NULL;
    }
    if (event->type == SDL_EVENT_GAMEPAD_ADDED && input->pad == NULL)
    {
        input->pad = SDL_OpenGamepad(event->gdevice.which);
        if (input->pad == NULL)
            SDL_LogWarn(SDL_LOG_CATEGORY_INPUT,
                        "Cannot open gamepad; keyboard remains available: %s", SDL_GetError());
    }
    return true;
}

Uint16 LaboratoryInput_KeyboardButtons(const bool *keys)
{
    Uint16 buttons = 0;
    if (keys[SDL_SCANCODE_TAB] || keys[SDL_SCANCODE_M])
        buttons |= SOTN_BUTTON_SELECT;
    if (keys[SDL_SCANCODE_RETURN])
        buttons |= SOTN_BUTTON_START;
    if (keys[SDL_SCANCODE_LEFT])
        buttons |= SOTN_BUTTON_LEFT;
    if (keys[SDL_SCANCODE_RIGHT])
        buttons |= SOTN_BUTTON_RIGHT;
    if (keys[SDL_SCANCODE_UP])
        buttons |= SOTN_BUTTON_UP;
    if (keys[SDL_SCANCODE_DOWN])
        buttons |= SOTN_BUTTON_DOWN;
    if (keys[SDL_SCANCODE_X] || keys[SDL_SCANCODE_SPACE])
        buttons |= SOTN_BUTTON_JUMP;
    if (keys[SDL_SCANCODE_Z])
        buttons |= SOTN_BUTTON_LEFT_HAND;
    if (keys[SDL_SCANCODE_D])
        buttons |= SOTN_BUTTON_RIGHT_HAND;
    return buttons;
}

Uint16 LaboratoryInput_Buttons(const LaboratoryInput *input)
{
    const bool *keys = SDL_GetKeyboardState(NULL);
    Uint16 buttons = LaboratoryInput_KeyboardButtons(keys);
    if (input->menu_open && keys[SDL_SCANCODE_A])
        buttons |= SOTN_BUTTON_CANCEL;
    if (input->pad != NULL)
    {
        const Sint16 x = SDL_GetGamepadAxis(input->pad, SDL_GAMEPAD_AXIS_LEFTX);
        const Sint16 y = SDL_GetGamepadAxis(input->pad, SDL_GAMEPAD_AXIS_LEFTY);
        if (x < -16000 || SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT))
            buttons |= SOTN_BUTTON_LEFT;
        if (x > 16000 || SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT))
            buttons |= SOTN_BUTTON_RIGHT;
        if (y < -16000 || SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_DPAD_UP))
            buttons |= SOTN_BUTTON_UP;
        if (y > 16000 || SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN))
            buttons |= SOTN_BUTTON_DOWN;
        if (input->menu_open && SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_NORTH))
            buttons |= SOTN_BUTTON_CANCEL;
        if (SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_BACK))
            buttons |= SOTN_BUTTON_SELECT;
        if (SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_START))
            buttons |= SOTN_BUTTON_START;
        if (SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_SOUTH))
            buttons |= SOTN_BUTTON_JUMP;
        if (SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_WEST))
            buttons |= SOTN_BUTTON_LEFT_HAND;
        if (SDL_GetGamepadButton(input->pad, SDL_GAMEPAD_BUTTON_EAST))
            buttons |= SOTN_BUTTON_RIGHT_HAND;
    }
    return buttons;
}

void LaboratoryInput_Quit(LaboratoryInput *input)
{
    if (input->pad != NULL)
        SDL_CloseGamepad(input->pad);
    if (input->initialized)
        SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
    *input = (LaboratoryInput){0};
}
