#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x1000], stage[0x1000], tiles[80 * 32 * 2], types[2];
    SotnActorLayout *layout;
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static SotnCollisionMap Map(Fixture *f)
{
    return (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 80, 32};
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(*out, f->layout, Map(f), 0, 0));
    return true;
}
static Actor Exit(Uint16 variant)
{
    return (Actor){.slot = 16,
                   .dynamic = true,
                   .state = {.id = 0x16,
                             .generation = 1,
                             .parameters = variant,
                             .x = 128 * 65536,
                             .y = 128 * 65536}};
}
static bool Variants(Fixture *f, SotnActors *s)
{
    const Sint32 thresholds[] = {640, 384, 768, 256, 768, 128, 1152, 1152,
                                 128, 640, 384, 640, 384, 768, 768};
    for (Uint16 i = 0; i < SDL_arraysize(thresholds); ++i)
    {
        CHECK(SotnActors_Enter(s, f->layout, Map(f), 0, 0));
        const bool left = i == 1 || i == 3 || i == 5 || i == 8 || i == 10 || i == 12;
        const bool shift = i == 1 || i == 3 || i == 10 || i == 12;
        const bool shrink = i == 0 || i == 2 || i == 4 || i == 11 || i == 13 || i == 14;
        const Sint32 past = thresholds[i] + (left ? -1 : 1);
        Actor a = Exit(i);
        s->player = (SotnPlayerState){
            .x = past * 65536 + 123, .y = 128 * 65536, .velocity_x = 98304, .velocity_y = -65536};
        s->frame_start_x = past;
        const SotnPlayerState original = s->player;
        const SotnRoomWindow initial = s->window;
        CHECK(ActorChapel(s, &a) && a.state.step == 1 &&
              !SDL_memcmp(&initial, &s->window, sizeof(initial)));
        // Both the previous room X and the predicted post-motion X must be strictly past.
        s->frame_start_x = thresholds[i];
        CHECK(ActorChapel(s, &a) && a.state.step == 1);
        s->frame_start_x = past;
        s->player.x = thresholds[i] * 65536 + 65535;
        CHECK(ActorChapel(s, &a) && a.state.step == 1);
        s->player = original;
        s->player.y = 176 * 65536;
        CHECK(ActorChapel(s, &a) && a.state.step == 1);
        s->player.y = 80 * 65536;
        CHECK(ActorChapel(s, &a) && a.state.step == 1);
        s->player = original;
        CHECK(ActorChapel(s, &a) && a.state.step == 2 &&
              s->window.start_x == (left ? thresholds[i] : 0) &&
              s->window.end_x == (left ? 1280 : thresholds[i]) &&
              s->window.origin_cell == (shift ? 1 : 0) && s->window.last_cell == (shrink ? 3 : 4) &&
              !SDL_memcmp(&original, &s->player, sizeof(original)));
        SotnRoomCrossing crossing = {0};
        CHECK(SotnStageRoute_WindowCrossing(s->window, past, 128, 1280, 512, false, &crossing) &&
              crossing.active && !crossing.arrival_y && crossing.y == 128 &&
              crossing.x == (left ? (shift ? 0 : -256) : (shrink ? 1024 : 1280)));
        // The actual source permits four successful updates (steps 1 through 4).
        for (Uint16 step = 2; step < 5; ++step)
        {
            s->frame_start_x = past + s->window.origin_cell * 256;
            s->player.x = s->frame_start_x * 65536;
            CHECK(ActorChapel(s, &a) && a.state.step == step + 1);
        }
        const SotnRoomWindow dormant = s->window;
        CHECK(ActorChapel(s, &a) && a.state.step == 5 &&
              !SDL_memcmp(&dormant, &s->window, sizeof(dormant)));
    }
    return true;
}
static bool TickAndReset(Fixture *f, SotnActors *s)
{
    CHECK(SotnActors_Enter(s, f->layout, Map(f), 0, 0));
    s->slots[16] = Exit(0);
    SotnPlayerState player = {.x = 639 * 65536, .y = 128 * 65536, .hp = 70, .hp_max = 70};
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->frame_start_x == 639 &&
          s->slots[16].state.step == 1);
    player.x = 641 * 65536;
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->frame_start_x == 639 &&
          s->slots[16].state.step == 1 && s->window.end_x == 1280);
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->frame_start_x == 641 &&
          s->slots[16].state.step == 2 && s->window.end_x == 640);
    CHECK(SotnActors_Enter(s, f->layout, Map(f), 0, 0) && !s->player_initialized);
    SotnRoomWindow window = {0};
    CHECK(SotnActors_GetRoomWindow(s, &window) && window.end_x == 1280 && !window.start_x &&
          !window.origin_cell && window.last_cell == 4);
    s->slots[16] = Exit(1);
    player.x = 383 * 65536;
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->frame_start_x == 383);
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->window.origin_cell == 1);
    // A virtual origin shift applies to both copies; the physical X stays fixed.
    CHECK(SotnActors_Tick(s, player, 0, 0) && s->window.origin_cell == 2 &&
          s->player.x == player.x && s->frame_start_x == 383);
    CHECK(!SotnActors_GetRoomWindow(NULL, &window) && !SotnActors_GetRoomWindow(s, NULL));
    const bool entered = s->entered;
    s->entered = false;
    CHECK(!SotnActors_GetRoomWindow(s, &window));
    s->entered = entered;
    return true;
}
static bool DeathAndFailures(Fixture *f, SotnActors *s)
{
    CHECK(SotnActors_Enter(s, f->layout, Map(f), 0, 0));
    Actor a = Exit(0);
    CHECK(ActorChapel(s, &a));
    s->player = (SotnPlayerState){.x = 700 * 65536,
                                  .y = 175 * 65536,
                                  .status = 0x40000,
                                  .velocity_x = 98304,
                                  .velocity_y = 65536};
    s->frame_start_x = 700;
    CHECK(ActorChapel(s, &a) && a.state.step == 1 && !s->player.velocity_x &&
          s->player.velocity_y == 65536 && s->motion.stop_horizontal && !s->motion.stop_velocity &&
          s->window.end_x == 1280);
    a.state.parameters = 15;
    CHECK(!ActorChapelExit(s, &a));
    a.state.parameters = 0;
    a.state.step = 6;
    CHECK(!ActorChapelExit(s, &a));
    a.state.step = 1;
    s->player.status = 0;
    s->window.origin_cell = SDL_MAX_SINT32;
    CHECK(!ActorChapelExit(s, &a));
    s->window.origin_cell = 0;
    s->window.end_x = SDL_MAX_SINT32;
    CHECK(!ActorChapelExit(s, &a));
    CHECK(!ActorChapelExit(NULL, &a) && !ActorChapelExit(s, NULL));
    s->stage_id = 2;
    CHECK(!ActorChapelExit(s, &a));
    s->stage_id = 6;
    a.state.id = 0x17;
    CHECK(!ActorChapelExit(s, &a));
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    const bool ok =
        f && Open(f, &s) && Variants(f, s) && TickAndReset(f, s) && DeathAndFailures(f, s);
    SotnActors_Destroy(s);
    if (f != NULL)
        SotnActorLayout_Close(f->layout);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_EXIT: PASS");
    return 0;
}
