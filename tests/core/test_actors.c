#include <SDL3/SDL.h>
#include <sotn/actors.h>

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

typedef struct Fixture
{
    Uint8 dra[0x4000], nz0[0x3000], tiles[32 * 16 * 2], types[2];
} Fixture;

static void Put16(Uint8 *image, size_t offset, Uint16 value)
{
    image[offset] = (Uint8)value;
    image[offset + 1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *image, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        image[offset + i] = (Uint8)(value >> (i * 8));
}

static SotnCollisionMap MakeFixture(Fixture *fixture)
{
    SDL_memset(fixture, 0, sizeof(*fixture));
    Put32(fixture->nz0, 28, 0x80180040);
    for (size_t i = 0; i < 106; ++i)
        Put32(fixture->nz0, 0x40 + i * 4, 0x80180300);
    Put16(fixture->nz0, 0x300, 0xfffe);
    Put16(fixture->nz0, 0x302, 0xfffe);
    const Uint16 ids[] = {1, 0x23, 0x34, 0x2b, 0x4d};
    for (size_t i = 0; i < SDL_arraysize(ids); ++i)
    {
        const size_t offset = 0x300 + (i + 1) * 10;
        Put16(fixture->nz0, offset, (Uint16)(40 + i * 40));
        Put16(fixture->nz0, offset + 2, 140);
        Put16(fixture->nz0, offset + 4, ids[i]);
        Put16(fixture->nz0, offset + 6, (Uint16)i);
    }
    Put16(fixture->nz0, 0x33c, 0xffff);
    Put16(fixture->nz0, 0x33e, 0xffff);
    Put32(fixture->dra, 0x98, 0x800a0200);
    Put32(fixture->dra, 0x224, 0xa8000000);
    const size_t initializers[] = {0xbc8, 0xc34, 0xcc4, 0xc7c};
    for (size_t i = 0; i < SDL_arraysize(initializers); ++i)
    {
        Put16(fixture->nz0, initializers[i], 0x8001);
        Put16(fixture->nz0, initializers[i] + 2, 1);
    }
    Put32(fixture->nz0, 0xe04, 0x80180f00);
    Put16(fixture->nz0, 0xe34, 3);
    fixture->nz0[0xe44] = 0x70;
    const Uint8 candle[] = {2, 1, 1, 2, 0, 0};
    const Uint8 globe[] = {1, 3, 0, 0};
    const Uint8 walk[] = {1, 1, 1, 2, 0, 0};
    const Uint8 attack[] = {1, 3, 1, 4, 255, 0};
    SDL_memcpy(fixture->nz0 + 0xf00, candle, sizeof(candle));
    SDL_memcpy(fixture->nz0 + 0xef0, globe, sizeof(globe));
    SDL_memcpy(fixture->nz0 + 0x22ec, walk, sizeof(walk));
    SDL_memcpy(fixture->nz0 + 0x22fc, attack, sizeof(attack));
    SDL_memcpy(fixture->nz0 + 0x237c, walk, sizeof(walk));
    Put16(fixture->nz0, 0x22d6, 20);
    Put16(fixture->nz0, 0x22da, 3);
    Put16(fixture->nz0, 0x22dc, 6);
    Put16(fixture->nz0, 0x22de, 65533);
    Put16(fixture->nz0, 0x22e0, 65524);
    Put16(fixture->nz0, 0x22e6, 20);
    Put16(fixture->nz0, 0x22e8, 6);
    fixture->types[1] = 3;
    for (size_t y = 10; y < 16; ++y)
        for (size_t x = 0; x < 32; ++x)
            fixture->tiles[(y * 32 + x) * 2] = 1;
    return (SotnCollisionMap){
        {fixture->tiles, sizeof(fixture->tiles)}, {fixture->types, sizeof(fixture->types)}, 32, 16};
}

static bool TestBehaviors(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)},
                       nz0 = {fixture->nz0, sizeof(fixture->nz0)};
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create(dra, nz0, &actors));
    CHECK(SotnActorLayout_Open(nz0, 0, &layout));
    SotnPlayerState player = {0};
    player.x = 10 * 65536;
    player.y = 140 * 65536;
    player.facing_left = true;
    CHECK(!SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    SotnActorState state = {0};
    CHECK(SotnActors_Get(actors, 0, &state) && state.animation_set == 3 &&
          state.blend_mode == 0x70);
    CHECK(SotnActors_Get(actors, 1, &state) && state.frame == 7 && state.priority == 0x70);
    CHECK(SotnActors_Get(actors, 2, &state) && state.frame == 3);
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 2);
    CHECK(SotnActors_Get(actors, 4, &state) && state.unsupported);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.frame == 1);
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 1 && state.y == 140 * 65536);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state));
    CHECK(state.x == 160 * 65536 - 24576 && state.y == 140 * 65536 && state.frame == 2);
    player.x = 150 * 65536;
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 3);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 4);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.frame == 3);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.frame == 4);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 3, &state) && state.step == 1);
    CHECK(SotnActors_Get(actors, 0, &state));
    const Uint32 generation = state.generation;
    CHECK(SotnActors_Tick(actors, player, 500, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.generation == 0);
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Get(actors, 0, &state) && state.generation != 0 &&
          state.generation != generation);
    CHECK(!SotnActors_Tick(actors, player, -1, 0));
    player.x = SDL_MAX_SINT32;
    CHECK(!SotnActors_Tick(actors, player, 0, 0));
    CHECK(!SotnActors_Get(actors, 192, &state));
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    return true;
}

static bool TestFailures(Fixture *fixture)
{
    const SotnCollisionMap map = MakeFixture(fixture);
    Put32(fixture->nz0, 0xe04, 0x8018ffff);
    const SotnDataView dra = {fixture->dra, sizeof(fixture->dra)},
                       nz0 = {fixture->nz0, sizeof(fixture->nz0)};
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    CHECK(SotnActors_Create(dra, nz0, &actors));
    CHECK(SotnActorLayout_Open(nz0, 0, &layout));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    CHECK(SotnActors_Tick(actors, (SotnPlayerState){0}, 0, 0));
    CHECK(!SotnActors_Tick(actors, (SotnPlayerState){0}, 0, 0));
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    CHECK(!SotnActors_Create((SotnDataView){0}, nz0, &actors) && actors == NULL);
    SotnActors_Destroy(NULL);
    return true;
}

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = TestBehaviors(fixture) && TestFailures(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_ACTORS_TEST: PASS");
    return 0;
}
