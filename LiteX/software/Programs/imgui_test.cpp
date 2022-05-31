// This file is Copyright (c) 2017-2021 Fupy/LiteX-MicroPython Developers
// This file is Copyright (c) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
// License: BSD-2-Clause

#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern "C"
{
#include "irq.h"
#include "uart.h"
}


#if 0


void /*NORETURN*/ hard_reset() { ctrl_reset_write(1); for(;;); } //TODO: move to SDK
extern "C" int upython_main(int argc, char **argv, char *stack_top_arg);
void start_micropython(int argc, char **argv)
{
    //safe way to determine stack top: no other variables in this function //TODO: use alloca()
    int stack_dummy;
    while(upython_main(argc, argv, (char*)&stack_dummy) == 0)
        /*soft_reset()*/;
}

#ifndef LEARNFPGA_LITEX
int main(int argc, char **argv)
#else
extern "C" int litex_demo_main(int argc, char **argv)
#endif
{
    irq_setmask(0);
    irq_setie(1);
    uart_init();

    printf("Starting MP!\n");
    start_micropython(argc, argv);

    irq_setie(0);
    irq_setmask(~0);

    hard_reset();
}

#include "imgui.h"

void* operator new(size_t size) {
   return ImGui::MemAlloc(size);
}

void operator delete(void *p) {
   ImGui::MemFree(p);
}

#else
// dear imgui: "null" example application
// (compile and link imgui, create context, run headless with NO INPUTS, NO GRAPHICS OUTPUT)
// This is useful to test building, but you cannot interact with anything here!
#include "imgui.h"
#include "imgui_sw.h"

extern "C" {
#include "lite_fb.h"
#include "irq.h"
#include "uart.h"
}

#include <stdio.h>

#ifndef LEARNFPGA_LITEX
#include <libbase/uart.h>
#include <libbase/console.h>
#include <generated/csr.h>
#endif

void* operator new(size_t size) {
   return ImGui::MemAlloc(size);
}

void operator delete(void *p) {
   return ImGui::MemFree(p);
}

#ifndef LEARNFPGA_LITEX
int main(int argc, char **argv)
#else
extern "C" int litex_demo_main(int argc, char **argv)
#endif
{
    irq_setmask(0);
    irq_setie(1);
    uart_init();

    printf("Initializing framebuffer...\n");
    fb_init();
    fb_set_dual_buffering(1);
   
    printf("Initializing ImGui...\n");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    imgui_sw::bind_imgui_painting();
    imgui_sw::make_style_fast();

    printf("Starting ImGui...\n");
   
    int n = 0;
    unsigned x = 0, y = 0;
#ifndef LEARNFPGA_LITEX
    for (;;)
#endif
    {
        ++n;
        io.DisplaySize = ImVec2(VIDEO_FRAMEBUFFER_HRES, VIDEO_FRAMEBUFFER_VRES);
        io.DeltaTime = 1.0f / 60.0f;
        ImGui::NewFrame();

#ifndef LEARNFPGA_LITEX
        ImGui::ShowDemoWindow(NULL);
#endif       
        ImGui::SetNextWindowSize(ImVec2(150, 100));
        ImGui::Begin("Test");
        ImGui::Text("Hello, world!");
        ImGui::Text("Frame: %d",n);       
        ImGui::Text("X, Y: %d, %d", x++, y--);       
        ImGui::End();
       
        
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

        ImGui::Render();
        imgui_sw::paint_imgui((uint32_t*)fb_base,VIDEO_FRAMEBUFFER_HRES,VIDEO_FRAMEBUFFER_VRES);
        fb_swap_buffers();
        fb_clear();

#ifndef LEARNFPGA_LITEX
        if (readchar_nonblock()) {
	        getchar();
	        break;
        }
#endif
    }

    printf("DestroyContext()\n");
    ImGui::DestroyContext();
    return 0;
}
#endif
