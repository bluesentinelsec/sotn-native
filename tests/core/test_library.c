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
        p[i] = (Uint8)(value >> (i * 8U));
}
static void Reset(SotnActors *s)
{
    SDL_zero(s->slots);
    SDL_zero(s->combat);
    SDL_zero(s->fallback_world);
    s->slots[64] = (Actor){.state = {.id = 1, .generation = 1, .x = 128 * 65536, .y = 128 * 65536},
                           .slot = 64};
}
static bool Breakables(SotnActors *s, Uint8 *image)
{
    for (Uint16 type = 0; type < 10; ++type)
    {
        Put32(image + 0xba8 + type * 4U, 0x80183000);
        image[0xbd0 + type] = (Uint8)(8 + type);
        image[0xbdc + type] = 0;
        Put16(image + 0xbe8 + type * 2U, (Uint16)(0x200 + type));
        Put16(image + 0xbfc + type * 2U, 0x8001);
        image[0xc10 + type] = 0x30;
        Put16(image + 0xc1c + type * 2U, (Uint16)(0x50 + type));
        Put16(image + 0xc30 + type * 2U, (Uint16)(-(Sint32)type));
    }
    image[0x3000] = image[0x3002] = 1;
    image[0x3001] = image[0x3003] = 5;
    image[0xb58] = image[0xb60] = 1;
    image[0xb59] = image[0xb61] = 11;
    for (Uint16 type = 0; type < 10; ++type)
    {
        Reset(s);
        Actor *sample = &s->slots[64];
        sample->state.parameters = (Uint16)(type << 12);
        CHECK(ActorLibraryBreakable(s, sample) && sample->state.frame == 5 &&
              sample->state.palette == 0x200 + type &&
              sample->state.texture_offset == 0x50 + type &&
              sample->state.hitbox_height == 8 + type && sample->state.hitbox_y == -(Sint16)type);
        CHECK((s->slots[65].state.generation != 0) == (type < 5));
    }
    Reset(s);
    Actor *a = &s->slots[64];
    CHECK(ActorLibraryBreakable(s, a) && a->state.step == 1 && a->state.frame == 5 &&
          a->state.priority == 128 && a->state.hitbox_height == 8 &&
          a->state.texture_offset == 0x50);
    Actor *helper = &s->slots[65];
    CHECK(helper->state.id == 0x3f && helper->state.parameters == 0 &&
          helper->state.priority == 128);
    CHECK(ActorLibraryBreakable(s, helper) && helper->state.frame == 11 &&
          helper->state.blend_mode == 0x30);
    a->hit_params = 1;
    CHECK(ActorLibraryBreakable(s, a) && a->remove && a->permanent && a->state.id == 1);
    CHECK(s->combat.sound_count == 1 && s->combat.sounds[0] == 0x61c &&
          s->combat.volumes[0] == 127 && s->combat.pans[0] == 0);
    a->state.id = 0;
    CHECK(ActorLibraryBreakable(s, helper) && helper->state.frame == 13);
    Reset(s);
    a = &s->slots[64];
    s->world->relics[SOTN_RELIC_CUBE] = 3;
    a->state.x = 320 * 65536;
    a->state.parameters = 0x2003;
    CHECK(ActorLibraryBreakable(s, a) && a->state.palette == 0x202 && a->state.hitbox_y == -2);
    a->hit_params = 1;
    CHECK(ActorLibraryBreakable(s, a) && a->state.id == 3 && a->state.parameters == 3 &&
          a->state.step == 0 && a->state.frame == 0 && a->timer == 16 && a->dynamic &&
          a->permanent && !a->remove);
    CHECK(s->combat.sound_count == 1 && s->combat.sounds[0] == 0x61d &&
          s->combat.volumes[0] == 79 && s->combat.pans[0] == 5);
    Reset(s);
    a = &s->slots[64];
    s->world->relics[SOTN_RELIC_CUBE] = 3;
    a->state.parameters = 0x1000 + 128 + 18;
    CHECK(ActorLibraryBreakable(s, a));
    a->hit_params = 1;
    CHECK(ActorLibraryBreakable(s, a) && a->state.id == 0xa && a->state.parameters == 18);
    CHECK(s->combat.sounds[0] == 0x634 && s->combat.volumes[0] == SDL_MAX_UINT16);
    Reset(s);
    a = &s->slots[64];
    a->state.parameters = 0x9004;
    CHECK(ActorLibraryBreakable(s, a));
    a->hit_params = 1;
    CHECK(ActorLibraryBreakable(s, a) && a->remove && a->permanent);
    CHECK(s->slots[96].state.id == 0x47 && s->slots[96].state.parameters == 256 &&
          s->slots[97].state.id == 0xc && s->slots[97].state.parameters == 4);
    CHECK(ActorLibraryBreakable(s, &s->slots[96]) && s->slots[96].state.step == 256 &&
          s->slots[96].state.animation_set == 0x800c && s->slots[96].state.frame == 21 &&
          s->slots[96].state.palette == 0x26c && s->slots[96].state.texture_offset == 91);
    CHECK(ActorLibraryBreakable(s, &s->slots[96]) && s->slots[96].state.frame == 21);
    Reset(s);
    a = &s->slots[64];
    a->state.parameters = 0x8009;
    CHECK(ActorLibraryBreakable(s, a));
    a->hit_params = 1;
    CHECK(ActorLibraryBreakable(s, a) && s->slots[96].state.id == 0xc &&
          s->slots[96].state.parameters == 3);
    Reset(s);
    a = &s->slots[64];
    a->state.parameters = 0x9004;
    CHECK(ActorLibraryBreakable(s, a));
    a->hit_params = 1;
    for (Uint16 i = 96; i < 128; ++i)
        s->slots[i].state.generation = 1;
    CHECK(ActorLibraryBreakable(s, a) && a->remove && a->permanent);
    Reset(s);
    a = &s->slots[64];
    a->state.parameters = 0xa000;
    CHECK(!ActorLibraryBreakable(s, a) && a->state.step == 0);
    a->state.parameters = 0x900b;
    CHECK(!ActorLibraryBreakable(s, a) && a->state.step == 0);
    a->state.parameters = 0;
    Put32(image + 0xba8, 0x80183001);
    CHECK(!ActorLibraryBreakable(s, a) && a->state.step == 0);
    CHECK(!ActorLibraryBreakable(NULL, a) && !ActorLibraryBreakable(s, NULL));
    Actor bad_helper = {.state = {.id = 0x3f, .generation = 99}};
    CHECK(!ActorLibraryBreakable(s, &bad_helper));
    bad_helper.slot = 1;
    bad_helper.state.parameters = 5;
    CHECK(!ActorLibraryBreakable(s, &bad_helper));
    return true;
}
int main(void)
{
    Uint8 *image = SDL_calloc(1, 0x4000), dra[0x400] = {0};
    SotnActors *s = NULL;
    Put32(dra + 0x98, 0x800a0200);
    const bool passed = image &&
                        SotnActors_CreateStage((SotnDataView){dra, sizeof(dra)},
                                               (SotnDataView){image, 0x4000}, 2, &s) &&
                        Breakables(s, image);
    SotnActors_Destroy(s);
    SDL_free(image);
    return passed ? 0 : 1;
}
