#include <SDL3/SDL.h>
#include <sotn/player_factory.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Blueprint(Uint8 *image, Uint16 id, Uint8 child, Uint8 amount, Uint8 cycle_flags,
                      Uint8 cycle, Uint8 kind_origin, Uint8 delay)
{
    Uint8 *p = image + 0xd1d4 + id * 6U;
    p[0] = child;
    p[1] = amount;
    p[2] = cycle_flags;
    p[3] = cycle;
    p[4] = kind_origin;
    p[5] = delay;
}
static bool Scheduling(Uint8 *image)
{
    Blueprint(image, 1, 9, 3, 0x42, 3, 0x20, 2);
    image[0xd4b8] = 32;
    image[0xd4b9] = 35;
    const SotnDataView dra = {image, 0xe000};
    SotnPlayerFactory f = {0};
    SotnPlayerFactoryEvents e = {0};
    SotnPlayerFactorySource source = {
        .x = 10, .y = 20, .flags = SDL_MAX_UINT32, .priority = 144, .facing_left = true};
    CHECK(SotnPlayerFactory_Begin(&f, source, 0x120001, -1));
    CHECK(f.entity_id == 1 && f.parameter == 0x1200 && f.source.flags == 0x10000);
    SotnPlayerFactoryInput in = {
        .occupied = ~(((Uint64)1 << 34) | ((Uint64)1 << 35)), .player_x = 30, .player_y = 40};
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 0 && f.delay == 1 &&
          f.source.x == 30 && f.source.y == 40);
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e));
    CHECK(e.count == 2 && e.children[0].slot == 35 && e.children[1].slot == 34 &&
          e.children[0].parameter == 0x1200 && e.children[1].parameter == 0x1201 &&
          e.children[0].argument_low == 511 && e.children[0].argument_high == -1 &&
          e.children[0].source.flags == 0x10000 && e.children[0].source.facing_left &&
          f.spawn_index == 2 && !f.removed && f.delay == 3);
    in.occupied = e.occupied;
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 0);
    CHECK(!f.removed && f.delay == 3);
    in.occupied &= ~((Uint64)1 << 33);
    for (Uint16 i = 0; i < 2; ++i)
        CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 0);
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 1 && e.children[0].slot == 33 &&
          e.children[0].parameter == 0x1202 && f.removed && !f.entity_id);
    CHECK(!SotnPlayerFactory_Tick(&f, dra, in, &e));
    return true;
}
static bool Capacity(Uint8 *image)
{
    Blueprint(image, 2, 6, 3, 0x83, 0, 2, 0);
    image[0xd4bc] = 32;
    image[0xd4bd] = 33;
    const SotnDataView dra = {image, 0xe000};
    SotnPlayerFactory f = {0};
    SotnPlayerFactoryEvents e = {0};
    CHECK(SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){0}, 2, 0));
    SotnPlayerFactoryInput in = {.occupied = ~(((Uint64)1 << 32) | ((Uint64)1 << 33))};
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e));
    CHECK(e.count == 2 && f.removed && f.spawn_index == 2 && e.children[0].slot == 32 &&
          e.children[1].slot == 33 && e.children[1].parameter == 1 && !e.clear_weapon_state);
    Blueprint(image, 3, 8, 1, 1, 0, 3, 0);
    image[0xd4be] = image[0xd4bf] = 16;
    CHECK(SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){0}, 3, 0));
    in.occupied = SDL_MAX_UINT64;
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 1 && e.clear_weapon_state &&
          e.children[0].replace && e.children[0].slot == 16 && e.occupied == SDL_MAX_UINT64 &&
          f.removed);
    // Split allocation rotates three real ranges and resumes with a per-cycle parameter reset.
    Blueprint(image, 4, 7, 4, 4, 0, 8, 0);
    image[0xd4c8] = 17;
    image[0xd4c9] = 63;
    CHECK(SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){0}, 4, 0));
    in.occupied = ~(((Uint64)1 << 17) | ((Uint64)1 << 32));
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 2 && e.children[0].slot == 17 &&
          e.children[1].slot == 32 && !f.removed && f.spawn_index == 2);
    in.occupied = ~(((Uint64)1 << 48) | ((Uint64)1 << 18));
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 2 && f.removed &&
          e.children[0].slot == 48 && e.children[1].slot == 18 && e.children[0].parameter == 0 &&
          e.children[1].parameter == 1);
    // Original empty-child blueprints clear a weapon or leave a selected free slot reusable.
    Blueprint(image, 6, 0, 2, 2, 0, 3, 0);
    CHECK(SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){0}, 6, 0));
    in.occupied = SDL_MAX_UINT64;
    CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 2 && f.removed &&
          e.children[0].slot == 16 && e.children[1].slot == 16 && !e.children[0].entity_id &&
          !(e.occupied & ((Uint64)1 << 16)));
    return true;
}
static bool Origins(Uint8 *image)
{
    const SotnDataView dra = {image, 0xe000};
    for (Uint8 origin = 0; origin < 8; ++origin)
    {
        Blueprint(image, 5, 8, 2, 1, 0, (Uint8)(origin << 4), 0);
        SotnPlayerFactory f = {0};
        SotnPlayerFactoryEvents e = {0};
        CHECK(SotnPlayerFactory_Begin(
            &f, (SotnPlayerFactorySource){.x = 10, .y = 20, .priority = 144}, 5, 0));
        SotnPlayerFactoryInput in = {.player_x = 30, .player_y = 40};
        CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && e.count == 1 && !f.removed);
        const bool follow = origin == 2 || origin == 4 || origin == 5 || origin == 7;
        CHECK(f.source.x == (follow ? 30 : 10) && f.source.y == (follow ? 40 : 20));
        CHECK(((f.source.flags & 0x8000000U) != 0) == (origin == 0 || origin == 6));
        CHECK(((f.source.flags & 0x20000U) != 0) == (origin == 4 || origin == 5));
        in.player_x = 50;
        in.player_y = 60;
        CHECK(SotnPlayerFactory_Tick(&f, dra, in, &e) && f.removed);
        CHECK(e.count == ((origin == 4 || origin == 5 || origin == 7) ? 0 : 1));
    }
    return true;
}
static bool Invalid(Uint8 *image)
{
    const SotnDataView dra = {image, 0xe000};
    SotnPlayerFactory f = {0};
    SotnPlayerFactoryEvents e = {.count = 42};
    CHECK(SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){.flags = SDL_MAX_UINT32}, 0x12f003,
                                  -1025));
    CHECK(f.entity_id == 0xfd && f.entity_modifier == 0xf0 && f.parameter == 0x1200);
    CHECK(SotnPlayerFactory_Tick(&f, dra, (SotnPlayerFactoryInput){0}, &e) &&
          e.children[0].entity_id == 0xf8 && e.children[0].argument_low == 511 &&
          e.children[0].argument_high == -3);
    CHECK(SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){0}, 1, 0));
    const SotnPlayerFactory before = f;
    e.count = 42;
    CHECK(!SotnPlayerFactory_Tick(&f, (SotnDataView){0}, (SotnPlayerFactoryInput){0}, &e));
    CHECK(!SDL_memcmp(&f, &before, sizeof(f)) && e.count == 42);
    image[0xd4b9] = 64;
    CHECK(!SotnPlayerFactory_Tick(&f, dra, (SotnPlayerFactoryInput){0}, &e) && e.count == 42 &&
          !f.initialized);
    CHECK(!SotnPlayerFactory_Begin(&f, (SotnPlayerFactorySource){0}, 0xfff, 0));
    CHECK(!SotnPlayerFactory_Tick(NULL, dra, (SotnPlayerFactoryInput){0}, &e));
    return true;
}
int main(void)
{
    Uint8 *image = SDL_calloc(1, 0xe000);
    if (!image)
        return 1;
    const bool ok = Scheduling(image) && Capacity(image) && Origins(image) && Invalid(image);
    SDL_free(image);
    return ok ? 0 : 1;
}
