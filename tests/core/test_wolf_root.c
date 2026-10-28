#include <SDL3/SDL.h>
#include <sotn/wolf_root.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Room
{
    Uint32 calls, effects;
    Uint8 mode;
} Room;
static bool Query(void *context, Sint32 x, Sint32 y, SotnCollider *out)
{
    Room *room = context;
    (void)x;
    ++room->calls;
    if (room->mode == 3)
        return SDL_SetError("Authored room query failure");
    *out = (SotnCollider){0};
    if (room->mode == 1 && y >= 148)
        *out = (SotnCollider){.effects = room->effects, .top = 148 - y};
    if (room->mode == 2)
        out->effects = 1;
    if (room->mode == 4)
        out->top = 1025;
    return true;
}
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static SotnWolfControllerFrame Player(void)
{
    return (SotnWolfControllerFrame){
        .x = 100 * 65536 + 123,
        .y = 120 * 65536 + 456,
        .priority = 144,
        .joint_distance = 14,
        .motion = {.body_angle = 2048, .substep = 1, .animation = {.id = 0xde}}};
}
static bool AllocationAndFlight(SotnDataView dra, SotnDataView executable)
{
    SotnWolfRoot root = {.player_palette = 0x104};
    SotnWolfArticulation b = {0};
    SotnWolfControllerFrame p = Player();
    Room room = {0};
    SotnWolfRootEvents e = {0};
    p.motion.animation.id = 0xe1;
    p.motion.animation.pose = 2;
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, false, dra, (SotnDataView){0}, Query, &room, &e) &&
          !b.parts[0].step && p.joint_distance == 7 && e.image_index == -1 && !room.calls);
    p = Player();
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e));
    CHECK(b.parts[0].step == 1 && b.parts[0].x == 86 * 65536 + 123 &&
          b.parts[0].y == 122 * 65536 + 456 && b.parts[0].bank == 15 && b.parts[0].sprite == 111 &&
          b.parts[0].priority == 142 && b.parts[0].flags == 0x04860000U && !room.calls);
    p.motion.facing_left = true;
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          b.parts[0].x == 114 * 65536 + 123 && p.motion.body_angle == 2048);
    p.x = 257 * 65536 + 123;
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          b.parts[0].x == 271 * 65536 + 123);
    p = Player();
    p.motion.velocity_x = 4 * 65536;
    root.image_cycle = SDL_MAX_SINT32;
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          e.image_index == 2 && root.image_cycle == SDL_MIN_SINT32 && e.rush_image &&
          root.player_palette == 0x810d && root.player_opacity == 223 &&
          root.player_draw_flags == 8 && root.player_blend == 0x70 && b.parts[0].palette == 0x810d);
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          e.image_index == -3);
    return true;
}
static bool FloorAndLedge(SotnDataView dra, SotnDataView executable)
{
    SotnWolfRoot root = {.player_palette = 0x104};
    SotnWolfArticulation b = {0};
    SotnWolfControllerFrame p = Player();
    p.motion.contacts = 1;
    Room room = {0};
    SotnWolfRootEvents e = {0};
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          room.calls == 6 && p.motion.substep == 7 && p.motion.velocity_x == -65536 &&
          !p.motion.velocity_y && !p.motion.animation.pose && !p.motion.animation.remaining);
    p = Player();
    p.motion.contacts = 1;
    room = (Room){.mode = 1, .effects = 0x8001};
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          b.parts[0].sprite == 112 && room.calls > 2 && p.motion.substep == 1);
    p = Player();
    p.motion.contacts = 1;
    p.motion.facing_left = true;
    room.calls = 0;
    CHECK(SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          b.parts[0].sprite == 127);
    return true;
}
static bool Failures(SotnDataView dra, SotnDataView executable)
{
    SotnWolfRoot root = {.player_palette = 0x104}, before = root;
    SotnWolfArticulation b = {0}, body = b;
    SotnWolfControllerFrame p = Player(), player = p;
    SotnWolfRootEvents e = {.image_index = 3, .rush_image = true}, events = e;
    Room room = {0};
    CHECK(!SotnWolfRoot_Tick(&root, &b, &p, true, dra, (SotnDataView){0}, Query, &room, &e));
    CHECK(!SDL_memcmp(&root, &before, sizeof(root)) && !SDL_memcmp(&b, &body, sizeof(b)) &&
          !SDL_memcmp(&p, &player, sizeof(p)) && !SDL_memcmp(&e, &events, sizeof(e)));
    p.motion.contacts = 1;
    room.mode = 2;
    CHECK(!SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e) &&
          room.calls == 4096);
    room = (Room){.mode = 3};
    CHECK(!SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e));
    room = (Room){.mode = 4};
    CHECK(!SotnWolfRoot_Tick(&root, &b, &p, true, dra, executable, Query, &room, &e));
    b.parts[0].step = 1;
    CHECK(!SotnWolfRoot_Tick(&root, &b, &p, false, dra, executable, Query, &room, &e));
    p.motion.animation.id = 0xe1;
    p.motion.animation.pose = 6;
    b.parts[0].step = 0;
    CHECK(!SotnWolfRoot_Tick(&root, &b, &p, false, dra, executable, Query, &room, &e));
    Uint8 *bad = SDL_calloc(1, dra.size);
    CHECK(bad != NULL);
    p = player;
    b = body;
    room = (Room){0};
    const bool rejected = !SotnWolfRoot_Tick(&root, &b, &p, true, (SotnDataView){bad, dra.size},
                                             executable, Query, &room, &e);
    SDL_free(bad);
    CHECK(rejected && !SDL_memcmp(&root, &before, sizeof(root)) &&
          !SDL_memcmp(&b, &body, sizeof(b)) && !SDL_memcmp(&p, &player, sizeof(p)) &&
          !SDL_memcmp(&e, &events, sizeof(e)));
    return true;
}
int main(void)
{
    const size_t size = 0x23220;
    Uint8 *sdk = SDL_calloc(1, size), *data = SDL_calloc(1, 0x11200);
    if (!sdk || !data)
    {
        SDL_free(sdk);
        SDL_free(data);
        return 1;
    }
    for (Uint32 i = 0; i <= 1024; ++i)
        Put16(sdk + 0x1dbcc + i * 2, (Uint16)(i * 4));
    for (Uint32 i = 0; i < 192; ++i)
        Put16(sdk + 0x1e3e0 + i * 2, 4096);
    for (Uint32 i = 0; i < 17; ++i)
    {
        Put32(data + 0x109f8 + i * 4, 0x800b1000U + i * 16);
        for (Uint16 j = 0; j < 8; ++j)
            Put16(data + 0x11000 + i * 16 + j * 2, (Uint16)(111 + i));
    }
    for (Uint32 i = 0; i < 6; ++i)
        Put32(data + 0x10924 + i * 4, 5 + i);
    const SotnDataView dra = {data, 0x11200}, executable = {sdk, size};
    const bool ok = AllocationAndFlight(dra, executable) && FloorAndLedge(dra, executable) &&
                    Failures(dra, executable);
    SDL_free(data);
    SDL_free(sdk);
    return ok ? 0 : 1;
}
