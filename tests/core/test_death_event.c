#include "actors_internal.h"
#include "character_fixture.h"
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
    Uint8 stage[0x6000], dra[0x400];
} Fixture;
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->dra + 0x98, 0x800a0200);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 65, out));
    CHECK(SotnCamera_Enter((*out)->camera, 256, 512, 52, 388));
    (*out)->camera_y = (*out)->camera->y;
    (*out)->player = (SotnPlayerState){.x = 52 * 65536, .y = 440 * 65536};
    return true;
}
static bool Scripts(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    /* Original synthetic dialogue and event programs, unrelated to the game's text. */
    const Uint8 script[] = {12,  0x85, 0x51, 0x10, 0, 17, 8,   16, 8,   7, 60,
                            148, 5,    1,    128,  2, 0,  'T', 1,  'U', 0};
    const Uint8 events[] = {0, 0, 0, 32, 0x5a, 0, 12, 1, 2, 255, 255};
    SDL_memcpy(f->stage + 0x4ce0, script, sizeof(script));
    SDL_memcpy(f->stage + 0x5100, events, sizeof(events));
    Actor a = {.state = {.id = 0x57, .generation = 1}};
    CHECK(ActorDeath(s, &a) && s->death_event.active);
    for (Uint32 tick = 0; tick < 128 && !a.remove; ++tick)
        CHECK(ActorDeath(s, &a));
    CHECK(a.remove && s->world->flags[53] == 1 && !s->death_event.active);
    CHECK(SDL_strcmp(s->death_event.display.lines[0], "T") == 0);
    CHECK(SDL_strcmp(s->death_event.display.lines[1], "U") == 0);
    CHECK(s->slots[32].state.id == 0x5a && Pixel(s->slots[32].state.x) == 12);
    CHECK(Pixel(s->slots[32].state.y) == s->camera_y + 18);
    a = (Actor){.state = {.id = 0x57, .generation = 2}};
    CHECK(ActorDeath(s, &a) && a.remove && s->death_event.display.background_enabled);
    SotnActors_Destroy(s);
    return true;
}
static bool VoiceWaits(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    const Uint8 script[] = {9, 0x35, 0x5f, 10, 11, 0};
    SDL_memcpy(f->stage + 0x4ce0, script, sizeof(script));
    Actor a = {.state = {.id = 0x57, .generation = 1}};
    CHECK(ActorDeath(s, &a));
    CHECK(ActorDeath(s, &a) && !a.remove && s->combat.voice_change && s->combat.voice == 0x35f);
    CHECK(ActorDeath(s, &a) && !a.remove);
    CHECK(SotnActors_Dialogue(s, false, NULL));
    CHECK(ActorDeath(s, &a) && a.remove);
    CHECK(!SotnActors_Dialogue(NULL, false, NULL));
    SotnActors_Destroy(s);
    return true;
}
static bool Manager(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Uint8 *image = SDL_calloc(1, 0x11000);
    CHECK(image);
    TestCharacter_Fill(image);
    SotnCharacter *character = NULL;
    CHECK(SotnCharacter_Create((SotnDataView){image, 0x11000}, &character));
    CHECK(SotnActors_SetCharacter(s, character));
    Actor a = {.state = {.id = 0x58, .generation = 1}};
    CHECK(ActorDeath(s, &a) && s->control.buttons == SOTN_BUTTON_RIGHT);
    CHECK(s->camera->top == 252 && (s->death_event.flags & 0x100));
    CHECK(ActorDeath(s, &a) && a.state.step == 2 && (s->death_event.flags & 0x80));
    s->death_event.flags |= 0x20;
    CHECK(ActorDeath(s, &a) && a.state.step == 3 && s->control.mode == 1 && s->control.timer == 1 &&
          s->control.block_stopwatch);
    CHECK(ActorDeath(s, &a) && s->control.mode == 0 && s->control.timer == 1);
    a.state.step = 3;
    a.timer = 1;
    s->death_event.flags |= 0x40;
    CHECK(ActorDeath(s, &a) && a.state.step == 4);
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(ActorDeath(s, &a));
    CHECK(a.state.step == 5 && s->death_event.display.fade == 240);
    CHECK(s->combat.music_change && s->combat.music == 0x30f);
    CHECK(s->combat.sounds[s->combat.sound_count - 1] == 0xa1);
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(ActorDeath(s, &a));
    CHECK(a.remove && s->death_event.display.fade == 0 && s->control.timer == 0);
    CHECK(s->camera->top == 0 && s->death_event.display.background_enabled);
    a = (Actor){.state = {.id = 0x5b, .generation = 5}, .enemy_state = {.owner_generation = 99}};
    CHECK(ActorDeath(s, &a) && a.remove);
    a = (Actor){.state = {.id = 0x5e, .generation = 6, .frame = 12}};
    CHECK(ActorDeath(s, &a) && a.state.frame == 12 && a.timer == 32);
    for (Uint32 i = 0; i < 32; ++i)
        CHECK(ActorDeath(s, &a));
    CHECK(a.remove);
    SotnActors_Destroy(s);
    SotnCharacter_Destroy(character);
    SDL_free(image);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = {.state = {.id = 0x57, .generation = 1}};
    f->stage[0x4ce0] = 12; // Zero pointer is rejected before event execution.
    CHECK(ActorDeath(s, &a));
    CHECK(!ActorDeath(s, &a));
    s->death_event.script = 0x4ce0;
    f->stage[0x4ce0] = 17;
    f->stage[0x4ce1] = 32;
    CHECK(!ActorDeath(s, &a));
    f->stage[0x4ce0] = 'T';
    s->death_event.script = 0x4ce0;
    s->death_event.display.column = 24;
    CHECK(!ActorDeath(s, &a));
    const Uint8 loop[] = {15, 0x84, 0x4c, 0xce, 0xe0};
    SDL_memcpy(f->stage + 0x4ce0, loop, sizeof(loop));
    s->death_event.script = 0x4ce0;
    CHECK(!ActorDeath(s, &a) && SDL_strstr(SDL_GetError(), "budget") != NULL);
    a = (Actor){.state = {.id = 0x59, .parameters = 6, .generation = 3}};
    CHECK(!ActorDeath(s, &a));
    a = (Actor){.state = {.id = 0x58, .step = 5, .generation = 4}};
    s->death_event.display.fade = 0;
    CHECK(!ActorDeath(s, &a));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture fixture = {0};
    if (!Scripts(&fixture) || !VoiceWaits(&fixture) || !Manager(&fixture) || !Failures(&fixture))
        return 1;
    SDL_Log("SOTN_DEATH_EVENT_TEST: PASS");
    return 0;
}
