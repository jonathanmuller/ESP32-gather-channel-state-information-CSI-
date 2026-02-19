# 🚨 This project is no longer maintained
> **It has been replaced by the more advanced [ESP-PPB](https://github.com/jonathanmuller/esp-ppb/).**

# Gather CSI (Channel State Information) frames with the use of an ESP32 WiFi chip

This project allows to extract WiFi CSI (Channel State Information) frames from compatible AP by using an ESP32 chip. Those CSI frames can be processed in order to precisely localize it using phase aggregation or fingerprinting.

### A priori
This project is under active developement, do not hesitate to contact me (email on my bio/issues) if you have question, suggestions, informations or spotted errors, I usually respond within few days.


### Actual progress
The goal of this project is :
- To send 802.11n frames (So AP respond with CSI frames) 
   * Partially done : CSI frames consistently works between 2 ESP32 boards. It should also work with any commercial AP. The only part left is to send CSI without being connected
   * You can help by finding a way to use esp_wifi_80211_tx at the same time as the esp_wifi_internal_set_fix_rate private function
- To receive CSI frames 
   * Done : CSI frames are catched and loged (in an unfriendly format for now)
   * You can help by understanding why some frames are dropped by filter_promi_ctrl_field (see "WIFI_PROMIS_CTRL_FILTER_MASK_ALL" in code)
- To localize the ESP32 with those frames 
   * To do : Need more progress
- To receive 802.11n frames and transfer them to wireshark 
   * Done : 802.11n frames are catched, logged, and converted to pcap (wireshark friendly)
- To gather and regroup all information about CSI frames and ESP32 
   * Partially done : Need to validate the informations
   * You can help by verifying the informations
 
### How to use 

First figure how to upload a code on your ESP32 board (it should be easy, many tutorials are available, including a detailed one on Espressif github : https://docs.espressif.com/projects/esp-idf/en/stable/get-started/), then you can use one of the folliwng folders :

- "create_STA_and_AP" -> Using 2 ESP32 boards connected over WiFi. One send an UDP packet, which triggers the CSI callbal on the other.
- "gather_csi_in_promiscuous" -> Loop across the 13 WiFi channels and print any CSI frame
- "read_non_csi_frames" -> Print NON-CSI frames in HEX (so you can dump them and see them on WireShark for example)
- "request_csi_frames" -> (Missing esp_wifi_80211_tx call) Send a OFDM/802.11n to an AP which should answer with a CSI frame
    
    
### Current understanding
- CSI is a preamble to frames. 
- It is used to determin the "noise" between the STA and the AP (which is assumed to be the same as the AP to the STA)
- It is a specificity of the 802.11n mode, which is enabled only under certain conditions.
Exemple :
STA send a frame to the AP at speed > 6mb/s [which is OFDM]
AP respond with a frame at speed > 6 mb/s [which is OFDM] -> The ESP32 extract the CSI header
This means that to generate CSI preamble with an ESP32 you need to be connected to the AP, else you can't send at >6mb/s ans there will be no CSI preamble. You can't only use "esp_wifi_80211_tx" without being connected (because it will only send at 1 mb/s). So you also need to use the hidden "esp_wifi_internal_set_rate" function.

If you have more informations, please let me know so it is (finally ..) possible to continously receive CSI frames from any AP without being connected to it

### BUG
There is currently a bug in Espressif SDK where CSI callback is triggered instead of promiscuous callback.
See issue https://github.com/espressif/esp-idf/issues/2909

This means for now I have to apply a filter on rx_ctrl.sig_mode so only valid CSI frames are shown (thanks @kraoa).

If your code is dater before 03.04.2019 your results may be affected.
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
### Results
![Alt text](plot_csi_data/amplitude.png?raw=true "Title")
![Alt text](plot_csi_data/phase.png?raw=true "Title")


### Extra
NB : I have a modified/hacked WiFi library which allow to send any frame but I will NOT upload it. Nevertheless I can explain you approximately how to get the same result
