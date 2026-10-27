#include <SDL3/SDL.h>
#include <sotn/transformation.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)

static int Gates(void)
{
    const Uint16 relics[] = {SOTN_RELIC_BAT, SOTN_RELIC_MIST, SOTN_RELIC_WOLF};
    for (Uint16 i = 0; i < SDL_arraysize(relics); ++i)
    {
        const SotnTransformationForm form = (SotnTransformationForm)i;
        SotnWorldState world = {0};
        SotnTransformationCharge charge = {0};
        CHECK(SotnTransformation_MP(&world, form, 99, 0, true, &charge) && !charge.allowed &&
              !charge.spent);
        world.relics[relics[i]] = 1;
        CHECK(SotnTransformation_MP(&world, form, 99, 0, true, &charge) && !charge.allowed);
        world.relics[relics[i]] = 3;
        const Uint16 boundary = form == SOTN_FORM_MIST ? 10 : 1;
        for (Uint16 mp = 0; mp <= boundary; ++mp)
        {
            CHECK(SotnTransformation_MP(&world, form, mp, 0, true, &charge) && !charge.allowed &&
                  !charge.spent);
            // An unbilled tick still enforces the same strict eligibility threshold.
            CHECK(SotnTransformation_MP(&world, form, mp, 1, true, &charge) && !charge.allowed);
        }
        CHECK(SotnTransformation_MP(&world, form, boundary + 1, 0, false, &charge) &&
              charge.allowed && !charge.spent);
        SotnWorldState before = {0};
        SDL_memcpy(&before, &world, sizeof(before));
        CHECK(SotnTransformation_MP(&world, form, boundary + 1, 0, true, &charge) &&
              charge.allowed && charge.spent == boundary &&
              SDL_memcmp(&before, &world, sizeof(world)) == 0);
    }
    return 0;
}
static int Cadence(void)
{
    SotnWorldState world = {0};
    world.relics[SOTN_RELIC_BAT] = world.relics[SOTN_RELIC_WOLF] = world.relics[SOTN_RELIC_MIST] =
        3;
    // Independently specified original boundary frames, including 32-bit timer wrap.
    const Uint32 ticks[] = {0, 7, 8, 29, 30, 59, 60, 119, 120, SDL_MAX_UINT32, 0};
    const Uint16 bat[] = {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1};
    const Uint16 wolf[] = {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1};
    const Uint16 mist[] = {10, 0, 10, 0, 0, 0, 0, 0, 10, 0, 10};
    const Uint16 power[] = {2, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2};
    for (Uint16 i = 0; i < SDL_arraysize(ticks); ++i)
    {
        SotnTransformationCharge charge = {0};
        CHECK(SotnTransformation_MP(&world, SOTN_FORM_BAT, 99, ticks[i], true, &charge) &&
              charge.allowed && charge.spent == bat[i]);
        CHECK(SotnTransformation_MP(&world, SOTN_FORM_WOLF, 99, ticks[i], true, &charge) &&
              charge.allowed && charge.spent == wolf[i]);
        world.relics[SOTN_RELIC_POWER_MIST] = 1;
        CHECK(SotnTransformation_MP(&world, SOTN_FORM_MIST, 99, ticks[i], true, &charge) &&
              charge.allowed && charge.spent == mist[i]);
        world.relics[SOTN_RELIC_POWER_MIST] = 3;
        CHECK(SotnTransformation_MP(&world, SOTN_FORM_MIST, 3, ticks[i], true, &charge) &&
              charge.allowed && charge.spent == power[i]);
        CHECK(SotnTransformation_MP(&world, SOTN_FORM_MIST, 2, ticks[i], true, &charge) &&
              !charge.allowed && !charge.spent);
    }
    return 0;
}
int main(void)
{
    CHECK(Gates() == 0 && Cadence() == 0);
    SotnWorldState world = {0};
    SotnTransformationCharge charge = {.allowed = true, .spent = 123};
    CHECK(!SotnTransformation_MP(NULL, SOTN_FORM_BAT, 99, 0, true, &charge));
    CHECK(!SotnTransformation_MP(&world, (SotnTransformationForm)3, 99, 0, true, &charge));
    CHECK(!SotnTransformation_MP(&world, SOTN_FORM_BAT, 99, 0, true, NULL));
    world.relics[SOTN_RELIC_BAT] = 2;
    CHECK(!SotnTransformation_MP(&world, SOTN_FORM_BAT, 99, 0, true, &charge) && charge.allowed &&
          charge.spent == 123);
    return 0;
}
