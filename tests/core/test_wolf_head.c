#include <SDL3/SDL.h>
#include <sotn/wolf_head.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *bytes, size_t offset, Sint16 value)
{
    const Uint16 bits = (Uint16)value;
    bytes[offset] = (Uint8)bits;
    bytes[offset + 1] = (Uint8)(bits >> 8);
}
static SotnWolfControllerFrame Player(void)
{
    return (SotnWolfControllerFrame){
        .priority = 144,
        .motion = {.body_angle = 2048, .substep = 1, .animation = {.pose = 2, .sprite = 7}}};
}
static bool Bite(SotnDataView dra, SotnDataView executable)
{
    SotnWolfHead h = {0};
    SotnWolfArticulation b = {0};
    b.parts[1].x = 32767 * 65536 + 123;
    b.parts[1].y = 120 * 65536 + 456;
    SotnWolfControllerFrame p = Player();
    SotnWolfHeadEvents e = {0};
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e));
    CHECK(h.pose == 4 && h.remaining == 1 && b.parts[3].step == 1 && b.parts[3].sprite == 72 &&
          b.parts[3].x == b.parts[1].x && b.parts[3].y == b.parts[1].y + 4 * 65536 &&
          b.parts[3].bank == 15 && b.parts[3].texture_offset == 0x7e &&
          b.parts[3].priority == 146 && b.parts[3].width == 10 && b.parts[3].height == 8 &&
          !e.sound && !e.spell);
    CHECK(SotnWolfHead_Animate(&h, &b.parts[3], dra) && h.pose == 4 && h.remaining == -1 &&
          b.parts[3].sprite == 104);
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, true, dra, executable, &e) && h.pose == 0 &&
          !h.remaining && e.sound == 0x6f7 && e.spell == 14 &&
          b.parts[3].x == -32767 * 65536 + 123);
    CHECK(SotnWolfHead_Animate(&h, &b.parts[3], dra) && h.remaining == 2 &&
          b.parts[3].sprite == 100);
    for (Uint16 i = 0; i < 30; ++i)
        CHECK(SotnWolfHead_Animate(&h, &b.parts[3], dra));
    CHECK(h.pose == 4 && h.remaining == -1 && b.parts[3].sprite == 104);
    p.motion.substep = 2;
    p.motion.mode = 4;
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e) && e.spell == 4);
    p.motion.substep = 8;
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, true, dra, executable, &e) && !e.sound &&
          h.remaining == -1);
    return true;
}
static bool TurningAndBreathing(SotnDataView dra, SotnDataView executable)
{
    SotnWolfHead h = {.pose = 4, .remaining = -1};
    SotnWolfArticulation b = {0};
    b.parts[1].x = 100 * 65536 + 123;
    b.parts[1].y = 120 * 65536 + 456;
    b.parts[3] = (SotnWolfPart){.step = 1, .sprite = 72, .hitbox_state = 9};
    SotnWolfControllerFrame p = Player();
    p.motion.substep = 2;
    p.motion.animation.sprite = 33;
    p.fall_water_level = 25;
    SotnWolfHeadEvents e = {0};
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, true, dra, (SotnDataView){0}, &e) && !e.sound &&
          !e.spell && !e.bubble && b.parts[3].sprite == 73 && h.remaining == -1 &&
          b.parts[3].x == b.parts[1].x + 4 * 65536 && b.parts[3].hitbox_state == 9);
    CHECK(SotnWolfHead_Animate(&h, &b.parts[3], dra) && b.parts[3].sprite == 73);
    p.motion.substep = 1;
    b.parts[3].sprite = 72;
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e) && e.bubble &&
          !e.sound && !e.spell && h.breath_timer == 64 &&
          e.bubble_source.x == b.parts[3].x + 4 * 65536 &&
          e.bubble_source.y == b.parts[3].y + 2 * 65536 && !b.parts[3].hitbox_state);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e) && !e.bubble);
    CHECK(!h.breath_timer);
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e) && e.bubble);
    p.motion.facing_left = true;
    h.breath_timer = 0;
    CHECK(SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e) &&
          e.bubble_source.x == b.parts[3].x - 4 * 65536 && e.bubble_source.facing_left);
    return true;
}
static bool Failure(SotnDataView dra, SotnDataView executable)
{
    SotnWolfHead h = {0}, before = h;
    SotnWolfArticulation b = {0}, body = b;
    SotnWolfControllerFrame p = Player();
    SotnWolfHeadEvents e = {.sound = 7}, events = e;
    CHECK(!SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, (SotnDataView){0}, &e));
    CHECK(!SDL_memcmp(&h, &before, sizeof(h)) && !SDL_memcmp(&b, &body, sizeof(b)) &&
          !SDL_memcmp(&e, &events, sizeof(e)));
    CHECK(!SotnWolfHead_Tick(&h, &b, &p, 0x104, false, (SotnDataView){dra.bytes, 0x10b0b},
                             executable, &e));
    h.pose = 5;
    CHECK(!SotnWolfHead_Tick(&h, &b, &p, 0x104, false, dra, executable, &e) && h.pose == 5);
    h = before;
    Uint8 *bad = SDL_calloc(1, dra.size);
    CHECK(bad != NULL);
    bad[0x10aec + 8] = bad[0x10aec + 9] = bad[0x10aec + 10] = 255;
    bad[0x10aec + 11] = 127;
    p.motion.substep = p.motion.mode = 2;
    const bool rejected =
        !SotnWolfHead_Tick(&h, &b, &p, 0x104, false, (SotnDataView){bad, dra.size}, executable, &e);
    SDL_free(bad);
    CHECK(rejected && !SDL_memcmp(&h, &before, sizeof(h)) && !SDL_memcmp(&b, &body, sizeof(b)) &&
          !SDL_memcmp(&e, &events, sizeof(e)));
    return true;
}
int main(void)
{
    Uint8 *data = SDL_calloc(1, 0x10b0c), *sdk = SDL_calloc(1, 0x1e3ce);
    if (!data || !sdk)
    {
        SDL_free(data);
        SDL_free(sdk);
        return 1;
    }
    const Sint16 times[6] = {2, 3, 4, 3, 1, -1};
    for (Uint16 i = 0; i < 6; ++i)
    {
        Put16(data, 0x10abc + i * 4U, times[i]);
        Put16(data, 0x10abe + i * 4U, (Sint16)(100 + i));
    }
    data[0x10ad4] = 2;
    const bool ok =
        Bite((SotnDataView){data, 0x10b0c}, (SotnDataView){sdk, 0x1e3ce}) &&
        TurningAndBreathing((SotnDataView){data, 0x10b0c}, (SotnDataView){sdk, 0x1e3ce}) &&
        Failure((SotnDataView){data, 0x10b0c}, (SotnDataView){sdk, 0x1e3ce});
    SDL_free(data);
    SDL_free(sdk);
    return ok ? 0 : 1;
}
