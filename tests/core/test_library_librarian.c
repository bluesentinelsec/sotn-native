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
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    f->dra[0x99] = 2;
    f->dra[0x9a] = 10;
    f->dra[0x9b] = 128;
    const size_t streams[] = {0x11fc, 0x1204, 0x1210, 0x121c, 0x1234,
                              0x1244, 0x12c0, 0x12c8, 0x12d0};
    for (Uint16 i = 0; i < SDL_arraysize(streams); ++i)
    {
        f->stage[streams[i]] = 1;
        f->stage[streams[i] + 1] = 1;
        f->stage[streams[i] + 2] = 255;
    }
    if (!SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                (SotnDataView){f->stage, sizeof(f->stage)}, 2, out))
        return false;
    (*out)->player = (SotnPlayerState){.x = 240 * 65536, .y = 128 * 65536, .hp = 70, .contacts = 1};
    (*out)->game_seconds = 1234;
    return true;
}
static bool Manager(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor trigger = {.slot = 33,
                     .state = {.id = 0x2b, .generation = 1, .x = 240 * 65536, .y = 128 * 65536}};
    CHECK(ActorLibrary(s, &trigger) && trigger.state.step == 1 && s->combat.music_change &&
          !s->combat.music);
    CHECK(ActorLibrary(s, &trigger) && trigger.state.step == 2 && s->slots[128].state.id == 0x25);
    Actor *a = &s->slots[130];
    *a = (Actor){.slot = 130, .state = {.id = 0x23, .generation = 2}};
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && s->combat.sequence_change &&
          s->combat.sequence == 0x202);
    CHECK(ActorLibrary(s, a) && a->state.step == 2 && s->control.buttons == SOTN_BUTTON_LEFT);
    s->player.x = 231 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 3 && s->motion.x == 65536 &&
          (s->death_event.flags & 1U));
    s->player.x = 232 * 65536;
    SDL_zero(s->motion);
    CHECK(ActorLibrary(s, a) && a->state.step == 3 && !s->motion.x && !s->control.buttons);
    s->death_event.flags |= 0x40U;
    CHECK(ActorLibrary(s, a) && s->control.buttons == SOTN_BUTTON_LEFT);
    s->player.x = 115 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 4 && s->motion.x == 65536);
    s->player.x = 116 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 5 && s->control.hold);
    CHECK(ActorLibrary(s, a) && a->state.step == 6 && s->world->flags[SOTN_LIBRARY_MET] == 1 &&
          s->world->time_attack[SOTN_TIME_LIBRARIAN] == 2034);
    s->control.timer = 0;
    CHECK(ActorLibrary(s, a) && a->state.step == 6 && s->control.hold && !s->control.timer);
    s->death_event.flags |= 0x100U;
    CHECK(ActorLibrary(s, a) && a->state.step == 16 && (s->death_event.flags & 0x2000U));
    CHECK(ActorLibrary(s, a) && a->state.step == 17 && s->control.timer == 32 &&
          s->control.buttons == SOTN_BUTTON_RIGHT && !s->control.hold);
    s->control.timer = 0;
    CHECK(ActorLibrary(s, a) && a->state.step == 9);
    s->death_event.flags = 0;
    CHECK(ActorLibrary(s, a) && a->substep == 1 && s->control.timer == 16);
    CHECK(ActorLibrary(s, a) && a->substep == 2 && s->control.buttons == SOTN_BUTTON_LEFT);
    CHECK(ActorLibrary(s, a) && a->state.step == 10 && s->control.timer == 128);
    s->control.timer = 0;
    s->player.contacts = 0;
    CHECK(ActorLibrary(s, a) && a->state.step == 10);
    s->player.contacts = 1;
    CHECK(ActorLibrary(s, a) && a->state.step == 11 && s->control.hold);
    s->death_event.flags = 0x100;
    CHECK(ActorLibrary(s, a) && a->state.step == 16);
    a->state.step = 9;
    s->player.x = 256 * 65536;
    CHECK(ActorLibrary(s, a) && a->remove && !s->combat.sequence);
    a->remove = false;
    a->state.step = 0;
    CHECK(ActorLibrary(s, a) && a->state.step == 8);
    a->state.step = 255;
    CHECK(!ActorLibrary(s, a));
    SotnActors_Destroy(s);
    return true;
}
static bool Chair(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[129];
    *a = (Actor){.slot = 129,
                 .state = {.id = 0x24, .generation = 1, .x = 44 * 65536, .y = 172 * 65536}};
    CHECK(ActorLibrary(s, a) && a->state.frame == 2 && a->state.animation_set == 0x800b &&
          a->librarian.timer == 32);
    s->death_event.flags = 2;
    CHECK(ActorLibrary(s, a) && a->state.step == 2 && a->librarian.timer == 31);
    CHECK(ActorLibrary(s, a) && ActorLibrary(s, a) && a->state.step == 3);
    for (Uint16 step = 3; step <= 6; ++step)
    {
        s->death_event.flags = 1U << (step - 1);
        CHECK(ActorLibrary(s, a) && a->state.step == step + 1);
    }
    CHECK(ActorLibrary(s, a) && ActorLibrary(s, a) && a->state.step == 11);
    s->death_event.flags = 0x800;
    CHECK(ActorLibrary(s, a) && a->state.step == 12 && a->librarian.timer == 48);
    for (Uint16 i = 0; i < 48; ++i)
        CHECK(ActorLibrary(s, a));
    CHECK(a->state.step == 11 && !(s->death_event.flags & 0x800U));
    s->death_event.flags = 0x1000;
    CHECK(ActorLibrary(s, a) && a->state.step == 13);
    s->player.x = 40 * 65536;
    s->player.y = 180 * 65536;
    s->player.velocity_y = -65536;
    s->player.contacts = 0;
    CHECK(ActorLibrary(s, a) && a->state.step == 16 && Pixel(a->state.y) == 152 &&
          s->world->flags[SOTN_LIBRARY_DROPS] == 1);
    CHECK(s->slots[96].state.id == 3 && s->slots[96].state.parameters == 23);
    CHECK(s->combat.sound_count && s->combat.sounds[s->combat.sound_count - 1] == 0x683);
    s->player.y = 200 * 65536;
    CHECK(ActorLibrary(s, a) && a->state.step == 1 && Pixel(a->state.y) == 172);
    a->librarian.total_hits = 63;
    a->librarian.consecutive_hits = 15;
    s->player.y = 180 * 65536;
    CHECK(ActorLibrary(s, a) && s->world->flags[SOTN_LIBRARY_DROPS] == 7);
    CHECK(s->slots[97].state.id == 0xa && s->slots[97].state.parameters == SOTN_HAND_ITEMS + 25);
    CHECK(s->slots[98].state.id == 0xa && s->slots[98].state.parameters == SOTN_HAND_ITEMS + 75);
    a->state.step = 1;
    a->librarian.consecutive_hits = 23;
    s->world->flags[SOTN_INVERTED_UNLOCKED] = 1;
    CHECK(ActorLibrary(s, a) && s->world->flags[SOTN_LIBRARY_DROPS] == 15);
    CHECK(s->slots[99].state.id == 0xa && s->slots[99].state.parameters == SOTN_HAND_ITEMS + 23);
    s->player.velocity_y = 0;
    s->player.contacts = 1;
    CHECK(ActorLibrary(s, a) && a->librarian.consecutive_hits == 0);
    s->world->flags[SOTN_LIBRARY_DROPS] = 16;
    CHECK(!SotnWorld_Validate(s->world));
    SotnActors_Destroy(s);
    return true;
}
static bool FullGiftPool(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *a = &s->slots[129];
    *a = (Actor){.slot = 129, .state = {.id = 0x24, .generation = 1, .y = 172 * 65536}};
    CHECK(ActorLibrary(s, a));
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 100;
    s->player = (SotnPlayerState){.x = 40 * 65536, .y = 180 * 65536, .velocity_y = -65536};
    CHECK(ActorLibrary(s, a) && a->state.step == 16 && !s->world->flags[SOTN_LIBRARY_DROPS]);
    a->state.step = 1;
    s->slots[96] = (Actor){0};
    CHECK(ActorLibrary(s, a) && s->world->flags[SOTN_LIBRARY_DROPS] == 1 &&
          s->slots[96].state.id == 3);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Manager(f) && Chair(f) && FullGiftPool(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
