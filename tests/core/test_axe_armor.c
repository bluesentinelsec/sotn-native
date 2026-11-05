#include <SDL3/SDL.h>
#include <sotn/axe_armor.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static SotnAxeArmorFrame Frame(Uint16 step)
{
    return (SotnAxeArmorFrame){.step = step, .animation = {.id = 0xcf, .remaining = 4}};
}
static bool Movement(void)
{
    SotnAxeArmor s = {0};
    SotnAxeArmorEvents e = {0};
    SotnAxeArmorFrame f = Frame(0);
    CHECK(SotnAxeArmor_Enter(&f, 1) && f.step == 40 && f.animation.id == 0xcf && !f.velocity_x &&
          !f.velocity_y && !f.animation.remaining);
    CHECK(SotnAxeArmor_Enter(&f, 0) && f.step == 42 && f.animation.id == 0xcd);
    f = Frame(40);
    f.velocity_x = 16384;
    CHECK(SotnAxeArmor_Tick(
              &s, &f, (SotnAxeArmorInput){.contacts = 1, .pressed = 0x2000, .tapped = 0xc0}, &e) &&
          f.step == 41 && f.velocity_x == 163840 && f.animation.id == 0xcd && !e.factory_count);
    f = Frame(40);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1, .tapped = 0xc0}, &e) &&
          f.step == 40 && f.substep == 1 && f.animation.id == 0xce && e.sound_count == 1 &&
          e.sounds[0] == 0x6f0 && e.factory_count == 1 && e.factories[0] == 87);
    f.animation.remaining = -1;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1}, &e) && f.step == 40 &&
          !f.substep && f.animation.id == 0xcf && !e.factory_count);
    const Uint16 poses[] = {1, 4, 7, 10};
    for (Uint16 i = 0; i < SDL_arraysize(poses); ++i)
    {
        f = Frame(40);
        f.animation.pose = poses[i];
        CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1}, &e) &&
              e.sound_count == 1 && e.sounds[0] == (i == 3 ? 0x6ef : 0x6ee));
    }
    f = Frame(40);
    CHECK(SotnAxeArmor_Tick(&s, &f,
                            (SotnAxeArmorInput){.contacts = 1, .tapped = 0x40, .tight_space = true},
                            &e) &&
          f.step == 40);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1, .tapped = 0x40}, &e) &&
          f.step == 42 && f.velocity_y == -4 * 65536 && f.animation.id == 0xd0);
    f = Frame(40);
    f.velocity_x = 65536;
    f.velocity_y = -65536;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){0}, &e) && f.step == 42 && !f.velocity_x &&
          !f.velocity_y);
    f = Frame(41);
    f.facing_left = true;
    CHECK(SotnAxeArmor_Tick(
              &s, &f, (SotnAxeArmorInput){.contacts = 1, .pressed = 0x8000, .clock = 8}, &e) &&
          f.step == 41 && f.velocity_x == -163840 && e.sound_count == 1 && e.sounds[0] == 0x64b &&
          e.factories[0] == 0x10045);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1}, &e) && f.step == 40 &&
          f.velocity_x == -81920 && !e.sound_count);
    f = Frame(41);
    f.facing_left = true;
    CHECK(SotnAxeArmor_Tick(
              &s, &f, (SotnAxeArmorInput){.contacts = 1, .pressed = 0x2000, .clock = 1}, &e) &&
          !f.facing_left && f.turned && f.velocity_x == -163840);
    f = Frame(41);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.pressed = 0x2000}, &e) && f.step == 42 &&
          f.velocity_x == 163840 && !f.velocity_y);
    return true;
}
static bool AirAndHold(void)
{
    SotnAxeArmor s = {0};
    SotnAxeArmorEvents e = {0};
    SotnAxeArmorFrame f = Frame(42);
    f.velocity_y = -65536;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.pressed = 0x2000, .tapped = 0x80}, &e) &&
          f.step == 42 && f.substep == 1 && f.velocity_x == 49152 && f.velocity_y == -43008 &&
          e.factories[0] == 87);
    f.animation.remaining = -1;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.tapped = 0x80}, &e) && !f.substep &&
          f.animation.id == 0xd0 && !e.factory_count);
    f = Frame(42);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1, .pressed = 0x2000}, &e) &&
          f.step == 41 && f.velocity_x == 163840 && !f.velocity_y && e.sounds[0] == 0x64b);
    f = Frame(42);
    f.substep = 1;
    f.animation.id = 0xce;
    f.animation.remaining = 3;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1, .pressed = 0x2000}, &e) &&
          f.step == 40 && f.substep == 1 && f.animation.id == 0xce && f.animation.remaining == 3 &&
          !f.velocity_x && !f.velocity_y);
    f = Frame(50);
    f.velocity_x = f.velocity_y = 123;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.simulated_mode = 2}, &e) && f.step == 50 &&
          !f.velocity_x && !f.velocity_y);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){0}, &e) && f.step == 40 && !f.substep &&
          f.animation.id == 0xcf && !f.animation.remaining);
    return true;
}
static bool Hurt(void)
{
    Uint8 exe[0x1dbcc + 2050] = {0};
    const SotnDataView image = {exe, sizeof(exe)};
    SotnAxeArmor s = {.angle = 0xffffff80U};
    SotnAxeArmorEvents e = {0};
    SotnAxeArmorFrame f = Frame(43);
    f.knock_right = f.facing_left = true;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){0}, &e) && !s.angle && f.rotation == -512 &&
          f.pivot_y == 20 && f.substep == 1 && f.draw_flags == 4 && !e.factory_count);
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 3}, &e) && f.substep == 2 &&
          !f.velocity_y && f.rotation == -896 && s.recovery == 48 && e.sound_count == 1 &&
          e.sounds[0] == 0x64b && e.factory_count == 1 && !e.factories[0]);
    f.velocity_x = -65536;
    const SotnAxeArmor before = s;
    const SotnAxeArmorFrame old = f;
    const SotnAxeArmorEvents pending = e;
    CHECK(!SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.contacts = 1}, &e) &&
          !SDL_memcmp(&before, &s, sizeof(s)) && !SDL_memcmp(&old, &f, sizeof(f)) &&
          !SDL_memcmp(&pending, &e, sizeof(e)));
    for (Uint16 i = 0; i < 48; ++i)
    {
        CHECK(
            SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.executable = image, .contacts = 1}, &e));
        CHECK(s.recovery == 47 - i && !e.factory_count && !e.sound_count);
        if (i < 47)
            CHECK(f.step == 43 && f.draw_flags == 12 && f.opacity == 192 &&
                  f.rotation == (i < 43 ? -896 : -512));
    }
    CHECK(f.step == 40 && !f.substep && !f.rotation && !f.draw_flags && f.animation.id == 0xcf);
    f = Frame(43);
    f.substep = 2;
    f.rotation = 896;
    CHECK(SotnAxeArmor_Tick(&s, &f, (SotnAxeArmorInput){.executable = image}, &e) && f.step == 42 &&
          !f.substep && !f.velocity_y && !f.rotation && !f.draw_flags);
    return true;
}
int main(void)
{
    return Movement() && AirAndHold() && Hurt() ? 0 : 1;
}
