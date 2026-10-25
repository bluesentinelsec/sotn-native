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
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static bool Run(void)
{
    CHECK(SotnStageProfile_Get(0) != NULL && SotnStageProfile_Get(1) != NULL);
    CHECK(SotnStageProfile_Get(2) == NULL);
    CHECK(SotnStageProfile_SaveRoom(0, 31) && SotnStageProfile_SaveRoom(0, 32));
    CHECK(SotnStageProfile_SaveRoom(1, 14) && SotnStageProfile_SaveRoom(1, 17));
    CHECK(!SotnStageProfile_SaveRoom(0, SDL_MAX_UINT16));
    CHECK(!SotnStageProfile_SaveRoom(1, 15) && !SotnStageProfile_SaveRoom(1, 16));
    CHECK(!SotnStageProfile_SaveRoom(2, 14));
    Uint8 dra[0x400] = {0}, stage[0x4000] = {0};
    Put32(dra + 0x98, 0x800a0200);
    const Uint16 ids[] = {0, 1};
    const size_t initializers[] = {0xc38, 0xa58}, velocities[] = {0x272c, 0x27d8};
    for (Uint32 i = 0; i < 2; ++i)
    {
        SotnActors *s = NULL;
        CHECK(SotnActors_CreateStage((SotnDataView){dra, sizeof(dra)},
                                     (SotnDataView){stage, sizeof(stage)}, ids[i], &s));
        Put16(stage + initializers[i], 0x8001);
        Actor a = {
            .state = {
                .id = i == 0 ? 0x4d : 0x3e, .generation = 1, .x = 100 * 65536, .y = 100 * 65536}};
        CHECK(ActorEarlyCastle(s, &a) && a.state.step == 1 && !a.state.unsupported);
        CHECK(a.state.id == (i == 0 ? 0x4d : 0x3e));
        Put32(stage + velocities[i] + 2 * 4, (Uint32)-65536);
        a = (Actor){
            .state = {
                .id = i == 0 ? 0x4e : 0x3f, .generation = 2, .x = 100 * 65536, .y = 100 * 65536}};
        s->player.x = 164 * 65536;
        CHECK(ActorEarlyCastle(s, &a) && a.vx == -65536 && a.vy == -294912);
        Actor *child = NULL;
        CHECK(ActorSpawn(s, &a, ActorStageId(s, 0x30), 0, 160, 192, &child));
        CHECK(child != NULL && child->state.id == (i == 0 ? 0x4f : 0x40));
        CHECK(!ActorSpawn(s, &a, SotnStageProfile_Get(ids[i])->actor_limit, 0, 160, 192, &child));
        const size_t bad = ActorStageOffset(s, 0x5000);
        SotnDataView view = {0};
        CHECK(!Slice(s->nz0, bad + 16, 2, &view));
        SotnActors_Destroy(s);
    }
    return true;
}
static bool EntranceEntries(void)
{
    const Uint32 before[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 18, 19, 20, 24, 25, 26};
    for (Uint32 i = 0; i < SDL_arraysize(before); ++i)
        CHECK(!SotnStageProfile_EntranceAfterDeath(before[i]));
    const Uint32 after[] = {12, 13, 14, 15, 16, 17, 21, 22, 23, 27};
    for (Uint32 i = 0; i < SDL_arraysize(after); ++i)
        CHECK(SotnStageProfile_EntranceAfterDeath(after[i]));
    CHECK(!SotnStageProfile_EntranceAfterDeath(SDL_MAX_UINT32));
    return true;
}

int main(void)
{
    if (!Run() || !EntranceEntries())
        return 1;
    SDL_Log("EARLY_CASTLE_PROFILES: PASS");
    return 0;
}
