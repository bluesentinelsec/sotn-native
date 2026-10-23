#include "data_internal.h"
#include <SDL3/SDL.h>

#define CHECK(expression)                                                                          \
    do                                                                                             \
    {                                                                                              \
        if (!(expression))                                                                         \
        {                                                                                          \
            SDL_LogError(SDL_LOG_CATEGORY_TEST, "Line %d: %s: %s", __LINE__, #expression,          \
                         SDL_GetError());                                                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

static const char abc_hash[] = "a9993e364706816aba3e25717850c26c9cd0d89d";

static bool TestHash(void)
{
    static const struct
    {
        size_t size;
        const char *hash;
    } vectors[] = {
        {0, "da39a3ee5e6b4b0d3255bfef95601890afd80709"},
        {55, "8ae2d46729cfe68ff927af5eec9c7d1b66d65ac2"},
        {56, "636e2ec698dac903498e648bd2f3af641d3c88cb"},
        {63, "6d942da0c4392b123528f2905c713a3ce28364bd"},
        {64, "c6138d514ffa2135bfce0ed0b8fac65669917ec7"},
        {65, "69bd728ad6e13cd76ff19751fde427b00e395746"},
        {127, "89d7312a903f65cd2b3e34a975e55dbea9033353"},
        {128, "e6434bc401f98603d7eda504790c98c67385d535"},
        {129, "3352e41cc30b40ae80108970492b21014049e625"},
        {1000, "af0b191c2de46fe13fe0908f5a6a4e90e0cafc46"},
    };
    Uint8 bytes[1000] = {0};
    for (size_t i = 0; i < SDL_arraysize(bytes); ++i)
    {
        bytes[i] = (Uint8)(i % 256);
    }
    char digest[41] = {0};
    for (size_t i = 0; i < SDL_arraysize(vectors); ++i)
    {
        SotnData_Sha1(bytes, vectors[i].size, digest);
        CHECK(SDL_strcmp(digest, vectors[i].hash) == 0);
    }
    SotnData_Sha1((const Uint8 *)"abc", 3, digest);
    CHECK(SDL_strcmp(digest, abc_hash) == 0);
    Uint8 *large = SDL_malloc(1000000);
    CHECK(large != NULL);
    SDL_memset(large, 'a', 1000000);
    SotnData_Sha1(large, 1000000, digest);
    SDL_free(large);
    CHECK(SDL_strcmp(digest, "34aa973cd4c4daa4f61eeb2bdbad27316534016f") == 0);
    return true;
}

typedef struct TestStream
{
    Sint64 reported_size;
    const char *bytes;
    size_t length;
    size_t offset;
    bool fail;
} TestStream;

static Sint64 SDLCALL StreamSize(void *userdata)
{
    const TestStream *stream = userdata;
    return stream->reported_size;
}

static size_t SDLCALL StreamRead(void *userdata, void *output, size_t size, SDL_IOStatus *status)
{
    TestStream *stream = userdata;
    if (stream->fail)
    {
        *status = SDL_IO_STATUS_ERROR;
        SDL_SetError("Injected read failure");
        return 0;
    }
    if (stream->offset >= stream->length)
    {
        *status = SDL_IO_STATUS_EOF;
        return 0;
    }
    if (size != 0)
    {
        *(Uint8 *)output = (Uint8)stream->bytes[stream->offset++];
        *status = SDL_IO_STATUS_READY;
        return 1;
    }
    return 0;
}

static bool ReadTestStream(TestStream *source, bool expected)
{
    SDL_IOStreamInterface interface = {0};
    SDL_INIT_INTERFACE(&interface);
    interface.size = StreamSize;
    interface.read = StreamRead;
    SDL_IOStream *stream = SDL_OpenIO(&interface, source);
    CHECK(stream != NULL);
    const SotnDataSpec spec = {"synthetic", 3, abc_hash};
    Uint8 buffer[3] = {0};
    const bool valid = SotnData_ReadVerified(stream, buffer, &spec);
    CHECK(SDL_CloseIO(stream));
    CHECK(valid == expected);
    if (valid)
    {
        CHECK(SDL_memcmp(buffer, "abc", sizeof(buffer)) == 0);
    }
    return true;
}

static bool TestStreams(void)
{
    TestStream partial = {3, "abc", 3, 0, false};
    TestStream unknown = {-1, "abc", 3, 0, false};
    TestStream wrong_size = {2, "ab", 2, 0, false};
    TestStream truncated = {3, "ab", 2, 0, false};
    TestStream changed = {3, "abcd", 4, 0, false};
    TestStream corrupt = {3, "abd", 3, 0, false};
    TestStream failed = {3, "abc", 3, 0, true};
    CHECK(ReadTestStream(&partial, true));
    CHECK(ReadTestStream(&unknown, false));
    CHECK(ReadTestStream(&wrong_size, false));
    CHECK(ReadTestStream(&truncated, false));
    CHECK(ReadTestStream(&changed, false));
    CHECK(ReadTestStream(&corrupt, false));
    CHECK(ReadTestStream(&failed, false));
    return true;
}

static bool WriteFixture(const char *path, const char *bytes)
{
    SDL_IOStream *stream = SDL_IOFromFile(path, "wb");
    CHECK(stream != NULL);
    const size_t size = SDL_strlen(bytes);
    const bool written = SDL_WriteIO(stream, bytes, size) == size;
    CHECK(SDL_CloseIO(stream));
    CHECK(written);
    return true;
}

static bool TestSnapshot(const char *directory)
{
    static const char *names[SOTN_GAME_FILE_COUNT] = {"main.txt", "dra.txt", "ric.txt", "st0.txt",
                                                      "nz0.txt"};
    static const char *contents[SOTN_GAME_FILE_COUNT] = {"a", "ab", "abc", "abcd", "abcde"};
    static const char *hashes[SOTN_GAME_FILE_COUNT] = {
        "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8", "da23614e02469a0d7c7bd1bdab5c9c474b1904dc",
        "a9993e364706816aba3e25717850c26c9cd0d89d", "81fe8bfe87576c3ecb22426f8e57847382917acf",
        "03de6c570bfe24bfc328ccd7ca46b76eadaf4334"};
    char *paths[SOTN_GAME_FILE_COUNT] = {0};
    SotnDataSpec specs[SOTN_GAME_FILE_COUNT] = {0};
    for (size_t i = 0; i < SOTN_GAME_FILE_COUNT; ++i)
    {
        CHECK(SDL_asprintf(&paths[i], "%s/%s", directory, names[i]) >= 0);
        CHECK(WriteFixture(paths[i], contents[i]));
        specs[i] = (SotnDataSpec){names[i], SDL_strlen(contents[i]), hashes[i]};
    }
    SotnGameData *data = NULL;
    CHECK(SotnData_OpenReference(directory, specs, &data));
    CHECK(data != NULL);
    CHECK(WriteFixture(paths[0], "changed after validation"));
    for (int i = 0; i < SOTN_GAME_FILE_COUNT; ++i)
    {
        SotnDataView view = {0};
        CHECK(SotnGameData_GetFile(data, (SotnGameFileId)i, &view));
        CHECK(view.size == SDL_strlen(contents[i]));
        CHECK(SDL_memcmp(view.bytes, contents[i], view.size) == 0);
    }
    SotnDataView view = {NULL, 123};
    CHECK(!SotnGameData_GetFile(data, SOTN_GAME_FILE_COUNT, &view));
    CHECK(view.bytes == NULL && view.size == 0);
    CHECK(!SotnGameData_GetFile(data, (SotnGameFileId)-1, &view));
    CHECK(!SotnGameData_GetFile(data, SOTN_GAME_MAIN, NULL));
    SotnGameData_Close(data);
    data = NULL;
    CHECK(WriteFixture(paths[0], "z"));
    CHECK(!SotnData_OpenReference(directory, specs, &data));
    CHECK(data == NULL);
    CHECK(SDL_strstr(SDL_GetError(), "checksum mismatch") != NULL);
    CHECK(WriteFixture(paths[0], "a"));
    specs[SOTN_GAME_NZ0].path = "missing.txt";
    CHECK(!SotnData_OpenReference(directory, specs, &data));
    CHECK(data == NULL);
    specs[0].size = SIZE_MAX;
    CHECK(!SotnData_OpenReference(directory, specs, &data));
    CHECK(SDL_strstr(SDL_GetError(), "allocation budget") != NULL);
    specs[0].size = 0;
    CHECK(!SotnData_OpenReference(directory, specs, &data));
    CHECK(!SotnGameData_Open(paths[0], &data));
    CHECK(!SotnGameData_Open(NULL, &data));
    CHECK(!SotnGameData_Open("", &data));
    CHECK(!SotnGameData_Open(directory, NULL));
    CHECK(!SotnGameData_GetFile(NULL, SOTN_GAME_MAIN, &view));
    for (size_t i = 0; i < SOTN_GAME_FILE_COUNT; ++i)
    {
        CHECK(SDL_RemovePath(paths[i]));
        SDL_free(paths[i]);
    }
    SotnGameData_Close(NULL);
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2 || !TestHash() || !TestStreams() || !TestSnapshot(argv[1]))
    {
        return 1;
    }
    SDL_Log("SOTN_DATA_TEST: PASS");
    return 0;
}
