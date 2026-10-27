#include "actors_internal.h"
#include <SDL3/SDL.h>
#include <sotn/relic.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
int main(void)
{
    Uint8 bytes[0x1200] = {0};
    Put32(bytes + 0xe0, 0x800a0200);
    bytes[0x100] = 'T';
    bytes[0x101] = 0;
    for (Uint32 i = 0; i < SOTN_RELICS; ++i)
    {
        Put32(bytes + 0x200 + i * 16, 0x800a0100);
        bytes[0x208 + i * 16] = 12;
        bytes[0x20a + i * 16] = 13;
    }
    Put32(bytes + 0x200 + 18 * 16 + 12, 1);
    const SotnDataView dra = {bytes, sizeof(bytes)};
    SotnWorldState world = {0};
    SotnItem item = {0};
    CHECK(SotnRelic_Read(dra, SOTN_RELIC_CUBE, &item) && item.icon == 12 && item.palette == 13 &&
          item.name.size == 1);
    CHECK(SotnRelic_Collect(&world, dra, SOTN_RELIC_CUBE) && world.relics[SOTN_RELIC_CUBE] == 3);
    CHECK(SotnRelic_Collect(&world, dra, 18) && world.relics[18] == 1);
    CHECK(SotnWorld_Validate(&world));
    for (Uint16 id = 0; id < SOTN_RELICS; ++id)
    {
        const bool supported = id == 7 || id == 8 || id == 10 || id == 12 || id == 13 || id == 16;
        CHECK(SotnRelic_CanToggle(id) == supported);
        for (Uint8 bits = 0; bits < 4; ++bits)
        {
            world.relics[id] = bits;
            const SotnWorldState before = world;
            const bool allowed = supported && (bits == 1 || bits == 3);
            CHECK(SotnRelic_Toggle(&world, dra, id) == allowed);
            if (allowed)
            {
                CHECK(world.relics[id] == (bits ^ 2));
                CHECK(SotnRelic_Toggle(&world, dra, id) && world.relics[id] == bits);
            }
            CHECK(SDL_memcmp(&world, &before, sizeof(world)) == 0);
        }
        world.relics[id] = 0;
    }
    CHECK(!SotnRelic_CanToggle(30) && !SotnRelic_CanToggle(SDL_MAX_UINT16));
    CHECK(!SotnRelic_Toggle(&world, dra, 30) && !SotnRelic_Toggle(NULL, dra, 12));
    world.relics[12] = 3;
    const SotnWorldState before_toggle = world;
    CHECK(!SotnRelic_Toggle(&world, (SotnDataView){0}, 12));
    CHECK(SDL_memcmp(&world, &before_toggle, sizeof(world)) == 0);
    Put32(bytes + 0x200 + 12 * 16 + 12, 1);
    CHECK(!SotnRelic_Toggle(&world, dra, 12) && world.relics[12] == 3);
    Put32(bytes + 0x200 + 12 * 16 + 12, 0);
    world.relics[0] = 2;
    CHECK(!SotnWorld_Validate(&world));
    world.relics[0] = 0;
    CHECK(!SotnRelic_Read(dra, 30, &item) && item.id == SOTN_RELIC_CUBE);
    CHECK(!SotnRelic_Read((SotnDataView){0}, 0, &item));
    Put32(bytes + 0x200, 0xfffffff0);
    CHECK(!SotnRelic_Collect(&world, dra, 0) && world.relics[0] == 0);
    CHECK(!SotnRelic_Collect(NULL, dra, 1));
    Put32(bytes + 0xe0, 0xfffffff0);
    CHECK(!SotnRelic_Read(dra, 1, &item));
    /* A floating orb uses a movement timer; contact must not wait for timer zero. */
    Put32(bytes + 0xe0, 0x800a0200);
    Put32(bytes + 0x98, 0x800a1100);
    Uint8 *stage = SDL_calloc(1, 0x5000);
    CHECK(stage != NULL);
    Put32(stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(stage + 0x40 + i * 4, 0x80180300);
    stage[0x300] = stage[0x302] = 0xfe;
    stage[0x301] = stage[0x303] = 0xff;
    stage[0x30a] = stage[0x30c] = 100;
    stage[0x30e] = 0xb;
    stage[0x312] = SOTN_RELIC_CUBE;
    stage[0x314] = stage[0x315] = stage[0x316] = stage[0x317] = 0xff;
    SotnActors *actors = NULL;
    SotnActorLayout *layout = NULL;
    world = (SotnWorldState){0};
    const SotnDataView overlay = {stage, 0x5000};
    Uint8 tiles[32 * 16 * 2] = {0}, types[1] = {0};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    CHECK(SotnActors_CreateStage(dra, overlay, 65, &actors));
    CHECK(SotnActors_SetWorld(actors, &world));
    CHECK(SotnActorLayout_OpenStage(overlay, 65, 0, &layout));
    CHECK(SotnActors_Enter(actors, layout, map, 0, 0));
    const SotnPlayerState player = {.x = 100 * 65536,
                                    .y = 100 * 65536,
                                    .invulnerable = true,
                                    .animation = {.hitbox_width = 8, .hitbox_height = 20}};
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(SotnActors_Tick(actors, player, 0, 0));
    CHECK(world.relics[SOTN_RELIC_CUBE] == 3);
    CHECK(actors->slots[0].state.generation == 0);
    SotnActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    SDL_free(stage);
    SDL_Log("RELIC_PROGRESS: PASS");
    return 0;
}
