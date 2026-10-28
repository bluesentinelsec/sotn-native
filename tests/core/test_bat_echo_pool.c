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

static bool TestPool(void)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    CHECK(s);
    Actor a = {.state = {.generation = 1}};
    CHECK(ActorPrimitives(s, &a, 40));
    SotnPrimitiveSlots slots = {0};
    Uint16 indices[49] = {0};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(s, &slots));
    CHECK(SotnPrimitiveSlots_Reserve(&slots, 49, indices, &allocated) && allocated);
    CHECK(indices[0] == 40 && indices[48] == 88);
    SotnBatEcho echo = {.phase = 1, .priority = 142};
    echo.quads[0].x[1] = 22;
    echo.quads[0].red[1] = 93;
    CHECK(SotnActors_CommitBatEcho(s, 1, indices, &echo));
    SotnActorPrimitive p = {0};
    CHECK(SotnActors_GetBatEchoPrimitive(s, 1, indices[17], &p) && p.active && p.page == 0x60 &&
          p.offset_x[1] == 22 && p.vertex_red[1] == 93);
    CHECK(!SotnActors_GetBatEchoPrimitive(s, 2, indices[17], &p));
    const Uint16 first = indices[0];
    indices[0] = a.primitive[0];
    CHECK(!SotnActors_CommitBatEcho(s, 2, indices, &echo));
    CHECK(SotnActors_GetBatEchoPrimitive(s, 1, indices[17], &p));
    indices[0] = first;
    echo.phase = 4;
    CHECK(!SotnActors_CommitBatEcho(s, 2, indices, &echo));
    CHECK(SotnActors_GetBatEchoPrimitive(s, 1, indices[17], &p));
    echo.phase = 1;
    indices[0] = indices[1];
    CHECK(!SotnActors_CommitBatEcho(s, 2, indices, &echo));
    indices[0] = first;
    // Same numeric generations in the two namespaces cannot read/free each other's rows.
    Actor stale = {.state = {.generation = 1, .primitive_count = 1}};
    stale.primitive[0] = indices[17];
    CHECK(!ActorPrimitiveGet(s, &stale, 0));
    ActorPrimitivesClear(s, &stale);
    CHECK(SotnActors_GetBatEchoPrimitive(s, 1, indices[17], &p));
    Actor second = {.state = {.generation = 2}};
    CHECK(ActorPrimitives(s, &second, 1) && second.primitive[0] == 89);
    CHECK(SotnActors_CommitBatEcho(s, 0, NULL, NULL));
    CHECK(ActorPrimitiveGet(s, &a, 0) && ActorPrimitiveGet(s, &second, 0));
    CHECK(!SotnActors_GetBatEchoPrimitive(s, 1, indices[17], &p));
    ActorPrimitivesClear(s, &a);
    ActorPrimitivesClear(s, &second);
    CHECK(SotnActors_PrimitiveSlots(s, &slots));
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(slots.occupied[i] == 0);
    // Fragmentation preserves first-free linked order and ordinary exhaustion is atomic.
    for (Uint16 i = 0; i < 16; ++i)
        slots.occupied[i] = 0xaaaaaaaaaaaaaaaaULL;
    CHECK(SotnPrimitiveSlots_Reserve(&slots, 49, indices, &allocated) && allocated);
    CHECK(indices[0] == 0 && indices[48] == 96);
    for (Uint16 i = 0; i < 16; ++i)
        slots.occupied[i] = SDL_MAX_UINT64;
    const SotnPrimitiveSlots before = slots;
    CHECK(SotnPrimitiveSlots_Reserve(&slots, 49, indices, &allocated) && !allocated);
    CHECK(indices[48] == 96 && !SDL_memcmp(&slots, &before, sizeof(slots)));
    CHECK(!SotnPrimitiveSlots_Reserve(&slots, 0, indices, &allocated));
    SDL_free(s);
    return true;
}
static void Put16(Uint8 *p, Uint16 n)
{
    p[0] = (Uint8)n;
    p[1] = (Uint8)(n >> 8);
}
static void Put32(Uint8 *p, Uint32 n)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(n >> (i * 8U));
}
static bool TestRoomLifetime(void)
{
    Uint8 dra[256] = {0}, stage[4096] = {0}, tiles[32 * 16 * 2] = {0}, types[1] = {0};
    Put32(stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(stage + 0x40 + i * 4U, 0x80180300);
    Put16(stage + 0x300, 0xfffe);
    Put16(stage + 0x302, 0xfffe);
    Put16(stage + 0x30a, 0xffff);
    Put16(stage + 0x30c, 0xffff);
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    const SotnDataView overlay = {stage, sizeof(stage)};
    const SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 32, 16};
    CHECK(SotnActors_Create((SotnDataView){dra, sizeof(dra)}, overlay, &s));
    CHECK(SotnActorLayout_Open(overlay, 0, &layout));
    CHECK(SotnActors_Enter(s, layout, map, 0, 0));
    Actor a = {.state = {.generation = 9}};
    CHECK(ActorPrimitives(s, &a, 1));
    SotnPrimitiveSlots slots = {0};
    Uint16 indices[49] = {0};
    bool allocated = false;
    CHECK(SotnActors_PrimitiveSlots(s, &slots));
    CHECK(SotnPrimitiveSlots_Reserve(&slots, 49, indices, &allocated) && allocated);
    const SotnBatEcho echo = {.phase = 1, .priority = 142};
    CHECK(SotnActors_CommitBatEcho(s, 1, indices, &echo));
    CHECK(SotnActors_Enter(s, layout, map, 0, 0));
    CHECK(SotnActors_PrimitiveSlots(s, &slots));
    CHECK(!(slots.occupied[0] & 1U)); // Stage-owned row retires on room entry.
    SotnActorPrimitive packet = {0};
    CHECK(SotnActors_GetBatEchoPrimitive(s, 1, indices[17], &packet));
    CHECK(SotnActors_CommitBatEcho(s, 0, NULL, NULL));
    CHECK(SotnActors_PrimitiveSlots(s, &slots));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        CHECK(!slots.occupied[i]);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    return true;
}
int main(void)
{
    return TestPool() && TestRoomLifetime() ? 0 : 1;
}
