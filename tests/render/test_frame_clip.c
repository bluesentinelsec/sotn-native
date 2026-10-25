#include <SDL3/SDL.h>
#include <sotn/frame_clip.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s", __LINE__, SDL_GetError());                                       \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint16 frame[SOTN_FRAME_PIXELS], saved[SOTN_FRAME_PIXELS], before[SOTN_FRAME_PIXELS];
} Fixture;
static bool Draw(Fixture *f, SotnFrameClip clip)
{
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = (Uint16)(i & 0x7fffU);
    SDL_memcpy(f->before, f->frame, sizeof(f->frame));
    CHECK(SotnFrameClip_CopyOutside(clip, f->frame, f->saved, SOTN_FRAME_PIXELS));
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = 0xffff;
    CHECK(SotnFrameClip_CopyOutside(clip, f->saved, f->frame, SOTN_FRAME_PIXELS));
    for (Sint32 y = 0; y < SOTN_FRAME_HEIGHT; ++y)
        for (Sint32 x = 0; x < SOTN_FRAME_WIDTH; ++x)
        {
            const size_t i = (size_t)y * SOTN_FRAME_WIDTH + (size_t)x;
            const bool inside =
                x >= clip.left && x < clip.right && y >= clip.top && y < clip.bottom;
            CHECK(f->frame[i] == (inside ? 0xffff : f->before[i]));
        }
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok =
        Draw(f, (SotnFrameClip){0, 0, 256, 240}) && Draw(f, (SotnFrameClip){0, 170, 256, 207}) &&
        Draw(f, (SotnFrameClip){0, 220, 256, 207}) && Draw(f, (SotnFrameClip){16, 20, 201, 227}) &&
        Draw(f, (SotnFrameClip){-10, -20, 300, 500}) &&
        Draw(f, (SotnFrameClip){300, 400, 500, 600}) &&
        Draw(f, (SotnFrameClip){SDL_MIN_SINT32, SDL_MIN_SINT32, SDL_MAX_SINT32, SDL_MAX_SINT32});
    if (ok)
    {
        const SotnFrameClip clip = {0, 20, 256, 227};
        ok = !SotnFrameClip_CopyOutside(clip, NULL, f->frame, SOTN_FRAME_PIXELS) &&
             !SotnFrameClip_CopyOutside(clip, f->saved, NULL, SOTN_FRAME_PIXELS) &&
             !SotnFrameClip_CopyOutside(clip, f->frame, f->frame, SOTN_FRAME_PIXELS) &&
             !SotnFrameClip_CopyOutside(clip, f->saved, f->frame, SOTN_FRAME_PIXELS - 1);
    }
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_FRAME_CLIP: PASS");
    return 0;
}
