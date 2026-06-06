#include <stdint.h>

uint32_t* render(uint32_t* framebuffer, int width, int height)
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
    return framebuffer;
}