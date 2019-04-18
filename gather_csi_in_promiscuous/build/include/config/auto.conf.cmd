deps_config := \
	/home/jonathan/esp/esp-idf/components/app_trace/Kconfig \
	/home/jonathan/esp/esp-idf/components/aws_iot/Kconfig \
	/home/jonathan/esp/esp-idf/components/bt/Kconfig \
	/home/jonathan/esp/esp-idf/components/driver/Kconfig \
	/home/jonathan/esp/esp-idf/components/efuse/Kconfig \
	/home/jonathan/esp/esp-idf/components/esp32/Kconfig \
	/home/jonathan/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/jonathan/esp/esp-idf/components/esp_event/Kconfig \
	/home/jonathan/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/jonathan/esp/esp-idf/components/esp_http_server/Kconfig \
	/home/jonathan/esp/esp-idf/components/esp_https_ota/Kconfig \
	/home/jonathan/esp/esp-idf/components/espcoredump/Kconfig \
	/home/jonathan/esp/esp-idf/components/ethernet/Kconfig \
	/home/jonathan/esp/esp-idf/components/fatfs/Kconfig \
	/home/jonathan/esp/esp-idf/components/freemodbus/Kconfig \
	/home/jonathan/esp/esp-idf/components/freertos/Kconfig \
	/home/jonathan/esp/esp-idf/components/heap/Kconfig \
	/home/jonathan/esp/esp-idf/components/libsodium/Kconfig \
	/home/jonathan/esp/esp-idf/components/log/Kconfig \
	/home/jonathan/esp/esp-idf/components/lwip/Kconfig \
	/home/jonathan/esp/esp-idf/components/mbedtls/Kconfig \
	/home/jonathan/esp/esp-idf/components/mdns/Kconfig \
	/home/jonathan/esp/esp-idf/components/mqtt/Kconfig \
	/home/jonathan/esp/esp-idf/components/nvs_flash/Kconfig \
	/home/jonathan/esp/esp-idf/components/openssl/Kconfig \
	/home/jonathan/esp/esp-idf/components/pthread/Kconfig \
	/home/jonathan/esp/esp-idf/components/spi_flash/Kconfig \
	/home/jonathan/esp/esp-idf/components/spiffs/Kconfig \
	/home/jonathan/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/jonathan/esp/esp-idf/components/unity/Kconfig \
	/home/jonathan/esp/esp-idf/components/vfs/Kconfig \
	/home/jonathan/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/jonathan/esp/esp-idf/components/app_update/Kconfig.projbuild \
	/home/jonathan/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/jonathan/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/jonathan/Desktop/esp32/csi_git/csi_gather/main/Kconfig.projbuild \
	/home/jonathan/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/jonathan/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_TARGET)" "esp32"
include/config/auto.conf: FORCE
endif
ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
