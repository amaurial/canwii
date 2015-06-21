# esp8266-at
Custom esp8266 at commands.
This project takes the AT command line project and changes the protocol to more suitable for communication with oder microprocessors.
Basically it disappears with AT+COMMAND and creates a single byte for each command.
It returns a single byte for OK or ERROR.
The esp serial baud is set by default at 115200.
For protocol details see canwii_protocol.txt
