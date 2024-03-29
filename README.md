# fanremotecontrol

![image](images/FanRemoteControl.jpg)

**Hardware Design #1**

| Top                          |  Bottom                      |
|------------------------------|------------------------------|
|![image](images/Design1-1.jpg)|![image](images/Design1-2.jpg)|

**Hardware Design #2**

| Top                          |  Bottom                      |
|------------------------------|------------------------------|
|![image](images/Design2-1.jpg)|![image](images/Design2-2.jpg)|

**TX/RX Hardware**
|  Brand/Model |  TX  | RX |
|--------|-------------------------|-------------------------|
| Type 1 |![image](images/TX-1.jpg)|![image](images/RX-1.jpg)|
| Type 2 |![image](images/TX-2.jpg)|![image](images/RX-2.jpg)|


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
