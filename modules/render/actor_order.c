// Original dra/42398.c uses forward ClearOTag and draws from g_CurrentOT,
// unlike ClearOTagR. Higher original priorities cover lower priorities.
#include <sotn/actor_order.h>
#include <sotn/actors.h>
bool SotnActorOrder_Add(SotnActorDraw *items, size_t capacity, size_t *count, SotnActorDraw item)
{
    if (items == NULL || count == NULL || *count >= capacity || item.priority > 511 ||
        item.actor >= SOTN_STAGE_ACTOR_SLOTS || item.kind > SOTN_DRAW_HELLFIRE_SPRITE ||
        ((item.kind == SOTN_DRAW_SPIRIT || item.kind == SOTN_DRAW_SPIRIT_PORTAL ||
          item.kind == SOTN_DRAW_HELLFIRE_SPRITE) &&
         item.primitive >= 48) ||
        (item.kind == SOTN_DRAW_WOLF_PACKET && item.primitive >= 60) ||
        (item.kind == SOTN_DRAW_WOLF_IMAGE && item.primitive >= 6) ||
        (item.kind == SOTN_DRAW_WOLF_TARGET && item.primitive >= 2) ||
        (item.kind == SOTN_DRAW_PLAYER_SMOKE && item.primitive >= 48) ||
        (item.kind == SOTN_DRAW_PLAYER_PARTICLE && item.primitive >= 768) ||
        (item.kind == SOTN_DRAW_BAT_ECHO && item.primitive >= SOTN_PRIMITIVE_SLOTS) ||
        (item.kind == SOTN_DRAW_BAT_FIREBALL && item.primitive >= 48) ||
        (item.kind == SOTN_DRAW_MIST && item.primitive >= 80) ||
        (item.kind == SOTN_DRAW_PLAYER_BLINK && item.primitive >= 384) ||
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
