/*
  
  Since I had problems with my pc not booting when multiple Teensy USB Keyboards were connected at the same time,
  so I created a custom keyboard-less USB type, "Mouse/Jostick".
  
  In order to add it to the Teensy USB menu, you need to add these lines to the specified files (default path used for reference)

  in C:\Program Files (x86)\Arduino\hardware\teensy\avr\cores\teensy3\usb_desc.h:

  #elif defined(USB_MOUSE_JOYSTICK)
    #define VENDOR_ID    0x16C0
    #define PRODUCT_ID    0x047F
    #define MANUFACTURER_NAME {'T','e','e','n','s','y','d','u','i','n','o'}
    #define MANUFACTURER_NAME_LEN 11
    #define PRODUCT_NAME    {'M','o','u','s','e','/','J','o','y','s','t','i','c','k'}
    #define PRODUCT_NAME_LEN  14
    #define EP0_SIZE    64
    #define NUM_ENDPOINTS         4
    #define NUM_USB_BUFFERS 24
    #define NUM_INTERFACE   3
    #define SEREMU_INTERFACE      0 // Serial emulation
    #define SEREMU_TX_ENDPOINT    1
    #define SEREMU_TX_SIZE        64
    #define SEREMU_TX_INTERVAL    1
    #define SEREMU_RX_ENDPOINT    2
    #define SEREMU_RX_SIZE        32
    #define SEREMU_RX_INTERVAL    2
    #define MOUSE_INTERFACE       1 // Mouse
    #define MOUSE_ENDPOINT        3
    #define MOUSE_SIZE            8
    #define MOUSE_INTERVAL        1
    #define JOYSTICK_INTERFACE    3 // Joystick
    #define JOYSTICK_ENDPOINT     4
    #define JOYSTICK_SIZE         16
    #define JOYSTICK_INTERVAL     1
    #define ENDPOINT1_CONFIG  ENDPOINT_TRANSIMIT_ONLY
    #define ENDPOINT2_CONFIG  ENDPOINT_RECEIVE_ONLY
    #define ENDPOINT3_CONFIG  ENDPOINT_TRANSIMIT_ONLY
    #define ENDPOINT4_CONFIG  ENDPOINT_TRANSIMIT_ONLY


  in C:\Program Files (x86)\Arduino\hardware\teensy\avr\boards.txt:
  
    teensyLC.menu.usb.mouse_joystick=Mouse + Joystick
    teensyLC.menu.usb.mouse_joystick.build.usbtype=USB_MOUSE_JOYSTICK
    teensyLC.menu.usb.mouse_joystick.fake_serial=teensy_gateway
 */
