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
    Uint8 dra[0x11000], tiles[64 * 16 * 2], types[2];
    SotnPlayer *p;
    SotnCollisionMap map;
    Uint32 clock;
} Fixture;
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (8U * i));
}
static bool Create(Fixture *f)
{
    SotnPlayer_Close(f->p);
    SDL_memset(f, 0, sizeof(*f));
    for (Uint16 i = 0; i < 247; ++i)
        Put32(f->dra + 0x101b8 + i * 4U, 0x800a0100);
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put32(f->dra + 0x100 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
        Put32(f->dra + 0x200 + i * 4U, 2U | ((Uint32)i + 1U) << 16);
    }
    Put32(f->dra + 0x120, 0);
    Put32(f->dra + 0x220, 65535);
    Put32(f->dra + 0x101b8 + 0xdb * 4U, 0x800a0200);
    SDL_memset(f->dra + 0xcf18, 3, 20);
    SDL_memset(f->dra + 0xcf2c, 5, 16);
    SDL_memset(f->dra + 0xcf3c, 28, 16);
    f->types[1] = 3;
    for (Uint16 y = 10; y < 16; ++y)
        for (Uint16 x = 0; x < 64; ++x)
            f->tiles[(y * 64U + x) * 2U] = 1;
    f->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, 2}, 64, 16};
    CHECK(SotnPlayer_Create((SotnDataView){f->dra, sizeof(f->dra)}, f->map, 256, 135, &f->p));
    return true;
}
static bool Tick(Fixture *f, Uint16 b)
{
    CHECK(SotnPlayer_SetAfterimageContext(f->p, 17, ++f->clock) && SotnPlayer_Tick(f->p, b));
    return true;
}
static bool Start(Fixture *f)
{
    CHECK(Tick(f, SOTN_BUTTON_CANCEL));
    CHECK(f->p->step == SOTN_PLAYER_STAND && f->p->substep == 3 && f->p->animation.id == 0xdb &&
          f->p->sound == 0x617 && f->p->movement_effects.smoke &&
          !f->p->movement_effects.smoke_blueprint);
    return true;
}
static bool Motion(Fixture *f)
{
    for (Uint16 face = 0; face < 2; ++face)
    {
        CHECK(Create(f));
        if (face)
            CHECK(Tick(f, SOTN_BUTTON_LEFT) && Tick(f, 0));
        Sint32 x = f->p->x.value;
        CHECK(Start(f));
        SotnPlayerState s = {0};
        CHECK(SotnPlayer_GetState(f->p, &s));
        CHECK(s.velocity_x == (face ? 229376 : -229376) && s.x == x + s.velocity_x &&
              !s.velocity_y && s.mp == 20 && !s.mp_spent && s.facing_left == (face != 0) &&
              s.movement_effects.smoke_source.x == x &&
              s.movement_effects.smoke_source.y == 135 * 65536 && s.afterimage.samples[0].sprite &&
              !s.afterimage.disabled);
        const SotnPlayerMovementEffects before = s.movement_effects;
        const SotnAnimation animation = s.animation;
        CHECK(SotnPlayer_ChangeRoom(f->p, f->map, 16, 0) &&
              f->p->movement_effects.smoke_source.x == x + 16 * 65536 &&
              f->p->animation.pose == animation.pose);
        CHECK(SotnPlayer_ChangeRoom(f->p, f->map, -16, 0) &&
              !SDL_memcmp(&before, &f->p->movement_effects, sizeof(before)));
        Uint16 births = 1;
        for (Uint16 t = 0; t < 35; ++t)
        {
            CHECK(Tick(f, SOTN_BUTTON_CANCEL));
            births += f->p->movement_effects.smoke;
        }
        CHECK(births == 1 && f->p->substep == 0 && f->p->animation.id != 0xdb);
        CHECK(Tick(f, 0) && Start(f));
    }
    return true;
}
static bool Gates(Fixture *f)
{
    CHECK(Create(f) && Start(f));
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && f->p->substep == 3 && f->p->vx == -221184);
    f->p->animation.pose = 4;
    CHECK(Tick(f, SOTN_BUTTON_RIGHT) && f->p->step == SOTN_PLAYER_WALK && f->p->vx == 98304 &&
          f->p->movement_effects.smoke && f->p->movement_effects.smoke_blueprint == 0x50001);
    CHECK(Create(f) && Start(f) && Tick(f, SOTN_BUTTON_JUMP) && f->p->step == SOTN_PLAYER_JUMP &&
          !f->p->movement_effects.smoke);
    CHECK(Create(f) && Start(f) && Tick(f, SOTN_BUTTON_DOWN) && f->p->step == SOTN_PLAYER_CROUCH);
    CHECK(Create(f) && Tick(f, SOTN_BUTTON_JUMP | SOTN_BUTTON_CANCEL) &&
          f->p->step == SOTN_PLAYER_JUMP && !f->p->movement_effects.smoke);
    CHECK(Tick(f, SOTN_BUTTON_CANCEL) && f->p->step == SOTN_PLAYER_JUMP);
    CHECK(Create(f) && Tick(f, SOTN_BUTTON_DOWN | SOTN_BUTTON_CANCEL) &&
          f->p->step == SOTN_PLAYER_CROUCH && !f->p->movement_effects.smoke);
    CHECK(Create(f));
    f->p->step = SOTN_PLAYER_JUMP;
    f->p->substep = 1;
    f->p->vy = 7 * 65536;
    // An ordinary heavy Jump landing uses the sliding recovery, like the original controller.
    CHECK(Tick(f, 0) && f->p->step == SOTN_PLAYER_CROUCH && f->p->substep == 3 &&
          f->p->movement_effects.smoke && !f->p->movement_effects.smoke_blueprint &&
          f->p->sound == 0x647 && f->p->movement_effects.smoke_source.y == 135 * 65536);
    CHECK(Create(f));
    f->p->previous_buttons = SOTN_BUTTON_CANCEL;
    f->p->attack_mode = 1;
    CHECK(Start(f) && !f->p->attack_mode);
    CHECK(Create(f));
    Put32(f->dra + 0x101b8 + 0xdb * 4U, 0x800b1000);
    const SotnPlayer before = *f->p;
    CHECK(SotnPlayer_SetAfterimageContext(f->p, 17, ++f->clock));
    const SotnPlayer ready = *f->p;
    CHECK(!SotnPlayer_Tick(f->p, SOTN_BUTTON_CANCEL) && !SDL_memcmp(&ready, f->p, sizeof(ready)));
    CHECK(!f->p->movement_effects.smoke && f->p->mp == before.mp);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    bool ok = Motion(f) && Gates(f);
    SotnPlayer_Close(f->p);
    SDL_free(f);
    return ok ? 0 : 1;
}
