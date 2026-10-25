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
static bool Run(void)
{
    Uint8 dra[0x400] = {0}, stage[0x6000] = {0};
    dra[0x98] = 0;
    dra[0x99] = 2;
    dra[0x9a] = 10;
    dra[0x9b] = 128;
    SotnActors *s = NULL;
    CHECK(SotnActors_Create((SotnDataView){dra, sizeof(dra)}, (SotnDataView){stage, sizeof(stage)},
                            &s));
    const Uint8 program[] = {9, 1, 0, 11, 5, 1, 0, 9, 0x36, 0x69, 10, 11, 17, 13, 0};
    SDL_memcpy(stage + 0x3b0c, program, sizeof(program));
    stage[0x13d2] = 0x40;
    stage[0x13d3] = 2;
    Actor script = {.state = {.id = 0x4b, .generation = 1}};
    CHECK(ActorMaria(s, &script) && s->death_event.active);
    CHECK(ActorMaria(s, &script) && s->combat.cd_command == 0x10 && !script.remove);
    CHECK(ActorMaria(s, &script) && !script.remove);
    CHECK(SotnActors_SetDialogueBusy(s, false));
    for (Uint32 i = 0; i < 10 && !s->combat.voice_change; ++i)
        CHECK(ActorMaria(s, &script));
    CHECK(s->combat.voice_change && s->combat.voice == 0x369);
    CHECK(s->death_event.display.palette == 0x240 && s->death_event.voice_playing);
    CHECK(ActorMaria(s, &script) && !script.remove);
    CHECK(SotnActors_Dialogue(s, false, NULL));
    CHECK(ActorMaria(s, &script) && script.remove && s->world->flags[SOTN_LAB_MARIA] == 1);
    CHECK((s->death_event.flags & 0x2000) && !s->death_event.active);
    script = (Actor){.state = {.id = 0x4b, .generation = 2}};
    CHECK(ActorMaria(s, &script) && script.remove && !s->death_event.display.background_enabled);

    Actor manager = {.state = {.id = 0x4d, .generation = 3}};
    s->player.x = 200 * 65536;
    CHECK(ActorMaria(s, &manager) && s->control.buttons == SOTN_BUTTON_LEFT);
    CHECK(ActorMaria(s, &manager) && manager.state.step == 1);
    s->player.x = 175 * 65536;
    CHECK(ActorMaria(s, &manager) && manager.state.step == 2);
    CHECK(ActorMaria(s, &manager) && manager.remove && s->control.timer == 0);

    const size_t streams[] = {0x1474, 0x1490, 0x14a0, 0x14bc, 0x14c8, 0x14e8,
                              0x14f8, 0x1510, 0x151c, 0x1530, 0x1538};
    for (Uint32 i = 0; i < SDL_arraysize(streams); ++i)
    {
        stage[streams[i]] = 1;
        stage[streams[i] + 1] = 1;
        stage[streams[i] + 2] = 255;
    }
    Actor maria = {.state = {.id = 0x4c, .generation = 4}};
    CHECK(ActorMaria(s, &maria) && maria.state.animation_set == 0x800f &&
          Pixel(maria.state.x) == -16);
    s->death_event.flags = 4;
    CHECK(!ActorMaria(s, &maria)); // Never silently replace a missing door.
    s->slots[128] = (Actor){.state = {.id = 5, .generation = 9, .step = 1}};
    CHECK(ActorMaria(s, &maria) && maria.state.step == 2 && s->slots[128].state.step == 2);
    s->slots[128].state.step = 3;
    CHECK(ActorMaria(s, &maria) && maria.state.step == 3 && maria.vx == 49152);
    maria.state.x = 26 * 65536;
    CHECK(ActorMaria(s, &maria) && maria.state.step == 4 && (s->death_event.flags & 2));
    CHECK(s->slots[128].state.step == 5);
    maria.state.x = 82 * 65536;
    CHECK(ActorMaria(s, &maria) && maria.state.step == 5 && maria.vx == 0);
    for (Uint16 step = 5; step <= 13; ++step)
    {
        s->death_event.flags = 0;
        CHECK(ActorMaria(s, &maria) && maria.state.step == step);
        s->death_event.flags = 1U << (step - 2);
        CHECK(ActorMaria(s, &maria) && maria.state.step == step + 1);
    }
    maria.state.x = 226 * 65536;
    CHECK(ActorMaria(s, &maria) && maria.state.step == 15 && (s->death_event.flags & 0x1000));
    maria.state.x = 274 * 65536;
    CHECK(ActorMaria(s, &maria) && maria.remove);
    CHECK(!SotnActors_SetDialogueBusy(NULL, false));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    if (!Run())
        return 1;
    SDL_Log("LABORATORY_MARIA: PASS");
    return 0;
}
