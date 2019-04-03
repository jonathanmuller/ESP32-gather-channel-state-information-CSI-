# Gather CSI (Channel State Information) frames with the use of an ESP32 WiFi chip

This project allows to extract WiFi CSI (Channel State Information) frames from compatible AP by using an ESP32 chip. Those CSI frames can be processed in order to precisely localize it using phase aggregation or fingerprinting.

### A priori
This project is under active developement, do not hesitate to contact me (email on my bio/issues) if you have question, suggestions, informations or spotted errors


### Actual progress
The goal of this project is :
- To send CSI request frames 
   * Partially done : Able to send any kind of frame, but don't know what CSI request look like
   * You can help by finding the structure of a CSI request frame (see in folder "request_csi" for more details)
- To receive CSI frames 
   * Done : CSI frames are catched and loged (in an unfriendly format for now)
   * You can help by understanding why some frames are dropped by filter_promi_ctrl_field (see source code)
- To localize the ESP32 with those frames 
   * To do : Need more progress
- To receive 802.11n frames and transfer them to wireshark 
   * Done : 802.11n frames are catched, logged, and copnverted to pcap (wireshark friendly)
   * You can help understand the structure of CSI frames gathered (see in folder "read_frames" for details)
- To gather and regroup all information about CSI frames and ESP32 
   * Partially done : Need to validate the informations
   * You can help by verifying the informations
 
### How to use 

1) Install Espressif SDK
    ```
    Follow Espressif installation totorial (Windows, Mac and Linux avaible) 
    https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html
    ```

2) Get a copy of this project
    ```
    git clone https://github.com/jonathanmuller/ESP32-gather-channel-state-information-CSI-.git
    ```

3) Source espressif variables
    ```
    export PATH="$HOME/esp/xtensa-esp32-elf/bin:$PATH" if you have default conf.
   ```

4) Adapt the project to your conf
   ```
   execute "make menuconfig" in the repo, and select your interface 
   (in Serial Flasher Config->Default serial port)
    ```

5) Build and flash the project
    ```
    make flash monitor (you should now see the raw dump of the frames in hexa)
    ```

6) Execute script to get frames (csi/read_frames)
    ```
    ./my_read.sh
    ```
    (in an other terminal and in the script folder execute) 
    ```
    rm output_of_minicom.txt ;minicom -D /dev/ttyUSB3 -C output_of_minicom.txt
    ```
    
The frames are stored in "tot.pcap"
CSI frames are stored in "output_of_minicom.txt" (they will be stored in a more friendly format soon)
    
    
    
### Current understanding
- CSI is a preamble to frames. 
- It is used to determin the "noise" between the STA and the AP (which is assumed to be the same as the AP to the STA)
- It is a specificity of the 802.11n mode, which is enabled only under certain conditions.
Exemple :
STA send a frame to the AP at speed > 6mb/s [which is OFDM]
AP respond with a frame at speed > 6 mb/s [which is OFDM] -> The ESP32 extract the CSI header
This means that to generate CSI preamble with an ESP32 you need to be connected to the AP, else you can't send at >6mb/s ans there will be no CSI preamble. You can't only use "esp_wifi_80211_tx" without being connected (because it will only send at 1 mb/s)

If you have more informations, please let me know so it is (finally ..) possible to continously receive CSI frames with the ESP32 instead of being forced to use the IWL5300

### Working method
One method which was proved working is to :
0) Be sure that mode 11b/g/n are activated
1) Connect the ESP32 to a STA (otherwise you won't be able to achieve >1mb/s, which is too low for 11g/n to activate). This can be an other ESP32
2) Send RAW null data frame (or probably whetever frame you want) to the AP (use "esp_wifi_80211_tx")
3) As both are connected over 11n, the frame will be HT and contain CSI informations
3) AP should respond with an ACK embemed in a HT frame, containing CSI data (in theory)

### BUG
There is currently a bug in Espressif SDK where CSI callback is triggered instead of promiscuous callback.
See issue https://github.com/espressif/esp-idf/issues/3165
This means for now only CSI received between the ESP32 and the AP it is connected to are safe (hopefully) under the condition that monitor mode is disabled

### FAQ
Can the ESP32 do the same as the IWL5300 ?
```
No, it physically can't. IWL5300 has multiple antennas while the ESP32 only has one
```
I don't receive CSI frames
```
Normally this code scan across the 13 avaible channels, is it so ? 
If yes, maybe your AP don't send any CSI frames
```
I dont understand what is in the CSI frame
```
Please see
https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/wifi.html?highlight=channel%20state%20information#wi-fi-channel-state-information
```
How can I access X (header, gain control, raw info, ...)
```
It is very specific, you can suggest me to add this information by "issue" or by contacting me via email 
```

### Extra
NB : I have a modified/hacked WiFi library which allow to send any frame but I will NOT upload it. Nevertheless I can explain you approximately how to get the same result
