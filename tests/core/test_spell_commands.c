#include <sotn/player.h>
#include <sotn/spell_commands.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    SotnSpellCommands commands;
    SotnSpellCommandFrame frame;
    SotnSpellCommandEvents event;
    Uint8 dra[0x8500];
    Uint32 previous;
} Fixture;
static void Init(Fixture *f, Uint16 id, bool left)
{
    *f = (Fixture){
        .frame = {
            .available = (Uint8)(1U << id), .facing_left = left, .sword_active = true, .mp = 10}};
    f->dra[0x8410U + id * 28U + 12] = 10;
}
static bool Tick(Fixture *f, Uint32 buttons, Uint16 step)
{
    f->frame.pressed = buttons;
    f->frame.tapped = buttons & ~f->previous;
    f->frame.step = step;
    f->previous = buttons;
    return SotnSpellCommands_Tick(&f->commands, (SotnDataView){f->dra, sizeof(f->dra)}, &f->frame,
                                  &f->event);
}
static bool Replays(Fixture *f)
{
    const Uint16 ids[] = {0, 1, 2, 3, 5, 7};
    for (Uint16 face = 0; face < 2; ++face)
        for (Uint16 n = 0; n < SDL_arraysize(ids); ++n)
        {
            const Uint16 id = ids[n];
            Init(f, id, face != 0);
            const Uint32 forward = face ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT,
                         back = face ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT, up = SOTN_BUTTON_UP,
                         down = SOTN_BUTTON_DOWN;
            if (id == 0)
            {
                const Uint32 sequence[] = {back, up | back, up, up | forward, forward};
                for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
                    CHECK(Tick(f, sequence[i], 0) && !f->event.cast);
            }
            if (id == 1)
            {
                const Uint32 sequence[] = {back, forward, up, down};
                for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
                    CHECK(Tick(f, sequence[i], 2) && !f->event.cast);
            }
            if (id == 2)
            {
                const Uint32 sequence[] = {up, down, down | forward, forward};
                for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
                    CHECK(Tick(f, sequence[i], 0) && !f->event.cast);
            }
            if (id == 3)
            {
                CHECK(Tick(f, up, 0));
                for (Uint16 i = 0; i < 32; ++i)
                    CHECK(Tick(f, up, 0) && !f->event.cast);
                CHECK(f->commands.spells[id].phase == 2);
                const Uint32 sequence[] = {up | forward, forward, down | forward, down};
                for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
                    CHECK(Tick(f, sequence[i], 2) && !f->event.cast);
            }
            if (id == 5)
            {
                const Uint32 sequence[] = {back,        forward, down | forward, down,
                                           down | back, back,    forward};
                for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
                    CHECK(Tick(f, sequence[i], 0) && !f->event.cast);
            }
            if (id == 7)
            {
                const Uint32 sequence[] = {down, down | forward, forward, up | forward, up};
                for (Uint16 i = 0; i < SDL_arraysize(sequence); ++i)
                    CHECK(Tick(f, sequence[i], 0) && !f->event.cast);
                for (Uint16 i = 0; i < 64; ++i)
                    CHECK(Tick(f, up, 0) && !f->event.cast);
                CHECK(f->commands.spells[id].phase == 6 && Tick(f, 0, 0) && Tick(f, down, 2));
            }
            const Uint16 step = id == 1 || id == 3 || id == 7 ? 2 : 0;
            CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND, step) && f->event.cast && f->event.spell == id &&
                  f->event.mp_spent == 10 && !f->frame.mp);
            if (id == 7)
            {
                CHECK(f->commands.spells[id].phase == 255 && f->event.sword_notify);
                for (Uint16 i = 0; i < 20; ++i)
                    CHECK(Tick(f, 0, 0) && !f->event.cast && f->event.sword_notify == (i == 19));
            }
        }
    return true;
}
static bool Boundaries(Fixture *f)
{
    const Uint16 ids[] = {0, 1, 2, 3, 5, 7};
    const Sint16 final[] = {5, 4, 4, 7, 7, 8};
    for (Uint16 i = 0; i < 6; ++i)
    {
        const Uint16 id = ids[i];
        Init(f, id, false);
        f->commands.spells[id] = (SotnSpellCommand){.phase = final[i], .timer = 1};
        CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND, id == 1 || id == 3 || id == 7 ? 2 : 0));
        CHECK(f->event.cast == (id == 1 || id == 3 || id == 7));
        Init(f, id, false);
        f->commands.spells[id] = (SotnSpellCommand){.phase = final[i], .timer = 20};
        f->frame.mp = 9;
        CHECK(Tick(f, SOTN_BUTTON_RIGHT_HAND, id == 1 || id == 3 || id == 7 ? 2 : 0) &&
              !f->event.cast && f->frame.mp == 9);
        f->frame.mp = 10;
        f->frame.attack_mode = 0x8000;
        CHECK(Tick(f, 0, 0) && Tick(f, SOTN_BUTTON_LEFT_HAND, 2) && !f->event.cast);
    }
    Init(f, 0, false);
    CHECK(Tick(f, SOTN_BUTTON_LEFT | SOTN_BUTTON_JUMP, 0) && f->commands.spells[0].phase == 1 &&
          !f->commands.spells[1].phase);
    CHECK(Tick(f, SOTN_BUTTON_UP | SOTN_BUTTON_LEFT, 0));
    CHECK(Tick(f, SOTN_BUTTON_UP | SOTN_BUTTON_JUMP, 0) && f->commands.spells[0].phase == 2);
    CHECK(Tick(f, SOTN_BUTTON_UP, 0) && f->commands.spells[0].phase == 3);
    Init(f, 1, false);
    CHECK(Tick(f, SOTN_BUTTON_LEFT, 0) && Tick(f, 0, 0) && f->commands.spells[1].phase == 2);
    Init(f, 5, false);
    CHECK(Tick(f, SOTN_BUTTON_LEFT, 0) && f->commands.spells[5].timer == 24);
    Init(f, 5, true);
    CHECK(Tick(f, SOTN_BUTTON_RIGHT, 0) && f->commands.spells[5].timer == 20);
    Init(f, 2, false);
    f->commands.spells[2] = (SotnSpellCommand){.phase = 4, .timer = 0};
    f->frame.cramped = true;
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND, 0) && !f->event.cast && !f->commands.spells[2].phase);
    Init(f, 0, false);
    f->commands.spells[0] = (SotnSpellCommand){.phase = 1, .timer = SDL_MIN_SINT16};
    CHECK(Tick(f, 0, 0) && f->commands.spells[0].timer == SDL_MAX_SINT16);
    return true;
}
static bool PriorityAndFailure(Fixture *f)
{
    Init(f, 0, false);
    f->frame.available = 0xaf;
    f->commands.spells[2] = (SotnSpellCommand){.phase = 4, .timer = 20};
    f->commands.spells[0] = (SotnSpellCommand){.phase = 5, .timer = 20};
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND, 0) && f->event.cast && f->event.spell == 2 &&
          f->commands.spells[0].timer == 20);
    Init(f, 0, false);
    f->frame.available = 0;
    f->commands.spells[0] = (SotnSpellCommand){.phase = 5, .timer = 20};
    f->frame.pressed = f->frame.tapped = SOTN_BUTTON_LEFT_HAND;
    CHECK(SotnSpellCommands_Tick(&f->commands, (SotnDataView){0}, &f->frame, &f->event) &&
          !f->event.cast && f->frame.mp == 10);
    f->frame.available = 1;
    const Fixture before = *f;
    CHECK(!SotnSpellCommands_Tick(&f->commands, (SotnDataView){0}, &f->frame, &f->event) &&
          !SDL_memcmp(&before, f, sizeof(before)));
    f->frame.available = 0x40;
    const Fixture invalid = *f;
    CHECK(!SotnSpellCommands_Tick(&f->commands, (SotnDataView){f->dra, sizeof(f->dra)}, &f->frame,
                                  &f->event) &&
          !SDL_memcmp(&invalid, f, sizeof(invalid)));
    Init(f, 1, false);
    f->commands.spells[7] = (SotnSpellCommand){.phase = 255, .timer = 1};
    f->commands.spells[1] = (SotnSpellCommand){.phase = 4, .timer = 20};
    CHECK(Tick(f, SOTN_BUTTON_LEFT_HAND, 2) && f->event.cast && f->event.spell == 1 &&
          f->event.sword_notify);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Replays(f) && Boundaries(f) && PriorityAndFailure(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
