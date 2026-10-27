// Original dra/42398.c uses forward ClearOTag and draws from g_CurrentOT,
// unlike ClearOTagR. Higher original priorities cover lower priorities.
#include <sotn/actor_order.h>
#include <sotn/actors.h>
bool SotnActorOrder_Add(SotnActorDraw *items, size_t capacity, size_t *count, SotnActorDraw item)
{
    if (items == NULL || count == NULL || *count >= capacity || item.priority > 511 ||
        item.actor >= SOTN_STAGE_ACTOR_SLOTS || item.kind > SOTN_DRAW_PLAYER_BLINK ||
        (item.kind == SOTN_DRAW_MIST && item.primitive >= 80) ||
        (item.kind == SOTN_DRAW_PLAYER_BLINK && item.primitive >= 16) ||
        (item.kind == SOTN_DRAW_PRIMITIVE && item.primitive >= SOTN_ACTOR_PRIMITIVES_PER_ACTOR_MAX))
        return SDL_SetError(
            "Actor draw order requires bounded commands and available frame storage");
    size_t position = *count;
    while (position > 0 && items[position - 1].priority > item.priority)
    {
        items[position] = items[position - 1];
        --position;
    }
    items[position] = item;
    ++*count;
    return true;
}
