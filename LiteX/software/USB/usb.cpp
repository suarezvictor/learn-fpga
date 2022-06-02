// This file is Copyright (c) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
// License: BSD-2-Clause

//#define DEBUG_ALL
//#define USE_IMGUI

#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern "C" {
#include "lite_fb.h"
}
#ifdef USE_IMGUI
#include "imgui.h"
#include "imgui_sw.h"
#endif

#define LED_BUILTIN 0
#define PROFILE_NAME "LiteX"
#define DP_P0  8
#define DM_P0  12

//#include "usb_host.h"
#include <ESP32-USBSoftHost.hpp>


static void my_USB_DetectCB( uint8_t usbNum, void * dev )
{
  sDevDesc *device = (sDevDesc*)dev;
  printf("New device detected on USB#%d\n", usbNum);
  printf("desc.bcdUSB             = 0x%04x\n", device->bcdUSB);
  printf("desc.bDeviceClass       = 0x%02x\n", device->bDeviceClass);
  printf("desc.bDeviceSubClass    = 0x%02x\n", device->bDeviceSubClass);
  printf("desc.bDeviceProtocol    = 0x%02x\n", device->bDeviceProtocol);
  printf("desc.bMaxPacketSize0    = 0x%02x\n", device->bMaxPacketSize0);
  printf("desc.idVendor           = 0x%04x\n", device->idVendor);
  printf("desc.idProduct          = 0x%04x\n", device->idProduct);
  printf("desc.bcdDevice          = 0x%04x\n", device->bcdDevice);
  printf("desc.iManufacturer      = 0x%02x\n", device->iManufacturer);
  printf("desc.iProduct           = 0x%02x\n", device->iProduct);
  printf("desc.iSerialNumber      = 0x%02x\n", device->iSerialNumber);
  printf("desc.bNumConfigurations = 0x%02x\n", device->bNumConfigurations);
  // if( device->iProduct == mySupportedIdProduct && device->iManufacturer == mySupportedManufacturer ) {
  //   myListenUSBPort = usbNum;
  // }
}


static void my_USB_PrintCB(uint8_t usbNum, uint8_t byte_depth, uint8_t* data, uint8_t data_len)
{
  // if( myListenUSBPort != usbNum ) return;
  printf("in: ");
  for(int k=0;k<data_len;k++) {
    printf("0x%02x ", data[k] );
  }
  printf("\n");
}

#ifdef DEBUG_ALL
extern volatile uint8_t received_NRZI_buffer_bytesCnt;
extern uint16_t received_NRZI_buffer[];
#endif

unsigned activity_count = 0;
void my_LedBlinkCB(int on_off)
{
  hal_gpio_set_level(BLINK_GPIO, on_off);
#ifdef DEBUG_ALL
  if(on_off)
  {
    if(received_NRZI_buffer_bytesCnt <= 13) //this is for debugging no-data packets
      /*initStates(-1,-1,-1,-1,-1,-1,-1,-1)*/; //disable all to stop processing
    else
      ++activity_count;
  }
#endif
}

usb_pins_config_t USB_Pins_Config =
{
  DP_P0, DM_P0,
  -1, -1,
  -1, -1,
  -1, -1
};


extern "C" void loop();
extern "C" void setup();
void do_ui_update(int mousex, int mousey, int buttons, int wheel);
void ui_init();
void do_ui();

void delay(int ms)
{
 int t1 = micros() + ms*1000;
 while(int(micros() - t1) < 0);
}

extern "C" USBMessage usb_msg_queue_buffer[];
USBMessage FAST_DATA usb_msg_queue_buffer[100];

void setup()
{
  ui_init();
/*
  for(;;)
  {
    static uint32_t color = 0;
    //quarter frame 25600 times: 68s/60s single/double pixel DMA (about 94FPS-106FPS full frame)
    fb_fillrect(1, 1, FB_WIDTH/2, FB_HEIGHT/2, color);
    color += 0x010101;
    fb_swap_buffers();
  }
 */ 
  printf("USB init...\n");
  USH.init( USB_Pins_Config, usb_msg_queue_buffer, sizeof(usb_msg_queue_buffer)/sizeof(usb_msg_queue_buffer[0]), my_USB_DetectCB, my_USB_PrintCB );
  USH.setActivityBlinker(my_LedBlinkCB);
  printf("USB init done\n");
 
}

void loop()
{
    printState();
    /*int msgcount = tu_fifo_count(&usb_msg_queue);
    if(msgcount)
      printf("Elements in FIFO: %d\n", msgcount);*/

    struct USBMessage msg;
    while( hal_queue_receive(usb_msg_queue, &msg) ) {
      if( printDataCB ) {
#ifndef USE_IMGUI      
        printDataCB( msg.src/4, 32, msg.data, msg.len );
#endif
      }

#ifdef DEBUG_ALL
  static unsigned prev_count = 0;
  if(activity_count != prev_count && received_NRZI_buffer_bytesCnt > 0)
  {
    prev_count = activity_count;
    int xcount = received_NRZI_buffer_bytesCnt;
    uint16_t buf[256];
    memcpy(buf, received_NRZI_buffer, xcount*sizeof(*buf));
    printf("activity %d, received %d transitions\n", activity_count, xcount);
    uint8_t prev_time = buf[0] & 0xFF;
    for(int i=0; i < xcount; ++i)
    {
      uint8_t pins = buf[i]>>8;
      uint8_t bit_deltat = (buf[i] & 0xFF) - prev_time;
      prev_time = (buf[i] & 0xFF);
      printf("0x%02X %d\n", pins, bit_deltat); 
    }
  }
#endif

      bool ismousepacket = (msg.len == 6); //FIXME: too hacky a way of discriminating a mouse packet (HID spec allows just 3 bytes as valid, some mouses report 20 byte packets)

      static int x = FB_WIDTH/2, y = FB_HEIGHT/2;
      
      if(ismousepacket)
      {
        //packet decoding in 12-bit values (some mouses reports 8 bit values)
        //see https://forum.pjrc.com/threads/45740-USB-Host-Mouse-Driver
        uint8_t buttons = msg.data[0];
        int16_t dx = ((msg.data[2] & 0x0f) << 8) | (msg.data[1] & 0xff); dx <<= 4; dx >>= 4; //sign correction
        int16_t dy = ((msg.data[3] & 0xff) << 4) | (msg.data[2] >> 4) & 0x0f; dy <<= 4; dy >>= 4; //sign correction
		int16_t wheel = (int8_t) msg.data[4];
		        
        //coordinate update
        x += dx;
        y += dy;
        if(x < 0) x = 0; if(x >= FB_WIDTH) x = FB_WIDTH-1; 
        if(y < 0) y = 0; if(y >= FB_HEIGHT) y = FB_HEIGHT-1;
#ifndef USE_IMGUI         
        printf("dx %d, dy %d buttons 0x%02X wheel %d\n", dx, dy, buttons, wheel);
#endif
        do_ui_update(x, y, buttons, wheel);
      }
    }

    do_ui();
}


#if !defined(TIMER_INTERVAL0_SEC)
#error avoid polling
#else
extern "C" void litex_timer_setup(uint32_t cycles, timer_isr_t handler);
void hal_timer_setup(timer_idx_t timer_num, uint32_t alarm_value, timer_isr_t timer_isr)
{
  litex_timer_setup(alarm_value, timer_isr);
}
#endif


#ifdef USE_IMGUI
static uint8_t justPressed = 0;
void do_ui_update(int mousex, int mousey, int buttons, int wheel)
{
  ImGuiIO& io = ImGui::GetIO();
  io.MousePos = ImVec2((float)mousex, (float)mousey);
  justPressed |= buttons;
}

void do_ui()
{
    //return;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 p = io.MousePos;
    for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
        io.MouseDown[i] = (justPressed >> i) & 1;
    justPressed = 0;

    static int n = 0;
    {
        //printf("Frame %d\n", n);
        ++n;
        //delay(100); return;
        io.DisplaySize = ImVec2(VIDEO_FRAMEBUFFER_HRES, VIDEO_FRAMEBUFFER_VRES);
        io.DeltaTime = 1.0f / 60.0f;
        ImGui::NewFrame();
        ImColor linecolor = IM_COL32(255, 0, 0, 255);

        //ImGui::ShowDemoWindow(NULL); //this makes mouse to stop working
        ImGui::SetNextWindowSize(ImVec2(100, 100));
        ImGui::Begin("Test");
        ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, 0), ImVec2(p.x, VIDEO_FRAMEBUFFER_VRES-1), linecolor, 1);
        ImGui::GetWindowDrawList()->AddLine(ImVec2(0, p.y), ImVec2(VIDEO_FRAMEBUFFER_HRES-1,p.y), linecolor, 1);
        //ImGui::Text("X: %d", int(p.x));       
        //ImGui::Text("Y: %d", int(p.y));       
        ImGui::Text("Frame: %d",n);       
        ImGui::End();
       
        /*
        //this requires floating point support in printf-like functions
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        */

        ImGui::Render();
        imgui_sw::paint_imgui((uint32_t*)fb_base,VIDEO_FRAMEBUFFER_HRES,VIDEO_FRAMEBUFFER_VRES);
        fb_swap_buffers();
        fb_clear();
    }
}

size_t alloc_total = 0;
static void *custom_malloc(size_t size, void* user_data)
{
  IM_UNUSED(user_data);
  size_t *ptr = (size_t *) malloc(size+sizeof(size_t));
  if(!ptr) return ptr;
  *ptr++ = size;
  alloc_total += size;
  //printf("alloc: 0x%p, size %d, total %d\n", ptr, size, alloc_total);
  return ptr;
}

static void custom_free(void* ptr, void* user_data)
{
  IM_UNUSED(user_data);
  if(!ptr) return;
  size_t *ptra = (size_t*)ptr;
  size_t size = *(--ptra);
  alloc_total -= size;
  //printf("free: 0x%p, size %d, total %d\n", ptr, size, alloc_total);
  free(ptra);
}

void ui_init()
{
  fb_init();
  fb_set_dual_buffering(1);

    printf("Initializing ImGui...\n");
    IMGUI_CHECKVERSION();
    ImGui::SetAllocatorFunctions(custom_malloc, custom_free, nullptr);
    ImGui::CreateContext();
    //ImGui::GetIO().MouseDrawCursor = true; //makes things hang up

    imgui_sw::bind_imgui_painting();
    imgui_sw::make_style_fast();

    printf("Starting ImGui...\n");
}

void* operator new(size_t size) {
   void *p = ImGui::MemAlloc(size);
   return p;
}

void operator delete(void *p) {
   return ImGui::MemFree(p);
}
#else
void do_ui_update(int mousex, int mousey, int buttons, int wheel)
{
  static int lastx = FB_WIDTH/2, lasty = FB_HEIGHT/2;
  uint32_t color = 0;
  //fancy color select algorithm
  if(buttons & 1) color ^= 0x4080FF;
  if(buttons & 2) color ^= 0x80FF40;
  if(buttons && mousey < lasty) color = ~color;
        
  fb_fillrect(lastx < mousex ? lastx : mousex, lasty < mousey ? lasty : mousey,
    lastx < mousex ? mousex : lastx, lasty < mousey ? mousey : lasty,
    color);

  lastx = mousex; lasty = mousey;
}

void do_ui()
{
  //delay(500);
}

void ui_init()
{
  fb_init();
  printf("Dummy UI init...\n");
  delay(1000);
  printf("Dummy UI done\n");
}



void* operator new(size_t size) {
   return malloc(size);
}

void operator delete(void *p) {
   free(p);
}

#endif
