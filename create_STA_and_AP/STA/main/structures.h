/*
 * Contains all the structures shared accross at least 2 files
 * 
 */

#ifndef _STRUCTURE_H
#define _STRUCTURE_H

/* ESP32 headers */
#include "esp_wifi.h"
			
typedef int func(void);	// Allow to declare function "hidden" in memory			


typedef struct {
// Header of the raw packet
	/*unsigned frame_ctrl: 16;
	unsigned duration_id: 16;*/
	uint8_t subtype[1];
	uint8_t ctrl_field[1];
	uint8_t duration[2];
	uint8_t addr1[6]; /* receiver address */
	uint8_t addr2[6]; /* sender address */
	uint8_t addr3[6]; /* filtering address */

	uint8_t SC[2];
	uint64_t timestamp_abs;
	uint8_t beacon_interval[2];
	uint8_t capability_info[2];

} wifi_ieee80211_mac_hdr_t;

typedef struct {
// Packet is header + payload
	wifi_ieee80211_mac_hdr_t hdr;
	uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;


#endif








