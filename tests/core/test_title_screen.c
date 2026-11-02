#include <sotn/combat.h>
#include <sotn/title_screen.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    SotnTitleScreen s = {0};
    Uint32 random = 7;
    SotnTitleScreenEvents e = {0};
    CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){.disc = true}, &random, &e) &&
          s.phase == 1 && e.load_graphics && e.clear && !e.title_display && random == 7);
    for (Uint16 i = 0; i < 17; ++i)
        CHECK(s.sprites[i].draw_mode == 8);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){.engine_clock = i}, &random, &e) &&
              e.title_display && random == 7 && s.brightness == (i + 1) * 8);
    CHECK(s.phase == 2 && s.retained_word == 0x800 && s.sprites[7].red[0] == 96 &&
          s.sprites[7].green[0] == 112 && s.sprites[7].red[1] == 128);
    Uint32 expected = random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){.engine_clock = SDL_MAX_UINT32}, &random,
                               &e) &&
          random == expected && s.sprites[0].draw_mode == 0 && !e.sound);
    // Supply a real libc cursor whose next result starts the first lightning.
    for (Uint32 seed = 0;; ++seed)
    {
        Uint32 next = seed;
        if (!(SotnCombat_Random(&next) & 127U))
        {
            random = seed;
            break;
        }
        CHECK(seed < 10000);
    }
    s.sprites[15].phase = s.sprites[16].phase = 0;
    CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){.engine_clock = 0}, &random, &e) &&
          s.sprites[0].draw_mode == 8 && s.sprites[15].phase == 1 && s.sprites[15].timer == 6 &&
          !s.sprites[16].phase);
    const Uint32 lightning = random;
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, &random, &e) &&
              random == lightning && !s.sprites[16].phase);
    expected = random;
    (void)SotnCombat_Random(&expected);
    CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){.start_tapped = true}, &random, &e) &&
          s.phase == 3 && e.sound == 0x63d && random == expected && s.sprites[16].phase == 1 &&
          s.sprites[16].timer >= 6 && s.sprites[16].timer <= 9 && s.sprites[16].width == 112);
    CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){.start_tapped = true}, &random, &e) &&
          s.phase == 6 && !e.sound && random == expected);
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, &random, &e) &&
              s.brightness == 120 - i * 8 && e.memory_card == (i == 15) && !e.main_menu &&
              random == expected);
    CHECK(s.phase == 5 && !s.completed &&
          SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, &random, &e) && s.completed &&
          e.clear && e.stage_display && e.restore_cluts && e.release_rows && e.main_menu &&
          !e.memory_card && random == expected);
    SotnTitleScreen before;
    SotnTitleScreenEvents events;
    SDL_memcpy(&before, &s, sizeof(s));
    SDL_memcpy(&events, &e, sizeof(e));
    CHECK(!SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, &random, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && !SDL_memcmp(&e, &events, sizeof(e)) &&
          random == expected);
    s = (SotnTitleScreen){.phase = 4};
    SDL_memcpy(&before, &s, sizeof(s));
    CHECK(!SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, &random, &e) &&
          !SDL_memcmp(&s, &before, sizeof(s)) && random == expected);
    CHECK(!SotnTitleScreen_Tick(NULL, (SotnTitleScreenInput){0}, &random, &e) &&
          !SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, NULL, &e) &&
          !SotnTitleScreen_Tick(&s, (SotnTitleScreenInput){0}, &random, NULL));
    SDL_Log("TITLE_SCREEN: PASS authored source phase/clock/lightning/fade/request/atomicity "
            "fixtures; no native startup readiness");
    return 0;
}
