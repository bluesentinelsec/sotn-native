#include <SDL3/SDL.h>
#include <sotn/animation.h>

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

static void Put32(Uint8 *image, size_t offset, Uint32 value)
{
    for (unsigned int i = 0; i < 4; ++i)
        image[offset + i] = (Uint8)(value >> (i * 8));
}

static void Record(Uint8 *image, size_t offset, Uint16 duration, Uint16 frame)
{
    Put32(image, offset, duration | (Uint32)frame << 16);
}

static bool TestAnimation(void)
{
    Uint8 image[0x11000] = {0};
    const SotnDataView dra = {image, sizeof(image)};
    Put32(image, 0x101b8, 0x800a0100);
    Put32(image, 0x101bc, 0x800a0200);
    image[0x10130] = 0xfe;
    image[0x10131] = 0xff;
    image[0x10132] = 7;
    image[0x10133] = 8;
    Record(image, 0x100, 2, 12);
    Record(image, 0x104, 1, 13);
    Record(image, 0x108, 0, 0);
    Record(image, 0x200, 3, 22);
    Record(image, 0x204, 0xffff, 0);
    SotnAnimation animation = {0};
    CHECK(SotnAnimation_Set(&animation, 0));
    CHECK(SotnAnimation_Tick(dra, &animation));
    CHECK(animation.pose == 0 && animation.remaining == 2 && animation.sprite == 12);
    CHECK(animation.hitbox_x == -2 && animation.hitbox_y == -1);
    CHECK(animation.hitbox_width == 7 && animation.hitbox_height == 8);
    CHECK(SotnAnimation_Tick(dra, &animation));
    CHECK(animation.remaining == 1 && animation.sprite == 12);
    CHECK(SotnAnimation_Tick(dra, &animation));
    CHECK(animation.pose == 1 && animation.remaining == 1 && animation.sprite == 13);
    CHECK(SotnAnimation_Tick(dra, &animation));
    CHECK(animation.pose == 0 && animation.remaining == 2 && animation.sprite == 12);
    Record(image, 0x108, 0xfffe, 1);
    for (int i = 0; i < 3; ++i)
        CHECK(SotnAnimation_Tick(dra, &animation));
    CHECK(animation.id == 1 && animation.pose == 0 && animation.remaining == 3 &&
          animation.sprite == 22);
    for (int i = 0; i < 10; ++i)
        CHECK(SotnAnimation_Tick(dra, &animation));
    CHECK(animation.remaining == -1 && animation.sprite == 22);
    Put32(image, 0x10594, 0x800a0300);
    Record(image, 0x100, 0xfffd, 0);
    Record(image, 0x104, 1, 15);
    Record(image, 0x108, 0, 0);
    Record(image, 0x300, 1, 12);
    Record(image, 0x304, 1, 13);
    Record(image, 0x308, 0xffff, 0);
    CHECK(SotnAnimation_Set(&animation, 0));
    const Uint16 expected[] = {12, 13, 15, 12, 13, 15};
    for (size_t i = 0; i < SDL_arraysize(expected); ++i)
    {
        CHECK(SotnAnimation_Tick(dra, &animation));
        CHECK(animation.sprite == expected[i]);
    }
    const Uint16 pose = animation.pose;
    const Sint32 remaining = animation.remaining;
    Put32(image, 0x101b8, 0x80090000);
    CHECK(!SotnAnimation_Tick(dra, &animation));
    CHECK(animation.pose == pose && animation.remaining == remaining);
    CHECK(!SotnAnimation_Tick((SotnDataView){0}, &animation));
    CHECK(!SotnAnimation_Set(&animation, 247));
    CHECK(!SotnAnimation_Set(NULL, 0));
    CHECK(!SotnAnimation_Tick(dra, NULL));
    animation.remaining = -2;
    CHECK(!SotnAnimation_Tick(dra, &animation));
    return true;
}

static bool TestStageAnimation(void)
{
    Uint8 image[0x4200] = {0};
    const SotnDataView overlay = {image, sizeof(image)};
    Put32(image, 0x3cb4, 0x80180100);
    Put32(image, 0x3cb8, 0x80180120);
    Put32(image, 0x3cbc, 0x80180140);
    Put32(image, 0x3cc0, 0x801801a0);
    Put32(image, 0x4024, 0x80180180);
    image[0x3c74] = 0xfd;
    image[0x3c75] = 4;
    image[0x3c76] = 5;
    image[0x3c77] = 6;
    Record(image, 0x100, 2, 7 | (1U << 9));
    Record(image, 0x104, 0xfffe, 1);
    Record(image, 0x120, 1, 8);
    Record(image, 0x124, 0xffff, 0);
    Record(image, 0x140, 0xfffd, 0);
    Record(image, 0x144, 0xffff, 0);
    Record(image, 0x180, 1, 9);
    Record(image, 0x184, 1, 10);
    Record(image, 0x188, 0xffff, 0);
    Record(image, 0x1a0, 1, 1 | (17U << 9));
    SotnAnimation cursor = {0};
    CHECK(SotnAnimation_SetStage(&cursor, 26, 0));
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor));
    CHECK(cursor.sprite == 7 && cursor.remaining == 2 && cursor.hitbox_x == -3 &&
          cursor.hitbox_y == 4 && cursor.hitbox_width == 5 && cursor.hitbox_height == 6);
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.remaining == 1);
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.id == 1 && cursor.sprite == 8);
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.remaining == -1);
    CHECK(SotnAnimation_SetStage(&cursor, 26, 2));
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.sprite == 9);
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.sprite == 10);
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.remaining == -1);
    CHECK(SotnAnimation_SetStage(&cursor, 26, 3));
    const SotnAnimation before = cursor;
    CHECK(!SotnAnimation_TickStage(overlay, 26, &cursor));
    CHECK(cursor.id == before.id && cursor.pose == before.pose &&
          cursor.remaining == before.remaining && cursor.sprite == before.sprite &&
          cursor.hitbox_x == before.hitbox_x && cursor.hitbox_y == before.hitbox_y &&
          cursor.hitbox_width == before.hitbox_width &&
          cursor.hitbox_height == before.hitbox_height);
    CHECK(!SotnAnimation_SetStage(&cursor, 26, 220));
    CHECK(!SotnAnimation_SetStage(&cursor, 1, 0));
    CHECK(!SotnAnimation_TickStage(overlay, 1, &cursor));
    CHECK(!SotnAnimation_TickStage((SotnDataView){image, 0x3cb4}, 26, &cursor));
    CHECK(cursor.id == before.id && cursor.pose == before.pose &&
          cursor.remaining == before.remaining && cursor.sprite == before.sprite &&
          cursor.hitbox_x == before.hitbox_x && cursor.hitbox_y == before.hitbox_y &&
          cursor.hitbox_width == before.hitbox_width &&
          cursor.hitbox_height == before.hitbox_height);
    Record(image, 0x1a0, 1, 1 | (19U << 9));
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor));
    CHECK(cursor.hitbox_x == 0x40 && cursor.hitbox_y == 1 && cursor.hitbox_width == 0x18 &&
          cursor.hitbox_height == 0x80);
    CHECK(SotnAnimation_SetStage(&cursor, 26, 3));
    Record(image, 0x1a0, 1, 1 | (18U << 9));
    CHECK(!SotnAnimation_TickStage(overlay, 26, &cursor));
    CHECK(SotnAnimation_SetStage(&cursor, 26, 2));
    Record(image, 0x140, 0xfffd, 29);
    CHECK(!SotnAnimation_TickStage(overlay, 26, &cursor));
    CHECK(cursor.pose == 0 && cursor.remaining == 0);
    CHECK(!SotnAnimation_SetStage(NULL, 26, 0));
    CHECK(!SotnAnimation_TickStage(overlay, 26, NULL));
    return true;
}

static bool TestDoppleFormPose(void)
{
    Uint8 image[0x4200] = {0};
    const SotnDataView overlay = {image, sizeof(image)};
    Put32(image, 0x3cb4 + 202 * 4, 0x80183b0c);
    Put32(image, 0x3cb4, 0x80180100);
    Record(image, 0x3b0c, 1, 6);
    Record(image, 0x3b10, 0xffff, 0);
    Record(image, 0x100, 1, 17);
    Record(image, 0x104, 0xffff, 0);
    SotnAnimation cursor = {0};
    CHECK(SotnAnimation_SetStage(&cursor, 26, 202));
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.sprite == 6);
    CHECK(SotnAnimation_SetStage(&cursor, 26, 202));
    CHECK(SotnAnimation_TickDopple(overlay, 99, &cursor) && cursor.sprite == 99);
    CHECK(SotnAnimation_TickDopple(overlay, 100, &cursor) && cursor.sprite == 100 &&
          cursor.remaining == -1 && image[0x3b0e] == 6);
    CHECK(SotnAnimation_TickStage(overlay, 26, &cursor) && cursor.sprite == 6);
    CHECK(!SotnAnimation_TickDopple(overlay, 512, &cursor) && cursor.sprite == 6);
    CHECK(SotnAnimation_SetStage(&cursor, 26, 0));
    CHECK(SotnAnimation_TickDopple(overlay, 101, &cursor) && cursor.sprite == 17);
    CHECK(!SotnAnimation_TickDopple(overlay, 6, NULL));
    return true;
}

static bool TestPlayerFormPose(void)
{
    Uint8 image[0x11000] = {0};
    const SotnDataView dra = {image, sizeof(image)};
    Put32(image, 0x101b8 + 202U * 4U, 0x800afda4);
    Put32(image, 0x101b8, 0x800a0100);
    Record(image, 0xfda4, 1, 6);
    Record(image, 0xfda8, 0xffff, 0);
    Record(image, 0x100, 1, 17);
    Record(image, 0x104, 0xffff, 0);
    SotnAnimation cursor = {0};
    CHECK(SotnAnimation_Set(&cursor, 202));
    CHECK(SotnAnimation_TickPlayerForm(dra, 24, &cursor) && cursor.sprite == 24);
    CHECK(SotnAnimation_TickPlayerForm(dra, 6, &cursor) && cursor.sprite == 6 &&
          cursor.remaining == -1 && image[0xfda6] == 6);
    const SotnAnimation before = cursor;
    CHECK(!SotnAnimation_TickPlayerForm(dra, 512, &cursor) && cursor.sprite == before.sprite &&
          cursor.pose == before.pose);
    CHECK(SotnAnimation_Set(&cursor, 0));
    CHECK(SotnAnimation_TickPlayerForm(dra, 24, &cursor) && cursor.sprite == 17);
    CHECK(!SotnAnimation_TickPlayerForm(dra, 6, NULL));
    return true;
}

int main(void)
{
    if (!TestAnimation() || !TestStageAnimation() || !TestDoppleFormPose() || !TestPlayerFormPose())
        return 1;
    SDL_Log("SOTN_ANIMATION_TEST: PASS");
    return 0;
}
