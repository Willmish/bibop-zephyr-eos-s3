# README for using an ESP32 board as a USB2UART interface for the quickfeather

1. Connect ESP32 TX2 (GPIO17) and RX2 (GPIO16) to QuickFeathers RX and TX pins.
2. Connect QuickFeather to power/PC and ESP32 to your PC
3. Open a Serial Port to ESP32 at baudrate `115200`:
```
picocom /dev/tty.usbserial-0001 -b 115200
```
4. You should be greeted by a message: `ESP32: becoming a serial copier:`.

