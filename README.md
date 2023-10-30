# Lab-5
Lab  5 code of EDL

This project used a ESP32 to host a web socket and send movement and camera information from the webserver. The esp32 can be configured as an AP or it can use another AP. 
The best results are when the esp32 is attached to another AP. Camera info is sent over TCP/ACP as well as the other data. The refresh rate depends on the quality of the connection
of the ESP32 and the AP. 
The arduino code just decodes the ESP32's serial commands sent, moving in 4 directions and playing sound. Very simple code.
