#include "input.h"
#include <sotn/player.h>

_Static_assert(SOTN_BUTTON_START == 0x0800 && SOTN_BUTTON_SELECT == 0x0100 &&
                   SOTN_BUTTON_MIST == 0x0004,
               "US original Start, Select and Mist pad words must remain distinct");

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

static bool TestKeyboard(void)
{
    bool keys[SDL_SCANCODE_COUNT] = {false};
    keys[SDL_SCANCODE_TAB] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == SOTN_BUTTON_SELECT);
    keys[SDL_SCANCODE_TAB] = false;
    keys[SDL_SCANCODE_M] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == SOTN_BUTTON_SELECT);
    keys[SDL_SCANCODE_M] = false;
    keys[SDL_SCANCODE_RETURN] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == SOTN_BUTTON_START);
    keys[SDL_SCANCODE_RETURN] = false;
    keys[SDL_SCANCODE_Z] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == SOTN_BUTTON_LEFT_HAND);
    keys[SDL_SCANCODE_Z] = false;
    keys[SDL_SCANCODE_Q] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == SOTN_BUTTON_MIST);
    keys[SDL_SCANCODE_Q] = false;
    keys[SDL_SCANCODE_RIGHT] = true;
    keys[SDL_SCANCODE_X] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == (SOTN_BUTTON_RIGHT | SOTN_BUTTON_JUMP));
    keys[SDL_SCANCODE_X] = false;
    keys[SDL_SCANCODE_SPACE] = true;
    CHECK(LaboratoryInput_KeyboardButtons(keys) == (SOTN_BUTTON_RIGHT | SOTN_BUTTON_JUMP));
    return true;
}

static void CountWarnings(void *userdata, int category, SDL_LogPriority priority,
                          const char *message)
{
    Uint32 *count = userdata;
    if (category == SDL_LOG_CATEGORY_APPLICATION && priority == SDL_LOG_PRIORITY_WARN &&
        SDL_strstr(message, "input ignored") != NULL)
        ++*count;
}

static bool TestUnsupportedWarning(LaboratoryInput *input)
{
    Uint32 warnings = 0;
    SDL_LogOutputFunction previous = NULL;
    void *previous_userdata = NULL;
    SDL_GetLogOutputFunction(&previous, &previous_userdata);
    SDL_SetLogOutputFunction(CountWarnings, &warnings);
    SDL_Event event = {0};
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.scancode = SDL_SCANCODE_A;
    bool handled = LaboratoryInput_Event(input, &event);
    event.key.repeat = true;
    handled = LaboratoryInput_Event(input, &event) && handled;
    event.key.repeat = false;
    event.key.scancode = SDL_SCANCODE_A;
    handled = LaboratoryInput_Event(input, &event) && handled;
    event.type = SDL_EVENT_GAMEPAD_BUTTON_DOWN;
    event.gbutton.which = SDL_GetGamepadID(input->pad);
    event.gbutton.button = SDL_GAMEPAD_BUTTON_NORTH;
    handled = LaboratoryInput_Event(input, &event) && handled;
    SDL_SetLogOutputFunction(previous, previous_userdata);
    CHECK(handled && warnings == 1);
    return true;
}

static bool TestVirtualPad(LaboratoryInput *input, SDL_JoystickID id)
{
    SDL_Event event = {0};
    event.type = SDL_EVENT_GAMEPAD_ADDED;
    event.gdevice.which = id;
    CHECK(LaboratoryInput_Event(input, &event));
    CHECK(input->pad != NULL);
    CHECK(TestUnsupportedWarning(input));
    SDL_Joystick *joystick = SDL_GetGamepadJoystick(input->pad);
    CHECK(joystick != NULL);
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 16000));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTY, -16000));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == 0);
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, true));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == SOTN_BUTTON_MIST);
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, false));
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
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_NORTH, true));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_EAST, true));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == (SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_RIGHT_HAND));
    input->menu_open = true;
    CHECK((LaboratoryInput_Buttons(input) & SOTN_BUTTON_CANCEL) != 0);
    input->menu_open = false;
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_WEST, false));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_EAST, false));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_SOUTH, true));
    CHECK(SDL_SetJoystickVirtualAxis(joystick, SDL_GAMEPAD_AXIS_LEFTX, 24000));
    SDL_UpdateGamepads();
    CHECK(LaboratoryInput_Buttons(input) == (SOTN_BUTTON_RIGHT | SOTN_BUTTON_JUMP));
    CHECK(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_START, true));
    SDL_UpdateGamepads();
    CHECK((LaboratoryInput_Buttons(input) & SOTN_BUTTON_START) != 0);
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
    const bool passed = id != 0 && TestKeyboard() && TestVirtualPad(&input, id);
    if (id != 0)
        SDL_DetachVirtualJoystick(id);
    LaboratoryInput_Quit(&input);
    SDL_Quit();
    if (!passed)
        return 1;
    SDL_Log("SOTN_INPUT_TEST: PASS");
    return 0;
}
