#include <SDL3/SDL.h>
#include <sotn/wolf_root_effect.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    SotnWolfRootEffect e = {0};
    e.quads[0].palette = 321;
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = 0}, 100, 120, 0, 144,
                                  false));
    CHECK(e.images[0].state == 1 && e.quads[0].draw_mode == 0x13f && e.quads[0].palette == 321 &&
          e.quads[0].page == 0x118 && e.quads[0].u[1] == 63 && e.quads[0].v[2] == 255);
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = 0}, 100, 120, 0, 144,
                                  false));
    CHECK(e.images[0].state == 3 && e.images[0].timer == 32 && !e.images[0].facing_left &&
          e.quads[0].x[0] == 56 && e.quads[0].x[1] == 119 && e.quads[0].y[0] == 96 &&
          e.quads[0].y[2] == 159 && e.quads[0].draw_mode == 0x137 && e.quads[0].green[0] == 128);
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1}, 200, 200, 0, 150,
                                  true));
    CHECK(e.quads[0].x[0] == 56 && e.quads[0].green[2] == 116 && e.quads[0].green[3] == 122 &&
          e.quads[0].priority == 144 && !e.images[0].facing_left);
    for (Uint16 i = 0; i < 31; ++i)
        CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1}, 200, 200, 0, 150,
                                      true));
    CHECK(e.images[0].state == 1 && !e.images[0].timer && !(e.quads[0].draw_mode & 8U));
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1}, 200, 200, 0, 150,
                                  true));
    CHECK(e.quads[0].draw_mode & 8U);
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = 0}, 200, 200, 0, 150,
                                  true));
    CHECK(e.images[0].timer == 32 && e.images[0].facing_left && e.quads[0].x[0] == 180);
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -3, .rush_image = true},
                                  100, 120, 3 * 65536 + 256, 144, false));
    CHECK(e.quads[5].draw_mode == 0x335 && e.quads[5].x[0] == 56 && e.quads[5].priority == 144 &&
          e.quads[5].green[0] == 127 && e.quads[5].green[1] == 129 && e.quads[5].blue[0] == 255 &&
          e.quads[5].blue[1] == 127 && e.quads[5].blue[2] == 255);
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1, .rush_image = true},
                                  200, 120, 3 * 65536 + 256, 144, true));
    CHECK(e.quads[5].x[0] == 180 && e.quads[5].green[2] == 129 && e.quads[5].green[3] == 127 &&
          e.quads[5].blue[0] == 129 && e.quads[5].blue[1] == 255 && e.quads[5].u[0] == 0);
    CHECK(SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1, .rush_image = true},
                                  SDL_MIN_SINT16, SDL_MAX_SINT16, 3 * 65536 - 256, 144, false));
    CHECK(e.quads[5].x[0] == 32724 && e.quads[5].green[0] == 129 && e.quads[5].green[1] == 127);
    const SotnWolfRootEffect before = e;
    CHECK(
        !SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = 5}, 0, 0, 0, 144, false));
    CHECK(!SDL_memcmp(&e, &before, sizeof(e)));
    CHECK(!SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1}, 0, 0,
                                   SDL_MIN_SINT32, 144, false));
    e.images[0].state = 3;
    e.images[0].timer = 0;
    const SotnWolfRootEffect malformed = e;
    CHECK(!SotnWolfRootEffect_Tick(&e, (SotnWolfRootEvents){.image_index = -1}, 0, 0, 0, 144,
                                   false) &&
          !SDL_memcmp(&e, &malformed, sizeof(e)));
    return 0;
}
