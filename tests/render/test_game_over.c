#include <SDL3/SDL.h>
#include <sotn/alucard_sprite.h>
#include <sotn/game_over.h>

#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #value, SDL_GetError());                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 art[0x1c000], common[0x42000];
    Uint16 frame[SOTN_FRAME_PIXELS], copy[SOTN_FRAME_PIXELS];
} Fixture;

static bool Presentation(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    SDL_memset(f->art, 0x11, 0x8000);
    SDL_memset(f->art + 0x8000, 1, 0x10000);
    f->art[0x18002] = 31;
    f->art[0x1a003] = 0x7c;
    for (Uint32 i = 0; i < SOTN_FRAME_PIXELS; ++i)
        f->frame[i] = 0x03e0;
    const SotnDataView art = {f->art, sizeof(f->art)};
    SotnGameOver *game_over = NULL;
    CHECK(SotnGameOver_Create(art, f->frame, SOTN_FRAME_PIXELS, &game_over));
    CHECK(SotnGameOver_Draw(game_over, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 0x03e0);
    for (Uint32 tick = 0; tick < 172; ++tick)
        CHECK(SotnGameOver_Tick(game_over, false));
    CHECK(!SotnGameOver_Finished(game_over));
    CHECK(SotnGameOver_Draw(game_over, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[0] == 0x7c00 && f->frame[96 * 256] == 31);
    SDL_memcpy(f->copy, f->frame, sizeof(f->copy));
    CHECK(SotnGameOver_Draw(game_over, f->frame, SOTN_FRAME_PIXELS));
    CHECK(SDL_memcmp(f->copy, f->frame, sizeof(f->copy)) == 0);
    CHECK(SotnGameOver_Tick(game_over, true));
    for (Uint32 tick = 0; tick < 128; ++tick)
        CHECK(SotnGameOver_Tick(game_over, false));
    CHECK(SotnGameOver_Finished(game_over));
    CHECK(SotnGameOver_Draw(game_over, f->frame, SOTN_FRAME_PIXELS) && f->frame[0] == 0);
    SotnGameOver_Destroy(game_over);
    CHECK(SotnGameOver_Create(art, f->frame, SOTN_FRAME_PIXELS, &game_over));
    for (Uint32 tick = 0; tick < 1000; ++tick)
        CHECK(SotnGameOver_Tick(game_over, false));
    CHECK(SotnGameOver_Finished(game_over));
    SotnGameOver_Destroy(game_over);
    game_over = NULL;
    CHECK(!SotnGameOver_Create((SotnDataView){0}, f->frame, SOTN_FRAME_PIXELS, &game_over) &&
          game_over == NULL);
    CHECK(!SotnGameOver_Create(art, NULL, SOTN_FRAME_PIXELS, &game_over) && game_over == NULL);
    CHECK(!SotnGameOver_Tick(NULL, false));
    CHECK(!SotnGameOver_Finished(NULL));
    SotnGameOver_Destroy(NULL);
    return true;
}

static bool DeathSprite(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    const Uint8 pixels[2] = {0x11, 0x11};
    const SotnAlucardSprite sprite = {{pixels, sizeof(pixels)}, 4, 1, 0, 0};
    f->common[0x40002] = 0xff;
    f->common[0x40003] = 0x7f;
    const SotnDataView common = {f->common, sizeof(f->common)};
    SotnPlayerDeath death = {.phase = 1, .red = 128, .blue = 128};
    CHECK(
        SotnAlucard_DrawDeath(&sprite, common, 10, 10, false, death, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[10 * 256 + 10] == 0x7fff && f->frame[10 * 256 + 13] == 0x7fff);
    death.red = 248;
    death.blue = 8;
    death.mask[0] = 1;
    SDL_memset(f->frame, 0, sizeof(f->frame));
    CHECK(
        SotnAlucard_DrawDeath(&sprite, common, 10, 10, false, death, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[10 * 256 + 10] == 0 && f->frame[10 * 256 + 11] == (31 | (1 << 5) | (1 << 10)));
    death.phase = 128;
    SDL_memset(f->frame, 0, sizeof(f->frame));
    CHECK(
        SotnAlucard_DrawDeath(&sprite, common, 10, 10, false, death, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[10 * 256 + 10] == 0);
    death.phase = 3;
    CHECK(
        !SotnAlucard_DrawDeath(&sprite, common, 10, 10, false, death, f->frame, SOTN_FRAME_PIXELS));
    return true;
}

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = Presentation(fixture) && DeathSprite(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_GAME_OVER_TEST: PASS");
    return 0;
}
