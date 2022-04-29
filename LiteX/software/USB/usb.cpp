// This file is Copyright (c) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
// License: BSD-2-Clause

//#define DEBUG_ALL

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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


#define F_USB_LOWSPEED 1500000
#define TIMING_PREC 4
uint32_t gpio_test(void)
{
    const uint32_t TRANSMIT_TIME_DELAY = ((F_CPU/1000)*TIMING_PREC)/(F_USB_LOWSPEED/1000);
    uint8_t b = 1;
    hal_set_differential_gpio_value(DP_P0, DM_P0, 2);
    hal_gpio_set_direction(DP_P0, 1);
    hal_gpio_set_direction(DM_P0, 1);
    int k=0, td = 0, tdk=0;
#pragma GCC unroll 0
  for(int t1 = cpu_hal_get_cycle_count();k<13;++k) {
    td += TRANSMIT_TIME_DELAY;
    tdk = td/TIMING_PREC;
    while((int)(cpu_hal_get_cycle_count() - t1) < tdk);
    hal_set_differential_gpio_value(DP_P0, DM_P0, b);
      b^=1;
  }
  return TRANSMIT_TIME_DELAY;
}

void setup()
{
  /*uint32_t td = gpio_test();
  td = gpio_test();
  printf("bit delay %u (%d cycles)\n", td, td/TIMING_PREC);
  for(;;);*/
  USH.init( USB_Pins_Config, my_USB_DetectCB, my_USB_PrintCB );
  USH.setActivityBlinker(my_LedBlinkCB);
}

void loop()
{
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
      printf("0x%02d %d\n", pins, bit_deltat); 
    }
  }
#endif

    struct USBMessage msg;
    if( hal_queue_receive(usb_msg_queue, &msg) ) {
      if( printDataCB ) {
        printDataCB( msg.src/4, 32, msg.data, msg.len );
      }
    }
    printState();


#if !defined(TIMER_INTERVAL0_SEC)
#warning avoid polling
  static int t = -1;
  int tnow = micros()/1000;
  if(tnow != t)
  {
    t = tnow;
    usb_process();
  }
#endif
}


#if defined(TIMER_INTERVAL0_SEC)
extern "C" void litex_timer_setup(uint32_t cycles, timer_isr_t handler);
void hal_timer_setup(timer_idx_t timer_num, uint32_t alarm_value, timer_isr_t timer_isr)
{
  litex_timer_setup(alarm_value, timer_isr); //fixed 1ms value for 100MHz
}
#endif

