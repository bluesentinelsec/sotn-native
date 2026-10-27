#include <sotn/actor_graphics.h>

static bool Close(SotnActorGraphics *graphics, SotnActorSurface main, SotnActorTargetCursor *cursor)
{
    if (cursor->mode && !SotnActorGraphics_EndTarget(graphics, true))
        return false;
    *cursor = (SotnActorTargetCursor){.surface = main};
    return true;
}
bool SotnActorGraphics_ApplyTarget(SotnActorGraphics *graphics, SotnActorSurface main,
                                   SotnActorPrimitive p, SotnActorTargetCursor *cursor)
{
    if (!graphics || !cursor || !main.pixels || !main.width || main.width > 256 || !main.height ||
        main.height > 256 || main.count < (size_t)main.width * main.height ||
        (cursor->mode != 0 && cursor->mode != 1 && cursor->mode != 3))
        return SDL_SetError(
            "Ordered actor target requires an owned texture service and valid cursor");
    if (p.target == 1 || p.target == 3)
    {
        if (p.clip_left < 0 || p.clip_top < 0 || p.clip_right <= p.clip_left ||
            p.clip_bottom <= p.clip_top || p.clip_right > 1024 || p.clip_bottom > 512 ||
            p.clip_right - p.clip_left > 256 || p.clip_bottom - p.clip_top > 256 ||
            (cursor->mode && (p.target != 3 || cursor->mode != 3)))
            return SDL_SetError("Ordered actor target has invalid bounds or nesting");
        const SotnActorTarget target = {
            .x = (Uint16)p.clip_left,
            .y = (Uint16)p.clip_top,
            .width = (Uint16)(p.clip_right - p.clip_left),
            .height = (Uint16)(p.clip_bottom - p.clip_top),
            .clear = p.target_clear,
            .color = (Uint16)((p.red / 8U) | (p.green / 8U) << 5 | (p.blue / 8U) << 10)};
        if (cursor->mode && !Close(graphics, main, cursor))
            return false;
        SotnActorSurface surface = {0};
        if (!SotnActorGraphics_BeginTarget(graphics, target, &surface))
            return false;
        *cursor = (SotnActorTargetCursor){.surface = surface,
                                          .offset_x = p.target_offset_x - p.clip_left,
                                          .offset_y = p.target_offset_y - p.clip_top,
                                          .mode = p.target};
        return true;
    }
    if ((p.target == 2 && cursor->mode == 1) ||
        (p.target == 4 && (cursor->mode == 0 || cursor->mode == 3)))
        return Close(graphics, main, cursor);
    return SDL_SetError("Ordered actor target lost its matching restore owner");
}
