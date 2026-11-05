#include <SDL3/SDL.h>
#include <sotn/actor_graphics.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d: %s: %s", __LINE__, #v, SDL_GetError());                               \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
int main(void)
{
    Uint8 *dra = SDL_calloc(1, 0xe140), *stage = SDL_malloc(0x40000),
          *common = SDL_calloc(1, 0x42000);
    Uint8 overlay[0x80] = {0};
    CHECK(dra && stage && common);
    SDL_memset(stage, 0xba, 0x40000);
    Put32(overlay + 24, 0x80180040);
    Put32(overlay + 0x40, 0x80180060);
    Put32(overlay + 0x60, 5);
    Put32(overlay + 0x64, SDL_MAX_UINT32);
    Put16(dra + 0xe130, 600);
    Put16(dra + 0xe134, 32);
    Put16(dra + 0xe136, 80);
    Put16(dra + 0xe138, 632);
    Put16(dra + 0xe13c, 32);
    Put16(dra + 0xe13e, 80);
    SotnActorGraphics *g = NULL;
    CHECK(SotnActorGraphics_Create((SotnDataView){dra, 0xe140},
                                   (SotnDataView){overlay, sizeof(overlay)},
                                   (SotnTileSources){{stage, 0x40000}, {common, 0x42000}}, &g));
    SotnDeathImage image = {0}, check = {0};
    CHECK(SotnActorGraphics_StoreDeathImage(g, &image) && image.captured && image.remaining == 64);
    for (size_t i = 0; i < sizeof(image.pixels); ++i)
        CHECK(image.pixels[i] == 0xba);
    const SotnDeathImage original = image;
    SotnDissolveTextures *snapshot = SDL_malloc(sizeof(*snapshot));
    CHECK(snapshot && SotnActorGraphics_SaveDissolve(g, snapshot));
    SotnDeathImage changed = original;
    SDL_memset(changed.pixels, 0x56, sizeof(changed.pixels));
    CHECK(SotnActorGraphics_LoadDissolve(g, &changed, true) &&
          SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(check.pixels, original.pixels, sizeof(check.pixels)));
    Put16(dra + 0xe130, 632);
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(check.pixels, changed.pixels, sizeof(check.pixels)));
    Put16(dra + 0xe130, 600);
    CHECK(SotnActorGraphics_LoadDissolve(g, &changed, false) &&
          SotnActorGraphics_RestoreDissolve(g, snapshot) &&
          SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(check.pixels, original.pixels, sizeof(check.pixels)));
    Put16(dra + 0xe130, 632);
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(check.pixels, original.pixels, sizeof(check.pixels)));
    Put16(dra + 0xe130, 0);
    Put16(dra + 0xe138, 32);
    CHECK(SotnActorGraphics_SaveDissolve(g, snapshot) &&
          SotnActorGraphics_LoadDissolve(g, &changed, true));
    Put16(dra + 0xe130, 32);
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check));
    Put16(dra + 0xe130, 0);
    CHECK(SotnActorGraphics_RestoreDissolve(g, snapshot));
    Put16(dra + 0xe130, 32);
    const SotnDeathImage held = check;
    CHECK(!SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(&held, &check, sizeof(check)));
    Put16(dra + 0xe130, 600);
    Put16(dra + 0xe138, 632);
    CHECK(SotnActorGraphics_SaveDissolve(g, snapshot));
    Put16(dra + 0xe13c, 31);
    CHECK(!SotnActorGraphics_LoadDissolve(g, &changed, true) &&
          !SotnActorGraphics_RestoreDissolve(g, snapshot));
    Put16(dra + 0xe13c, 32);
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(check.pixels, original.pixels, sizeof(check.pixels)));
    SDL_free(snapshot);
    CHECK(SotnDeathImage_Tick(&image) && SotnActorGraphics_LoadDeathImage(g, &image) &&
          SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(image.pixels, check.pixels, sizeof(image.pixels)));
    Put16(dra + 0xe130, 632);
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(original.pixels, check.pixels, sizeof(check.pixels)));
    Put16(dra + 0xe130, 600);
    const SotnDeathImage before = image;
    Put16(dra + 0xe130, 1000);
    CHECK(!SotnActorGraphics_StoreDeathImage(g, &image) &&
          !SotnActorGraphics_LoadDeathImage(g, &image) &&
          !SDL_memcmp(&image, &before, sizeof(image)));
    Put16(dra + 0xe130, 0);
    CHECK(!SotnActorGraphics_StoreDeathImage(g, &image) &&
          !SDL_memcmp(&image, &before, sizeof(image)));
    Put16(dra + 0xe130, 600);
    Put16(dra + 0xe136, 79);
    CHECK(!SotnActorGraphics_StoreDeathImage(g, &image) &&
          !SotnActorGraphics_LoadDeathImage(g, &image));
    Put16(dra + 0xe136, 80);
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check) &&
          !SDL_memcmp(before.pixels, check.pixels, sizeof(check.pixels)));
    image.captured = false;
    CHECK(!SotnActorGraphics_LoadDeathImage(g, &image));
    image = before;
    while (image.remaining)
        CHECK(SotnDeathImage_Tick(&image) && SotnActorGraphics_LoadDeathImage(g, &image));
    CHECK(SotnActorGraphics_StoreDeathImage(g, &check));
    for (size_t i = 0; i < sizeof(check.pixels); ++i)
        CHECK(!check.pixels[i]);
    for (size_t i = 0; i < 0x40000; ++i)
        CHECK(stage[i] == 0xba);
    SotnActorGraphics_Destroy(g);
    SDL_free(dra);
    SDL_free(stage);
    SDL_free(common);
    SDL_Log("DEATH_CAPTURE: PASS actual packed texture store/load/decay, unrelated words/source "
            "immutability, missing residency and malformed rectangle rollback");
    return 0;
}
