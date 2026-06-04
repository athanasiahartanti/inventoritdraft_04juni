#ifndef __DEVICE_H__
#define __DEVICE_H__

#if defined(ESP32) 
  #if !defined(LED_BUILTIN) 
    #define LED_BUILTIN 33 
  #endif 
  #define LED_RED     2 
  #define LED_GREEN  12 
  
  // Hardwired Hardware Peripherals
  #define PIN_SDA    21 
  #define PIN_SCL    22 
  #define PN532_IRQ   4
  #define PN532_RESET 2
#endif 

#define LED_ON  1 
#define LED_OFF 0 

#endif