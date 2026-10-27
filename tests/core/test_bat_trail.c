#include <SDL3/SDL.h>
#include <sotn/bat_trail.h>

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
    SotnBatTrail trail = {.x = 100 * 65536 + 123, .y = 120 * 65536 + 456};
    SotnBatTrailInput in = {.frame = 6,
                            .bank = 13,
                            .priority = 144,
                            .rotation = 384,
                            .wing_smashing = true,
                            .facing_left = true,
                            .rotated = true};
    CHECK(SotnBatTrail_Tick(&trail, in));
    CHECK(trail.initialized && !trail.removed && trail.scale == 256 && trail.opacity == 128 &&
          trail.frame == 0x8006 && trail.bank == 13 && trail.priority == 142 &&
          trail.rotation == 384 && trail.facing_left && trail.rotated);
    in.frame = 7;
    in.priority = 160;
    in.rotation = -128;
    in.facing_left = false;
    in.rotated = false;
    CHECK(SotnBatTrail_Tick(&trail, in));
    CHECK(trail.frame == 0x8007 && trail.scale == 248 && trail.opacity == 123 &&
          trail.priority == 142 && trail.rotation == 384 && trail.facing_left && trail.rotated &&
          trail.x == 100 * 65536 + 123 && trail.y == 120 * 65536 + 456);
    for (Uint16 i = 0; i < 24; ++i)
        CHECK(SotnBatTrail_Tick(&trail, in));
    CHECK(!trail.removed && trail.scale == 56 && trail.opacity == 3);
    CHECK(SotnBatTrail_Tick(&trail, in) && trail.removed && trail.scale == 48);
    CHECK(!SotnBatTrail_Tick(&trail, in));
    trail = (SotnBatTrail){0};
    in.wing_smashing = false;
    in.bank = 1;
    CHECK(SotnBatTrail_Tick(&trail, in) && trail.removed && !trail.initialized);
    in.wing_smashing = true;
    in.bank = 13;
    trail = (SotnBatTrail){0};
    CHECK(SotnBatTrail_Tick(&trail, in));
    in.wing_smashing = false;
    in.bank = 1;
    CHECK(SotnBatTrail_Tick(&trail, in) && trail.removed);
    in.wing_smashing = true;
    in.bank = 1;
    trail = (SotnBatTrail){0};
    CHECK(!SotnBatTrail_Tick(&trail, in) && !trail.initialized);
    CHECK(!SotnBatTrail_Tick(NULL, in));
    // Unrotated geometry needs no SDK input; negative scaled distances round toward -infinity.
    trail = (SotnBatTrail){
        .initialized = true, .scale = 128, .opacity = 64, .bank = 13, .priority = 142};
    SotnPlayerEffectQuad q = {0};
    const SotnPlayerImage image = {21, 33, -11, -21};
    CHECK(SotnBatTrail_Quad(&trail, image, (SotnDataView){0}, 100, 120, &q));
    CHECK(q.x[0] == 94 && q.x[1] == 105 && q.y[0] == 109 && q.y[2] == 126 && q.u[0] == 4 &&
          q.u[1] == 25 && q.v[0] == 1 && q.v[2] == 34 && q.page == 0x18 && q.palette == 0x102 &&
          q.priority == 142 && q.draw_mode == 0x25 && q.red[0] == 64 && q.green[3] == 64);
    trail.facing_left = true;
    CHECK(SotnBatTrail_Quad(&trail, image, (SotnDataView){0}, 100, 120, &q));
    CHECK(q.x[0] == 95 && q.x[1] == 106 && q.u[0] == 24 && q.u[1] == 3);
    const SotnPlayerEffectQuad before = q;
    trail.rotated = true;
    CHECK(!SotnBatTrail_Quad(&trail, image, (SotnDataView){0}, 100, 120, &q));
    CHECK(!SDL_memcmp(&before, &q, sizeof(q)));
    trail.removed = true;
    CHECK(!SotnBatTrail_Quad(&trail, image, (SotnDataView){0}, 100, 120, &q));
    return 0;
}
