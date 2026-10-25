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
    Uint8 dra[0x5000], stage[0x1800], exe[0x23220], tiles[80 * 16 * 2], types[8];
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
static bool Open(Fixture *f, SotnActors **out)
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
    Put16(f->stage + 0x9b0, 0x8001);
    // Synthetic orientation recipe: a left neighbor marks floor launch, unlike the original.
    f->stage[0xeb7] = 8;
    f->stage[0xeb4] = 1;
    f->stage[0xebc] = 2;
    for (Uint16 i = 0; i < 3; ++i)
    {
        f->stage[0xea4 + i * 2U] = 2;
        f->stage[0xea5 + i * 2U] = (Uint8)(i + 1U);
    }
    f->stage[0xeaa] = 255;
    Put16(f->stage + 0x1474 + 64 * 2, 1024);
    Put16(f->stage + 0x1474, -512);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    f->types[1] = 3;
    f->types[4] = 244;
    f->types[5] = 245;
    f->types[6] = 247;
    f->types[7] = 248;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    CHECK(SotnActorLayout_OpenStage((SotnDataView){f->stage, sizeof(f->stage)}, 6, 0, &f->layout));
    CHECK(SotnActors_Enter(
        *out, f->layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 80, 16}, 0,
        0));
    CHECK(SotnActors_SetServices(*out, (SotnDataView){f->exe, sizeof(f->exe)}, 1, false));
    (*out)->slots[32] =
        (Actor){.slot = 32,
                .dynamic = true,
                .state = {.id = 0x2e, .generation = 1, .x = 16 * 65536, .y = 128 * 65536}};
    (*out)->player = (SotnPlayerState){.x = 168 * 65536,
                                       .y = 136 * 65536,
                                       .hp = 70,
                                       .hp_max = 70,
                                       .animation = {.hitbox_width = 8, .hitbox_height = 20}};
    return true;
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
static bool ScanDamage(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32], *d = &s->slots[33];
    CHECK(ActorChapel(s, a) && a->state.step == 1 && d->state.id == 0x31 &&
          d->enemy_state.owner_generation == 1);
    CHECK(ActorChapel(s, d) && d->attack == 15 && d->attack_element == 0x51 &&
          d->state.hitbox_width == 4 && d->remaining == 4);
    CHECK(ActorTile(s, 7 * 80 + 10, 4) && ActorTile(s, 8 * 80 + 10, 5) &&
          ActorTile(s, 9 * 80 + 10, 6));
    CHECK(ActorChapel(s, a) && d->state.x == 168 * 65536 && d->state.y == 152 * 65536);
    CHECK(SotnActors_Tick(s, s->player, 0, 0) && s->combat.contact_damage == 15 &&
          s->combat.contact_element == 0x51);
    CHECK(Read16(s->mutable_tiles + (9 * 80 + 10) * 2) == 6);
    CHECK(ActorTile(s, 7 * 80 + 10, 0) && ActorTile(s, 8 * 80 + 10, 0) &&
          ActorTile(s, 9 * 80 + 10, 7));
    CHECK(ActorChapel(s, a) && d->state.x == -16 * 65536 && d->state.y == -16 * 65536);
    s->camera_x = 64;
    s->camera_y = 16;
    CHECK(ActorChapel(s, a) && d->state.x == 48 * 65536 && d->state.y == 0);
    s->player.x = -65536;
    CHECK(ActorChapel(s, a));
    s->player.x = 1280 * 65536;
    CHECK(ActorChapel(s, a));
    s->player.x = 168 * 65536;
    s->player.y = 0;
    CHECK(ActorChapel(s, a));
    s->player.y = 255 * 65536;
    CHECK(ActorChapel(s, a));
    s->player.y = 136 * 65536;
    d->state.x = s->player.x;
    d->state.y = s->player.y;
    a->state.generation = 0;
    CHECK(SotnActors_Tick(s, s->player, 64, 16) && !d->state.generation &&
          !s->combat.contact_damage);
    Close(f, s);
    return true;
}
static bool Breaker(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s) && Equip(f, s));
    Actor *a = &s->slots[32];
    CHECK(ActorChapel(s, a));
    CHECK(ActorTile(s, 8 * 80 + 10, 4) && ActorTile(s, 8 * 80 + 9, 1));
    CHECK(ActorChapel(s, a) && !Read16(s->mutable_tiles + (8 * 80 + 10) * 2));
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(s->slots[160 + i].state.id == 0x2f &&
              s->slots[160 + i].state.parameters == 8 + i * 256U &&
              s->slots[160 + i].state.x == 168 * 65536 && s->slots[160 + i].state.y == 136 * 65536);
    CHECK(s->slots[163].state.id == 6 && s->slots[163].state.parameters == 16 &&
          s->slots[164].state.id == 0x30 && s->combat.sounds[0] == 0x652);
    // An unequipped inventory copy does not break spikes.
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, 0));
    CHECK(ActorTile(s, 8 * 80 + 10, 4) && ActorChapel(s, a) &&
          Read16(s->mutable_tiles + (8 * 80 + 10) * 2) == 4);
    CHECK(SotnCharacter_Equip(f->character, SOTN_EQUIP_ARMOR, 14));
    for (Uint16 i = 160; i < 192; ++i)
        s->slots[i] = (Actor){.state = {.id = 0x2f, .generation = 1}};
    CHECK(ActorChapel(s, a) && !Read16(s->mutable_tiles + (8 * 80 + 10) * 2));
    Close(f, s);
    return true;
}
static bool DebrisDust(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x2f, .parameters = 0x102, .x = 160 * 65536, .y = 128 * 65536}};
    Uint32 random = s->random;
    const Sint32 vx = (Sint32)(ActorRandom(s) & 3U) * 8192 - 12288 - 16384;
    const Sint32 vy = (Sint32)(ActorRandom(s) & 3U) * 8192 - 12288;
    const Sint16 rotation = (Sint16)((Sint32)(ActorRandom(s) & 3U) * 16 - 24 + 72);
    const Uint32 next_random = s->random;
    s->random = random;
    CHECK(ActorChapel(s, &a) && a.state.frame == 15 && a.state.rotation == -1008 &&
          a.enemy_state.angle_delta == rotation && a.vx == vx && a.vy == vy &&
          s->random == next_random && (a.state.flags & 0xc0000000U) == 0xc0000000U);
    const Sint32 x = a.state.x, y = a.state.y;
    CHECK(ActorChapel(s, &a) && a.state.x == x + vx && a.state.y == y + vy && a.vy == vy + 10240);
    CHECK(ActorTile(s, 8 * 80 + 10, 1));
    a.state.x = 168 * 65536;
    a.state.y = 136 * 65536;
    a.vx = 65536;
    a.vy = 0;
    a.enemy_state.angle_delta = 10000;
    CHECK(ActorChapel(s, &a) && a.remove && a.vy == -5120 && a.vx == -65536 &&
          a.enemy_state.angle_delta == -25536);
    a = (Actor){.state = {.id = 0x30, .x = 128 * 65536, .y = 136 * 65536}};
    CHECK(ActorChapel(s, &a) && a.vx == 40960 && a.vy == -20480 && a.state.animation_set == 8 &&
          a.state.palette == 0x8161 && a.state.frame == 1);
    CHECK(ActorChapel(s, &a) && a.state.x == 128 * 65536 + 40960 && a.state.frame == 1);
    for (Uint16 i = 0; i < 20 && !a.remove; ++i)
        CHECK(ActorChapel(s, &a));
    CHECK(a.remove);
    Close(f, s);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[32];
    s->slots[33] = (Actor){.state = {.id = 0x35, .generation = 8}};
    CHECK(!ActorChapel(s, a) && s->slots[33].state.id == 0x35);
    *a = (Actor){.slot = 32, .dynamic = true, .state = {.id = 0x2e, .generation = 2}};
    s->slots[33] = (Actor){0};
    CHECK(ActorChapel(s, a));
    const Uint32 generation = s->slots[33].state.generation;
    *a = (Actor){.slot = 32, .dynamic = true, .state = {.id = 0x2e, .generation = 3}};
    CHECK(ActorChapel(s, a) && s->slots[33].state.generation != generation &&
          s->slots[33].enemy_state.owner_generation == 3);
    s->slots[33].state.generation++;
    CHECK(!ActorChapel(s, a));
    a->state.step = 0;
    s->effect_serial = 0x7fffffffU;
    CHECK(!ActorChapel(s, a));
    s->map.width = 32;
    CHECK(!ActorChapel(s, a));
    s->map.width = 80;
    s->effect_serial = 1;
    *a = (Actor){.slot = 191, .state = {.id = 0x2e, .generation = 4}};
    CHECK(!ActorChapel(s, a));
    Actor part = {.state = {.id = 0x2f, .parameters = 0x300}};
    CHECK(!ActorChapel(s, &part));
    CHECK(!ActorChapelSpikes(NULL, &part) && !ActorChapelSpikes(s, NULL));
    Close(f, s);
    CHECK(Open(f, &s) && Equip(f, s));
    a = &s->slots[32];
    CHECK(ActorChapel(s, a) && ActorTile(s, 8 * 80 + 10, 4));
    f->stage[0xeb4] = 16;
    CHECK(!ActorChapel(s, a) && Read16(s->mutable_tiles + (8 * 80 + 10) * 2) == 4);
    f->stage[0xeb4] = 1;
    CHECK(ActorTile(s, 8 * 80 + 10, 0));
    s->mutable_tiles[(8 * 80 + 10) * 2] = 99;
    CHECK(!ActorChapel(s, a));
    Close(f, s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && ScanDamage(f) && Breaker(f) && DebrisDust(f) && Failures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_CHAPEL_SPIKES: PASS");
    return 0;
}
