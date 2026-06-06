#include <stdint.h>
#include "../tab/render.h"

uint32_t* tab(uint32_t* framebuffer, int width, int height,const char *url){
    return render(framebuffer,width,height);
}