# SNTP Client Example
Retrieves the current time from an NTP server.   

# Build
```Shell
git clone https://github.com/nopnop2002/esp-idf-http-ota
cd esp-idf-http-ota/sntp_client
idf.py menuconfig
idf.py build
```

# Configuration
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/78cd8cd7-cef8-48de-a5b8-cf9656d5f8d9" />
<img width="659" height="486" alt="Image" src="https://github.com/user-attachments/assets/7ce72ef0-7b78-4b1c-8f9b-d8c64d96018f" />

# OTA upgrade using curl interface
```
curl --progress-bar -X POST --data-binary @build/esp-idf-http-ota.bin http://esp32-server.local/update  | tee /dev/null
```


