# MQTT Publish Example
Publish using MQTT.   

# Build
```Shell
git clone https://github.com/nopnop2002/esp-idf-http-ota
cd esp-idf-http-ota/mqtt_publish
idf.py menuconfig
idf.py build
```

# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/b4309756-3e70-4d69-9944-08ca23911c0d" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/ba4a8d07-1a0d-440e-bccb-b535ac6fac3c" />

# OTA upgrade using curl interface
```
curl --progress-bar -X POST --data-binary @build/esp-idf-http-ota.bin http://esp32-server.local/update  | tee /dev/null
```

# MQTT Subscrive using mosquitto-clients
```
sudo apt install mosquitto-clients moreutils
./mqtt_sub.sh
```


