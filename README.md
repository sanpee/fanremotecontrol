# fanremotecontrol

**Prerequisites:** 
* Install esp32 in Arduino (https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) 
* Other libraries needed
	* Add from Library Manager
		* rc-switch (https://github.com/sui77/rc-switch)
		* U8g2 Library (https://github.com/olikraus/u8g2/)
		* WiFiManager (https://github.com/tzapu/WiFiManager)
	* Add by zip
		* ESPAynscWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
		* AsyncTCP (https://github.com/me-no-dev/AsyncTCP)
		* QRCode (https://github.com/ricmoo/qrcode/)
	
Upload all files in FanControlApp/data folder to esp32's flash, refer to  https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/ 

Notes:
* For Heltec Wifi Kit 32, please install also Heltec ESP32 Dev Board Library using Library Manager (https://github.com/HelTecAutomation/Heltec_ESP32)
