// Original dra/42398.c uses forward ClearOTag and draws from g_CurrentOT,
// unlike ClearOTagR. Higher original priorities cover lower priorities.
#include <sotn/actor_order.h>
#include <sotn/actors.h>
#include <sotn/player_particles.h>
#include <sotn/subweapon_entity.h>
bool SotnActorOrder_Add(SotnActorDraw *items, size_t capacity, size_t *count, SotnActorDraw item)
{
    if (items == NULL || count == NULL || *count >= capacity || item.priority > 511 ||
        item.actor >= SOTN_STAGE_ACTOR_SLOTS || item.kind > SOTN_DRAW_SUBWEAPON_QUAD ||
        (item.kind == SOTN_DRAW_SUBWEAPON_QUAD &&
         (item.primitive < 16 * 64 || item.primitive >= 48 * 64)) ||
        (item.kind == SOTN_DRAW_SUBWEAPON_LINE &&
         (item.primitive < 16 * 16 || item.primitive >= 32 * 16)) ||
        (item.kind == SOTN_DRAW_SUBWEAPON_TILE &&
         (item.primitive < 16 * 16 || item.primitive >= 32 * 16 ||
          item.primitive % 16U >= SOTN_VIBHUTI_PARTICLES)) ||
        (item.kind == SOTN_DRAW_WEAPON_SECONDARY && (item.primitive < 1 || item.primitive > 47)) ||
        (item.kind == SOTN_DRAW_WEAPON_SECONDARY_LINE &&
         (item.primitive < 64 || item.primitive >= 48 * 64)) ||
        (item.kind == SOTN_DRAW_WEAPON_SECONDARY_QUAD && item.primitive >= 48 * 64) ||
        (item.kind == SOTN_DRAW_WEAPON_SPARKLE &&
         (item.primitive < 80 || item.primitive >= 48 * 80)) ||
        (item.kind == SOTN_DRAW_HAND_WEAPON && item.primitive != 0) ||
        (item.kind == SOTN_DRAW_HAND_TILE && item.primitive >= 16) ||
        ((item.kind == SOTN_DRAW_FOOD || item.kind == SOTN_DRAW_NEUTRON ||
          item.kind == SOTN_DRAW_SIRE || item.kind == SOTN_DRAW_MAGIC_MISSILE) &&
         (item.primitive < 1 || item.primitive > 15)) ||
        ((item.kind == SOTN_DRAW_THROWN || item.kind == SOTN_DRAW_STAR) &&
         (item.primitive < 1 || item.primitive > 47)) ||
        (item.kind == SOTN_DRAW_STAR_SPARK && item.primitive >= 48 * 32) ||
        (item.kind == SOTN_DRAW_KARMA && (item.primitive < 1 || item.primitive > 31)) ||
        (item.kind == SOTN_DRAW_KARMA_QUAD && (item.primitive < 26 || item.primitive >= 16 * 26)) ||
        (item.kind == SOTN_DRAW_PENTAGRAM && (item.primitive < 25 || item.primitive >= 400)) ||
        (item.kind == SOTN_DRAW_FOOD_TEXT && item.primitive >= 68) ||
        ((item.kind == SOTN_DRAW_FAIRY_SPRITE || item.kind == SOTN_DRAW_FAIRY_TILE ||
          item.kind == SOTN_DRAW_FAIRY_LIGHT) &&
         item.actor >= 4) ||
        (item.kind == SOTN_DRAW_FAIRY_TILE && item.primitive >= SOTN_FAIRY_ITEM_PARTICLES) ||
        (item.kind == SOTN_DRAW_FAIRY_LIGHT && item.primitive >= SOTN_FAIRY_APPLE_ROWS) ||
        (item.kind == SOTN_DRAW_TELEPORT && item.primitive >= 48 * 36) ||
        (item.kind == SOTN_DRAW_PLAYER_CIRCLE && item.primitive >= 48 * 32) ||
        (item.kind == SOTN_DRAW_HOLY_HIT && item.primitive >= 48 * 16) ||
        (item.kind == SOTN_DRAW_STONE_DISSOLVE && item.primitive >= 48 * 36) ||
        (item.kind == SOTN_DRAW_ICE_HIT && item.primitive >= 48 * 24) ||
        (item.kind == SOTN_DRAW_THUNDER_HIT && item.primitive >= 48 * 6) ||
        (item.kind == SOTN_DRAW_HEALING_NUMBER &&
         (item.primitive < 800 || item.primitive >= 960)) ||
        (item.kind == SOTN_DRAW_IMMUNITY_TEXT && item.primitive >= 48 * 18) ||
        ((item.kind == SOTN_DRAW_SPIRIT || item.kind == SOTN_DRAW_SPIRIT_PORTAL ||
          item.kind == SOTN_DRAW_HELLFIRE_SPRITE || item.kind == SOTN_DRAW_RISING_HEART ||
          item.kind == SOTN_DRAW_SWORD || item.kind == SOTN_DRAW_AXE_SHADOW ||
          item.kind == SOTN_DRAW_DARK_HIT || item.kind == SOTN_DRAW_FIRE_HIT) &&
         item.primitive >= 48) ||
        (item.kind == SOTN_DRAW_WOLF_PACKET && item.primitive >= 60) ||
        (item.kind == SOTN_DRAW_WOLF_IMAGE && item.primitive >= 6) ||
        (item.kind == SOTN_DRAW_WOLF_TARGET && item.primitive >= 2) ||
        (item.kind == SOTN_DRAW_PLAYER_SMOKE && item.primitive >= 48) ||
        (item.kind == SOTN_DRAW_PLAYER_PARTICLE &&
         item.primitive >= 48 * SOTN_PLAYER_PARTICLE_LIMIT) ||
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
