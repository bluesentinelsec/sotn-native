#include <SDL3/SDL.h>
#include <sotn/form_collision.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #v, SDL_GetError());                              \
            return 1;                                                                              \
        }                                                                                          \
    } while (0)
int main(void)
{
    Uint8 tiles[4] = {2, 0, 2, 0}, types[3] = {0, 3, 0xf8};
    SotnCollisionMap map = {{tiles, sizeof(tiles)}, {types, sizeof(types)}, 2, 1};
    SotnFormBarrier barrier = {0};
    CHECK(SotnFormCollision_Barrier(&map, 16, 8, false, &barrier) && barrier.blocked &&
          barrier.velocity_x == 49152);
    CHECK(SotnFormCollision_Barrier(&map, 16, 8, true, &barrier) && barrier.blocked &&
          barrier.velocity_x == -49152);
    tiles[0] = 0;
    CHECK(SotnFormCollision_Barrier(&map, 16, 8, false, &barrier) && barrier.blocked &&
          barrier.velocity_x == -49152);
    tiles[0] = 2;
    tiles[2] = 0;
    CHECK(SotnFormCollision_Barrier(&map, 16, 8, true, &barrier) && barrier.blocked &&
          barrier.velocity_x == 49152);
    tiles[0] = 0;
    CHECK(SotnFormCollision_Barrier(&map, 16, 8, true, &barrier) && !barrier.blocked &&
          !barrier.velocity_x);
    SotnFormEdge edge = {0};
    tiles[2] = 1;
    CHECK(SotnFormCollision_FloorEdge(&map, 12, 8, 1, true, &edge) && edge.displaced &&
          edge.shift_x == -5);
    CHECK(SotnFormCollision_FloorEdge(&map, 7, 8, 1, true, &edge) && edge.displaced &&
          !edge.shift_x);
    tiles[0] = 1;
    tiles[2] = 0;
    CHECK(SotnFormCollision_FloorEdge(&map, 20, 8, 1, true, &edge) && edge.displaced &&
          edge.shift_x == 4);
    // A Mist-only second probe still blocks; clearing it would change original behavior.
    tiles[0] = 2;
    tiles[2] = 1;
    CHECK(SotnFormCollision_FloorEdge(&map, 12, 8, 1, true, &edge) && !edge.displaced);
    // A first-probe Mist-only tile is ignored on the Mist reset path.
    tiles[0] = 0;
    tiles[2] = 2;
    CHECK(SotnFormCollision_FloorEdge(&map, 12, 8, 1, true, &edge) && !edge.displaced);
    CHECK(SotnFormCollision_FloorEdge(&map, 12, 8, 1, false, &edge) && edge.displaced &&
          edge.shift_x == -5);
    edge = (SotnFormEdge){.displaced = true, .shift_x = 123};
    tiles[2] = 9;
    CHECK(!SotnFormCollision_FloorEdge(&map, 12, 8, 1, true, &edge) && edge.displaced &&
          edge.shift_x == 123);
    CHECK(!SotnFormCollision_FloorEdge(NULL, 12, 8, 1, true, &edge));
    CHECK(!SotnFormCollision_FloorEdge(&map, 32768, 8, 1, true, &edge));
    CHECK(!SotnFormCollision_Barrier(&map, 12, 8, true, NULL));
    CHECK(!SotnFormCollision_Barrier(&map, 12, -32769, true, &barrier));
    return 0;
}
