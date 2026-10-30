#include <SDL3/SDL.h>
#include <sotn/water_damage.h>
#include <sotn/water_death.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static bool Exposure(void)
{
    SotnWaterDamage s = {0};
    SotnWaterDamageInput in = {.hp = 70, .hp_max = 70, .waist = 1};
    SotnWaterDamageEvents e = {0};
    Uint16 damage = 0, voices = 0;
    for (Uint32 t = 0; t < 100; ++t)
    {
        in.game_timer = t;
        CHECK(SotnWaterDamage_Tick(&s, in, &e));
        CHECK(e.scheduled && !e.delegated && e.response == 9 && e.blink == 0x4e &&
              s.pain_timer == 32 && e.scrape == (e.damage != 0));
        damage += e.damage;
        voices += e.voice ? 1 : 0;
        in.hp = e.hp;
    }
    CHECK(damage == 10 && voices == 1 && in.hp == 60);
    in.waist = 0;
    for (Uint16 i = 0; i < 31; ++i)
        CHECK(SotnWaterDamage_Tick(&s, in, &e) && !e.response && !e.voice);
    CHECK(s.pain_timer == 1);
    in.waist = 1;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.voice && s.pain_timer == 32);
    in.game_timer = SDL_MAX_UINT32;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && !e.damage);
    in.game_timer = 0;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.damage == 1);
    return true;
}
static bool Priority(void)
{
    SotnWaterDamage s = {0};
    SotnWaterDamageInput in = {
        .hp = 1, .hp_max = 70, .waist = 1, .hit_params = 0x1000, .invincible = true};
    SotnWaterDamageEvents e = {0};
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.hit_params == 6 && e.scheduled && !e.response &&
          e.hp == 1 && !s.pain_timer);
    in.invincible = false;
    in.grabbed = true;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.scheduled && !e.response);
    in.grabbed = false;
    in.holy_symbol = true;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && !e.scheduled && e.delegated &&
          e.hit_params == 0x1000 && e.hp == 1);
    in.holy_symbol = false;
    in.hit_params = 2;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && !e.scheduled && e.delegated && e.hit_params == 2);
    in.hit_params = 0x1006;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && !e.scheduled && e.delegated);
    in.hit_params = 0;
    in.status = 0x80;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.scheduled && e.delegated && !e.response);
    in.status = 0;
    in.fury_plate = true;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.hp == 0 && e.damage == 1 && e.response == 6 &&
          !e.blink && !e.voice && !e.scrape && !e.defense_buff_min && !s.pain_timer);
    in.game_timer = 1;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.hp == 1 && !e.damage && e.response == 9 &&
          e.defense_buff_min == 512 && e.voice);
    in.holy_symbol = true;
    in.waist = 0;
    in.hit_params = 6;
    CHECK(SotnWaterDamage_Tick(&s, in, &e) && e.response == 9 && !e.scheduled);
    const SotnWaterDamage before = s;
    const SotnWaterDamageEvents old = e;
    in.hp = 0;
    CHECK(!SotnWaterDamage_Tick(&s, in, &e) && !SDL_memcmp(&s, &before, sizeof(s)) &&
          !SDL_memcmp(&e, &old, sizeof(e)));
    in.hp = 1;
    in.waist = 32768;
    CHECK(!SotnWaterDamage_Tick(&s, in, &e) && !SDL_memcmp(&s, &before, sizeof(s)));
    return true;
}
static bool Death(void)
{
    SotnWaterDeath s = {.step = 13, .velocity_x = 99, .velocity_y = 100};
    SotnWaterDeathEvents e = {0};
    CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){.fade = 0xfff}, &e));
    CHECK(s.step == 13 && s.phase == 1 && !s.velocity_x && s.velocity_y == -106496 &&
          !s.draw_flags && s.rotation == 512 && s.blend == 0x30 && s.color_blend && s.red == 128 &&
          s.blue == 128 && e.count == 6);
    CHECK(e.actions[0].kind == SOTN_WATER_DEATH_SOUND && e.actions[0].value == 0x6f6 &&
          e.actions[1].kind == SOTN_WATER_DEATH_RESET_NORMAL &&
          e.actions[2].kind == SOTN_WATER_DEATH_ANIMATION && e.actions[2].value == 0xc1 &&
          e.actions[3].kind == SOTN_WATER_DEATH_GLOBAL_EFFECT && e.actions[3].value == 1 &&
          e.actions[4].value == 0x59002c && e.actions[5].value == 0x60031);
    CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){.fade = 0xfff, .remaining = -1}, &e) &&
          s.step == 17 && !s.phase && !s.velocity_y && s.draw_flags == 4 && !e.count &&
          !e.begin_disintegration && s.red == 128);
    s = (SotnWaterDeath){.step = 13};
    CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){.tight_space = true}, &e) && !s.velocity_y);
    for (Uint16 i = 0; i < 120; ++i)
        CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){0}, &e) && !e.count &&
              s.velocity_y >= 4096 && s.velocity_y <= 16384);
    CHECK(s.red == 8 && s.blue == 248);
    s.velocity_y = 12288;
    CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){0}, &e) && s.velocity_y == 16384);
    CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){0}, &e) && s.velocity_y == 4096);
    s.velocity_y = SDL_MAX_SINT32;
    CHECK(SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){.remaining = -1}, &e) &&
          s.velocity_y == SDL_MIN_SINT32 + 4095 && s.step == 16 && s.phase == 2 && e.count == 1 &&
          e.actions[0].kind == SOTN_WATER_DEATH_CAPTURE && e.begin_disintegration);
    const SotnWaterDeath before = s;
    const SotnWaterDeathEvents old = e;
    CHECK(!SotnWaterDeath_Tick(&s, (SotnWaterDeathInput){0}, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && !SDL_memcmp(&e, &old, sizeof(e)));
    return true;
}
int main(void)
{
    const bool ok = Exposure() && Priority() && Death();
    if (ok)
        SDL_Log("WATER_DAMAGE_DEATH: PASS cadence/priority/protection/voice/buff/lethality and "
                "ordered death/reset/color/velocity/fade/capture contexts");
    return ok ? 0 : 1;
}
