#include <windows.h>
#include <stdint.h>
#include <cstdlib>
#include "../tab/main.h"

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
        framebuffer = tab(framebuffer, width, height,"http://example.com");

        Sleep(16);
    }

    UnmapViewOfFile(framebuffer);
    CloseHandle(hMap);

    return 0;
}