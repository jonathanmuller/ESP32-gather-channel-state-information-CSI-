# Gather CSI (Channel State Information) frames with the use of an ESP32 WiFi chip

!!! This project is under active developement, do not hesitate to contact me (email on my bio/issues) if you have question/suggestions/informations !!!


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
 
How to use :


1) Install Espressif SDK
    
    Follow Espressif installation totorial (Windows, Mac and Linux avaible) : https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html


2) Get a copy of this project
    
    git clone https://github.com/jonathanmuller/ESP32-gather-channel-state-information-CSI-.git
    

3) Source espressif variables
    
    export PATH="$HOME/esp/xtensa-esp32-elf/bin:$PATH" if you have default conf.
   

4) Adapt the project to your conf
   
   execute "make menuconfig" in the repo, and select your interface in Serial Flasher Config->Default serial port


5) Build and flash the project
    
    make flash monitor (you should now see the raw dump of the frames in hexa)
    

6) Execute script to get frames (csi/read_frames)
    
    ./my_read.sh
    
    (in an other terminal and in the script folder execute) rm output_of_minicom.txt ;minicom -D /dev/ttyUSB3 -C output_of_minicom.txt
    
    
The frames are stored in "tot.pcap"
CSI frames are stored in "output_of_minicom.txt" (they will be stored in a more friendly format soon)
    
    
    
    
The current understanding of CSI frames mecanism is :
STA sends a TRQ request to the AP
Ap respond with a sounding frame (CSI), which is a "blank" frame
STA receive the "blank" frame, which was modified during its flight
STA sends the "steering matrix" (which is the "angle" the "blank" frame got during its flight)
AP sends following frames with the complementary angle so the STA receive the frames as best as possible

If you have more informations, please let me know so it is (finally ..) possible to continously receive CSI frames with the ESP32 instead of being forced to use the IWL5300


NB : I have a modified/hacked WiFi library which allow to send any frame but I will NOT upload it. Nevertheless I can explain you approximately how to get the same result
