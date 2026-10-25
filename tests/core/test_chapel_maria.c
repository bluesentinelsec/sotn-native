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
    Uint8 dra[0x1000], stage[0x5000], tiles[32 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
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
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    // Synthetic durations/sprites and portrait palettes, not original runtime arrays.
    f->stage[0x1024] = 1;
    f->stage[0x1025] = 4;
    f->stage[0x1026] = 255;
    for (Uint16 i = 0; i < 9; ++i)
    {
        f->stage[0x1030 + i * 2U] = 1;
        f->stage[0x1031 + i * 2U] = (Uint8)(i + 5);
    }
    f->stage[0x1044] = 5;
    f->stage[0x1045] = 2;
    f->stage[0x1046] = 255;
    Put16(f->stage + 0xee8, 0x230);
    Put16(f->stage + 0xeea, 0x240);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 6, out));
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_OpenStage((*out)->nz0, 6, 0, &layout));
    const bool entered = SotnActors_Enter(
        *out, layout,
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16}, 192,
        0);
    SotnActorLayout_Close(layout);
    CHECK(entered);
    (*out)->player = (SotnPlayerState){.hp = 70, .x = 500 * 65536 + 123, .y = 135 * 65536};
    return true;
}
static bool Manager(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x46, .generation = 1}};
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 1 && s->control.timer == 1 &&
          s->control.pause_disabled && s->control.block_stopwatch &&
          s->control.buttons == SOTN_BUTTON_LEFT);
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 1);
    s->player.x = 20 * 65536 + 123;
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 2 && (s->death_event.flags & 1U) &&
          s->motion.align_horizontal && s->motion.alignment_x == 368 &&
          s->player.x == 368 * 65536 + 123);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(s->camera->anchor_x == 176 && !a.remove);
    s->death_event.flags |= 2;
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 3);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(s->camera->anchor_x == 128 && !a.remove && s->control.pause_disabled);
    s->death_event.flags |= 8;
    CHECK(ActorChapelMaria(s, &a) && a.remove && !s->control.pause_disabled &&
          !s->control.block_stopwatch);
    a.state.step = 4;
    CHECK(!ActorChapelMaria(s, &a));
    CHECK(!ActorChapelMaria(NULL, &a) && !ActorChapelMaria(s, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Maria(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x47, .generation = 1, .x = 280 * 65536, .y = 160 * 65536}};
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 1 && a.state.animation_set == 0x800f &&
          a.state.frame == 10 && a.state.palette == 0x251 && a.state.texture_offset == 72);
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 1);
    s->death_event.flags |= 4;
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 2);
    CHECK(ActorChapelMaria(s, &a) && ActorChapelMaria(s, &a) && a.state.step == 3 && a.vx == 98304);
    a.state.x = 369 * 65536;
    CHECK(ActorChapelMaria(s, &a) && a.state.step == 4 && a.vy == -4 * 65536);
    for (Uint16 i = 0; i < 90 && a.state.step == 4; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(a.state.step == 5 && Pixel(a.state.y) == 151 && !a.vy);
    for (Uint16 i = 0; i < 80 && a.state.step == 5; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(a.state.step == 6 && (s->death_event.flags & 8U));
    for (Uint16 i = 0; i < 90 && a.state.step == 6; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(a.state.step == 7 && Pixel(a.state.y) == 135);
    for (Uint16 i = 0; i < 80 && !a.remove; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(a.remove && s->combat.sound_count);
    for (Uint16 i = 0; i < s->combat.sound_count; ++i)
        CHECK(s->combat.sounds[i] == 0x64c && s->combat.volumes[i] == 80 &&
              s->combat.pans[i] >= -8 && s->combat.pans[i] <= 8);
    a.state.step = 8;
    CHECK(!ActorChapelMaria(s, &a));
    a.state.step = 2;
    f->stage[0x1024] = 0;
    CHECK(!ActorChapelMaria(s, &a));
    a.state.id = 0x44;
    CHECK(!ActorChapelMaria(s, &a));
    SotnActors_Destroy(s);
    return true;
}
static bool Dialogue(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    // Synthetic event and dialogue programs; text and coordinates differ from the game.
    const Uint8 events[] = {0, 0, 0,   130,  0x46, 10, 0,  6, 0,   0,
                            0, 0, 131, 0x47, 17,   8,  10, 0, 255, 255};
    SDL_memcpy(f->stage + 0x4200, events, sizeof(events));
    const Uint8 program[] = {12,   0x84, 0x42, 0x20, 0,  9, 1,  0, 11, 5, 1, 0, 9,
                             0x37, 0x75, 10,   11,   17, 2, 17, 1, 20, 1, 1, 0};
    SDL_memcpy(f->stage + 0x3a94, program, sizeof(program));
    Actor a = {.state = {.id = 0x45, .generation = 1}};
    CHECK(ActorChapelMaria(s, &a) && s->death_event.script == 0x3a94 &&
          !s->control.pause_disabled && s->death_event.display.portrait_page == 0x90);
    CHECK(ActorChapelMaria(s, &a) && s->combat.cd_command == 0x10);
    CHECK(ActorChapelMaria(s, &a));
    CHECK(s->slots[130].state.id == 0x46 && s->slots[130].state.x == 160 * 65536 &&
          s->slots[131].state.id == 0x47 && s->slots[131].state.x == 280 * 65536);
    CHECK(SotnActors_SetDialogueBusy(s, false));
    for (Uint16 i = 0; i < 20 && !s->combat.voice_change; ++i)
        CHECK(ActorChapelMaria(s, &a));
    CHECK(s->combat.voice_change && s->combat.voice == 0x375 &&
          s->death_event.display.palette == 0x240);
    CHECK(ActorChapelMaria(s, &a) && !a.remove);
    CHECK(SotnActors_Dialogue(s, false, NULL) && ActorChapelMaria(s, &a));
    CHECK(a.remove && s->world->flags[SOTN_CHAPEL_MARIA] && !s->world->flags[SOTN_ENTRANCE_DEATH] &&
          s->combat.cd_command == 0x11 && (s->death_event.flags & 6U) == 6U);
    a = (Actor){.state = {.id = 0x45, .generation = 2}};
    CHECK(ActorChapelMaria(s, &a) && a.remove && !s->death_event.active);
    s->world->flags[SOTN_CHAPEL_MARIA] = 0;
    a = (Actor){.state = {.id = 0x45, .generation = 3}};
    CHECK(ActorChapelMaria(s, &a));
    s->death_event.script = s->nz0.size;
    CHECK(!ActorChapelMaria(s, &a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Manager(f) && Maria(f) && Dialogue(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
