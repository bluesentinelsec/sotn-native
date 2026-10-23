#include <SDL3/SDL.h>
#include <sotn/stage.h>

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

static void Put32(Uint8 *bytes, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        bytes[offset + i] = (Uint8)(value >> (i * 8));
}

static void MakeImage(Uint8 image[2048])
{
    SDL_memset(image, 0, 2048);
    Put32(image, 16, 0x80180040);
    Put32(image, 32, 0x80180060);
    const Uint8 rooms[] = {1, 2, 2, 2, 0, 0, 5, 6, 3, 2, 3, 2, 0, 255, 0, 0, 64};
    SDL_memcpy(image + 0x40, rooms, sizeof(rooms));
    Put32(image, 0x60, 0x80180080);
    Put32(image, 0x64, 0x80180090);
    Put32(image, 0x80, 0x80180100);
    Put32(image, 0x84, 0x801800a0);
    Put32(image, 0x88, 1U | 2U << 6 | 2U << 12 | 2U << 18 | 1U << 24);
    Put32(image, 0x8c, 0x00030060);
    Put32(image, 0xa0, 0x80180700);
    Put32(image, 0xa4, 0x80180710);
    Put32(image, 0xa8, 0x80180720);
    Put32(image, 0xac, 0x80180730);
    image[0x100] = 1;
    image[0x731] = 3;
}

static bool TestStage(void)
{
    Uint8 image[2048] = {0};
    MakeImage(image);
    SotnStage *stage = NULL;
    CHECK(SotnStage_Open((SotnDataView){image, sizeof(image)}, &stage));
    CHECK(stage != NULL && SotnStage_RoomCount(stage) == 2);
    SotnStageRoom room = {0};
    CHECK(SotnStage_GetRoom(stage, 0, &room));
    CHECK(!room.transition && room.left == 1 && room.right == 2);
    CHECK(room.object_graphics_id == 5 && room.object_layout_id == 6);
    CHECK(room.foreground.present && !room.background.present);
    CHECK(room.foreground.collision.width == 32 && room.foreground.collision.height == 16);
    CHECK(room.foreground.collision.tiles.size == 1024);
    CHECK(room.foreground.collision.types.size == 2 && room.foreground.priority == 0x60);
    CHECK(room.foreground.flags == 3 && room.foreground.parameters == 1);
    SotnCollider collider = {0};
    CHECK(SotnCollision_Query(&room.foreground.collision, 0, 0, &collider));
    CHECK(collider.effects == SOTN_EFFECT_BLOCK);
    CHECK(SotnCollision_Query(&room.foreground.collision, 16, 0, &collider));
    CHECK(collider.effects == 0);
    CHECK(SotnStage_GetRoom(stage, 1, &room));
    CHECK(room.transition && !room.foreground.present && !room.background.present);
    room.left = 33;
    CHECK(!SotnStage_GetRoom(stage, 2, &room));
    CHECK(room.left == 33);
    CHECK(!SotnStage_GetRoom(stage, SDL_MAX_UINT32, &room));
    CHECK(!SotnStage_GetRoom(stage, 0, NULL));
    CHECK(!SotnStage_GetRoom(NULL, 0, &room));
    SotnStage_Close(stage);
    for (size_t size = 0; size < 0x732; ++size)
    {
        stage = NULL;
        CHECK(!SotnStage_Open((SotnDataView){image, size}, &stage));
        CHECK(stage == NULL);
    }
    CHECK(SotnStage_Open((SotnDataView){image, 0x732}, &stage));
    SotnStage_Close(stage);
    CHECK(SotnStage_RoomCount(NULL) == 0);
    SotnStage_Close(NULL);
    return true;
}

static bool TestBadReferences(void)
{
    static const struct
    {
        size_t offset;
        Uint32 value;
    } cases[] = {
        {16, 0},
        {16, 0x80180041},
        {32, 0xfffffff0},
        {0x60, 0x801807f4},
        {0x64, 0},
        {0x80, 0x80180101},
        {0x84, 0x801807ff},
        {0xa0, 0x801807ff},
        {0xac, 0x8017fff0},
        {0x88, 1U | 2U << 6 | 0U << 12 | 2U << 18},
    };
    Uint8 image[2048] = {0};
    for (size_t i = 0; i < SDL_arraysize(cases); ++i)
    {
        MakeImage(image);
        Put32(image, cases[i].offset, cases[i].value);
        SotnStage *stage = NULL;
        CHECK(!SotnStage_Open((SotnDataView){image, sizeof(image)}, &stage));
        CHECK(stage == NULL);
    }
    SotnStage *stage = NULL;
    MakeImage(image);
    image[0x40] = 64;
    CHECK(!SotnStage_Open((SotnDataView){image, sizeof(image)}, &stage));
    MakeImage(image);
    image[0x101] = 255;
    CHECK(!SotnStage_Open((SotnDataView){image, sizeof(image)}, &stage));
    MakeImage(image);
    image[0x42] = 64;
    CHECK(!SotnStage_Open((SotnDataView){image, sizeof(image)}, &stage));
    CHECK(!SotnStage_Open((SotnDataView){image, 0x80001}, &stage));
    CHECK(!SotnStage_Open((SotnDataView){0}, &stage));
    CHECK(!SotnStage_Open((SotnDataView){image, sizeof(image)}, NULL));
    return true;
}

int main(void)
{
    if (!TestStage() || !TestBadReferences())
        return 1;
    SDL_Log("SOTN_STAGE_TEST: PASS");
    return 0;
}
