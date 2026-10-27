#include <SDL3/SDL.h>
#include <sotn/bat_command.h>
#include <sotn/player.h>

#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static SotnBatCommandFrame Frame(bool left)
{
    return (SotnBatCommandFrame){.animation = {.id = 0xc3, .pose = 7, .remaining = 3},
                                 .substep = 1,
                                 .mp = 20,
                                 .facing_left = left};
}
static bool Sequence(SotnBatCommand *c, SotnDataView dra, SotnBatCommandFrame *f)
{
    const Uint16 backward = f->facing_left ? SOTN_BUTTON_RIGHT : SOTN_BUTTON_LEFT,
                 forward = f->facing_left ? SOTN_BUTTON_LEFT : SOTN_BUTTON_RIGHT;
    const Uint16 directions[] = {SOTN_BUTTON_UP,   (Uint16)(SOTN_BUTTON_UP | backward),
                                 backward,         (Uint16)(SOTN_BUTTON_DOWN | backward),
                                 SOTN_BUTTON_DOWN, (Uint16)(SOTN_BUTTON_DOWN | forward),
                                 forward};
    SotnBatCommandEvents e = {0};
    for (Uint16 i = 0; i < SDL_arraysize(directions); ++i)
    {
        f->buttons = directions[i] | SOTN_BUTTON_JUMP;
        CHECK(SotnBatCommand_Tick(c, dra, f, &e));
        CHECK(c->counter == i + 1U && c->timer == 20 && !e.wing_attack && !e.mp_spent);
    }
    return true;
}
static bool Commands(SotnDataView dra)
{
    for (Uint16 left = 0; left < 2; ++left)
    {
        SotnBatCommand c = {0};
        SotnBatCommandFrame f = Frame(left != 0);
        SotnBatCommandEvents e = {0};
        CHECK(Sequence(&c, dra, &f));
        f.mp = 8;
        f.buttons = 0;
        f.game_timer = 3;
        CHECK(SotnBatCommand_Tick(&c, dra, &f, &e));
        CHECK(e.wing_attack && e.wing_flash && e.learn_wing_smash && e.mp_spent == 8 && e.handled &&
              e.trail && !e.return_requested && f.mp == 0 && c.duration == 63 && c.counter == 0 &&
              f.substep == 3 && f.animation.id == 0xc6 && f.velocity_x == (left ? -6 : 6) * 65536);
        // Original US input permits another paid Wing Smash while already smashing.
        CHECK(Sequence(&c, dra, &f));
        f.mp = 8;
        f.buttons = 0;
        CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && e.wing_attack && c.duration == 63);
        f.contacts = left ? 8 : 4;
        const Uint32 duration = c.duration;
        CHECK(SotnBatCommand_Tick(&c, dra, &f, &e));
        CHECK(e.wall_impact && e.return_requested && f.velocity_x == 0 && c.duration == duration);
    }
    SotnBatCommand c = {0};
    SotnBatCommandFrame f = Frame(false);
    SotnBatCommandEvents e = {0};
    CHECK(Sequence(&c, dra, &f));
    f.mp = 7;
    f.buttons = 0;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && !e.wing_attack && f.mp == 7);
    c = (SotnBatCommand){.counter = 1, .timer = 20};
    f.buttons = SOTN_BUTTON_JUMP;
    for (Uint16 i = 0; i < 20; ++i)
        CHECK(SotnBatCommand_Tick(&c, dra, &f, &e));
    CHECK(c.counter == 0 && c.timer == 0);
    c = (SotnBatCommand){0};
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && c.timer == SDL_MAX_UINT32);
    c = (SotnBatCommand){.counter = 3, .timer = 20};
    f.buttons = 0;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && c.counter == 0);
    return true;
}
static bool Motion(SotnDataView dra)
{
    SotnBatCommand c = {.duration = 5};
    SotnBatCommandFrame f = Frame(false);
    SotnBatCommandEvents e = {0};
    f.substep = 3;
    f.velocity_x = 6 * 65536;
    f.velocity_y = -6 * 65536;
    f.buttons = SOTN_BUTTON_UP;
    f.contacts = 0x8800;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && f.velocity_y == -6 * 65536);
    f.contacts = 0x800;
    f.buttons = SOTN_BUTTON_DOWN;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && f.velocity_y == 6 * 65536);
    f.contacts = 3;
    f.buttons = 0;
    f.game_timer = 3;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e));
    CHECK(e.trail && e.floor_dust && e.ceiling_dust && f.velocity_y == 6 * 65536 - 8192);
    c.duration = 1;
    const Sint32 vy = f.velocity_y;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && e.return_requested && !e.trail &&
          f.velocity_y == vy && !c.duration);
    c = (SotnBatCommand){0};
    f = Frame(false);
    f.echo_enabled = f.fire_enabled = true;
    f.tapped = SOTN_BUTTON_CANCEL | SOTN_BUTTON_LEFT_HAND;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && e.echo && !e.fire_flash && !e.mp_spent);
    f.tapped = SOTN_BUTTON_LEFT_HAND;
    f.mp = 2;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && e.fire_flash && e.mp_spent == 2 && f.mp == 0 &&
          f.animation.id == 0xc9 && f.substep == 4);
    f.velocity_x = 65536;
    f.velocity_y = -65536;
    f.animation.remaining = -1;
    f.tapped = 0;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && e.fireball && f.substep == 5 &&
          f.velocity_x == -98304 && f.velocity_y == -51200 && f.animation.id == 0xc3);
    f.animation.id = 0xc4;
    f.animation.pose = 8;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && f.substep == 1);
    f.fire_enabled = f.echo_enabled = false;
    f.tapped = SOTN_BUTTON_LEFT_HAND | SOTN_BUTTON_CANCEL;
    CHECK(SotnBatCommand_Tick(&c, dra, &f, &e) && !e.fire_flash && !e.echo);
    return true;
}
static bool Invalid(SotnDataView dra)
{
    SotnBatCommand c = {.counter = 7, .timer = 20};
    SotnBatCommandFrame f = Frame(false);
    SotnBatCommandEvents e = {.fireball = true};
    const SotnBatCommand before = c;
    const SotnBatCommandFrame frame = f;
    CHECK(!SotnBatCommand_Tick(&c, (SotnDataView){0}, &f, &e));
    CHECK(c.counter == before.counter && c.timer == before.timer && f.mp == frame.mp &&
          f.substep == frame.substep && e.fireball);
    CHECK(!SotnBatCommand_Tick(NULL, dra, &f, &e));
    c.counter = 8;
    CHECK(!SotnBatCommand_Tick(&c, dra, &f, &e));
    c.counter = 0;
    f.velocity_y = SDL_MAX_SINT32;
    CHECK(!SotnBatCommand_Tick(&c, dra, &f, &e));
    return true;
}
int main(void)
{
    Uint8 *bytes = SDL_calloc(1, 0x9000);
    if (!bytes)
        return 1;
    bytes[0x8410 + 6 * 28 + 12] = 8;
    bytes[0x8410 + 9 * 28 + 12] = 2;
    const SotnDataView dra = {bytes, 0x9000};
    const bool ok = Commands(dra) && Motion(dra) && Invalid(dra);
    SDL_free(bytes);
    return ok ? 0 : 1;
}
