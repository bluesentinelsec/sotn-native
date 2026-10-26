#include "giant_knights_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d %s: %s", __LINE__, #v, SDL_GetError());                               \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x400], stage[0x1000], exe[0x23220], tiles[32 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Sint32 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)((Uint32)v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(f->exe + 0x1dbcc + i * 2U, (Sint32)i * 4);
        Put16(f->exe + 0x22a10 + i * 2U, (Sint32)i / 2);
    }
    // Authored SDK fixture; intermediate square-root entries deliberately stay biased.
    for (Uint32 i = 0; i < 192; ++i)
        Put16(f->exe + 0x1e3e0 + i * 2U, 4096);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 4, out));
    SotnActors *s = *out;
    s->executable = (SotnDataView){f->exe, sizeof(f->exe)};
    s->map = (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    f->types[1] = 3;
    for (Uint32 i = 32 * 12; i < 32 * 16; ++i)
        Put16(f->tiles + i * 2U, 1);
    Actor *root = &s->slots[32];
    *root = (Actor){
        .slot = 32,
        .dynamic = true,
        .state = {.id = 0x37, .generation = 1, .x = 100 * 65536 + 77, .y = 100 * 65536 + 99}};
    for (Uint16 i = 1; i < 20; ++i)
    {
        Actor *part = NULL;
        CHECK(ActorOwnedSpawnAt(s, root, 0x38, i, 32U + i, &part));
        root->giant.generation[i] = part->state.generation;
        part->giant.parent = 0;
        part->giant.length = 8;
    }
    Put16(f->stage + 0x800, 1);
    Put16(f->stage + 0x802, 255);
    Put16(f->stage + 0x804, 2);
    Put16(f->stage + 0x880, 2);
    Put32(f->stage + 0x884, 0x80180900);
    Put16(f->stage + 0x888, 1);
    Put32(f->stage + 0x88c, 0x80180906);
    Put16(f->stage + 0x900, 128);
    Put16(f->stage + 0x902, 777);
    Put16(f->stage + 0x904, -128);
    Put16(f->stage + 0x906, -128);
    Put16(f->stage + 0x908, 999);
    Put16(f->stage + 0x90a, 128);
    return true;
}
static bool Placement(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = &s->slots[33], *resolved = NULL;
    a->giant.length = 10;
    CHECK(GiantPart(s, root, 0, &resolved) && resolved == root);
    CHECK(GiantPart(s, root, 1, &resolved) && resolved == a);
    CHECK(GiantPlace(s, root, a, false) && a->state.x == root->state.x &&
          a->state.y == 110 * 65536 + 99);
    a->giant.rotation = 1024;
    CHECK(GiantPlace(s, root, a, false) && a->state.x == 90 * 65536 + 77 &&
          a->state.y == root->state.y);
    root->state.facing_left = true;
    CHECK(GiantPlace(s, root, a, false) && a->state.x == 110 * 65536 + 77);
    root->state.facing_left = false;
    a->state.x = 130 * 65536 + 456;
    a->state.y = 120 * 65536 + 123;
    CHECK(GiantPlace(s, root, a, true) && root->state.x == 140 * 65536 + 456 &&
          root->state.y == a->state.y);
    // Behaviors operate on a copy; geometry must update that copy's root rather than the old slot.
    Actor next = *root;
    next.state.x = 300 * 65536;
    CHECK(GiantPlace(s, &next, a, false) && a->state.x == 290 * 65536);
    a->state.x = 320 * 65536;
    CHECK(GiantPlace(s, &next, a, true) && next.state.x == 330 * 65536 &&
          root->state.x == 140 * 65536 + 456);
    ++root->state.generation;
    CHECK(!GiantPart(s, root, 1, &resolved));
    --root->state.generation;
    ++a->state.generation;
    CHECK(!GiantPart(s, root, 1, &resolved));
    CHECK(!GiantPart(NULL, root, 1, &resolved) && !GiantPart(s, root, 20, &resolved) &&
          !GiantPlace(s, root, NULL, false));
    CHECK(!GiantParts(NULL, root, 0x820, false));
    SotnActors_Destroy(s);
    return true;
}
static bool Pose(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = &s->slots[33], *b = &s->slots[34];
    CHECK(GiantPose(s, root, 0x800, 0x880, 0) && root->giant.remaining[0] == 1 &&
          a->giant.rotation_velocity == 64 && b->giant.rotation_velocity == -64);
    CHECK(GiantAdvance(s, root, 0x800, 0) && a->giant.rotation == 64 && b->giant.rotation == -64);
    CHECK(GiantPose(s, root, 0x800, 0x880, 0) && root->giant.pose[0] == 1 &&
          !root->giant.remaining[0]);
    CHECK(GiantAdvance(s, root, 0x800, 0) && a->giant.rotation == 128 && b->giant.rotation == -128);
    CHECK(GiantPose(s, root, 0x800, 0x880, 0) && !root->giant.pose[0] &&
          a->giant.rotation_velocity == -256 && b->giant.rotation_velocity == 256);
    CHECK(GiantAdvance(s, root, 0x800, 1) && a->giant.rotation == -128 &&
          b->giant.rotation == 128 && a->giant.placed && b->giant.placed);
    CHECK(GiantPose(s, root, 0x800, 0x880, 1) && root->giant.remaining[1] == 1);
    root->substep = 7;
    GiantClearPose(root);
    CHECK(root->substep == 7 && !root->giant.remaining[1]);
    a->giant.rotation = 0;
    a->giant.target = 4090;
    CHECK(GiantBend(s, root, a, 2) && a->giant.rotation == -3 && a->giant.remaining[0] == 1);
    CHECK(GiantBend(s, root, a, 2) && a->giant.rotation == -6 && !a->giant.remaining[0]);
    CHECK(!GiantAim(a, 0, 0) && !GiantBend(s, root, NULL, 2));
    GiantStep(root, 12);
    CHECK(root->state.step == 12 && !root->substep && !root->giant.pose[0] &&
          !root->giant.pose[1] && !root->giant.remaining[1]);
    Put16(f->stage + 0x880, 0);
    CHECK(!GiantPose(s, root, 0x800, 0x880, 0));
    Put16(f->stage + 0x880, 2);
    Put32(f->stage + 0x884, 0x80000000);
    CHECK(!GiantPose(s, root, 0x800, 0x880, 0));
    CHECK(!GiantPose(s, root, 0x800, 0x880, 2) && !GiantAdvance(s, root, 0x800, 2));
    for (Uint16 i = 0; i < 20; ++i)
        Put16(f->stage + 0x800 + i * 2U, 1);
    CHECK(!GiantAdvance(s, root, 0x800, 0));
    CHECK(!GiantAdvance(s, root, SIZE_MAX, 0));
    SotnActors_Destroy(s);
    return true;
}
static bool Anchors(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = &s->slots[33], *b = &s->slots[34], *c = &s->slots[35],
          *d = &s->slots[36];
    for (Uint16 i = 0; i < 4; ++i)
        Put16(f->stage + 0x820 + i * 2U, i + 1);
    a->giant.length = 10;
    b->giant.parent = 1;
    b->giant.length = 8;
    c->giant.length = 12;
    d->giant.parent = 3;
    d->giant.length = 9;
    b->state.y = 160 * 65536;
    CHECK(GiantAdvance(s, root, 0x820, 2) && root->state.y == 142 * 65536 &&
          a->state.y == 152 * 65536 && b->state.y == 160 * 65536 && c->state.y == 154 * 65536 &&
          d->state.y == 163 * 65536);
    CHECK(GiantAdvance(s, root, 0x820, 3) && d->state.y == 163 * 65536);
    s->slots[50].giant.length = 2;
    s->slots[50].state.y = 170 * 65536;
    CHECK(GiantAdvance(s, root, 0x820, 4) && root->state.y == 168 * 65536 &&
          c->state.y == 180 * 65536);
    const Sint32 old = a->state.y;
    root->state.y += 20 * 65536;
    CHECK(GiantParts(s, root, 0x820, false) && a->state.y == old);
    root->state.facing_left = true;
    CHECK(GiantParts(s, root, 0x820, true) && !a->giant.placed && a->state.facing_left);
    CHECK(GiantParts(s, root, 0x820, false) && a->state.y == 198 * 65536);
    SotnActors_Destroy(s);
    return true;
}
static bool GroundAndSolve(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *root = &s->slots[32], *a = &s->slots[33], *b = &s->slots[34];
    SotnCollider collider = {.top = 9};
    a->state.x = 128 * 65536;
    a->state.y = 183 * 65536;
    CHECK(GiantGround(s, root, a, &collider) && a->giant.terrain == 1 && !collider.top);
    a->state.y = 177 * 65536;
    Uint16 ahead = 99;
    CHECK(GiantAhead(s, a, true, &ahead) && !ahead);
    Put16(f->tiles + (12 * 32 + 12) * 2, 0);
    CHECK(GiantAhead(s, a, true, &ahead) && ahead == 4);
    Put16(f->tiles + (11 * 32 + 12) * 2, 1);
    CHECK(GiantAhead(s, a, true, &ahead) && ahead == 6);
    CHECK(GiantAhead(s, a, false, &ahead) && !ahead);
    root->state.x = 100 * 65536;
    root->state.y = 100 * 65536;
    b->giant.parent = 1;
    b->state.x = root->state.x;
    b->state.y = 116 * 65536;
    CHECK(GiantSolve(s, root, a, b) && !a->giant.target && !b->giant.target);
    b->state.y = 112 * 65536;
    CHECK(GiantSolve(s, root, a, b) && a->giant.target == 512 && b->giant.target == -256);
    root->state.facing_left = true;
    CHECK(GiantSolve(s, root, a, b) && a->giant.target == 512 && b->giant.target == -256);
    b->state.y = 164 * 65536;
    // The signed SDK path reads surrounding authored sine-table words before
    // its nominal square-root table. Retain their quantized overextension result.
    CHECK(GiantSolve(s, root, a, b) && a->giant.target == 182 && b->giant.target == -12);
    a->giant.length = b->giant.length = 32767;
    CHECK(!GiantSolve(s, root, a, b));
    a->giant.length = 0;
    CHECK(!GiantSolve(s, root, a, b) && !GiantGround(s, root, a, NULL) &&
          !GiantAhead(s, a, false, NULL));
    Sint16 angle = 0;
    CHECK(!GiantToward(&angle, 64, 32) && angle == 32);
    CHECK(!GiantToward(&angle, 64, 32) && angle == 64);
    CHECK(GiantToward(&angle, 64, 32) && !GiantToward(&angle, 65, 0));
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    const bool ok = f && Placement(f) && Pose(f) && Anchors(f) && GroundAndSolve(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_GIANT_KNIGHT_MOTION: PASS");
    return 0;
}
