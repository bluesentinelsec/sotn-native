#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8));
}
typedef struct Fixture
{
    Uint8 dra[1024], nz0[0x3000], tiles[32 * 128 * 2], types[2048];
} Fixture;
static bool Open(Fixture *f, SotnActors **out, SotnActorLayout **layout)
{
    Put32(f->dra + 0x98, 0x800a0200);
    Put32(f->nz0 + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->nz0 + 0x40 + i * 4, 0x80180300);
    Put16(f->nz0 + 0x300, 0xfffe);
    Put16(f->nz0 + 0x302, 0xfffe);
    Put16(f->nz0 + 0x30a, 0xffff);
    Put16(f->nz0 + 0x30c, 0xffff);
    CHECK(SotnActorLayout_Open((SotnDataView){f->nz0, sizeof(f->nz0)}, 0, layout));
    CHECK(SotnActors_Create((SotnDataView){f->dra, sizeof(f->dra)},
                            (SotnDataView){f->nz0, sizeof(f->nz0)}, out));
    CHECK(SotnActors_Enter(
        *out, *layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 128}, 0,
        0));
    return true;
}
static bool Map(void)
{
    SotnWorldState state = {0};
    CHECK(SotnWorld_Visit(&state, 0, 0, false));
    CHECK(state.map[0] == 64 && state.rooms == 1);
    CHECK(SotnWorld_Visit(&state, 0, 0, false) && state.rooms == 1);
    CHECK(SotnWorld_Visit(&state, 3, 0, false) && state.map[0] == 65 && state.rooms == 2);
    CHECK(SotnWorld_Visit(&state, 63, 63, true) && state.map[2047] == 1 && state.rooms == 3);
    CHECK(SotnWorld_Validate(&state));
    CHECK(!SotnWorld_Visit(&state, 64, 0, false) && state.rooms == 3);
    CHECK(!SotnWorld_Visit(&state, -1, 0, true) && state.rooms == 3);
    state.map[0] |= 0xaa;
    CHECK(SotnWorld_Validate(&state));
    ++state.rooms;
    CHECK(!SotnWorld_Validate(&state));
    --state.rooms;
    state.flags[SOTN_LAB_CANNON] = 2;
    CHECK(!SotnWorld_Validate(&state));
    return true;
}
static bool Collision(SotnActors *s)
{
    Actor a = {.state = {.x = 128 * 65536, .y = 120 * 65536}};
    s->player =
        (SotnPlayerState){.x = 128 * 65536, .y = 94 * 65536, .solid_width = 7, .solid_height = 24};
    s->motion = (SotnActorMotion){0};
    Uint16 result = 0;
    CHECK(ActorObjectCollision(s, &a, 16, 5, 4, &result));
    CHECK(result == 4 && s->motion.y == -3 * 65536 && s->motion.contacts == 0x41);
    CHECK(s->player.y == 91 * 65536);
    s->player.y = 147 * 65536;
    s->player.velocity_y = -65536;
    s->motion = (SotnActorMotion){0};
    CHECK(ActorObjectCollision(s, &a, 16, 5, 2, &result) && result == 2);
    CHECK(s->motion.y == 2 * 65536 && s->motion.contacts == 0x42);
    s->player.x = 149 * 65536;
    s->player.y = 120 * 65536;
    s->motion = (SotnActorMotion){0};
    CHECK(ActorObjectCollision(s, &a, 16, 5, 1, &result) && result == 1);
    CHECK(s->motion.x == 2 * 65536 && s->motion.contacts == 0x44);
    s->player.x = 108 * 65536;
    s->motion = (SotnActorMotion){0};
    CHECK(ActorObjectCollision(s, &a, 16, 5, 9, &result) && result == 1);
    CHECK(s->motion.x == -2 * 65536 && s->motion.contacts == 0x48);
    a.state.x = 400 * 65536;
    s->motion = (SotnActorMotion){0};
    CHECK(ActorObjectCollision(s, &a, 16, 5, 7, &result) && result == 0 && s->motion.x == 0);
    CHECK(!ActorDisplace(s, 65 * 65536, 0, 0));
    return true;
}
static bool Switches(SotnActors *s)
{
    Actor button = {.state = {.generation = 100, .id = 0x24, .x = 120 * 65536, .y = 128 * 65536},
                    .slot = 10};
    s->slots[11] = (Actor){
        .state = {.generation = 101, .id = 0x25, .x = 144 * 65536, .y = 144 * 65536}, .slot = 11};
    Actor *spikes = &s->slots[11];
    s->player =
        (SotnPlayerState){.x = 120 * 65536, .y = 96 * 65536, .solid_width = 7, .solid_height = 24};
    s->motion = (SotnActorMotion){0};
    CHECK(ActorMechanism(s, &button));
    CHECK(ActorMechanism(s, spikes));
    CHECK(spikes->state.y == 116 * 65536 && spikes->state.hitbox_state == 1);
    for (Uint32 i = 0; i < 4; ++i)
    {
        s->motion = (SotnActorMotion){0};
        CHECK(ActorMechanism(s, &button));
        CHECK(ActorMechanism(s, spikes));
    }
    CHECK(button.state.step == 2 && button.state.y == 132 * 65536 && spikes->mechanism.pressed);
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(ActorMechanism(s, spikes));
    CHECK(spikes->state.y == 144 * 65536 && spikes->state.hitbox_state == 0);
    s->player.x = 30 * 65536;
    for (Uint32 i = 0; i < 6; ++i)
        CHECK(ActorMechanism(s, &button));
    CHECK(button.state.step == 1 && button.state.y == 128 * 65536);
    // A room owns its copy: public/private source tile bytes cannot be changed by the mechanism.
    CHECK(s->map.tiles.bytes != NULL);
    CHECK(Read16(s->map.tiles.bytes + (8 * 32 + 7) * 2) == 0x5af);
    ActorPrimitivesClear(s, &button);
    ActorPrimitivesClear(s, spikes);
    return true;
}
static bool Lifts(SotnActors *s, Fixture *f)
{
    // Synthetic stop data deliberately differs from the original game tables.
    Put16(f->nz0 + 0x13aa, 90);
    Put16(f->nz0 + 0x13ac, 96);
    Put16(f->nz0 + 0x13ae, 104);
    Actor lift = {
        .state = {
            .id = 0x47, .generation = 102, .parameters = 2, .x = 128 * 65536, .y = 96 * 65536}};
    s->player =
        (SotnPlayerState){.x = 128 * 65536, .y = 135 * 65536, .solid_width = 7, .solid_height = 24};
    s->motion = (SotnActorMotion){0};
    s->buttons = 0;
    s->elevator_target = 0;
    CHECK(ActorMechanism(s, &lift));
    CHECK(lift.state.primitive_count == 32);
    CHECK(SotnActors_SetButtons(s, SOTN_BUTTON_UP));
    CHECK(ActorMechanism(s, &lift));
    CHECK(lift.state.step == 2 && s->elevator_target == 1);
    const Sint32 start = s->player.y;
    s->buttons = 0;
    for (Uint32 i = 0; i < 7; ++i)
    {
        s->motion = (SotnActorMotion){0};
        CHECK(ActorMechanism(s, &lift));
    }
    CHECK(lift.state.step == 1 && lift.state.y == 90 * 65536 && s->player.y == start - 6 * 65536);
    ActorPrimitive *chain = ActorPrimitiveGet(s, &lift, 31);
    CHECK(chain != NULL && chain->state.active);
    ActorPrimitivesClear(s, &lift);
    return true;
}
static bool Secrets(SotnActors *s, Fixture *f)
{
    for (Uint32 i = 0; i < 32; ++i)
        Put16(f->nz0 + 0xe54 + i * 2, (Uint16)(10 + i));
    Actor wall = {.state = {.id = 0x17, .generation = 103, .x = 64 * 65536, .y = 200 * 65536}};
    CHECK(ActorMechanism(s, &wall));
    CHECK(Read16(s->map.tiles.bytes + 0x260 * 2) == 10);
    for (Uint32 hit = 0; hit < 3; ++hit)
    {
        wall.hit_params = 1;
        CHECK(ActorMechanism(s, &wall));
        wall.hit_params = 0;
        CHECK(ActorMechanism(s, &wall));
        CHECK(Read16(s->map.tiles.bytes + 0x260 * 2) == 10 + (hit + 1) * 8);
        if (hit < 2)
            for (Uint32 i = 0; i < 32; ++i)
                CHECK(ActorMechanism(s, &wall));
    }
    CHECK(s->world->flags[SOTN_LAB_SECRET_WALL] == 1 && wall.remove);
    wall = (Actor){.state = {.id = 0x17, .generation = 104}};
    CHECK(ActorMechanism(s, &wall) && wall.remove);
    CHECK(Read16(s->map.tiles.bytes + 0x260 * 2) == 34);
    Uint16 before = Read16(s->map.tiles.bytes);
    CHECK(!ActorTile(s, 32 * 128, 1));
    CHECK(!ActorTile(s, 0, 2048));
    CHECK(!ActorTilePatch(s, 32 * 128 - 1, 2, 2, 32, 0xe54));
    CHECK(Read16(s->map.tiles.bytes) == before);
    return true;
}
static bool Doors(SotnActors *s, Fixture *f)
{
    Actor door = {.state = {.id = 5, .generation = 110, .x = 128 * 65536, .y = 128 * 65536}};
    s->player =
        (SotnPlayerState){.x = 160 * 65536, .y = 159 * 65536, .solid_width = 7, .solid_height = 24};
    for (Uint32 i = 0; i < 8; ++i)
        Put16(f->nz0 + 0xd8c + i * 2, (Uint16)(100 + i));
    CHECK(ActorDoor(s, &door));
    CHECK(door.state.step == 1 && door.state.frame == 1);
    CHECK(Read16(s->map.tiles.bytes + (8 * 32 + 8) * 2) == 100);
    s->player.x = 136 * 65536;
    s->player.facing_left = true;
    s->player.step = SOTN_PLAYER_WALK;
    CHECK(ActorDoor(s, &door));
    CHECK(door.state.step == 2 && s->control.timer == 2);
    CHECK(Read16(s->map.tiles.bytes + (8 * 32 + 8) * 2) == 104);
    for (Uint32 i = 0; i < 32; ++i)
        CHECK(ActorDoor(s, &door));
    CHECK(door.state.step == 3 && s->control.timer == 24 && s->motion.protect);
    --s->control.timer;
    CHECK(ActorDoor(s, &door));
    CHECK(s->control.timer == 23);
    s->control.timer = 3;
    CHECK(ActorDoor(s, &door));
    CHECK(s->control.buttons == SOTN_BUTTON_LEFT && s->control.timer == 3);
    ActorPrimitivesClear(s, &door);
    // Entering the adjoining room starts on its already-open door, then closes it.
    door = (Actor){
        .state = {
            .id = 5, .generation = 111, .x = 128 * 65536, .y = 128 * 65536, .parameters = 0x100}};
    s->motion = (SotnActorMotion){0};
    CHECK(ActorDoor(s, &door));
    CHECK(door.state.step == 4 && s->motion.halt_vertical);
    s->player.x = 180 * 65536;
    CHECK(ActorDoor(s, &door));
    CHECK(door.state.step == 5);
    for (Uint32 i = 0; i < 32; ++i)
        CHECK(ActorDoor(s, &door));
    CHECK(door.state.step == 1 && door.state.frame == 1);
    ActorPrimitivesClear(s, &door);
    return true;
}
static bool Cannon(SotnActors *s, Fixture *f)
{
    s->world->flags[SOTN_LAB_CANNON] = 0;
    s->cannon_fired = false;
    Actor lever = {.state = {.id = 0x1c, .generation = 112, .x = 200 * 65536, .y = 140 * 65536}};
    Actor cannon = {.state = {.id = 0x1d, .generation = 113, .x = 160 * 65536, .y = 140 * 65536},
                    .slot = 20};
    Actor wall = {.state = {.id = 0x1f, .generation = 114}};
    for (Uint32 i = 0; i < 12; ++i)
        Put16(f->nz0 + 0xed4 + i * 2, (Uint16)(140 + i));
    s->player.x = 200 * 65536;
    CHECK(ActorCannon(s, &lever));
    CHECK(ActorCannon(s, &cannon));
    CHECK(ActorCannon(s, &wall));
    CHECK(Read16(s->map.tiles.bytes + 0x46 * 2) == 140);
    lever.hit_params = 1;
    CHECK(ActorCannon(s, &lever));
    CHECK(lever.state.step == 2);
    lever.hit_params = 0;
    CHECK(ActorCannon(s, &lever));
    CHECK(lever.state.step == 3);
    // Preserve the PS1 boolean comparison: the negative lever moves only one tick.
    CHECK(lever.state.x == 196 * 65536);
    CHECK(ActorCannon(s, &lever));
    CHECK(s->cannon_fired);
    CHECK(ActorCannon(s, &cannon));
    CHECK(cannon.state.step == 2);
    Actor *shot = &s->slots[21];
    CHECK(shot->state.id == 0x1e && shot->dynamic);
    for (Uint32 i = 0; i < 7; ++i)
        CHECK(ActorCannon(s, shot));
    CHECK(shot->remove && s->world->flags[SOTN_LAB_CANNON] == 1);
    CHECK(ActorCannon(s, &wall));
    CHECK(ActorCannon(s, &wall));
    CHECK(wall.remove && Read16(s->map.tiles.bytes + 0x46 * 2) == 146);
    ActorPrimitivesClear(s, &lever);
    ActorPrimitivesClear(s, &cannon);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    SotnActors *s = NULL;
    SotnActorLayout *layout = NULL;
    const bool ok = f != NULL && Map() && Open(f, &s, &layout) && Collision(s) && Switches(s) &&
                    Lifts(s, f) && Secrets(s, f) && Doors(s, f) && Cannon(s, f);
    if (ok && Read16(f->tiles + (8 * 32 + 7) * 2) != 0)
    {
        SDL_Log("Mechanism modified immutable input");
        return 1;
    }
    SotnActors_Destroy(s);
    SotnActorLayout_Close(layout);
    SDL_free(f);
    return ok ? 0 : 1;
}
