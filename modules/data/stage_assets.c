#include "data_internal.h"
#include <sotn/stage_assets.h>
typedef struct StageSpec
{
    SotnStageIdentity identity;
    SotnDataSpec overlay, graphics;
} StageSpec;
#include "stage_reference.h"
struct SotnStageAssets
{
    SotnResource *overlay, *graphics;
};
Uint32 SotnStageAssets_Count(void)
{
    return (Uint32)SDL_arraysize(reference_stages);
}
bool SotnStageAssets_Identity(Uint32 index, SotnStageIdentity *out)
{
    if (out == NULL || index >= SotnStageAssets_Count())
        return SDL_SetError("Stage catalog index is outside its verified identities");
    *out = reference_stages[index].identity;
    return true;
}
bool SotnStageAssets_Find(const char *code, SotnStageIdentity *out)
{
    if (code == NULL || out == NULL)
        return SDL_SetError("Stage lookup requires a canonical code and output");
    for (Uint32 i = 0; i < SotnStageAssets_Count(); ++i)
        if (SDL_strcasecmp(code, reference_stages[i].identity.code) == 0)
            return SotnStageAssets_Identity(i, out);
    return SDL_SetError("Unknown canonical US stage code '%s'", code);
}
bool SotnStageAssets_Open(const char *directory, Uint16 id, SotnStageAssets **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    const StageSpec *spec = NULL;
    for (Uint32 i = 0; i < SotnStageAssets_Count(); ++i)
        if (reference_stages[i].identity.id == id)
            spec = &reference_stages[i];
    if (spec == NULL)
        return SDL_SetError("US stage ID %u has no canonical verified assets", id);
    return SotnData_OpenStageSpecs(directory, &spec->overlay, &spec->graphics, out);
}
bool SotnData_OpenStageSpecs(const char *directory, const SotnDataSpec *overlay,
                             const SotnDataSpec *graphics, SotnStageAssets **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    SotnStageAssets *assets = SDL_calloc(1, sizeof(*assets));
    if (assets == NULL)
        return false;
    if (!SotnData_OpenResourceSpec(directory, overlay, &assets->overlay) ||
        !SotnData_OpenResourceSpec(directory, graphics, &assets->graphics))
    {
        SotnStageAssets_Close(assets);
        return false;
    }
    *out = assets;
    return true;
}
SotnDataView SotnStageAssets_Overlay(const SotnStageAssets *assets)
{
    return SotnResource_View(assets != NULL ? assets->overlay : NULL);
}
SotnDataView SotnStageAssets_Graphics(const SotnStageAssets *assets)
{
    return SotnResource_View(assets != NULL ? assets->graphics : NULL);
}
void SotnStageAssets_Close(SotnStageAssets *assets)
{
    if (assets != NULL)
    {
        SotnResource_Close(assets->overlay);
        SotnResource_Close(assets->graphics);
        SDL_free(assets);
    }
}
