#include <SDL3/SDL.h>
#include <sotn/water_volume.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put(Uint8 *p, Sint16 v)
{
    const Uint16 bits = (Uint16)v;
    p[0] = (Uint8)bits;
    p[1] = (Uint8)(bits >> 8);
}
static bool Queries(void)
{
    Uint8 rows[32] = {0}, aspects[16] = {0};
    Put(aspects + 2, 3);
    Put(rows, 100);
    Put(rows + 2, 50);
    Put(rows + 6, 100);
    Put(rows + 8, 200);
    Put(rows + 16, 90);
    Put(rows + 18, 200);
    Put(rows + 20, 1);
    Put(rows + 22, 80);
    Put(rows + 24, 200);
    SotnWaterVolumeInput in = {.volumes = {rows, sizeof(rows)},
                               .aspects = {aspects, sizeof(aspects)},
                               .player_x = 110,
                               .player_y = 100,
                               .parameters = 0x200};
    SotnWaterVolume out = {0};
    CHECK(SotnWaterVolume_Query(in, &out) && out.contact && out.raw == 0x8019 && out.feet == 25 &&
          out.waist == 9 && !out.head && !out.row && out.left == 10 && out.right == 40 &&
          out.body_height == 40 && out.foot_offset == 25);
    const Uint32 states[] = {0, 0x20, 4, 0x24, 1, 2, 0x21, 0x25};
    for (Uint16 i = 0; i < SDL_arraysize(states); ++i)
    {
        in.status = states[i];
        CHECK(SotnWaterVolume_Query(in, &out));
        const bool small = (in.status & 3U) && !(in.status & 0x20U),
                   crouch = (in.status & 0x20U) != 0, wolf = (in.status & 4U) != 0;
        CHECK(out.feet == (small ? 5 : 25) && out.foot_offset == (small ? 5 : 25));
        CHECK(out.body_height == (small ? 12 : crouch && wolf ? 10 : crouch || wolf ? 20 : 40));
        const Uint16 waist = small            ? 6
                             : crouch && wolf ? 4
                             : crouch || wolf ? 8
                                              : 16,
                     head = small            ? 16
                            : crouch && wolf ? 12
                            : crouch || wolf ? 24
                                             : 48;
        CHECK(out.waist == (out.feet > waist ? out.feet - waist : 0) &&
              out.head == (out.feet > head ? out.feet - head : 0));
    }
    in.status = 0;
    in.player_y = 75;
    CHECK(SotnWaterVolume_Query(in, &out) && out.contact && out.raw == 0x8000 && !out.feet &&
          !out.waist && !out.head);
    in.player_y = 124;
    CHECK(SotnWaterVolume_Query(in, &out) && out.feet == 49 && out.waist == 33 && out.head == 1);
    in.player_x = 150;
    in.player_y = 100;
    CHECK(SotnWaterVolume_Query(in, &out) && out.row == 8 && out.left == 60 && out.right == 140 &&
          out.feet == 65);
    Put(aspects + 2, -3);
    CHECK(SotnWaterVolume_Query(in, &out) && out.row == 8 && out.feet == 25);
    in.player_y = 175;
    CHECK(SotnWaterVolume_Query(in, &out) && !out.contact && !out.raw && !out.left && !out.right &&
          !out.feet && !out.waist && !out.head);
    Put(rows, (Sint16)-32760);
    Put(rows + 2, 24);
    in.parameters = 0x100;
    in.player_x = 32760;
    in.scroll_x = 20;
    in.player_y = 100;
    CHECK(SotnWaterVolume_Query(in, &out) && out.contact && out.x == -32756 && out.left == 4 &&
          out.right == 20);
    const SotnWaterVolume saved = out;
    in.parameters = 0;
    CHECK(!SotnWaterVolume_Query(in, &out) && !SDL_memcmp(&saved, &out, sizeof(out)));
    in.parameters = 0x102;
    CHECK(!SotnWaterVolume_Query(in, &out) && !SDL_memcmp(&saved, &out, sizeof(out)));
    in.parameters = 0x100;
    Put(rows + 4, 8);
    CHECK(!SotnWaterVolume_Query(in, &out) && !SDL_memcmp(&saved, &out, sizeof(out)));
    in.player_x = 0;
    in.scroll_x = 0;
    CHECK(SotnWaterVolume_Query(in, &out) && !out.contact);
    return true;
}
int main(void)
{
    if (!Queries())
        return 1;
    SDL_Log("WATER_VOLUME: PASS authored geometry/edge/slope/form/wrap/validation contexts");
    return 0;
}
