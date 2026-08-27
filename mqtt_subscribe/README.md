# MQTT Subscribe Example
Subscribe using MQTT.   

# Build
```Shell
git clone https://github.com/nopnop2002/esp-idf-http-ota
cd esp-idf-http-ota/mqtt_subscribe
idf.py menuconfig
idf.py build
```

# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/bf9dd7b1-549e-4670-a41f-55eec578b9a3" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/f167971a-fdc6-46e8-bd3a-b71800e73b9d" />

# OTA upgrade using curl interface
```
curl --progress-bar -X POST --data-binary @build/esp-idf-http-ota.bin http://esp32-server.local/update  | tee /dev/null
```

# MQTT Publish using mosquitto-clients
```
sudo apt install mosquitto-clients moreutils
./mqtt_pub.sh
```


