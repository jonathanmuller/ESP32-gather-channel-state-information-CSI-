/* WiFi AP / UDP server who responds with packets containing CSI preambles

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "protocol_examples_common.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#define EXAMPLE_ESP_WIFI_SSID      "example_ssid"		// AP SSID name
#define EXAMPLE_ESP_WIFI_PASS      "example_password"	// AP password
#define PORT 3333										// or any other unused port, but same as in udp_client
#define EXAMPLE_MAX_STA_CONN       20
#define LEN_MAC_ADDR 20


static const char *TAG = "wifi softAP";

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
	{
		ESP_LOGI(TAG, "Someone wants to connect!");
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
	}
	else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		ESP_LOGI(TAG, "Someone disconnected...");
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}


/*
 * Setting up a UDP server
 */
static void udp_server_task(void)
{
    char rx_buffer[128];
    char addr_str[128];
    int addr_family;
    int ip_protocol;

    while (1)
	{
		#ifdef CONFIG_EXAMPLE_IPV4
			struct sockaddr_in dest_addr;
			dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			dest_addr.sin_family = AF_INET;
			dest_addr.sin_port = htons(PORT);
			addr_family = AF_INET;
			ip_protocol = IPPROTO_IP;
			inet_ntoa_r(dest_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
		#else // IPV6
			struct sockaddr_in6 dest_addr;
			bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
			dest_addr.sin6_family = AF_INET6;
			dest_addr.sin6_port = htons(PORT);
			addr_family = AF_INET6;
			ip_protocol = IPPROTO_IPV6;
			inet6_ntoa_r(dest_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
		#endif

        int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
        if (sock < 0)
		{
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
		{
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        }
        ESP_LOGI(TAG, "Socket bound, port %d", PORT);

        while (1)
		{
            ESP_LOGI(TAG, "Waiting for data");
            struct sockaddr_in6 source_addr;
            socklen_t socklen = sizeof(source_addr);
            int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

            if (len < 0)
			{
                ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
                break;
            }
            else
			{
                if (source_addr.sin6_family == PF_INET)
				{
                    inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
                }
				else if (source_addr.sin6_family == PF_INET6)
				{
                    inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
                }

                rx_buffer[len] = 0;
                ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
                ESP_LOGI(TAG, "%s", rx_buffer);

                int err = sendto(sock, rx_buffer, len, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
                if (err < 0)
				{
                    ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
                    break;
                }
            }
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
 * Setting up a WiFi AP
 */
void wifi_init_softap(void)
{
	ESP_LOGI(TAG, "Entering wifi_init_softap");
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	cfg.csi_enable = 1;
	ESP_LOGI(TAG, "Starting esp_wifi_init");
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    wifi_config_t wifi_config =
	{
        .ap =
		{
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
	{
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }


	ESP_LOGI(TAG, "Starting esp_wifi_set_mode");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));

    // Set a fixed MAC, Tx AMPDU has to be disabled
    uint8_t mac[6]={0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
    ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_AP, mac));

 	ESP_LOGI(TAG, "Starting esp_wifi_start");
    ESP_ERROR_CHECK(esp_wifi_start());

    // Choose rate
    //esp_err_t esp_wifi_internal_set_fix_rate(wifi_interface_t ifx, bool en, wifi_phy_rate_t rate);
    //ESP_ERROR_CHECK(esp_wifi_internal_set_fix_rate(ESP_IF_WIFI_AP, 1, WIFI_PHY_RATE_11M_S));

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);

	uint8_t mac_STA[6];
	esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_STA);
	char mac_STA_str[LEN_MAC_ADDR] = {0};
	sprintf(mac_STA_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac_STA[0], mac_STA[1], mac_STA[2], mac_STA[3], mac_STA[4], mac_STA[5]);
	ESP_LOGI(TAG, "MAC STA:%s", mac_STA_str);

	uint8_t mac_AP[6];
	ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_AP, mac_AP));
	char mac_AP_str[LEN_MAC_ADDR] = {0};
	sprintf(mac_AP_str, "%02X:%02X:%02X:%02X:%02X:%02X", mac_AP[0], mac_AP[1], mac_AP[2], mac_AP[3], mac_AP[4], mac_AP[5]);
	ESP_LOGI(TAG, "MAC AP:%s", mac_AP_str);

    udp_server_task();
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

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
}
