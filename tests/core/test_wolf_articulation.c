#include <SDL3/SDL.h>
#include <sotn/wolf_articulation.h>
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
        .x = 100 * 65536 + 123,
        .y = 120 * 65536 + 456,
        .priority = 144,
        .mp = 6,
        .motion = {.body_angle = 2048, .substep = 1, .animation = {.pose = 2, .sprite = 7}}};
}
static bool LinkedParts(SotnDataView dra, SotnDataView executable)
{
    SotnWolfArticulation body = {0};
    body.parts[0] = (SotnWolfPart){.x = 80 * 65536 + 321, .y = 120 * 65536 + 654};
    body.parts[3] = (SotnWolfPart){.x = 105 * 65536 + 11, .y = 117 * 65536 + 22};
    const SotnWolfPart root = body.parts[0], head = body.parts[3];
    SotnWolfControllerFrame p = Player();
    SotnWolfArticulationEvents e = {0};
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e));
    CHECK(body.parts[1].x == p.x && body.parts[1].y == p.y && body.parts[1].step == 1 &&
          body.parts[1].bank == 15 && body.parts[1].sprite == 1 &&
          body.parts[1].texture_offset == 0x7e && body.parts[1].pivot_y == 9 &&
          body.parts[1].hitbox_x == -4 && body.parts[1].width == 20 && !e.rush_attack);
    CHECK(SotnWolfArticulation_Tick(&body, &p, 19, 0x104, dra, executable, &e));
    CHECK(body.parts[2].x == root.x && body.parts[2].y == root.y && body.parts[2].sprite == 35 &&
          body.parts[2].priority == 142 && body.parts[2].pivot_x == 1 &&
          body.parts[2].pivot_y == 8);
    for (Uint16 slot = 21; slot <= 30; ++slot)
        CHECK(SotnWolfArticulation_Tick(&body, &p, slot, 0x104, dra, executable, &e));
    CHECK(!SDL_memcmp(&body.parts[0], &root, sizeof(root)) &&
          !SDL_memcmp(&body.parts[3], &head, sizeof(head)));
    CHECK(body.wave == 0 && body.parts[4].sprite == 100 && body.parts[12].sprite == 108 &&
          body.parts[13].sprite == 80 && body.parts[13].x == root.x + 8 * 65536 &&
          body.parts[13].y == root.y + 3 * 65536 && body.parts[13].priority == 139);
    for (Uint16 slot = 21; slot <= 29; ++slot)
        CHECK(SotnWolfArticulation_Tick(&body, &p, slot, 0x104, dra, executable, &e));
    CHECK(body.wave == 24);
    p.motion.substep = 2;
    p.motion.mode = 0;
    p.motion.animation.sprite = 34;
    p.motion.facing_left = true;
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e));
    CHECK(SotnWolfArticulation_Tick(&body, &p, 19, 0x104, dra, executable, &e));
    CHECK(SotnWolfArticulation_Tick(&body, &p, 30, 0x104, dra, executable, &e));
    CHECK(body.parts[1].sprite == 32 && body.parts[2].sprite == 66 && body.parts[13].sprite == 82 &&
          body.parts[13].x == root.x + 5 * 65536 && body.parts[13].priority == 149);
    p.motion.substep = 1;
    CHECK(SotnWolfArticulation_Tick(&body, &p, 19, 0x104, dra, executable, &e) &&
          body.parts[2].sprite == 66);
    return true;
}
static bool PosesAndFeedback(SotnDataView dra, SotnDataView executable)
{
    SotnWolfArticulation body = {0};
    body.parts[0] = (SotnWolfPart){.x = 80 * 65536 + 321, .y = 120 * 65536 + 654};
    SotnWolfControllerFrame p = Player();
    p.motion.substep = p.motion.mode = 2;
    p.motion.body_angle = 2560;
    p.motion.contacts = 0x8000;
    SotnWolfArticulationEvents e = {0};
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e) &&
          body.parts[1].y == p.y + 2 * 65536 && body.parts[1].rotation == -448);
    CHECK(SotnWolfArticulation_Tick(&body, &p, 19, 0x104, dra, executable, &e) &&
          body.parts[2].y == body.parts[0].y + 3 * 65536 && body.parts[2].rotation == -256);
    p.motion.velocity_x = 4 * 65536;
    body.parts[1].hit_flags = 1;
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e) && e.rush_attack &&
          !e.mp_spent && p.mp == 6 && p.motion.mode == 3 && p.motion.animation.id == 0xe0 &&
          p.turn_timer == 24 && !body.parts[1].hit_flags);
    CHECK(SotnWolfArticulation_Tick(&body, &p, 19, 0x104, dra, executable, &e) &&
          body.parts[2].y == body.parts[0].y + 4 * 65536 && body.parts[2].blend == 0x70 &&
          body.parts[2].opacity == 239 && body.parts[2].draw_flags == 12);
    p.mp = 7;
    body.parts[1].hit_flags = 2;
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e) && e.rush_attack &&
          e.mp_spent == 7 && p.mp == 0);
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e) && !e.mp_spent);
    p.motion.mode = 4;
    body.parts[1].hit_flags = 1;
    CHECK(SotnWolfArticulation_Tick(&body, &p, 18, 0x104, dra, executable, &e) && !e.rush_attack);
    return true;
}
static bool Failure(SotnDataView dra, SotnDataView executable)
{
    SotnWolfArticulation b = {.wave = 123};
    SotnWolfControllerFrame p = Player();
    SotnWolfArticulationEvents e = {.mp_spent = 11, .rush_attack = true};
    const SotnWolfArticulation before = b;
    const SotnWolfControllerFrame player = p;
    const SotnWolfArticulationEvents events = e;
    CHECK(!SotnWolfArticulation_Tick(&b, &p, 21, 0x104, dra, (SotnDataView){0}, &e));
    CHECK(!SDL_memcmp(&b, &before, sizeof(b)) && !SDL_memcmp(&p, &player, sizeof(p)) &&
          !SDL_memcmp(&e, &events, sizeof(e)));
    CHECK(!SotnWolfArticulation_Tick(&b, &p, 20, 0x104, dra, executable, &e));
    b.target_angle = SDL_MIN_SINT32;
    CHECK(!SotnWolfArticulation_Tick(&b, &p, 22, 0x104, dra, executable, &e));
    CHECK(b.target_angle == SDL_MIN_SINT32);
    CHECK(!SotnWolfArticulation_Tick(&b, &p, 18, 0x104, (SotnDataView){dra.bytes, 0x10b31},
                                     executable, &e));
    p.motion.animation.pose = 8;
    CHECK(!SotnWolfArticulation_Tick(&b, &p, 19, 0x104, dra, executable, &e));
    return true;
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x10b32), *sdk = SDL_calloc(1, 0x1e3ce);
    if (!bytes || !sdk)
    {
        SDL_free(bytes);
        SDL_free(sdk);
        return 1;
    }
    // Authored zero-amplitude SDK and selected pose fixtures, not game tables.
    for (Uint16 i = 0; i < 9; ++i)
    {
        Put16(bytes, 0x10b0c + i * 2U, (Sint16)(100 + i));
        Put16(bytes, 0x10b20 + i * 2U, (Sint16)(32 * i));
    }
    Put16(bytes, 0x10a6c + 4, -2);
    Put16(bytes, 0x10a9c + 4, 3);
    Put16(bytes, 0x10a8c + 4, 4);
    bytes[0x8410 + 13 * 28 + 12] = 7;
    const SotnDataView dra = {bytes, 0x10b32}, exe = {sdk, 0x1e3ce};
    const bool ok = LinkedParts(dra, exe) && PosesAndFeedback(dra, exe) && Failure(dra, exe);
    SDL_free(bytes);
    SDL_free(sdk);
    return ok ? 0 : 1;
}
