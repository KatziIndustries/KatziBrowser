#include <windows.h>
#include <stdint.h>
#include <cstdlib>

void render(uint32_t* framebuffer, int width, int height)
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t r = (uint8_t)x;
            uint8_t g = (uint8_t)y;
            uint8_t b = 0;

            framebuffer[y * width + x] =
                (r << 16) |
                (g << 8)  |
                b;
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 4)
    {
        return 1;
    }

    const char* mappingName = argv[1];
    int width  = atoi(argv[2]);
    int height = atoi(argv[3]);

    HANDLE hMap = OpenFileMappingA(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        mappingName
    );

    if (!hMap)
    {
        return 2;
    }

    size_t framebufferSize =
        (size_t)width * height * sizeof(uint32_t);

    uint32_t* framebuffer =
        (uint32_t*)MapViewOfFile(
            hMap,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            framebufferSize
        );

    if (!framebuffer)
    {
        CloseHandle(hMap);
        return 3;
    }

    while (true)
    {
        render(framebuffer, width, height);

        Sleep(16);
    }

    UnmapViewOfFile(framebuffer);
    CloseHandle(hMap);

    return 0;
}