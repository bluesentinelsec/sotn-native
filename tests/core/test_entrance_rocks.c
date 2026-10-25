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
typedef struct Fixture
{
    Uint8 dra[1024], stage[0x2800], foreground[96 * 32 * 2], background[96 * 32 * 2], types[2048];
    Uint8 pages[2048], graphics[2048], palettes[2048];
    SotnActorLayout *layout;
} Fixture;
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
static bool Open(Fixture *f, Uint16 stage, SotnActors **out)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 0x7fff);
    Put32(f->dra + 0x224, 0x08002000);
    Put32(f->stage + 28, 0x80180040);
    for (Uint32 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    const size_t left = stage == 65 ? 0x124c : 0x1114, initial = stage == 65 ? 0x1270 : 0x1138,
                 right = stage == 65 ? 0x1288 : 0x1150;
    const size_t other = stage == 65 ? 0x12ac : 0x1174, background = stage == 65 ? 0x1308 : 0x11d0;
    for (Uint32 i = 0; i < 18; ++i)
    {
        Put16(f->stage + left + i * 2, (Uint16)(i + 1));
        Put16(f->stage + right + i * 2, (Uint16)(i + 21));
    }
    for (Uint32 i = 0; i < 12; ++i)
        Put16(f->stage + initial + i * 2, (Uint16)(i + 41));
    for (Uint32 i = 0; i < 45; ++i)
        Put16(f->stage + other + i * 2, (Uint16)(i + 61));
    for (Uint32 i = 0; i < 26; ++i)
        Put16(f->stage + background + i * 2, (Uint16)(i + 121));
    const size_t offsets = stage == 65 ? 0x133c : 0x1204, recipe = stage == 65 ? 0x1344 : 0x120c;
    for (Uint32 i = 0; i < 4; ++i)
        Put16(f->stage + offsets + i * 2, 6);
    for (Uint32 i = 0; i < 6; ++i)
        f->stage[recipe + i] = (Uint8)(i % 3);
    const size_t explosions = stage == 65 ? 0x25a0 : 0x1f2c;
    for (Uint32 i = 0; i < 5; ++i)
        Put32(f->stage + explosions + i * 4, 0x80180380);
    f->stage[0x380] = 1;
    f->stage[0x381] = 1;
    f->stage[0x382] = 255;
    f->types[1] = 3;
    for (Uint32 y = 20; y < 32; ++y)
        for (Uint32 x = 0; x < 48; ++x)
            Put16(f->foreground + (y * 48 + x) * 2, 1);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, stage, out));
    CHECK(SotnActorLayout_OpenStage((*out)->nz0, stage, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){
            {f->foreground, sizeof(f->foreground)}, {f->types, sizeof(f->types)}, 48, 32},
        0, 0));
    SotnCollisionMap copy = {0};
    CHECK(SotnActors_SetBackground(
        *out,
        (SotnCollisionMap){
            {f->background, sizeof(f->background)}, {f->types, sizeof(f->types)}, 48, 32},
        &copy));
    CHECK(copy.tiles.bytes != f->background && copy.tiles.bytes == (*out)->background_tiles);
    return true;
}
static bool Run(Fixture *f, Uint16 stage)
{
    SotnActors *s = NULL;
    CHECK(Open(f, stage, &s));
    Actor *left = &s->slots[16];
    *left = (Actor){.state = {.id = 0x24, .generation = 100, .x = 200 * 65536, .y = 180 * 65536},
                    .slot = 16,
                    .dynamic = true};
    SotnPlayerState player = {
        .x = 180 * 65536,
        .y = 180 * 65536,
        .hp = 70,
        .invulnerable = true,
        .attack = {
            .active = true, .hit_type = 2, .width = 32, .height = 24, .invincibility_frames = 8}};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(s->background_tiles[0x1f1 * 2] == 47 && f->background[0x1f1 * 2] == 0);
    CHECK(SotnActors_Tick(s, player, 0, 0) && left->enemy_state.counter == 1 &&
          left->state.step == 1);
    CHECK(s->mutable_tiles[0x1f1 * 2] == 7 && f->foreground[0x1f1 * 2] == 0);
    left->hit_flags = 2;
    CHECK(ActorEntranceRock(s, left) && left->state.step == 2 &&
          s->world->flags[SOTN_ENTRANCE_ROCKS] == 1);
    CHECK(s->mutable_tiles[0x1f1 * 2] == 13 && s->slots[96].state.id == 0xa &&
          s->slots[96].state.parameters == 0x43);
    Actor right = {.state = {.id = 0x25, .generation = 200, .x = 500 * 65536, .y = 180 * 65536}};
    CHECK(ActorEntranceRock(s, &right));
    right.hit_flags = 2;
    CHECK(ActorEntranceRock(s, &right) && right.enemy_state.counter == 1);
    CHECK(ActorEntranceRock(s, &right) && right.state.step == 2 &&
          s->world->flags[SOTN_ENTRANCE_ROCKS] == 3);
    Actor restored = {.state = {.id = 0x24, .generation = 201}};
    CHECK(ActorEntranceRock(s, &restored) && restored.state.step == 2 &&
          restored.state.hitbox_state == 1);
    Actor door = {.state = {.id = 0x26, .generation = 202}};
    CHECK(ActorEntranceRock(s, &door) && ActorEntranceRock(s, &door) && door.state.step == 1 &&
          !s->world->flags[SOTN_ENTRANCE_JEWEL_DOOR]);
    s->world->flags[SOTN_ENTRANCE_ROCKS] = 15;
    CHECK(ActorEntranceRock(s, &door) && door.state.step == 2);
    CHECK(ActorEntranceRock(s, &door) && door.state.step == 3 &&
          s->world->flags[SOTN_ENTRANCE_JEWEL_DOOR] == 1);
    CHECK(s->mutable_tiles[0x420 * 2] == 88 && s->mutable_tiles[(0x420 + 48) * 2] == 89 &&
          s->mutable_tiles[(0x420 + 1) * 2] == 93);
    CHECK(s->background_tiles[0x420 * 2] == 132 && f->background[0x420 * 2] == 0);
    CHECK(SotnWorld_Validate(s->world));
    s->world->flags[SOTN_ENTRANCE_ROCKS] = 16;
    CHECK(!SotnWorld_Validate(s->world));
    s->world->flags[SOTN_ENTRANCE_ROCKS] = 3;
    s->world->flags[SOTN_ENTRANCE_JEWEL_DOOR] = 2;
    CHECK(!SotnWorld_Validate(s->world));
    s->world->flags[SOTN_ENTRANCE_JEWEL_DOOR] = 1;
    Actor fragment = {.state = {.id = 0x27,
                                .generation = 203,
                                .parameters = 1,
                                .x = 100 * 65536,
                                .y = 319 * 65536},
                      .vy = 5 * 65536};
    CHECK(ActorEntranceRock(s, &fragment) && fragment.state.frame == 32 &&
          ActorEntranceRock(s, &fragment) && fragment.remove);
    Actor bounce = {
        .state = {
            .id = 0x27, .generation = 204, .parameters = 1, .x = 100 * 65536, .y = 314 * 65536}};
    CHECK(ActorEntranceRock(s, &bounce) && ActorEntranceRock(s, &bounce) && bounce.vy == -14336 &&
          !bounce.remove);
    Actor invalid = {.state = {.id = 0x27, .generation = 205, .parameters = 4}};
    CHECK(!ActorEntranceRock(s, &invalid));
    SotnCollisionMap before = s->background, unchanged = {.width = 999};
    CHECK(!SotnActors_SetBackground(s, (SotnCollisionMap){.width = SDL_MAX_UINT32, .height = 1},
                                    &unchanged));
    CHECK(unchanged.width == 999 && s->background.tiles.bytes == before.tiles.bytes);
    CHECK(SotnActors_SetBackground(s, s->background, &unchanged) &&
          unchanged.tiles.bytes == s->background.tiles.bytes &&
          s->background_tiles[0x1f1 * 2] == 47);
    CHECK(SotnActors_Enter(
        s, f->layout,
        (SotnCollisionMap){
            {f->foreground, sizeof(f->foreground)}, {f->types, sizeof(f->types)}, 48, 32},
        0, 0));
    CHECK(!s->background_tiles && !s->background.width);
    restored = (Actor){.state = {.id = 0x24, .generation = 206}};
    CHECK(!ActorEntranceRock(s, &restored));
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}
static bool Stair(Fixture *f, Uint16 stage)
{
    SotnActors *s = NULL;
    CHECK(Open(f, stage, &s));
    SDL_memset(f->foreground, 0, sizeof(f->foreground));
    for (Uint32 y = 14; y < 32; ++y)
        for (Uint32 x = 0; x < 96; ++x)
            Put16(f->foreground + (y * 96 + x) * 2, 1);
    const SotnCollisionMap map = {
        {f->foreground, sizeof(f->foreground)}, {f->types, sizeof(f->types)}, 96, 32};
    CHECK(SotnActors_Enter(s, f->layout, map, 1200, 0));
    f->graphics[0x409] = 0x2c;
    f->pages[0x409] = 2;
    f->palettes[0x409] = 4;
    CHECK(SotnActors_SetTileArt(s, (SotnDataView){f->pages, sizeof(f->pages)},
                                (SotnDataView){f->graphics, sizeof(f->graphics)},
                                (SotnDataView){f->palettes, sizeof(f->palettes)}));
    CHECK(!SotnActors_SetTileArt(s, (SotnDataView){f->pages, 1},
                                 (SotnDataView){f->graphics, sizeof(f->graphics)},
                                 (SotnDataView){f->palettes, sizeof(f->palettes)}) &&
          s->tile_pages.bytes == f->pages);
    const Uint16 id = stage == 65 ? 0x5c : 0x4b;
    Actor *root = &s->slots[16];
    *root = (Actor){.state = {.id = id, .generation = 300}, .slot = 16, .dynamic = true};
    const SotnPlayerState player = {
        .x = 1400 * 65536,
        .y = 200 * 65536,
        .hp = 70,
        .invulnerable = true,
        .attack = {.active = true, .hit_type = 2, .power = 100, .width = 32, .height = 16}};
    CHECK(SotnActors_Tick(s, player, 1200, 0) && root->state.hit_points == 16 &&
          root->state.hitbox_state == 2);
    CHECK(Read16(s->mutable_tiles + 0x4d9 * 2) == 0x408 &&
          Read16(s->mutable_tiles + 0x539 * 2) == 0x40d);
    CHECK(SotnActors_Tick(s, player, 1200, 0) && root->state.step == 2 &&
          (root->state.flags & 0x100));
    CHECK(ActorEntranceStairway(s, root) && root->state.step == 3 &&
          root->state.primitive_count == 16);
    CHECK(s->world->flags[SOTN_ENTRANCE_STAIRS] && s->slots[96].state.id == 0xa &&
          s->slots[96].state.parameters == 0x45);
    ActorPrimitive *tile = ActorPrimitiveGet(s, root, 0);
    CHECK(tile != NULL && tile->state.quad && tile->state.page == 10 && tile->state.palette == 4 &&
          tile->state.u[0] == 192 && tile->state.v[0] == 32);
    for (Uint32 t = 0; t < 100 && root->state.step == 3; ++t)
        CHECK(ActorEntranceStairway(s, root));
    CHECK(root->state.step == 4 && ActorEntranceStairway(s, root) && root->remove);
    ActorPrimitivesClear(s, root);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        CHECK(!s->primitives[i].owner);
    Actor *fragment = NULL;
    const Uint16 fragment_id = stage == 65 ? 0x5d : 0x4c;
    for (Uint32 i = 160; i < 192; ++i)
        if (s->slots[i].state.id == fragment_id)
            fragment = &s->slots[i];
    CHECK(fragment != NULL && ActorEntranceStairway(s, fragment) && fragment->state.scale_x == 96 &&
          fragment->state.rotated);
    fragment->state.x = 1400 * 65536;
    fragment->state.y = 220 * 65536;
    fragment->vx = fragment->vy = 0;
    CHECK(ActorEntranceStairway(s, fragment) && fragment->remove);
    Actor flash = {.state = {.id = 6, .generation = 301}};
    CHECK(ActorIntenseExplosion(s, &flash) && flash.state.animation_set == 5);
    for (Uint32 t = 0; t < 37; ++t)
        CHECK(ActorIntenseExplosion(s, &flash));
    CHECK(flash.remove);
    CHECK(SotnActors_Enter(s, f->layout, map, 1200, 0) && s->tile_pages.bytes == NULL);
    Actor restored = {.state = {.id = id, .generation = 302}};
    CHECK(ActorEntranceStairway(s, &restored) && restored.state.step == 32 &&
          !restored.state.hitbox_state);
    CHECK(Read16(s->mutable_tiles + 0x4d9 * 2) == 0x3ee &&
          Read16(s->mutable_tiles + 0x539 * 2) == 0x3d2);
    for (Uint32 i = 0; i < ACTOR_PRIMITIVE_CAPACITY; ++i)
        s->primitives[i].owner = 999;
    Actor exhausted = {.state = {.id = id, .step = 2, .generation = 303}};
    CHECK(ActorEntranceStairway(s, &exhausted) && exhausted.remove &&
          !exhausted.state.primitive_count);
    SotnActors_Destroy(s);
    SotnActorLayout_Close(f->layout);
    return true;
}

int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Run(f, 65) && Run(f, 7) && Stair(f, 65) && Stair(f, 7);
    SDL_free(f);
    return ok ? 0 : 1;
}
