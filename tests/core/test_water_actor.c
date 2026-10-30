#include <SDL3/SDL.h>
#include <sotn/water_actor.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put(Uint8 *p, Sint16 v)
{
    Uint16 b = (Uint16)v;
    p[0] = (Uint8)b;
    p[1] = (Uint8)(b >> 8);
}
static void Place(SotnWaterActorInput *in, Sint16 x, Sint16 y)
{
    in->volume.player_x = x;
    in->volume.player_y = y;
    in->x = x * 65536 + 123;
    in->y = y * 65536 + 456;
}
static bool Controller(void)
{
    Uint8 row[16] = {0}, aspects[16] = {0};
    Put(row + 2, 200);
    Put(row + 6, 100);
    Put(row + 8, 300);
    Put(aspects + 2, 3);
    SotnWaterActorInput in = {.volume = {.volumes = {row, sizeof(row)},
                                         .aspects = {aspects, sizeof(aspects)},
                                         .parameters = 0x100},
                              .priority = 148};
    Place(&in, 100, 75);
    SotnWaterActor a = {0};
    SotnWaterActorEvents e = {0};
    CHECK(SotnWaterActor_Tick(&a, in, &e) && a.phase == 1 && a.raw == 0x8000 && !e.depths_changed &&
          !e.count);
    a = (SotnWaterActor){.phase = 1};
    Place(&in, 100, 76);
    in.velocity_y = 5 * 65536;
    in.occupied = 5;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.depths_changed && e.count == 1 && e.attempts == 1 &&
          e.reserved == 2 && e.children[0].slot == 225 && e.children[0].kind == SOTN_WATER_SPLASH &&
          e.children[0].parameters == 0 && e.children[0].x == 100 * 65536 &&
          e.children[0].y == 100 * 65536);
    a = (SotnWaterActor){.phase = 1, .raw = 0x8005, .x = 100, .y = 105, .left = 100, .right = 100};
    Place(&in, 100, 60);
    in.velocity_y = -32768;
    in.occupied = 0;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.count == 1 &&
          e.children[0].kind == SOTN_WATER_SPLASH && e.children[0].parameters == 1 &&
          e.children[0].y == 100 * 65536 && !e.volume.raw);
    Put(row + 4, 1);
    Place(&in, 100, 43);
    in.velocity_y = 5 * 65536;
    in.occupied = ~(((Uint32)1 << 3) | ((Uint32)1 << 17));
    a = (SotnWaterActor){.phase = 1};
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.count == 2 && e.attempts == 3 &&
          e.children[0].slot == 227 && e.children[1].slot == 241 &&
          e.children[0].kind == SOTN_WATER_SIDE_SPLASH && e.children[0].parameters == 16 &&
          e.children[1].parameters == 17 && e.children[0].y == 67 * 65536);
    Put(row + 4, 0);
    Place(&in, 100, 75);
    in.velocity_y = 0;
    in.velocity_x = 65536;
    in.occupied = 0;
    a = (SotnWaterActor){.phase = 1, .x = 99};
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.count == 1 &&
          e.children[0].kind == SOTN_WATER_SURFACING && e.children[0].parameters == 4 &&
          a.cooldown == 7);
    CHECK(SotnWaterActor_Tick(&a, in, &e) && !e.count && a.cooldown == 6);
    a = (SotnWaterActor){.phase = 1, .x = 99};
    in.occupied = SDL_MAX_UINT32;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && !e.count && e.attempts == 1 && !e.reserved &&
          !a.cooldown);
    Put(row + 14, 8192);
    Place(&in, 100, 125);
    in.occupied = 0;
    a = (SotnWaterActor){.phase = 1};
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.current_x == 65536 && a.x == 100);
    in.contacts = 4;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && !e.current_x);
    Put(row + 14, -8192);
    in.contacts = 0;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.current_x == -65536);
    in.contacts = 8;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && !e.current_x);
    Put(row + 14, 8192);
    in.contacts = 0;
    in.volume.status = 1;
    Place(&in, 100, 145);
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.current_x == 98304);
    in.volume.status = 2;
    CHECK(SotnWaterActor_Tick(&a, in, &e) && !e.current_x && !e.count);
    in.volume.status = 0;
    Place(&in, 100, 76);
    CHECK(SotnWaterActor_Tick(&a, in, &e) && e.current_x == 1632);
    a = (SotnWaterActor){.phase = 1, .raw = 0x8005, .row = 8};
    Place(&in, 100, 60);
    in.velocity_y = -32768;
    const SotnWaterActor saved = a;
    const SotnWaterActorEvents prior = e;
    CHECK(!SotnWaterActor_Tick(&a, in, &e) && !SDL_memcmp(&a, &saved, sizeof(a)) &&
          !SDL_memcmp(&e, &prior, sizeof(e)));
    a = (SotnWaterActor){0};
    in.volume.player_x = 99;
    CHECK(!SotnWaterActor_Tick(&a, in, &e) && !a.phase);
    return true;
}
int main(void)
{
    if (!Controller())
        return 1;
    SDL_Log("WATER_ACTOR: PASS authored "
            "birth/entry/exit/partial-capacity/surfacing/current/rollback contexts");
    return 0;
}
