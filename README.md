# SSH Serial

This firmware will allow one to connect to an ESP32 via SSH and monitor serial values on UART1. The purpose is to allow monitoring a serial console of an SBC when doing kernel development, when the target device's network connection might not be ready yet.

The pins for UART1 can be configured in the menuconfig. Wifi provisioning is done as a hostpot you connect to.
All escape sequences will be forwarded to the target, except ^R and ^P, which will signal the ESP32 to toggle the reset and power on/off pin of the SBC, respectively. These pins can also be configured in the menuconfig.


