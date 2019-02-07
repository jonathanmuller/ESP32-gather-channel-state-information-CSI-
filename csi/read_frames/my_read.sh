#!/bin/bash
echo "Run this script with ./script.sh or face the 'illegal option' sanction"
INTERFACE="/dev/ttyUSB3"
FILENAME="output_of_minicom.txt"
echo "#######################################################################################################################"
echo "#######################################################################################################################"
echo "#######################################################################################################################"
echo "    Execute this command in an other terminal, in the same folder as this script :" 
echo "    rm "$FILENAME" ;minicom -D "$INTERFACE" -C "$FILENAME
echo "#######################################################################################################################"
echo "#######################################################################################################################"
echo "#######################################################################################################################"
cp init.pcap tot.pcap
rm $FILENAME
while true;
do
	cat $FILENAME > /dev/null && break
	sleep 1
done
i=0
last_line="@@@"
while true;
do
	sleep 0.1
	line=$(tail -n 1 $FILENAME)
	line_start=$(echo $line | cut -c1-5)
	if [ "$line_start" = "0000 " ]; then
		status="OK"
	else
		continue
	fi
	
	if [ "$last_line" = "$line" ]; then
		i=$i
		#echo "the same"
	else
		echo "New packet "$i
		i=$((i+1))
		echo $line > tmp.txt
		text2pcap tmp.txt tmp.pcap -l 105
		res=$(mergecap tmp.pcap tot.pcap -w tot_temp.pcap -F  pcap 2>&1 >/dev/null)
		echo "The response is" $res
		my_len=$(echo $res | wc -c)
		echo "With len " $my_len 
		#echo "wireshark -k -i <(tail -c+0 -F tot.pcap)"
		#echo "tcpreplay from pcap file"
		#tcpreplay -i wlan0mon tmp.pcap
		if (($my_len >> 5)); then
			break
		else
			mv tot_temp.pcap tot.pcap 
		fi
		
	fi
	last_line=$line
done
echo "End of script"
