#include <SDL3/SDL.h>
#include <sotn/actor_layout.h>

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
    Uint8 bytes[0x400];
} Fixture;

static void Put16(Uint8 *bytes, Uint16 value)
{
    bytes[0] = (Uint8)value;
    bytes[1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *bytes, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        bytes[i] = (Uint8)(value >> (i * 8));
}

static SotnDataView MakeFixture(Fixture *fixture)
{
    SDL_memset(fixture, 0, sizeof(*fixture));
    Put32(fixture->bytes + 28, 0x80180040);
    for (size_t i = 0; i < 53; ++i)
    {
        Put32(fixture->bytes + 0x40 + i * 4, 0x80180200);
        Put32(fixture->bytes + 0x40 + (53 + i) * 4, 0x80180300);
    }
    static const Uint16 records[][5] = {{20, 30, 1, 0x100, 7},    {100, 400, 0x0c01, 2, 0},
                                        {320, 50, 1, 1, 0},       {321, 50, 1, 3, 0},
                                        {600, 500, 0xa001, 0, 9}, {610, 30, 0x8001, 4, 0}};
    static const unsigned int vertical[] = {0, 5, 2, 3, 1, 4};
    for (unsigned int axis = 0; axis < 2; ++axis)
    {
        const size_t start = axis == 0 ? 0x200 : 0x300;
        Put16(fixture->bytes + start, 0xfffe);
        Put16(fixture->bytes + start + 2, 0xfffe);
        for (size_t i = 0; i < SDL_arraysize(records); ++i)
            for (size_t j = 0; j < 5; ++j)
                Put16(fixture->bytes + start + (i + 1) * 10 + j * 2,
                      records[axis == 0 ? i : vertical[i]][j]);
        Put16(fixture->bytes + start + 70, 0xffff);
        Put16(fixture->bytes + start + 72, 0xffff);
    }
    return (SotnDataView){fixture->bytes, sizeof(fixture->bytes)};
}

static bool ReadSlot(SotnRoomActors *actors, Uint32 slot, Uint32 *generation)
{
    SotnActorPlacement placement = {0};
    return SotnRoomActors_Get(actors, slot, &placement, generation);
}

static bool TestLayoutAndSpawn(Fixture *fixture)
{
    const SotnDataView image = MakeFixture(fixture);
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_Open(image, 1, &layout));
    CHECK(SotnActorLayout_Count(layout) == 6);
    SotnActorPlacement placement = {0};
    CHECK(SotnActorLayout_Get(layout, false, 1, &placement));
    CHECK(placement.id == 1 && placement.variant == 3 && placement.x == 100 && placement.y == 400);
    SotnRoomActors *actors = NULL;
    CHECK(SotnRoomActors_Create(&actors));
    CHECK(!SotnRoomActors_Scroll(actors, 0, 0));
    CHECK(SotnRoomActors_Enter(actors, layout, 0, 0));
    Uint32 first = 0, generation = 0;
    CHECK(ReadSlot(actors, 0, &first) && first != 0);
    CHECK(ReadSlot(actors, 1, &generation) && generation != 0);
    CHECK(ReadSlot(actors, 2, &generation) && generation == 0);
    CHECK(ReadSlot(actors, 3, &generation) && generation == 0);
    CHECK(SotnRoomActors_Scroll(actors, 1, 0));
    CHECK(ReadSlot(actors, 3, &generation) && generation != 0);
    CHECK(SotnRoomActors_Release(actors, 3, false));
    CHECK(SotnRoomActors_Scroll(actors, 1, 0));
    CHECK(ReadSlot(actors, 3, &generation) && generation == 0);
    CHECK(SotnRoomActors_Scroll(actors, 0, 0));
    CHECK(SotnRoomActors_Scroll(actors, 1, 0));
    CHECK(ReadSlot(actors, 3, &generation) && generation != 0);
    CHECK(SotnRoomActors_Scroll(actors, 1, 112));
    CHECK(ReadSlot(actors, 2, &generation) && generation != 0);
    CHECK(SotnRoomActors_Release(actors, 0, true));
    CHECK(SotnRoomActors_Scroll(actors, 500, 0));
    CHECK(SotnRoomActors_Scroll(actors, 0, 0));
    CHECK(ReadSlot(actors, 0, &generation) && generation == 0);
    CHECK(SotnRoomActors_Enter(actors, layout, 0, 0));
    CHECK(ReadSlot(actors, 0, &generation) && generation != 0);
    CHECK(SotnRoomActors_Scroll(actors, 280, 212));
    CHECK(SotnRoomActors_Get(actors, 0, &placement, &generation));
    CHECK(generation != 0 && placement.parameters == 9);
    CHECK(ReadSlot(actors, 4, &generation) && generation == 0);
    CHECK(!SotnRoomActors_Scroll(actors, SDL_MAX_SINT32, 0));
    CHECK(SotnRoomActors_Get(actors, 0, &placement, &generation) && placement.parameters == 9);
    CHECK(!SotnActorLayout_Get(layout, false, 6, &placement));
    CHECK(!SotnRoomActors_Get(actors, 192, &placement, &generation));
    CHECK(!SotnRoomActors_Release(actors, 192, true));
    SotnRoomActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    return true;
}

static bool TestInvalidLayouts(Fixture *fixture)
{
    SotnDataView image = MakeFixture(fixture);
    SotnActorLayout *layout = NULL;
    for (size_t size = 0; size < 0x350; ++size)
        CHECK(!SotnActorLayout_Open((SotnDataView){image.bytes, size}, 0, &layout) &&
              layout == NULL);
    CHECK(!SotnActorLayout_Open(image, 53, &layout));
    Put32(fixture->bytes + 28, 0x80180041);
    CHECK(!SotnActorLayout_Open(image, 0, &layout));
    image = MakeFixture(fixture);
    Put16(fixture->bytes + 0x210, 192);
    CHECK(!SotnActorLayout_Open(image, 0, &layout));
    image = MakeFixture(fixture);
    Put16(fixture->bytes + 0x314, 9);
    CHECK(!SotnActorLayout_Open(image, 0, &layout));
    image = MakeFixture(fixture);
    Put16(fixture->bytes + 0x20a, 400);
    CHECK(!SotnActorLayout_Open(image, 0, &layout));
    image = MakeFixture(fixture);
    Put16(fixture->bytes + 0x20e, 0x4e);
    CHECK(!SotnActorLayout_Open(image, 0, &layout));
    CHECK(!SotnActorLayout_Open((SotnDataView){0}, 0, &layout));
    image = MakeFixture(fixture);
    Put16(fixture->bytes + 0x20a, 0xffff);
    Put16(fixture->bytes + 0x20c, 0xffff);
    Put16(fixture->bytes + 0x30a, 0xffff);
    Put16(fixture->bytes + 0x30c, 0xffff);
    CHECK(SotnActorLayout_Open(image, 0, &layout));
    CHECK(SotnActorLayout_Count(layout) == 0);
    SotnRoomActors *actors = NULL;
    CHECK(SotnRoomActors_Create(&actors));
    CHECK(SotnRoomActors_Enter(actors, layout, 0, 0));
    CHECK(SotnRoomActors_Scroll(actors, 800, 800));
    CHECK(SotnRoomActors_Scroll(actors, 0, 0));
    SotnRoomActors_Destroy(actors);
    SotnActorLayout_Close(layout);
    CHECK(!SotnRoomActors_Create(NULL));
    SotnActorLayout_Close(NULL);
    SotnRoomActors_Destroy(NULL);
    return true;
}

int main(void)
{
    Fixture fixture = {0};
    if (!TestLayoutAndSpawn(&fixture) || !TestInvalidLayouts(&fixture))
        return 1;
    SDL_Log("SOTN_ACTOR_LAYOUT_TEST: PASS");
    return 0;
}
