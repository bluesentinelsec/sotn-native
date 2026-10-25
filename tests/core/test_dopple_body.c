#include <SDL3/SDL.h>
#include <sotn/dopple_body.h>

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
static void Put16(Uint8 *image, size_t at, Uint16 value)
{
    image[at] = (Uint8)value;
    image[at + 1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *image, size_t at, Uint32 value)
{
    Put16(image, at, (Uint16)value);
    Put16(image, at + 2, (Uint16)(value >> 16));
}
static void Fixture(Uint8 *image)
{
    /* Synthetic geometry, transition profiles and repeated animation poses. */
    for (size_t i = 0; i < 4; ++i)
    {
        const Sint16 x = i == 2 ? 2 : i == 3 ? -2 : 0;
        Put16(image, 0x126c + i * 2, (Uint16)-9);
        Put16(image, 0x1274 + i * 2, i == 0 ? 12 : 8);
        Put16(image, 0x128c + i * 4, (Uint16)x);
        Put16(image, 0x128e + i * 4, (Uint16)-9);
        Put16(image, 0x129c + i * 4, (Uint16)x);
        Put16(image, 0x129e + i * 4, i == 0 ? 12 : 8);
        image[0x1318 + i * 2] = i == 0 ? 0 : i == 1 ? 4 : i == 2 ? 1 : 3;
        image[0x1319 + i * 2] = (Uint8)(10 + i);
    }
    for (size_t i = 0; i < 14; ++i)
    {
        const Sint16 y = (Sint16)(7 - (Sint32)(i % 7) * 2);
        Put16(image, 0x12ac + i * 4, i < 7 ? 3 : (Uint16)-3);
        Put16(image, 0x12ae + i * 4, (Uint16)y);
        if (i < 7)
            Put16(image, 0x127c + i * 2, (Uint16)y);
    }
    for (size_t i = 0; i < 10; ++i)
        image[0x1320 + i] = (Uint8)(i + 2);
    Put16(image, 0x1334, 1);
    Put16(image, 0x1336, 15);
    Put16(image, 0x1338, 0);
    Put16(image, 0x133a, 16);
    for (size_t i = 0; i < 220; ++i)
        Put32(image, 0x3cb4 + i * 4, 0x80180100);
    for (size_t i = 0; i < 32; ++i)
    {
        Put16(image, 0x100 + i * 4, 1);
        Put16(image, 0x102 + i * 4, 1);
    }
    Put16(image, 0x180, 0xffff);
}
static bool Settle(SotnDataView image, const SotnCollisionMap *map, SotnDoppleBody *body)
{
    CHECK(SotnDoppleBody_Init(image, 96 * 65536, 132 * 65536, false, body));
    CHECK(body->step == 4 && body->animation.id == 28 && body->timers[5] == 8 &&
          body->timers[15] == 48);
    for (size_t i = 0; i < 30; ++i)
        CHECK(SotnDoppleBody_Tick(image, map, 0, 0, body));
    CHECK(body->step == 1 && body->motion.y == 152 * 65536 && body->motion.vx == 0);
    return true;
}
static bool Request(const SotnDoppleBody *body, Uint16 kind, Uint16 id, Uint16 parameter)
{
    for (size_t i = 0; i < body->request_count; ++i)
        if (body->requests[i].kind == kind && body->requests[i].id == id &&
            body->requests[i].parameter == parameter)
            return true;
    return false;
}
static bool Tests(void)
{
    Uint8 image[0x4200] = {0}, tiles[16 * 16 * 2] = {0}, types[2] = {0, 3};
    Fixture(image);
    for (size_t i = 16 * 10; i < 16 * 16; ++i)
        tiles[i * 2] = 1;
    const SotnDataView overlay = {image, sizeof(image)};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16};
    SotnDoppleBody body = {0};
    CHECK(Settle(overlay, &map, &body));
    const Sint32 start = body.motion.x;
    for (size_t i = 0; i < 10; ++i)
    {
        CHECK(SotnDoppleBody_Tick(overlay, &map, 0x2000, 0, &body));
        if (i == 0)
        {
            CHECK(body.request_count == 1 && body.requests[0].x == start &&
                  body.requests[0].y == 152 * 65536 && !body.requests[0].facing_left);
        }
    }
    CHECK(body.step == 2 && body.motion.x == start + 15 * 65536 && body.motion.vx == 98304);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x8000, 0, &body));
    CHECK(body.facing_left && body.motion.vx == -98304);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0, 0, &body) && body.step == 1);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x40, 0, &body));
    CHECK(body.step == 5 && body.motion.vy == -319488 && body.animation.id == 22);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0, 0, &body));
    CHECK(body.motion.vy == -54272);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x40, 0, &body));
    CHECK(body.motion.vy == -278528 && (body.jump_flags & 0x21) == 0x21 && body.animation.id == 32);
    for (size_t i = 0; i < 100; ++i)
        CHECK(SotnDoppleBody_Tick(overlay, &map, 0, 0, &body));
    CHECK(body.step == 1 && (body.motion.contact & 1));
    CHECK(Settle(overlay, &map, &body));
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x80, 0, &body));
    CHECK(body.substep == 0x41 && body.attack_state == 0x8002 && body.timers[9] == 4 &&
          body.animation.id == 0x41 && Request(&body, SOTN_DOPPLE_FACTORY, 48, 0));
    for (size_t i = 0; i < 40; ++i)
        CHECK(SotnDoppleBody_Tick(overlay, &map, 0, 0, &body));
    CHECK(body.attack_state == 0 && body.step == 1 && body.substep == 0);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x20, 0, &body));
    CHECK(body.substep == 0x42 && body.animation.id == 0xa7 &&
          Request(&body, SOTN_DOPPLE_FACTORY, 48, 1));
    CHECK(Settle(overlay, &map, &body));
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x1080, 0, &body));
    CHECK(body.timers[10] == 4 && Request(&body, SOTN_DOPPLE_FACTORY, 3, 0));
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0, 0, &body));
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x1080, 3, &body));
    CHECK(!Request(&body, SOTN_DOPPLE_FACTORY, 3, 0));
    CHECK(Settle(overlay, &map, &body));
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x4000, 0, &body));
    CHECK(body.step == 3 && body.substep == 1 && body.animation.id == 12 &&
          body.motion.status == 0x20);
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0, 0, &body));
    CHECK(body.substep == 2 && body.animation.id == 19 && body.animation.pose == 1);
    CHECK(Settle(overlay, &map, &body));
    CHECK(SotnDoppleBody_Tick(overlay, &map, 0x10, 0, &body));
    CHECK(body.substep == 3 && body.motion.vx == -229376 && body.animation.id == 219);
    CHECK(Request(&body, SOTN_DOPPLE_FACTORY, 0, 0));
    const Sint32 x = body.motion.x;
    const Uint16 pose = body.animation.pose, pressed = body.pressed;
    CHECK(!SotnDoppleBody_Tick(overlay, &map, 8, 0, &body));
    CHECK(body.motion.x == x && body.animation.pose == pose && body.pressed == pressed);
    CHECK(!SotnDoppleBody_Tick((SotnDataView){0}, &map, 0, 0, &body));
    CHECK(!SotnDoppleBody_Tick(overlay, NULL, 0, 0, &body));
    CHECK(!SotnDoppleBody_Tick(overlay, &map, 0, 17, &body));
    CHECK(!SotnDoppleBody_Tick(overlay, &map, 0, 0, NULL));
    CHECK(!SotnDoppleBody_Init(overlay, 0, 0, false, NULL));
    CHECK(body.motion.x == x && body.animation.pose == pose && body.pressed == pressed);
    return true;
}
int main(void)
{
    if (!Tests())
        return 1;
    SDL_Log("SOTN_DOPPLE_BODY_TEST: PASS");
    return 0;
}
