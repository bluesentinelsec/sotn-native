#include <SDL3/SDL.h>
#include <sotn/castle_map.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static Uint8 blueprint[32768], common[0x42000], dra[0x2c10];
static Uint16 frame[SOTN_FRAME_PIXELS];
int main(void)
{
    SDL_memset(blueprint, 0x22, sizeof(blueprint));
    common[0x41800 + 2] = 31; // Original revealed color 1 (red in this synthetic palette).
    common[0x41800 + 6] = 0xe0;
    common[0x41800 + 7] = 3; // Purchased color 3 = green.
    blueprint[14 * 128 + 4] = 0x20;
    const Uint8 passage[5] = {2, 3, 2, 129, 136};
    SDL_memcpy(dra + 0x2bc0, passage, 5);
    SotnMapRenderer *r = NULL;
    CHECK(SotnMapRenderer_Create((SotnDataView){blueprint, sizeof(blueprint)},
                                 (SotnDataView){common, sizeof(common)},
                                 (SotnDataView){dra, sizeof(dra)}, &r));
    SotnWorldState s = {0};
    CHECK(SotnWorld_Visit(&s, 2, 3, false));
    CHECK(SotnMapRenderer_Draw(r, &s, false, 64, frame, SOTN_FRAME_PIXELS));
    CHECK(frame[13 * 256 + 8] == 31 && frame[13 * 256 + 4] == 0);
    CHECK(frame[15 * 256 + 8] == 0);
    s.flags[SOTN_LAB_SECRET_WALL] = 1;
    CHECK(SotnMapRenderer_Draw(r, &s, false, 64, frame, SOTN_FRAME_PIXELS));
    CHECK(frame[15 * 256 + 8] == 31);
    s.map[0] = 0;
    s.map[3 * 16] |= 2; // Purchased cell (3,3) does not add to explored count.
    CHECK(SotnMapRenderer_Draw(r, &s, false, 64, frame, SOTN_FRAME_PIXELS));
    CHECK(frame[13 * 256 + 13] == 0x3e0 && frame[13 * 256 + 12] == 31 && frame[13 * 256 + 8] == 31);
    frame[0] = 0x1234;
    CHECK(!SotnMapRenderer_Draw(r, &s, false, 65, frame, SOTN_FRAME_PIXELS) && frame[0] == 0x1234);
    ++s.rooms;
    CHECK(!SotnMapRenderer_Draw(r, &s, false, 64, frame, SOTN_FRAME_PIXELS) && frame[0] == 0x1234);
    --s.rooms;
    CHECK(SotnMapRenderer_Draw(r, &s, false, 0, frame, SOTN_FRAME_PIXELS) && frame[0] == 0x1234);
    SDL_memset(&s, 0, sizeof(s));
    CHECK(SotnWorld_Visit(&s, 63, 63, true));
    CHECK(SotnMapRenderer_Draw(r, &s, true, 64, frame, SOTN_FRAME_PIXELS));
    CHECK(frame[239 * 256 + 255] == 31);
    CHECK(!SotnMapRenderer_Draw(r, &s, false, 1, frame, 1));
    SotnMapRenderer_Destroy(r);
    r = NULL;
    dra[0x2bc0] = 64;
    CHECK(!SotnMapRenderer_Create((SotnDataView){blueprint, sizeof(blueprint)},
                                  (SotnDataView){common, sizeof(common)},
                                  (SotnDataView){dra, sizeof(dra)}, &r) &&
          r == NULL);
    SotnMapRenderer_Destroy(NULL);
    SDL_Log("CASTLE_MAP: PASS");
    return 0;
}
