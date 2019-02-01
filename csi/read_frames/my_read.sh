#!/bin/bash
echo "Run this script with ./script.sh or face the illegal option sanction"
INTERFACE="/dev/ttyUSB2"
FILENAME="output_of_minicom.txt"
echo "Launch :" 
echo "rm "$FILENAME" ;minicom -D "$INTERFACE" -C "$FILENAME
cp init.pcap tot.pcap

while true;
do
	cat $FILENAME > /dev/null && break
	sleep 1
done
i=0
last_line="@@@"
while true;
do
	line=$(tail -n 1 $FILENAME)
	#echo $line
	
	if [ "$last_line" = "$line" ]; then
		i=$i
		#echo "the same"
	else
		echo "New packet "$i
		i=$((i+1))
		echo $line > tmp.txt
		text2pcap tmp.txt tmp.pcap -l 127
		mergecap tmp.pcap tot.pcap -w tot.pcap -F pcap
		echo "wireshark -k -i <(tail -c+0 -F tot.pcap)"
		echo "tcpreplay from pcap file"
		tcpreplay -i wlan0mon tmp.pcap
		sleep 1
		
		
	fi
	last_line=$line
done
