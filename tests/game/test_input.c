#include "input.h"
#include <sotn/player.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_LogError(SDL_LOG_CATEGORY_TEST, "Line %d: %s: %s", __LINE__, #condition,           \
                         SDL_GetError());                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static bool TestVirtualPad(LaboratoryInput *input, SDL_JoystickID id)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_GAMEPAD_ADDED;
    event.gdevice.which = id;
    CHECK(LaboratoryInput_Event(input, &event));
    CHECK(input->pad != NULL);
    SDL_Joystick *joystick = SDL_GetGamepadJoystick(input->pad);
    CHECK(joystick != NULL);
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 16000));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTY, -16000));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == 0);
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 24000));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTY, -24000));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) ==
          (SOTN_BUTTON_RIGHT | SOTN_BUTTON_UP | SOTN_BUTTON_JUMP));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 0));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTY, 0));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, false));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_DPAD_LEFT, true));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_DPAD_DOWN, true));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == (SOTN_BUTTON_LEFT | SOTN_BUTTON_DOWN));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_DPAD_LEFT, false));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_DPAD_DOWN, false));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_WEST, true));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == 0x0080);
    event.type = SDL_EVENT_GAMEPAD_REMOVED;
    CHECK(LaboratoryInput_Event(input, &event));
    CHECK(input->pad == NULL && LaboratoryInput_Buttons(input) == 0);
    return true;
}

int main(void)
{
    if (!SDL_Init(SDL_INIT_GAMEPAD))
        return 1;
    SDL_VirtualJoystickDesc description = {0};
    SDL_INIT_INTERFACE(&description);
    description.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    description.naxes = SDL_GAMEPAD_AXIS_COUNT;
    description.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    description.axis_mask = (1U << SDL_GAMEPAD_AXIS_COUNT) - 1U;
    description.button_mask = (1U << SDL_GAMEPAD_BUTTON_COUNT) - 1U;
    description.name = "SOTN synthetic controller";
    const SDL_JoystickID id = SDL_AttachVirtualJoystick(&description);
    LaboratoryInput input = {0};
    input.initialized = true;
    const bool passed = id != 0 && TestVirtualPad(&input, id);
    if (id != 0)
        SDL_DetachVirtualJoystick(id);
    LaboratoryInput_Quit(&input);
    SDL_Quit();
    if (!passed)
        return 1;
    SDL_Log("SOTN_INPUT_TEST: PASS");
    return 0;
}
