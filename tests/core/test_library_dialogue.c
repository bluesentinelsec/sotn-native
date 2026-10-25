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
    Uint8 dra[0x400], stage[0x7000];
} Fixture;
static void Word(Uint8 *p, Uint32 offset)
{
    const Uint32 value = 0x80000U + offset;
    p[0] = (Uint8)((value & 0xff000U) >> 12);
    p[1] = (Uint8)((value & 0xff00U) >> 8);
    p[2] = (Uint8)((value & 0xff0U) >> 4);
    p[3] = (Uint8)(value & 255U);
}
static bool Run(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 2, &s));
    f->stage[0x3f68] = 14;
    Word(f->stage + 0x3f69, 0x3f60);
    Word(f->stage + 0x3f6d, 0x4f00);
    Word(f->stage + 0x3f71, 0x4f20);
    const Uint8 first[] = {9, 0, 10, 5, 1, 1, 9, 0x3a, 0xa3, 10, 11, 17, 6, 23, 0};
    SDL_memcpy(f->stage + 0x4f00, first, sizeof(first));
    const Uint8 returning[] = {9, 0x3b, 0xb9, 10, 11, 17, 7, 23, 0};
    SDL_memcpy(f->stage + 0x4f20, returning, sizeof(returning));
    f->stage[0x19c6] = 0x28;
    f->stage[0x19c7] = 2;
    Actor a = {.slot = 128, .state = {.id = 0x25, .generation = 1}};
    CHECK(ActorDialogue(s, &a) && s->death_event.active && !s->death_event.library_met &&
          s->death_event.display.portrait_page == 0x91);
    CHECK(ActorDialogue(s, &a) && s->combat.music_change && !s->combat.music &&
          s->death_event.display.palette == 0x228);
    for (Uint16 i = 0; i < 10 && !s->combat.voice_change; ++i)
        CHECK(ActorDialogue(s, &a));
    CHECK(s->combat.voice_change && s->combat.voice == 0x3a3 && s->death_event.voice_playing);
    CHECK(ActorDialogue(s, &a) && !(s->death_event.flags & (1U << 6)));
    CHECK(SotnActors_Dialogue(s, false, NULL));
    CHECK(ActorDialogue(s, &a) && (s->death_event.flags & (1U << 6)));
    CHECK(ActorDialogue(s, &a) && a.remove && !s->world->flags[SOTN_LIBRARY_MET]);
    s->world->flags[SOTN_LIBRARY_MET] = 1;
    s->combat = (SotnActorCombat){0};
    a = (Actor){.slot = 128, .state = {.id = 0x25, .generation = 2}};
    CHECK(ActorDialogue(s, &a) && s->death_event.library_met && !a.remove);
    CHECK(ActorDialogue(s, &a) && s->combat.voice_change && s->combat.voice == 0x3b9);
    CHECK(SotnActors_Dialogue(s, false, NULL));
    CHECK(ActorDialogue(s, &a) && (s->death_event.flags & (1U << 7)));
    // Reply selection is a bounded native value, never an executable or writable PS1 pointer.
    Word(f->stage + 0x3f69, 0x3f64);
    s->death_event.library_reply = 1;
    a = (Actor){.slot = 128, .state = {.id = 0x25, .generation = 3}};
    CHECK(ActorDialogue(s, &a) && ActorDialogue(s, &a) && s->combat.voice == 0x3b9);
    s->death_event.library_reply = 2;
    a.state.step = 0;
    CHECK(ActorDialogue(s, &a) && !ActorDialogue(s, &a));
    s->death_event.library_reply = 0;
    Word(f->stage + 0x3f69, 0x3f62);
    a.state.step = 0;
    CHECK(ActorDialogue(s, &a) && !ActorDialogue(s, &a));
    s->world->flags[SOTN_LIBRARY_MET] = 2;
    CHECK(!SotnWorld_Validate(s->world));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Run(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
