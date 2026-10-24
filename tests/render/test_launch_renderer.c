#include <SDL3/SDL.h>
#include <sotn/launch_renderer.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static Uint8 common[0x42000] = {0};
static Uint16 frame[SOTN_FRAME_PIXELS] = {0};
int main(void)
{
    SDL_memset(common, 0x11, 0x40000);
    common[0x412c2] = 255;
    common[0x412c3] = 127;
    SotnLaunchSlot slots[15] = {{0}};
    slots[14] = (SotnLaunchSlot){
        .exists = true, .valid = true, .level = 5, .hp = 87, .room = 23, .stage = 12};
    SotnLaunchMenu m = {.slot = 15};
    CHECK(SotnLaunchRenderer_Draw((SotnDataView){common, sizeof(common)}, m, slots, NULL, frame,
                                  SOTN_FRAME_PIXELS));
    CHECK(frame[12 * 256 + 48] == 0x7fff && frame[12 * 256 + 8] != 0x7fff);
    m.phase = SOTN_LAUNCH_SLOTS;
    CHECK(SotnLaunchRenderer_Draw((SotnDataView){common, sizeof(common)}, m, slots, NULL, frame,
                                  SOTN_FRAME_PIXELS));
    CHECK(frame[188 * 256 + 16] == 0x7fff);
    m.phase = SOTN_LAUNCH_ERROR;
    CHECK(SotnLaunchRenderer_Draw((SotnDataView){common, sizeof(common)}, m, slots, "Missing save",
                                  frame, SOTN_FRAME_PIXELS));
    frame[0] = 123;
    CHECK(!SotnLaunchRenderer_Draw((SotnDataView){common, sizeof(common)}, m, slots, "bad\ntext",
                                   frame, SOTN_FRAME_PIXELS) &&
          frame[0] == 123);
    CHECK(!SotnLaunchRenderer_Draw((SotnDataView){common, sizeof(common) - 1}, m, slots, NULL,
                                   frame, SOTN_FRAME_PIXELS) &&
          frame[0] == 123);
    m.slot = 0;
    CHECK(!SotnLaunchRenderer_Draw((SotnDataView){common, sizeof(common)}, m, slots, NULL, frame,
                                   SOTN_FRAME_PIXELS) &&
          frame[0] == 123);
    SDL_Log("LAUNCH_RENDERER: PASS");
    return 0;
}
