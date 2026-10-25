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
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static bool Follow(void)
{
    SotnCamera c = {0};
    CHECK(SotnCamera_Enter(&c, 1024, 512, 512, 350));
    CHECK(c.x == 384 && c.y == 214);
    CHECK(SotnCamera_Tick(&c, 600, 250, false) && c.x == 472 && c.y == 210);
    CHECK(SotnCamera_Tick(&c, 600, 250, false) && c.y == 206);
    CHECK(SotnCamera_Tick(&c, 600, 250, true) && c.y == 114);
    CHECK(SotnCamera_Tick(&c, 20, 30, false) && c.x == 0 && c.y == 110);
    for (Uint32 i = 0; i < 30; ++i)
        CHECK(SotnCamera_Tick(&c, 20, 30, false));
    CHECK(c.y == 4);
    c.y = SDL_MIN_SINT32;
    CHECK(!SotnCamera_Tick(&c, 20, 30, false));
    c.y = 4;
    CHECK(SotnCamera_Tick(&c, 1000, 490, false) && c.x == 768 && c.y == 260);
    const SotnCamera before = c;
    CHECK(!SotnCamera_Bounds(&c, SDL_MAX_SINT32, 0, SDL_MIN_SINT32, 512));
    CHECK(!SotnCamera_Bounds(&c, 0, 0, 1025, 512));
    CHECK(!SotnCamera_Bounds(&c, 0, 0, 255, 512));
    CHECK(c.x == before.x && c.right == before.right);
    CHECK(!SotnCamera_Tick(&c, SDL_MAX_SINT32, 0, false));
    CHECK(c.x == before.x && c.y == before.y);
    CHECK(!SotnCamera_Enter(&c, 512, 0, 0, 0));
    CHECK(c.y == before.y);
    CHECK(SotnCamera_Bounds(&c, 0, 251, 1024, 507));
    CHECK(SotnCamera_Tick(&c, 600, 350, true) && c.y == 255);
    return true;
}
static bool Locks(void)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    CHECK(s != NULL);
    s->stage_id = 12;
    Uint8 nz0[0x1000] = {0}, dra[0x1000] = {0};
    s->nz0 = (SotnDataView){nz0, sizeof(nz0)};
    s->dra = (SotnDataView){dra, sizeof(dra)};
    // Synthetic valid definition and camera properties, with a distinct 512-pixel viewport span.
    dra[0x98] = 0;
    dra[0x99] = 2;
    dra[0x9a] = 10;
    dra[0x9b] = 128;
    nz0[0xdbc] = 32;
    nz0[0xdbd] = 32;
    for (Uint32 i = 0; i < 4; ++i)
    {
        Put16(nz0 + 0xdc4 + i * 8, 0);
        Put16(nz0 + 0xdc6 + i * 8, (Uint16)(i & 1 ? 251 : 0));
        Put16(nz0 + 0xdc8 + i * 8, 1024);
        Put16(nz0 + 0xdca + i * 8, (Uint16)(i & 1 ? 507 : 256));
    }
    s->camera = &s->fallback_camera;
    CHECK(SotnCamera_Enter(s->camera, 1024, 512, 600, 350));
    s->player = (SotnPlayerState){.x = 590 * 65536, .y = 350 * 65536};
    Actor a = {.state = {.id = 0x12, .generation = 1, .x = 600 * 65536, .y = 350 * 65536}};
    CHECK(ActorCamera(s, &a));
    CHECK(a.camera.side == 4 && a.camera.pending != 0);
    for (Uint32 i = 0; i < 15; ++i)
        CHECK(ActorCamera(s, &a));
    CHECK(a.camera.pending == 0 && s->camera->top == 251 && s->camera->bottom == 507);
    s->player.x = 610 * 65536;
    CHECK(ActorCamera(s, &a));
    for (Uint32 i = 0; i < 15; ++i)
        CHECK(ActorCamera(s, &a));
    CHECK(s->camera->top == 0 && s->camera->bottom == 256);
    s->player.x = 800 * 65536;
    CHECK(ActorCamera(s, &a) && a.camera.side == 2);
    a.state.parameters = 3;
    CHECK(!ActorCamera(s, &a));
    SDL_free(s);
    return true;
}
int main(void)
{
    return Follow() && Locks() ? 0 : 1;
}
