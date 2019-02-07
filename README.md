# Gather CSI (Channel State Information) frames with the use of an ESP32 WiFi chip

The goal of this project is :
- To gather and regroup all information about CSI frames and ESP32 (Partially done)
    Need to validate the informations
- To send CSI request frames (Partially done)
    Able to send any kind of frame, but don't know what CSI request look like
- To receive CSI frames (Done)
    CSI frames are catched and loged (in an unfriendly format for now)
- To localize the ESP32 with those frames (To do)
    Need more progress
- To receive 802.11n frames and transfer them to wireshark (done)
    802.11n frames are catched, logged, and copnverted to pcap (wireshark friendly)
    
    
    
The current understanding of CSI frames mecanism is :
STA sends a TRQ request to the AP
Ap respond with a sounding frame (CSI), which is a "blank" frame
STA receive the "blank" frame, which was modified during its flight
STA sends the "steering matrix" (which is the "angle" the "blank" frame got during its flight)
AP sends following frames with the complementary angle so the STA receive the frames as best as possible

If you have more informations, please let me know so it is (finally ..) possible to continously receive CSI frames with the ESP32 instead of being forced to use the IWL5300

Do not hesitate to contact me (email on my bio/issues) if you have question/suggestions/informations

NB : I have a modified/hacked WiFi library which allow to send any frame but I will NOT upload it. Nevertheless I can explain you approximately how to get the same result
