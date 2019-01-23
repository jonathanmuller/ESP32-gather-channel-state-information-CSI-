# Gather CSI (Channel State Information) frames with the use of an ESP32 WiFi chip
The goal of this project is :
- To gather and regroup all information about CSI frames and ESP32
- To send CSI request frames
    If anyone know the exact content of a TRQ frame let me know so I can send one
- To receive CSI frames 
    This works and is implemented on Espressif SDK (see example)
- To localize the ESP32 with those frames
    TODO
    
    
The current understanding of CSI frames mecanism is :
STA sends a TRQ request to the AP
Ap respond with a sounding frame (CSI), which is a "blank" frame
STA receive the "blank" frame, which was modified during its flight
STA sends the "steering matrix" (which is the "angle" the "blank" frame got during its flight)
AP sends following frames with the complementary angle so the STA receive the frames as best as possible

If you have more informations, please let me know so it is (finally ..) possible to continously receive CSI frames with the ESP32 instead of being forced to use the IWL5300
