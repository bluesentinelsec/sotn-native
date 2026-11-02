#include <SDL3/SDL.h>
#include <sotn/food.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s %s", __LINE__, #v, SDL_GetError());                                 \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static Uint8 tiles[512], types[2];
static SotnFoodInput Input(void)
{
    SDL_memset(tiles, 0, sizeof(tiles));
    types[0] = 0;
    types[1] = 3;
    return (SotnFoodInput){
        .collision = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 16, 16},
        .player = {.x = 40 * 65536, .y = 80 * 65536, .width = 8, .height = 16, .priority = 148},
        .attack = 37};
}
static bool Births(void)
{
    SotnFoodInput in = Input();
    SotnFoodEvents e = {0};
    for (Uint16 id = 29; id <= 70; ++id)
        for (Uint16 hand = 0; hand < 2; ++hand)
        {
            SotnFood f = {.x = 40 * 65536 + 123,
                          .y = 80 * 65536 + 456,
                          .parameter = (Uint16)((id - 29U) * 256U + hand * 0x8000U),
                          .item = id};
            in.player.facing_left = hand != 0;
            Uint32 rng = 0x12345678;
            CHECK(SotnFood_Tick(&f, in, &rng, &e) && !f.removed && !e.heal && !e.collect &&
                  !e.poison);
            if (id == 70)
                CHECK(f.phase == 5 && f.item >= 29 && f.item <= 69 &&
                      f.item - 29U == ((f.parameter >> 8) & 127U) && !f.frame &&
                      f.x == 40 * 65536 + 123 && f.y == 80 * 65536 + 456 && !e.clear_weapon);
            else
            {
                CHECK(f.phase == 1 && f.frame == id - 28U && f.attack == 37 && f.priority == 150 &&
                      !f.facing_left && e.clear_weapon && f.y == 76 * 65536 + 456 &&
                      (f.x & 65535) == 123 && (hand ? f.velocity_x < 0 : f.velocity_x > 0));
                if (id == 35)
                    CHECK(f.bounce == -360448 && f.velocity_y == -360448 &&
                          f.x == (hand ? 32 : 48) * 65536 + 123);
                else
                    CHECK(f.velocity_y == -131072 && f.x == 40 * 65536 + 123);
            }
        }
    return true;
}
static bool HealingAndPoison(void)
{
    SotnFoodInput in = Input();
    SotnFoodEvents e = {0};
    Uint32 rng = 123;
    SotnFood f = {.x = in.player.x, .y = in.player.y, .phase = 2, .timer = 100, .attack = 37};
    in.player.healing_pending = true;
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.phase == 3 && f.timer == 8 && !f.frame && !e.heal);
    for (Uint16 i = 0; i < 8; ++i)
        CHECK(SotnFood_Tick(&f, in, &rng, &e) && !e.heal && !f.removed);
    CHECK(f.timer == 8);
    in.player.healing_pending = false;
    for (Uint16 i = 0; i < 7; ++i)
        CHECK(SotnFood_Tick(&f, in, &rng, &e) && !e.heal && !f.removed);
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && e.heal && e.heal_amount == 37 && f.removed);
    for (Uint16 ground = 0; ground < 2; ++ground)
    {
        f = (SotnFood){.x = in.player.x,
                       .y = in.player.y,
                       .velocity_y = 2 * 65536,
                       .phase = ground ? 2 : 1,
                       .parameter = 7 * 256,
                       .timer = 100};
        CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.phase == 4 && f.timer == 12 &&
              e.simulated_ticks == 20 && e.crouch == !ground && !e.poison && !e.heal);
        for (Uint16 i = 0; i < 11; ++i)
            CHECK(SotnFood_Tick(&f, in, &rng, &e) && !e.poison && !f.removed);
        CHECK(SotnFood_Tick(&f, in, &rng, &e) && e.poison && f.removed && !e.heal);
    }
    f = (SotnFood){.phase = 2, .timer = 24, .x = 200 * 65536, .y = 100 * 65536};
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.draw_flags == 0x80 && f.timer == 23);
    return true;
}
static bool Peanut(void)
{
    SotnFoodInput in = Input();
    SotnFoodEvents e = {0};
    Uint32 rng = 123;
    SotnFood f = {.phase = 1,
                  .parameter = 6 * 256,
                  .x = in.player.x + 4 * 65536,
                  .y = in.player.y - 20 * 65536,
                  .velocity_y = 2 * 65536,
                  .attack = 50};
    in.player.frame = 0x7b;
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && e.peanut_pose && e.heal && e.heal_amount == 50 &&
          f.removed);
    f.removed = false;
    in.player.animation = 1;
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && !e.peanut_pose && !e.heal && !f.removed);
    return true;
}
typedef struct Factory
{
    Uint32 packed, calls;
    bool available;
} Factory;
static bool Reserve(void *context, Sint32 x, Sint32 y, Uint32 packed, bool *allocated)
{
    Factory *f = context;
    (void)x;
    (void)y;
    f->packed = packed;
    ++f->calls;
    *allocated = f->available;
    return true;
}
static bool TicketAndInventory(void)
{
    SotnFoodInput in = Input();
    SotnFoodEvents e = {0};
    Uint32 rng = 123;
    SotnFood f = {.phase = 5, .item = 35, .parameter = 0x8000 | 6 * 256}, held = f;
    CHECK(!SotnFood_Tick(&f, in, &rng, &e) && !SDL_memcmp(&f, &held, sizeof(f)) && rng == 123);
    Factory factory = {0};
    in.factory = Reserve;
    in.context = &factory;
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.phase == 5 && factory.calls == 1 &&
          factory.packed == (35U << 16 | 0x8038));
    factory.available = true;
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.phase == 6 && f.timer == 4 && rng == 123);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnFood_Tick(&f, in, &rng, &e) && !f.removed);
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.removed);
    f = (SotnFood){.inventory = true,
                   .phase = 2,
                   .timer = 2,
                   .parameter = 35 * 256,
                   .x = in.player.x,
                   .y = in.player.y};
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.removed && e.collect && e.item == 35 && !e.heal);
    f.removed = false;
    f.timer = 1;
    CHECK(SotnFood_Tick(&f, in, &rng, &e) && f.removed && !e.collect);
    return true;
}
int main(void)
{
    return Births() && HealingAndPoison() && Peanut() && TicketAndInventory() ? 0 : 1;
}
