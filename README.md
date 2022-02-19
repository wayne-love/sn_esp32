# SpaNet MQTT ESP32 bridge

SpaNet serial to mqtt bridge, including HomeAssitant autodiscovery.

Developed for the ESP32 Dev board but should work on any ESP32 platform. By default uses UART2 for communications with the SpaNet controller.

## Circuit
To keep things as simple as possible, off the shelf modules have been used.  
NOTE: The resitors on the RX/TX pins are recommended but optional.  


![Circuit](circuit/circuit.png)![Assembly](images/board.png){ height:360px }

 ![Compontents](images/disassembled.png){ height:360px } ![Wiring](images/wiring.png){ height:360px }
