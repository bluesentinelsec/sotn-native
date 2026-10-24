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

int main(void)
{
    if (!TestAnimation())
        return 1;
    SDL_Log("SOTN_ANIMATION_TEST: PASS");
    return 0;
}
