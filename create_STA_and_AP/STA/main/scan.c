/* Sniffing WiFi Station / UDP client who prints out CSI values which describe the channel
   between both ESP32 boards (client and server)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
//#include "protocol_examples_common.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include "structures.h"

#define EXAMPLE_ESP_WIFI_SSID      "example_ssid"		// connect to specified AP
#define EXAMPLE_ESP_WIFI_PASS      "example_password"	// with specified password
#define HOST_IP_ADDR "192.168.4.1"			 			// or set to whatever IP your AP uses
#define PORT 3333										// or any other unused port, but same as the one used by the UDP server
#define EXAMPLE_ESP_MAXIMUM_RETRY  20
#define LEN_MAC_ADDR 20


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

static const char *TAG = "wifi station";
static const char *payload = "Message from ESP32 ";
static bool can_print = 1;
static int s_retry_num = 0;



static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(TAG, "Starting esp_wifi_connect");
        esp_wifi_connect();
    }
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
		{
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    }
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


/*
 * Catches CSI preambles, prints sender and CSI content
 */
void receive_csi_cb(void *ctx, wifi_csi_info_t *data)
{
	wifi_csi_info_t received = data[0];

	char senddMacChr[LEN_MAC_ADDR] = {0}; // Sender
	sprintf(senddMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", received.mac[0], received.mac[1], received.mac[2], received.mac[3], received.mac[4], received.mac[5]);
	if(strcmp(senddMacChr, "24:00:00:00:00:00") == 0)
		{
        if (received.rx_ctrl.sig_mode==1)
        {
		// This filters for CSI only from your AP board, choose MAC accordingly

			while(can_print == 0){}
			can_print = 0;
			printf("<CSI><address>%s</address><len>%d</len>", senddMacChr, received.len);
			int8_t* my_ptr = received.buf;
			for(int i = 0; i < received.len; i++)
			{
				printf("%d ", my_ptr[i]);
			}
			printf("\n\n");
			can_print = 1;
		} else {
		    printf("This is invalid CSI until Espressif fix issue https://github.com/espressif/esp-idf/issues/2909\n");
		}
	}
	else
	{
        //printf("received and dropped from %s\n", senddMacChr);
	}
}


/*
 * Receives all frames, would they contain CSI preambles or not.
 * It gets the content of the frame, not the preamble.
 */
void promi_cb(void *buff, wifi_promiscuous_pkt_type_t type)
{
	const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
	const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
	const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
	uint8_t* my_ptr=ipkt;

	char senddMacChr[LEN_MAC_ADDR] = {0}; // Sender
	char recvdMacChr[LEN_MAC_ADDR] = {0}; // Receiver
	sprintf(recvdMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", hdr->addr1[0], hdr->addr1[1], hdr->addr1[2], hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
	sprintf(senddMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", hdr->addr2[0], hdr->addr2[1], hdr->addr2[2], hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);

	// This filters for packets between your two boards
	// Choose MACs accordingly, or simply ignore
	if(strcmp(senddMacChr, "24:00:00:00:00:00") == 0 && strcmp(recvdMacChr, "24:01:01:01:01:01") == 0)
	{
		while (can_print == 0){}
		can_print = 0;
		printf("Sender: %s, Receiver: %s \n", senddMacChr, recvdMacChr);
		can_print = 1;
	} else {
	    //printf("received and dropped from %s %s\n", senddMacChr, recvdMacChr);
	}
}


/*
 * Connects to the UDP server on the other ESP32 and repeatedly sends UDP packets,
 * so that the server responds with packets containing CSI preambles
 */
static void udp_client_task(void)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1)
	{
        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
		{
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

        while (1)
		{
            int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
            if (err < 0)
			{
                ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                break;
            }

            struct sockaddr_in source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0)
			{
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }

            else
			{
			    printf("Got a response !\n");
				// We got a response! No need to do anything with it though.
                // rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                // ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                // ESP_LOGI(TAG, "%s", rx_buffer);
            }

            vTaskDelay(2*1000 / portTICK_PERIOD_MS); // time between packets in milliseconds
        }

        if (sock != -1)
		{
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
}


/*
 * Connects to the WiFi AP on the other ESP32
 */
void wifi_init_sta(void)
{
	ESP_LOGI(TAG, "Entering wifi_init_sta");
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	cfg.csi_enable = 1;
	ESP_LOGI(TAG, "Starting esp_wifi_init");
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config =
	{
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };

	ESP_LOGI(TAG, "Starting esp_wifi_set_mode");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    uint8_t mac[6]={0x24, 0x01, 0x01, 0x01, 0x01, 0x01};
    ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, mac));
	ESP_LOGI(TAG, "Starting esp_wifi_start");
    ESP_ERROR_CHECK(esp_wifi_start() );

	wifi_promiscuous_filter_t filer_promi;
	wifi_promiscuous_filter_t filer_promi_ctrl;

	uint32_t filter_promi_field = WIFI_PROMIS_FILTER_MASK_ALL;
	uint32_t filter_promi_ctrl_field = (0xFF800000);
	uint32_t filter_event=WIFI_EVENT_MASK_ALL;

	filer_promi.filter_mask = filter_promi_field;
	filer_promi_ctrl.filter_mask = filter_promi_ctrl_field;

	esp_wifi_set_promiscuous_filter(&filer_promi);
	esp_wifi_set_event_mask(filter_event);
	esp_wifi_set_promiscuous_ctrl_filter(&filer_promi_ctrl);

	ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
	esp_wifi_set_promiscuous_rx_cb(promi_cb);

	ESP_ERROR_CHECK(esp_wifi_set_csi(1));

	// Set CSI configuration to whatever suits you best
	wifi_csi_config_t configuration_csi;
	configuration_csi.lltf_en = 1;
	configuration_csi.htltf_en = 1;
	configuration_csi.stbc_htltf2_en = 1;
	configuration_csi.ltf_merge_en = 1;
	configuration_csi.channel_filter_en = 1;
	configuration_csi.manu_scale = 0;
	//configuration_csi.shift = 0;

	ESP_ERROR_CHECK(esp_wifi_set_csi_config(&configuration_csi));
	ESP_ERROR_CHECK(esp_wifi_set_csi_rx_cb(&receive_csi_cb, NULL));

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

    // Choose rate
    //esp_err_t esp_wifi_internal_set_fix_rate(wifi_interface_t ifx, bool en, wifi_phy_rate_t rate);
    //ESP_ERROR_CHECK(esp_wifi_internal_set_fix_rate(WIFI_MODE_STA, 1, WIFI_PHY_RATE_MCS7_SGI));

	udp_client_task();
}


void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
}








