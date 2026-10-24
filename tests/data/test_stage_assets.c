#include "data_internal.h"
#include <SDL3/SDL.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(int argc, char **argv)
{
    CHECK(argc == 2);
    CHECK(SotnStageAssets_Count() == 50);
    bool seen[256] = {0};
    for (Uint32 i = 0; i < SotnStageAssets_Count(); ++i)
    {
        SotnStageIdentity identity = {0}, found = {0};
        CHECK(SotnStageAssets_Identity(i, &identity));
        CHECK(identity.id < 256 && !seen[identity.id]);
        seen[identity.id] = true;
        CHECK(SotnStageAssets_Find(identity.code, &found) && found.id == identity.id);
    }
    SotnStageIdentity identity = {0};
    CHECK(SotnStageAssets_Find("nz0", &identity) && identity.id == 12);
    CHECK(SotnStageAssets_Find("NO3", &identity) && identity.id == 65);
    CHECK(SotnStageAssets_Find("NP3", &identity) && identity.id == 7);
    CHECK(SotnStageAssets_Find("RNO0", &identity) && identity.id == 32);
    CHECK(SotnStageAssets_Find("BO6", &identity) && identity.id == 24);
    identity.id = 99;
    CHECK(!SotnStageAssets_Find("../NZ0", &identity) && identity.id == 99);
    CHECK(!SotnStageAssets_Find(NULL, &identity));
    CHECK(!SotnStageAssets_Identity(50, &identity) && identity.id == 99);
    CHECK(!SotnStageAssets_Identity(0, NULL));
    SotnStageAssets *assets = NULL;
    CHECK(!SotnStageAssets_Open(".", 255, &assets) && assets == NULL);
    CHECK(!SotnStageAssets_Open("missing-stage-directory", 12, &assets) && assets == NULL);
    CHECK(!SotnStageAssets_Open(".", 12, NULL));
    CHECK(SotnStageAssets_Overlay(NULL).bytes == NULL);
    CHECK(SotnStageAssets_Graphics(NULL).size == 0);
    char *overlay_path = NULL, *art_path = NULL;
    CHECK(SDL_asprintf(&overlay_path, "%s/stage-overlay.txt", argv[1]) >= 0);
    CHECK(SDL_asprintf(&art_path, "%s/stage-art.txt", argv[1]) >= 0);
    const SotnDataSpec overlay = {"stage-overlay.txt", 3,
                                  "a9993e364706816aba3e25717850c26c9cd0d89d"};
    const SotnDataSpec graphics = {"stage-art.txt", 3, "a9993e364706816aba3e25717850c26c9cd0d89d"};
    CHECK(SDL_SaveFile(overlay_path, "abc", 3) && SDL_SaveFile(art_path, "abc", 3));
    CHECK(SotnData_OpenStageSpecs(argv[1], &overlay, &graphics, &assets));
    CHECK(SotnStageAssets_Overlay(assets).size == 3 &&
          SotnStageAssets_Graphics(assets).bytes[2] == 'c');
    CHECK(SDL_SaveFile(art_path, "bad", 3));
    CHECK(SotnStageAssets_Graphics(assets).bytes[0] == 'a');
    SotnStageAssets_Close(assets);
    assets = NULL;
    CHECK(!SotnData_OpenStageSpecs(argv[1], &overlay, &graphics, &assets) && assets == NULL);
    CHECK(SDL_RemovePath(art_path));
    CHECK(!SotnData_OpenStageSpecs(argv[1], &overlay, &graphics, &assets) && assets == NULL);
    CHECK(SDL_RemovePath(overlay_path));
    SDL_free(overlay_path);
    SDL_free(art_path);
    SotnStageAssets_Close(NULL);
    SDL_Log("STAGE_CATALOG: PASS");
    return 0;
}
