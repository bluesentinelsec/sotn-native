#include "actors_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x5000], stage[0x2000], exe[0x23220], tiles[112 * 16 * 2], types[0x700];
    SotnActorLayout *layout;
    SotnCharacter *character;
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)(Uint16)v;
    p[1] = (Uint8)((Uint16)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool OpenWidth(Fixture *f, Uint16 width, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 32767);
    Put32(f->dra + 0x224, 0x08000000);
    Put32(f->dra + 0xc0, 0x800a1000);
    Put32(f->dra + 0xc4, 0x800a4000);
    f->dra[0x100] = 255;
    for (Uint16 body = 0; body < 2; ++body)
        for (Uint16 id = 0; id < (body ? 90 : 169); ++id)
        {
            Uint8 *p = f->dra + (body ? 0x4000 + id * 32U : 0x1000 + id * 52U);
            Put32(p, 0x800a0100);
            Put32(p + 4, 0x800a0180);
            if (body)
                p[28] = (Uint8)(id == 26 ? 0 : id == 48 ? 2 : id == 57 ? 3 : 1);
        }
    Put16(f->stage + 0x8a4, 0x8001);
    Put16(f->stage + 0x8a6, 11);
    for (Uint16 i = 0; i < 3; ++i)
    {
        f->stage[0xd38 + i * 2U] = 2;
        f->stage[0xd39 + i * 2U] = (Uint8)(i + 1U);
    }
    f->stage[0xd3e] = 255;
    Put16(f->stage + 0x142c + 64 * 2, 1024);
    Put16(f->stage + 0x142c, -512);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    f->types[1] = 3;
    f->types[0x6ae] = 244;
    f->types[0x6af] = 245;
    f->types[0x6b0] = 247;
    f->types[7] = 248;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 4, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){
            {f->tiles, (size_t)width * 16U * 2U}, {f->types, sizeof(f->types)}, width, 16},
        0, 0));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 1, false));
    (*out)->slots[32] =
        (Actor){.slot = 32,
                .dynamic = true,
                .state = {.id = 0x21, .generation = 1, .x = 16 * 65536, .y = 128 * 65536}};
    (*out)->player = (SotnPlayerState){.x = 168 * 65536,
                                       .y = 136 * 65536,
                                       .hp = 70,
                                       .hp_max = 70,
                                       .animation = {.hitbox_width = 8, .hitbox_height = 20}};
    return true;
}
static bool Open(Fixture *f, SotnActors **out)
{
    return OpenWidth(f, 96, out);
}
static void Close(Fixture *f, SotnActors *s)
{
    SotnActors_Destroy(s);
    SotnCharacter_Destroy(f->character);
    SotnActorLayout_Close(f->layout);
    f->character = NULL;
    f->layout = NULL;
}
static bool Equip(Fixture *f, SotnActors *s)
{
    CHECK(SotnCharacter_Create((SotnDataView){f->dra, sizeof(f->dra)}, &f->character));
    CHECK(SotnActors_SetCharacter(s, f->character));
    CHECK(SotnCharacter_Collect(f->character, true, 14));
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, 14));
    return true;
}
static bool Damage(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    CHECK(ActorTile(s, 8 * 96 + 10, 0x6ae));
    // Initialization scans immediately, using temporary 96..127 slots, never slot+1.
    CHECK(ActorOlroxQuarter(s, a) && a->state.step == 1 && !s->slots[33].state.generation &&
          s->backdrop_color == (2U | (2U << 5) | (2U << 10)));
    Actor *d = &s->slots[96];
    CHECK(d->state.id == 0x31 && d->enemy_state.owner_generation == 1 &&
          d->state.x == 168 * 65536 && d->state.y == 136 * 65536);
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !s->combat.contact_damage &&
          d->state.hitbox_width == 4 && d->remaining == 4);
    CHECK(ActorTile(s, 8 * 96 + 10, 0));
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !d->state.generation &&
          s->combat.contact_damage == 15 && s->combat.contact_element == 0x50);
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !s->combat.contact_damage);
    CHECK(ActorTile(s, 8 * 96 + 10, 0x6af) && ActorOlroxQuarter(s, a));
    ++a->state.generation;
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && !d->state.generation && !s->combat.contact_damage);
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i] = (Actor){.state = {.id = 0x29, .generation = 9}};
    CHECK(ActorOlroxQuarter(s, a));
    for (Uint16 i = 96; i < 128; ++i)
        CHECK(s->slots[i].state.id == 0x29 && s->slots[i].state.generation == 9);
    s->player.x = -65536;
    CHECK(ActorOlroxQuarter(s, a));
    s->player.x = 96 * 16 * 65536;
    CHECK(ActorOlroxQuarter(s, a));
    s->player.x = 168 * 65536;
    s->player.y = 0;
    CHECK(ActorOlroxQuarter(s, a));
    s->player.y = 255 * 65536;
    CHECK(ActorOlroxQuarter(s, a));
    Close(f, s);
    return true;
}
static bool Breaker(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Equip(f, s));
    Actor *a = &s->slots[32];
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(ActorTile(s, (7U + i) * 96U + 10U, (Uint16)(0x6ae + i)));
    CHECK(ActorOlroxQuarter(s, a));
    for (Uint16 i = 0; i < 3; ++i)
    {
        CHECK(Read16(s->mutable_tiles + ((7U + i) * 96U + 10U) * 2U) == 0x6b1 + i);
        for (Uint16 p = 0; p < 3; ++p)
        {
            Actor *part = &s->slots[160 + i * 5U + p];
            CHECK(part->state.id == 0x22 && part->state.parameters == p &&
                  part->state.x == 168 * 65536 && part->state.y == (120 + i * 16) * 65536);
        }
        CHECK(s->slots[163 + i * 5U].state.id == 6 &&
              s->slots[163 + i * 5U].state.parameters == 16 &&
              s->slots[164 + i * 5U].state.id == 0x23 && s->combat.sounds[i] == 0x652);
    }
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, 0) &&
          ActorTile(s, 8 * 96 + 10, 0x6ae) && ActorOlroxQuarter(s, a) &&
          Read16(s->mutable_tiles + (8 * 96 + 10) * 2) == 0x6ae && s->slots[96].state.id == 0x31);
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, 14));
    for (Uint16 i = 160; i < 192; ++i)
        s->slots[i] = (Actor){.state = {.id = 0x29, .generation = 8}};
    CHECK(ActorOlroxQuarter(s, a) && Read16(s->mutable_tiles + (8 * 96 + 10) * 2) == 0x6b1 &&
          s->slots[160].state.id == 0x29 && s->slots[191].state.id == 0x29);
    Close(f, s);
    return true;
}
static bool MixedStride(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(OpenWidth(f, 112, &s) && Equip(f, s));
    // The original ordinary prisoner room is 112 tiles wide, but spike scanning
    // and effect coordinates retain their independent hardcoded 96-tile stride.
    CHECK(ActorTile(s, 8 * 112 + 10 - 96, 0x6ae));
    CHECK(ActorTile(s, 7 * 112 + 10, 0x6af));
    CHECK(ActorOlroxQuarter(s, &s->slots[32]) &&
          Read16(s->mutable_tiles + (8 * 112 + 10 - 96) * 2) == 0x6b1 &&
          Read16(s->mutable_tiles + (7 * 112 + 10) * 2) == 0x6af &&
          s->slots[160].state.x == 680 * 65536 && s->slots[160].state.y == 136 * 65536);
    Close(f, s);
    return true;
}
static bool EffectsFailures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor part = {.state = {.id = 0x22, .parameters = 1, .x = 160 * 65536, .y = 128 * 65536}};
    s->random = 0;
    CHECK(ActorOlroxQuarter(s, &part) && part.state.frame == 11 && part.vx == 20480 &&
          part.vy == 45056 && part.state.rotation == 16 && part.enemy_state.angle_delta == 0);
    CHECK(ActorOlroxQuarter(s, &part) && part.state.x == 160 * 65536 + 20480 &&
          part.state.y == 128 * 65536 + 45056 && part.vy == 55296);
    Actor dust = {.state = {.id = 0x23, .x = 128 * 65536, .y = 136 * 65536}};
    CHECK(ActorOlroxQuarter(s, &dust) && dust.vx == 40960 && dust.vy == -20480 &&
          dust.state.palette == 0x8161);
    for (Uint16 i = 0; i < 20 && !dust.remove; ++i)
        CHECK(ActorOlroxQuarter(s, &dust));
    CHECK(dust.remove);
    part = (Actor){.state = {.id = 0x22, .parameters = 3}};
    CHECK(!ActorOlroxQuarter(s, &part));
    CHECK(!ActorChapelSpikes(NULL, &part) && !ActorChapelSpikes(s, NULL));
    CHECK(Equip(f, s));
    CHECK(ActorTile(s, 7 * 96 + 10, 0x6ae) && ActorTile(s, 8 * 96 + 10, 4));
    f->types[4] = 244;
    CHECK(!ActorOlroxQuarter(s, &s->slots[32]) &&
          Read16(s->mutable_tiles + (7 * 96 + 10) * 2) == 0x6ae && !s->slots[160].state.generation);
    CHECK(ActorTile(s, 8 * 96 + 10, 0));
    s->map.types.size = 0x6b1;
    CHECK(!ActorOlroxQuarter(s, &s->slots[32]));
    Close(f, s);
    return true;
}
static bool PositionBounds(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    const size_t bytes = 1024U * 208U * 2U;
    Uint8 *tiles = SDL_calloc(1, bytes);
    CHECK(tiles);
    CHECK(SotnActors_Enter(
        s, f->layout, (SotnCollisionMap){{tiles, bytes}, {f->types, sizeof(f->types)}, 1024, 208},
        0, 0));
    SDL_free(tiles);
    s->slots[32] = (Actor){.slot = 32, .dynamic = true, .state = {.id = 0x21, .generation = 1}};
    s->player.x = 168 * 65536;
    s->player.y = 3272 * 65536;
    CHECK(ActorTile(s, 204 * 1024 + 10, 0x6ae));
    CHECK(!ActorOlroxQuarter(s, &s->slots[32]) && !s->slots[96].state.generation);
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok =
        f && Damage(f) && Breaker(f) && MixedStride(f) && EffectsFailures(f) && PositionBounds(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_OLROX_SPIKES: PASS");
    return 0;
}
