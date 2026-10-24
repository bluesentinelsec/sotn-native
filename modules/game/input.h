#ifndef SOTN_GAME_INPUT_H
#define SOTN_GAME_INPUT_H
#include <SDL3/SDL.h>

typedef struct LaboratoryInput
{
    SDL_Gamepad *pad;
    bool initialized;
    bool warned_unsupported;
} LaboratoryInput;

bool LaboratoryInput_Init(LaboratoryInput *input);
bool LaboratoryInput_Event(LaboratoryInput *input, const SDL_Event *event);
Uint16 LaboratoryInput_KeyboardButtons(const bool *keys);
Uint16 LaboratoryInput_Buttons(const LaboratoryInput *input);
void LaboratoryInput_Quit(LaboratoryInput *input);

#endif
