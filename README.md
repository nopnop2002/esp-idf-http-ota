# esp-idf-http-ota
Minimal esp-idf example of HTTP portal to perform OTA updates of ESP32 in Station mode.

ESP-IDF comes with several OTA sample code examples.   
Since all the samples operate as HTTPS clients, a separate HTTPS server is required.   

An OTA upgrade project that operates as an HTTP server is available [here](https://github.com/Jeija/esp32-softap-ota).   
In that project, the ESP32 acts as an HTTP server.   
OTA updates can be performed via a web browser.   
There is no need to set up a separate HTTPS server.    
However, because it uses SoftAP mode for the Wi-Fi connection, it cannot connect to the Internet.

esp-idf-http-ota uses station mode for Wi-Fi connectivity, enabling access to the Internet.   
For this project, most of the code is used from [this](https://github.com/Jeija/esp32-softap-ota) project.   
The web interface is exactly the same as that of [this](https://github.com/Jeija/esp32-softap-ota) project.   
<img width="557" height="186" alt="Image" src="https://github.com/user-attachments/assets/259df8c7-1f7b-4a44-a72f-9004d9656981" />   

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   

# Hardware requirements
Since OTA partitions are used, 4MB of Flash memory is required.   
It does not work on the 2MB Flash model.   

# How to use
This project uses Station mode for the Wi-Fi connection.   
Therefore, to perform the setup, you must first connect to the ESP32 using USB or UART.   
Once setup is complete, you can subsequently flash new firmware via OTA.   


# Installation of setup
```Shell
git clone https://github.com/nopnop2002/esp-idf-http-ota
cd esp-idf-http-ota/setup
idf.py menuconfig
idf.py flash
```

# Configuration for setup
Set the information of your access point and mDNS hostname.   
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/65cd68a5-c2bf-4d7a-984b-6c8cf8090813" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/d687b55c-b6c9-40dd-928c-4a69f537cb99" />

# OTA upgrade using web interface
Change to a different directory and build the firmware.   
The firmware image (esp-idf-http-ota.bin) is created in the build directory.   
```
cd esp-idf-http-ota/sntp_client
idf.py menuconfig
idf.py build
ls build/*.bin
build/esp-idf-http-ota.bin  build/ota_data_initial.bin
```

Once setup is complete, a USB or UART connection is no longer required for firmware updates.   
However, switch to monitor mode to verify that the OTA has completed successfully.   
```
idf.py monitor
```

Open your browser and enter `esp32-server.local` in the address bar.   
<img width="682" height="415" alt="Image" src="https://github.com/user-attachments/assets/30342425-b17e-470e-88b3-3e874e2fa667" />

Click the Browse button and select "esp-idf-http-ota.bin" from the build folder.   
<img width="682" height="415" alt="Image" src="https://github.com/user-attachments/assets/f599af77-a94e-488d-9f96-18812b09f1ac" />

Pressing the Upload button executes the OTA upgrade.   
<img width="682" height="415" alt="Image" src="https://github.com/user-attachments/assets/1936df1b-aba1-4491-8b45-4f50a8e78a4d" />

Restarting with the new image.   
<img width="682" height="415" alt="Image" src="https://github.com/user-attachments/assets/ed7df8ae-0c50-4e90-826e-66254ebd80a2" />


# OTA upgrade using curl interface
You can use curl instead of a browser.   
```
cd esp-idf-http-ota/sntp_client
idf.py menuconfig
idf.py build
curl --progress-bar -X POST --data-binary @build/esp-idf-http-ota.bin http://esp32-server.local/update  | tee /dev/null
```

<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/d9fb654f-aecb-4660-a146-c6584a32d0d2" />


# If the connection in station mode fails
If the device fails to start in station mode upon startup, it connects in AP mode.   
If the device is restarted after connecting in AP mode, it will attempt to connect in Station mode again.   

- Launch the firmware   
	The firmware attempts to connect in station mode.   

- Failed to connect in station mode   
	The firmware will automatically restart.   
	The firmware attempts to connect in AP mode.   
	The internet is unavailable.   
	You can use OTA via AP mode.   
	The ESP32 broadcasts an open WiFi network with SSID ```ESP32 OTA Update```.   
	Connect to it and open http://192.168.4.1 in your web browser.   

- Reset while in AP mode   
	The firmware attempts to connect in station mode again.   

- Successfully connected in station mode   
	The internet is available.   
	You can use OTA via Station mode.   
	Open http://esp32-server.local in your web browser.   
