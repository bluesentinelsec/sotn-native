#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 stage[0x4000], dra[0x400], tiles[80 * 48 * 2], types[256];
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static bool Open(Fixture *f, SotnActors *s)
{
    SDL_zero(*f);
    SDL_zero(*s);
    f->dra[0x98] = 0;
    f->dra[0x99] = 2;
    f->dra[0x9a] = 10;
    f->dra[0x9b] = 128;
    s->stage_id = 2;
    s->nz0 = (SotnDataView){f->stage, sizeof(f->stage)};
    s->dra = (SotnDataView){f->dra, sizeof(f->dra)};
    s->world = &s->fallback_world;
    s->camera = &s->fallback_camera;
    s->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 80, 48};
    s->mutable_tiles = f->tiles;
    s->player = (SotnPlayerState){.x = 368 * 65536, .y = 640 * 65536};
    s->camera_x = 384;
    s->camera_y = 512;
    s->game_seconds = 125;
    for (Uint16 i = 0; i < 16; ++i)
        Put16(f->stage + 0x1aa8 + i * 2U, (Uint16)(100 + i));
    for (Uint16 i = 0; i < 7; ++i)
    {
        f->stage[0xad8 + i] = 80;
        for (Uint16 side = 0; side < 2; ++side)
        {
            Uint8 *p = f->stage + 0xae8 + i * 16U + side * 8U;
            Put16(p, 240);
            Put16(p + 2, 507);
            Put16(p + 4, 784);
            Put16(p + 6, 763);
        }
    }
    Put16(f->stage + 0x31bc + 14, 23);
    s->slots[8] = (Actor){.slot = 8, .state = {.id = 0x41, .generation = 1}};
    s->slots[131] = (Actor){.slot = 131, .state = {.id = 0x40, .generation = 2}};
    CHECK(SotnCamera_Enter(s->camera, 1280, 768, 512, 640));
    return SotnRoomActors_Create(&s->spawns);
}
static bool Manager(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s));
    Actor *a = &s->slots[8];
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && !s->library_arena_active);
    CHECK(s->slots[16].state.id == 0x1f && Pixel(s->slots[16].state.x) == 512 &&
          Pixel(s->slots[16].state.y) == 672);
    CHECK(s->slots[15].state.id == 0x12 && s->slots[15].state.parameters == 6);
    s->player.x = 656 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 1);
    s->player.x = 512 * 65536;
    s->player.y = 544 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 1);
    s->player.y = 640 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 2 && s->library_arena_active);
    CHECK(s->library_enemy_flags == 1 && s->world->time_attack_visited == (1U << 17) &&
          !s->world->time_attack[17]);
    s->music_playing = true;
    CHECK(ActorLibrary(s, a) && a->state.step == 3 && s->combat.music_change && !s->combat.music);
    SDL_zero(s->combat);
    CHECK(ActorLibrary(s, a) && a->state.step == 3 && !s->combat.music_change);
    s->music_playing = false;
    CHECK(ActorLibrary(s, a) && a->state.step == 4 && s->combat.music == 0x31d);
    s->library_enemy_flags |= 2U;
    s->music_playing = true;
    CHECK(ActorLibrary(s, a) && a->state.step == 5 && s->world->time_attack[17] == 205 &&
          !s->combat.music);
    CHECK(ActorLibrary(s, a) && a->state.step == 5 && s->library_arena_active);
    s->library_enemy_flags |= 4U;
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 20;
    CHECK(ActorLibrary(s, a) && a->state.step == 5 && !s->library_camera_unlocked);
    s->slots[97] = (Actor){0};
    CHECK(ActorLibrary(s, a) && a->state.step == 6 && s->library_camera_unlocked &&
          !s->library_arena_active);
    CHECK(s->library_enemy_flags == 6 && s->slots[97].state.id == 0x42 &&
          s->slots[97].state.parameters == 7);
    CHECK(Pixel(s->slots[97].state.x) == 512 && Pixel(s->slots[97].state.y) == 640 &&
          !s->slots[97].state.facing_left);
    CHECK(ActorLibrary(s, a) && a->state.step == 6);
    s->music_playing = false;
    CHECK(ActorLibrary(s, a) && a->state.step == 7 && s->combat.music == 0x309);
    s->slots[97].state.step = 6;
    CHECK(ActorLibrary(s, &s->slots[97]) && s->slots[97].state.id == 3 &&
          s->slots[97].state.parameters == 0x8017);
    // A fresh placed manager observes the ordinary original record and creates no children.
    s->slots[16] = (Actor){0};
    s->slots[15] = (Actor){0};
    a->state.step = 0;
    CHECK(ActorLibrary(s, a) && a->remove && !s->slots[16].state.generation &&
          !s->slots[15].state.generation);
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Seals(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s));
    Actor *root = &s->slots[131];
    CHECK(ActorLibrary(s, root) && root->state.step == 1);
    for (Uint16 i = 132; i <= 138; ++i)
        CHECK(ActorLibrary(s, &s->slots[i]));
    for (Uint16 i = 131; i <= 138; ++i)
    {
        CHECK(s->slots[i].state.frame == 15 + (i - 131) % 4);
        CHECK(s->slots[i].state.priority == 0x69);
    }
    s->slots[139] = (Actor){.slot = 139,
                            .state = {.id = 0x40, .generation = 3},
                            .enemy_state = {.owner_slot = 131, .owner_generation = 999}};
    s->library_arena_active = true;
    for (Uint16 t = 0; t < 100; ++t)
        for (Uint16 i = 131; i <= 138; ++i)
            CHECK(ActorLibrary(s, &s->slots[i]));
    CHECK(!s->slots[139].arena.triggered);
    for (Uint16 i = 0; i < 8; ++i)
    {
        Actor *a = &s->slots[131 + i];
        const Uint16 low = i % 4, side = i / 4;
        const Uint32 tile = (side ? 0xc0dU : 0xbf2U) + (3U - low) * 80;
        CHECK(a->state.step == 4 && Pixel(a->state.y) == (3 - low) * 16 + 616);
        CHECK(Read16(f->tiles + tile * 2) == 100 + (7 - low) * 2 + side);
    }
    s->library_arena_active = false;
    for (Uint16 t = 0; t < 2; ++t)
        for (Uint16 i = 131; i <= 138; ++i)
            CHECK(ActorLibrary(s, &s->slots[i]));
    for (Uint16 i = 0; i < 8; ++i)
    {
        const Uint16 low = i % 4, side = i / 4;
        const Uint32 tile = (side ? 0xc0dU : 0xbf2U) + (3U - low) * 80;
        CHECK(s->slots[131 + i].state.step == 6 && !s->slots[131 + i].state.frame);
        CHECK(Read16(f->tiles + tile * 2) == 100 + (3 - low) * 2 + side);
    }
    root->state.parameters = 4;
    CHECK(!ActorLibrary(s, root));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
static bool Camera(Fixture *f, SotnActors *s)
{
    CHECK(Open(f, s));
    Actor *a = &s->slots[15];
    *a = (Actor){
        .slot = 15,
        .state = {
            .id = 0x12, .generation = 1, .parameters = 6, .x = 512 * 65536, .y = 624 * 65536}};
    s->player.x = 512 * 65536;
    s->player.y = 624 * 65536;
    CHECK(ActorCamera(s, a) && a->camera.pending && a->camera.timer == 15);
    for (Uint16 i = 0; i < 15; ++i)
        CHECK(ActorCamera(s, a));
    CHECK(s->camera->left == 240 && s->camera->right == 784 && s->camera->top == 507 &&
          s->camera->bottom == 763);
    s->buttons = SOTN_BUTTON_CANCEL;
    CHECK(ActorCamera(s, a) && s->camera->left == 240);
    s->library_camera_unlocked = true;
    CHECK(ActorCamera(s, a) && s->camera->left == 0 && s->camera->right == 1280 &&
          s->camera->top == 507);
    a->state.parameters = 7;
    CHECK(!ActorCamera(s, a));
    SotnRoomActors_Destroy(s->spawns);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    if (!f || !s)
    {
        SDL_free(f);
        SDL_free(s);
        return 1;
    }
    const bool ok = Manager(f, s) && Seals(f, s) && Camera(f, s);
    SDL_free(f);
    SDL_free(s);
    return ok ? 0 : 1;
}
