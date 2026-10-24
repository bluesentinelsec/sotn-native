// SPDX-License-Identifier: AGPL-3.0-or-later
// Selected mesh/projection/UV paths from sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5, dra/62DEC.c and menu.c.
// Meshes, font, texture pixels, palettes, and UV arrays stay in private runtime data.
#include <SDL3/SDL.h>
#include <sotn/save_point_renderer.h>

typedef struct Vertex
{
    Sint32 x, y, z;
} Vertex;
typedef struct Face
{
    Vertex vertices[3], normal;
    Uint8 uv[6];
} Face;
struct SotnSavePointRenderer
{
    SotnDataView common;
    Face meshes[2][24];
    Uint16 coffin_palette[16];
};
static Uint16 Read16(const Uint8 *p)
{
    return (Uint16)(p[0] | (Uint16)p[1] << 8);
}
static Uint32 Read32(const Uint8 *p)
{
    return p[0] | (Uint32)p[1] << 8 | (Uint32)p[2] << 16 | (Uint32)p[3] << 24;
}
static Sint32 Signed16(const Uint8 *p)
{
    const Uint16 bits = Read16(p);
    return bits < 32768 ? bits : (Sint32)bits - 65536;
}
static bool Resolve(SotnDataView dra, Uint32 address, size_t size, const Uint8 **out)
{
    if (dra.bytes == NULL || address < 0x800a0000 || (address & 1) ||
        address - 0x800a0000 > dra.size || size > dra.size - (address - 0x800a0000))
        return SDL_SetError("Save point references invalid private mesh/palette data");
    *out = dra.bytes + address - 0x800a0000;
    return true;
}
bool SotnSavePointRenderer_Create(SotnDataView dra, SotnDataView common,
                                  SotnSavePointRenderer **out)
{
    if (out == NULL)
        return SDL_InvalidParamError("out");
    *out = NULL;
    if (common.bytes == NULL || common.size != 0x42000)
        return SDL_SetError("Save point requires a complete common graphics resource");
    SotnSavePointRenderer *renderer = SDL_calloc(1, sizeof(*renderer));
    if (renderer == NULL)
        return false;
    renderer->common = common;
    const Uint8 *table = NULL, *uv = NULL, *palette = NULL;
    if (!Resolve(dra, 0x800a3728, 144, &uv) || !Resolve(dra, 0x800a3a74, 16, &table) ||
        Read32(table) != 1 || Read32(table + 4) != 0x1f00 || Read32(table + 8) != 16 ||
        !Resolve(dra, Read32(table + 12), 32, &palette))
        goto fail;
    for (Uint32 i = 0; i < 16; ++i)
        renderer->coffin_palette[i] = Read16(palette + i * 2);
    for (Uint32 mesh = 0; mesh < 2; ++mesh)
    {
        if (!Resolve(dra, mesh == 0 ? 0x800a3210 : 0x800a33a0, 288, &table))
            goto fail;
        for (Uint32 i = 0; i < 24; ++i)
        {
            Face *face = &renderer->meshes[mesh][i];
            Vertex sum = {0};
            for (Uint32 j = 0; j < 3; ++j)
            {
                const Uint8 *v = NULL;
                if (!Resolve(dra, Read32(table + i * 12 + j * 4), 8, &v))
                    goto fail;
                face->vertices[j] = (Vertex){Signed16(v), Signed16(v + 2), Signed16(v + 4)};
                const Vertex vertex = face->vertices[j];
                if (SDL_abs(vertex.x) > 128 || SDL_abs(vertex.y) > 128 || SDL_abs(vertex.z) > 128)
                {
                    SDL_SetError("Save-point mesh vertex exceeds its projection bounds");
                    goto fail;
                }
                sum.x += vertex.x;
                sum.y += vertex.y;
                sum.z += vertex.z;
            }
            const double length =
                SDL_sqrt((double)sum.x * sum.x + (double)sum.y * sum.y + (double)sum.z * sum.z);
            if (length != 0)
                face->normal =
                    (Vertex){(Sint32)(sum.x * 4096 / length), (Sint32)(sum.y * 4096 / length),
                             (Sint32)(sum.z * 4096 / length)};
            SDL_memcpy(face->uv, uv + i * 6, 6);
        }
    }
    *out = renderer;
    return true;
fail:
    SDL_free(renderer);
    return false;
}

static Uint16 Sample(const SotnSavePointRenderer *renderer, Uint32 page, Uint32 palette, Uint32 u,
                     Uint32 v)
{
    const Uint32 x = (page & 15) * 64 - 512 + u / 4;
    const Uint32 block = (x / 64) * 4 + (x % 64) / 32 + (v / 128) * 2;
    const Uint32 offset = block * 8192 + (v % 128) * 64 + (x % 32) * 2;
    const Uint16 word = Read16(renderer->common.bytes + offset);
    const Uint32 index = (word >> ((u & 3) * 4)) & 15;
    if (palette == 0x1f0)
        return renderer->coffin_palette[index];
    return Read16(renderer->common.bytes + 0x40000 + ((palette - 256) * 16 + index) * 2);
}
static Sint64 Edge(Vertex a, Vertex b, Sint32 x, Sint32 y)
{
    return (Sint64)(b.x - a.x) * (y - a.y) - (Sint64)(b.y - a.y) * (x - a.x);
}
static void Triangle(const SotnSavePointRenderer *renderer, Vertex v[3], Uint8 uv[6],
                     Uint32 palette, Uint16 *frame)
{
    const Sint64 area = Edge(v[0], v[1], v[2].x, v[2].y);
    if (area == 0)
        return;
    const Sint32 left = SDL_max(0, SDL_min(v[0].x, SDL_min(v[1].x, v[2].x)));
    const Sint32 right = SDL_min(255, SDL_max(v[0].x, SDL_max(v[1].x, v[2].x)));
    const Sint32 top = SDL_max(0, SDL_min(v[0].y, SDL_min(v[1].y, v[2].y)));
    const Sint32 bottom = SDL_min(239, SDL_max(v[0].y, SDL_max(v[1].y, v[2].y)));
    for (Sint32 y = top; y <= bottom; ++y)
        for (Sint32 x = left; x <= right; ++x)
        {
            const Sint64 a = Edge(v[1], v[2], x, y), b = Edge(v[2], v[0], x, y);
            const Sint64 c = area - a - b;
            if ((area > 0 && (a < 0 || b < 0 || c < 0)) || (area < 0 && (a > 0 || b > 0 || c > 0)))
                continue;
            const Uint32 u = (Uint32)((a * uv[0] + b * uv[2] + c * uv[4]) / area);
            const Uint32 t = (Uint32)((a * uv[1] + b * uv[3] + c * uv[5]) / area);
            const Uint16 color = Sample(renderer, 0x1b, palette, u, t);
            if (color != 0)
                frame[(size_t)y * 256 + (size_t)x] = color & 0x7fff;
        }
}
bool SotnSavePointRenderer_Draw(const SotnSavePointRenderer *renderer, SotnSavePoint point,
                                bool front, Uint16 *frame, size_t count)
{
    if (renderer == NULL || frame == NULL || count < SOTN_FRAME_PIXELS || point.scale > 256 ||
        point.scatter > 127 || point.angle > 4095 || point.phase < SOTN_SAVE_POINT_IDLE ||
        point.phase > SOTN_SAVE_POINT_ERROR)
        return SDL_SetError("Save-point drawing requires bounded state and a full frame");
    const bool coffin = point.phase >= SOTN_SAVE_POINT_COFFIN;
    const Uint32 faces = coffin ? 24 : 20;
    const Sint32 sine = (Sint32)SDL_round(SDL_sin(point.angle * (2 * SDL_PI_D / 4096)) * 4096);
    const Sint32 cosine = (Sint32)SDL_round(SDL_cos(point.angle * (2 * SDL_PI_D / 4096)) * 4096);
    for (Uint32 side = 0; side < 2; ++side)
        for (Uint32 i = 0; i < faces; ++i)
        {
            const Face *face = &renderer->meshes[coffin ? 1 : 0][i];
            Vertex vertices[3] = {0};
            bool clipped = false;
            for (Uint32 j = 0; j < 3; ++j)
            {
                Vertex v = face->vertices[j];
                const Sint32 spread = point.phase == SOTN_SAVE_POINT_EXIT ||
                                              point.phase == SOTN_SAVE_POINT_SCATTER ||
                                              point.phase == SOTN_SAVE_POINT_COFFIN
                                          ? point.scatter
                                          : 0;
                v.x = v.x * point.scale / 256 + face->normal.x * spread / 4096;
                v.y = v.y * point.scale / 256 + face->normal.y * spread / 4096;
                v.z = v.z * point.scale / 256 + face->normal.z * spread / 4096;
                const Sint32 x = (v.x * cosine + v.z * sine) / 4096;
                const Sint32 z = (-v.x * sine + v.z * cosine) / 4096 + 256;
                if (z < 32)
                {
                    clipped = true;
                    break;
                }
                vertices[j] = (Vertex){128 + x * 256 / z, 136 + v.y * 256 / z, z};
            }
            if (clipped)
                continue;
            const bool facing = Edge(vertices[0], vertices[1], vertices[2].x, vertices[2].y) >= 0;
            const bool above_player = facing && (coffin || point.scatter >= 16);
            if (facing != (side != 0) || above_player != front)
                continue;
            Uint8 uv[6] = {219, 168, 188, 222, 250, 222};
            Uint32 palette = 0x1f1;
            if (coffin)
            {
                palette = 0x1f0;
                for (Uint32 j = 0; j < 6; ++j)
                    uv[j] = (Uint8)(face->uv[j] + 128);
                if (!facing)
                    SDL_memcpy(uv, (const Uint8[]){209, 241, 222, 254, 209, 254}, 6);
            }
            Triangle(renderer, vertices, uv, palette, frame);
        }
    return true;
}
static void Text(const SotnSavePointRenderer *renderer, const char *text, Sint32 x, Sint32 y,
                 Uint16 *frame)
{
    for (; *text != '\0'; ++text, x += 8)
    {
        const Uint8 glyph = (Uint8)((Uint8)*text - 32);
        const Uint32 u = (glyph & 15U) * 8, v = (glyph & 240U) >> 1;
        for (Sint32 j = 0; j < 8; ++j)
            for (Sint32 i = 0; i < 8; ++i)
            {
                if (x + i < 0 || x + i >= 256 || y + j < 0 || y + j >= 240)
                    continue;
                const Uint16 color = Sample(renderer, 0x1e, 0x196, u + (Uint32)i, v + (Uint32)j);
                if (color != 0)
                    frame[(size_t)(y + j) * 256 + (size_t)(x + i)] = color & 0x7fff;
            }
    }
}
bool SotnSavePointRenderer_Dialog(const SotnSavePointRenderer *renderer, SotnSavePoint point,
                                  bool gamepad, Uint16 *frame, size_t count)
{
    if (renderer == NULL || frame == NULL || count < SOTN_FRAME_PIXELS || point.slot == 0 ||
        point.slot > 15)
        return SDL_SetError("Save dialog requires initialized state and a full frame");
    if (!SotnSavePoint_Menu(&point))
        return true;
    for (Uint32 y = 76; y <= 160; ++y)
        for (Uint32 x = 24; x <= 232; ++x)
            frame[y * 256 + x] = x == 24 || x == 232 || y == 76 || y == 160 ? 0x7fff : 0x3000;
    char label[32] = {0};
    SDL_snprintf(label, sizeof(label), "Slot %02u / 15", point.slot);
    Text(renderer, label, 72, 86, frame);
    const bool occupied = (point.occupied & (1U << (point.slot - 1))) != 0;
    const char *message =
        point.phase == SOTN_SAVE_POINT_SLOT      ? (occupied ? "Existing data" : "Empty slot")
        : point.phase == SOTN_SAVE_POINT_CONFIRM ? (occupied ? "Overwrite data?" : "Save game?")
        : point.phase == SOTN_SAVE_POINT_SAVED   ? "Data saved."
                                                 : "Save failed.";
    Text(renderer, message, 48, 106, frame);
    const char *choice = point.phase == SOTN_SAVE_POINT_SLOT ? "Up/Down: select"
                         : point.phase == SOTN_SAVE_POINT_CONFIRM
                             ? (point.yes ? ">Yes     No" : " Yes    >No")
                         : point.phase == SOTN_SAVE_POINT_ERROR ? "Retry / Back"
                                                                : "Press OK";
    Text(renderer, choice, 48, 126, frame);
    Text(renderer, gamepad ? "A: OK   Y: Back" : "Enter: OK  A: Back", 40, 146, frame);
    return true;
}
void SotnSavePointRenderer_Destroy(SotnSavePointRenderer *renderer)
{
    SDL_free(renderer);
}
