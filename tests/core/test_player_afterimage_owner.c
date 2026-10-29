#include "player_internal.h"
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
    Uint8 dra[0x11000], tiles[32 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnCollisionMap map;
} Fixture;
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Create(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 6; ++i)
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    SDL_memset(f->dra + 0xcf18, 3, 20);
    SDL_memset(f->dra + 0xcf2c, 5, 16);
    SDL_memset(f->dra + 0xcf3c, 28, 16);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 32; ++x)
            f->tiles[(y * 32U + x) * 2U] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 32, 16};
    return SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 64, 135, &f->p);
}
static bool Tick(Fixture *f, Uint32 timer, Uint16 buttons)
{
    CHECK(SotnPlayer_SetAfterimageContext(f->p, 17, timer) && SotnPlayer_Tick(f->p, buttons));
    return true;
}
static bool Movement(Fixture *f)
{
    CHECK(Create(f));
    CHECK(!SotnPlayer_ResetAfterimages(f->p, 1));
    CHECK(Tick(f, 1, SOTN_BUTTON_RIGHT));
    SotnPlayerState p = {0};
    CHECK(SotnPlayer_GetState(f->p, &p));
    CHECK(p.afterimage_generation == 17 && p.x == 64 * 65536 + 98304 &&
          p.afterimage.samples[0].x == 64 && p.afterimage.samples[0].y == 135 &&
          p.afterimage.pose == 2 && p.afterimage.write_index == 1 &&
          p.afterimage.displays[0].x == 64 * 65536 &&
          p.afterimage.displays[0].sprite == p.animation.sprite);
    const SotnPlayer before = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, 0) && !SDL_memcmp(&before, f->p, sizeof(before)));
    CHECK(!SotnPlayer_SetAfterimageContext(f->p, 17, 1) &&
          !SotnPlayer_SetAfterimageContext(f->p, 18, 2) &&
          !SotnPlayer_SetAfterimageContext(f->p, 0, 2) &&
          !SDL_memcmp(&before, f->p, sizeof(before)));
    CHECK(Tick(f, 2, SOTN_BUTTON_RIGHT) && Tick(f, 3, SOTN_BUTTON_RIGHT) &&
          Tick(f, 4, SOTN_BUTTON_RIGHT));
    CHECK(f->p->afterimage.samples[1].x == 68 && f->p->afterimage.write_index == 2);
    const SotnPlayerAfterimage prior = f->p->afterimage;
    CHECK(SotnPlayer_SetAfterimageContext(f->p, 17, 15) &&
          !SDL_memcmp(&prior, &f->p->afterimage, sizeof(prior)));
    CHECK(SotnPlayer_Tick(f->p, SOTN_BUTTON_RIGHT) && f->p->afterimage.pose == 1);
    CHECK(SotnPlayer_ResetAfterimages(f->p, 1) && f->p->afterimage_timer15 == 4);
    for (Uint16 i = 0; i < 3; ++i)
        CHECK(!f->p->afterimage.displays[i].sprite);
    for (Uint32 t = 16; t <= 18; ++t)
        CHECK(Tick(f, t, SOTN_BUTTON_RIGHT) && f->p->afterimage.disabled);
    CHECK(Tick(f, 19, SOTN_BUTTON_LEFT) && !f->p->afterimage.disabled &&
          f->p->afterimage_timer15 == 0);
    const SotnPlayerAfterimage a = f->p->afterimage;
    CHECK(SotnPlayer_ChangeRoom(f->p, f->map, 16, 0));
    CHECK(f->p->afterimage.samples[0].x == a.samples[0].x + 16 && f->p->afterimage.pose == a.pose &&
          f->p->afterimage.timer == a.timer);
    CHECK(SotnPlayer_ChangeRoom(f->p, f->map, -16, 0) &&
          !SDL_memcmp(&a, &f->p->afterimage, sizeof(a)));
    const SotnPlayer room = *f->p;
    CHECK(!SotnPlayer_ChangeRoom(f->p, f->map, SDL_MAX_SINT32, 0) &&
          !SDL_memcmp(&room, f->p, sizeof(room)));
    CHECK(Tick(f, SDL_MAX_UINT32, 0) && Tick(f, 0, 0));
    SotnPlayer_Close(f->p);
    return true;
}
static bool StatesAndFailure(Fixture *f)
{
    CHECK(Create(f));
    CHECK(Tick(f, 1, SOTN_BUTTON_JUMP));
    CHECK(f->p->afterimage.samples[0].y == 135 && f->p->y.value < 135 * 65536);
    const SotnPlayerStep steps[] = {
        SOTN_PLAYER_HIT,          SOTN_PLAYER_STONE,       SOTN_PLAYER_KILL,
        SOTN_PLAYER_HIGH_JUMP,    SOTN_PLAYER_BAT,         SOTN_PLAYER_MIST,
        SOTN_PLAYER_WOLF,         SOTN_PLAYER_UNMORPH_BAT, SOTN_PLAYER_UNMORPH_MIST,
        SOTN_PLAYER_UNMORPH_WOLF, SOTN_PLAYER_BOSS_GRAB,   SOTN_PLAYER_STUCK,
        SOTN_PLAYER_SCRIPT};
    for (Uint16 i = 0; i < SDL_arraysize(steps); ++i)
    {
        f->p->step = steps[i];
        f->p->afterimage.displays[0].sprite = 1;
        f->p->afterimage.samples[0].sprite = 1;
        CHECK(SotnPlayer_AfterimageCapture(f->p) && f->p->afterimage.disabled &&
              f->p->afterimage_timer15 == 4 && !f->p->afterimage.displays[0].sprite &&
              !f->p->afterimage.samples[0].sprite);
    }
    f->p->step = SOTN_PLAYER_STAND;
    f->p->substep = 4;
    CHECK(SotnPlayer_AfterimageCapture(f->p) && f->p->afterimage.disabled);
    f->p->substep = 0;
    f->p->step = SOTN_PLAYER_JUMP;
    f->p->previous_step = SOTN_PLAYER_JUMP;
    f->p->poison_ticks = 1;
    CHECK(SotnPlayer_AfterimageCapture(f->p) && f->p->afterimage.disabled);
    f->p->poison_ticks = 0;
    f->p->curse_ticks = 1;
    CHECK(SotnPlayer_AfterimageCapture(f->p) && f->p->afterimage.disabled);
    f->p->curse_ticks = 0;
    f->p->poison_ticks = 1;
    CHECK(Tick(f, 2, 0) && !f->p->poison_ticks && f->p->afterimage.disabled &&
          f->p->afterimage_timer15 == 11);
    CHECK(SotnPlayer_ResetAfterimages(f->p, 65536) && !f->p->afterimage_timer15);
    CHECK(SotnPlayer_SetAfterimageContext(f->p, 17, 3));
    Put32(f->dra + 0x101b8 + f->p->animation.id * 4U, 0x800b1000);
    const SotnPlayer before = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, 0) && !SDL_memcmp(&before, f->p, sizeof(before)));
    CHECK(!SotnPlayer_SetAfterimageContext(NULL, 17, 2) && !SotnPlayer_ResetAfterimages(NULL, 1));
    SotnPlayer_Close(f->p);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Movement(f) && StatesAndFailure(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
