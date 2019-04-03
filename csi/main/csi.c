/* Scan Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

/*
    This example shows how to use the All Channel Scan or Fast Scan to connect
    to a Wi-Fi network.

    In the Fast Scan mode, the scan will stop as soon as the first network matching
    the SSID is found. In this mode, an application can set threshold for the
    authentication mode and the Signal strength. Networks that do not meet the
    threshold requirements will be ignored.

    In the All Channel Scan mode, the scan will end only after all the channels
    are scanned, and connection will start with the best network. The networks
    can be sorted based on Authentication Mode or Signal Strength. The priority
    for the Authentication mode is:  WPA2 > WPA > WEP > Open
*/
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <string.h>
#include <stdio.h>

#include "structures.h"


#define LEN_MAC_ADDR 20

/*Set the SSID and Password via "make menuconfig"*/
#define DEFAULT_SSID CONFIG_WIFI_SSID
#define DEFAULT_PWD CONFIG_WIFI_PASSWORD

#if CONFIG_WIFI_ALL_CHANNEL_SCAN
#define DEFAULT_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#elif CONFIG_WIFI_FAST_SCAN
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#else
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#endif /*CONFIG_SCAN_METHOD*/

#if CONFIG_WIFI_CONNECT_AP_BY_SIGNAL
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_WIFI_CONNECT_AP_BY_SECURITY
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY
#else
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#endif /*CONFIG_SORT_METHOD*/

#if CONFIG_FAST_SCAN_THRESHOLD
#define DEFAULT_RSSI CONFIG_FAST_SCAN_MINIMUM_SIGNAL
#if CONFIG_EXAMPLE_OPEN
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#elif CONFIG_EXAMPLE_WEP
#define DEFAULT_AUTHMODE WIFI_AUTH_WEP
#elif CONFIG_EXAMPLE_WPA
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA_PSK
#elif CONFIG_EXAMPLE_WPA2
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA2_PSK
#else
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#endif
#else
#define DEFAULT_RSSI -127
#define DEFAULT_AUTHMODE WIFI_AUTH_OPEN
#endif /*CONFIG_FAST_SCAN_THRESHOLD*/

static const char *TAG = "scan";
static bool can_print=1;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "Got IP: %s\n",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ESP_OK;
}

/*
 * This function ONLY receive the CSI preamble of frames (if there is any)
 */

void receive_csi_cb(void *ctx, wifi_csi_info_t *data) {
							
/* 
 * Goal : Get Channel State Information Packets and fill fields accordingly
 * In : Contexte (null), CSI packet
 * Out : Null, Fill fields of corresponding AP
 * 
 */ 

	wifi_csi_info_t received = data[0];
	
	char senddMacChr[LEN_MAC_ADDR] = {0}; // Sender
	sprintf(senddMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", received.mac[0], received.mac[1], received.mac[2], received.mac[3], received.mac[4], received.mac[5]);
	if (received.rx_ctrl.sig_mode==1){
		printf("Received CSI from adress %s\n", senddMacChr); 

		/*
		printf("Following packet :\n");
		printf("rate %d\n", received.rx_ctrl.rate);
		printf("sig_mode %d -> ",received.rx_ctrl.sig_mode);
		if (received.rx_ctrl.sig_mode==0)
			printf("non HT(11bg)\n");
		if (received.rx_ctrl.sig_mode==1)
			printf("HT(11n)\n");
		if (received.rx_ctrl.sig_mode==2)
			printf("UNKNOWN!!!");
		if (received.rx_ctrl.sig_mode==3)
			printf("VHT(11ac)\n");

		printf("HT20 (0) or HT40 (1) : %d\n",received.rx_ctrl.cwb);

		printf("Space time block present : %d\n", received.rx_ctrl.stbc);
		printf("Secondary channel : 0: none; 1: above; 2: below: %d\n", received.rx_ctrl.secondary_channel);
		printf("Length %d\n", received.len);
		printf("Last word is invalid %d\n", received.rx_ctrl.rx_state);
		*/
		
		uint8_t* my_ptr=&(data->buf);
		
		printf("0000 ");
		for(int i=0;i<data->len;i++){
			printf("%02x ", my_ptr[i]);
		}
		printf("\n\n");

	} else {
		printf("This is invalid CSI until Espressif fix issue https://github.com/espressif/esp-idf/issues/2909\n", received.rx_ctrl.sig_mode); 
	}
	
	
}

/*
 * This function receive all frames, would they contain CSI preamble or not.
 * It gets the content of the frame, not the preamble.
 */

void promi_cb(void *buff, wifi_promiscuous_pkt_type_t type) {
	if (can_print){
		can_print=0;
		const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
		const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
		const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
		uint8_t* my_ptr=ipkt;
		
		char senddMacChr[LEN_MAC_ADDR] = {0}; // Sender
		char recvdMacChr[LEN_MAC_ADDR] = {0}; // Receiver
		sprintf(recvdMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", hdr->addr1[0], hdr->addr1[1], hdr->addr1[2], hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
		sprintf(senddMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", hdr->addr2[0], hdr->addr2[1], hdr->addr2[2], hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
		
		/*
		if (ppkt->rx_ctrl.sig_mode>0){
			printf("Received 'ht' packet from %s to %s\n", senddMacChr, recvdMacChr);
			printf("0000 ");
			for (int i=0;i<ppkt->rx_ctrl.sig_len;i++){
				printf("%02x ", my_ptr[i]);
			}
			printf("\n\n");
		}
		*/
	}
	can_print=1;
	
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
    printf("my test\n");
    sleep(1);
    printf("my tes2t\n");

    
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	cfg.csi_enable = 1;	
	
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA)); // Forced to be in AP mode to send frame ...
	
	wifi_promiscuous_filter_t filer_promi;
	wifi_promiscuous_filter_t filer_promi_ctrl;
	
	uint32_t filter_promi_field=WIFI_PROMIS_FILTER_MASK_ALL;
	
	
	// WIFI_PROMIS_CTRL_FILTER_MASK_ALL == (0xFF800000) 
	uint32_t filter_promi_ctrl_field=(0xFF800000); // By setting it to 0xFFFFFFFF we can catch CSI ?! (but error)
	uint32_t filter_event=WIFI_EVENT_MASK_ALL;
   
	filer_promi.filter_mask = filter_promi_field;
	filer_promi_ctrl.filter_mask = filter_promi_ctrl_field;
	
	esp_wifi_set_promiscuous_filter(&filer_promi);
	esp_wifi_set_event_mask(filter_event);
	esp_wifi_set_promiscuous_ctrl_filter(&filer_promi_ctrl);
	
	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
	esp_wifi_set_promiscuous_rx_cb(promi_cb);
	
	ESP_ERROR_CHECK(esp_wifi_set_csi(1));

	wifi_csi_config_t configuration_csi; // CSI = Channel State Information
	configuration_csi.lltf_en = 1;
	configuration_csi.htltf_en = 1;
	configuration_csi.stbc_htltf2_en = 1;
	configuration_csi.ltf_merge_en = 1;
	configuration_csi.channel_filter_en = 1;
	configuration_csi.manu_scale = 0; // Automatic scalling
	//configuration_csi.shift=15; // 0->15
	
	ESP_ERROR_CHECK(esp_wifi_set_csi_config(&configuration_csi));
	ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(&receive_csi_cb, NULL));
	
	wifi_config_t ap_config;
	ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_AP, &ap_config));

	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config)); 
	ESP_ERROR_CHECK(esp_wifi_set_protocol(WIFI_IF_AP, WIFI_PROTOCOL_11B| WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N));
	
	esp_log_level_set("TEST", ESP_LOG_VERBOSE);
	printf("Watch for 'filter_promi_ctrl_field', we can possibly get RAW CSI frames if we disable the error!\n");
	sleep(1);
	
	while(1){
		for(int bandwith=0; bandwith<2;bandwith++){ // HT40- is not avaible for all channels
			int ht_chan=0;
			switch (bandwith){
				case 0:
					ht_chan=WIFI_SECOND_CHAN_NONE;
					break;
				case 1:
					ht_chan=WIFI_SECOND_CHAN_ABOVE;
					break;
				case 2:
					ht_chan=WIFI_SECOND_CHAN_BELOW;
					break;
				default:
					ht_chan=WIFI_SECOND_CHAN_NONE;
			}
			
			for (int chan=1;chan<=10;chan++){
				printf("Switching channel to %d with bandwith [None/above/bellow]=%d\n", chan, bandwith);
				ret=esp_wifi_set_channel(chan, ht_chan);
				ESP_ERROR_CHECK(ret);
				usleep(1*1000*1000);
			}
			
			/*
			esp_wifi_set_channel(6, ht_chan);
			ESP_ERROR_CHECK(ret);
			sleep(5);
			*/
		}
	}
	printf("fin\n");
	
}














