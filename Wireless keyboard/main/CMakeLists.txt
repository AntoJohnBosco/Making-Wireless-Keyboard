idf_component_register(
    SRCS
        "ble_hidd_demo_main.c"
        "keyboard.c"
        "usb_host.c"
        "esp_hidd_prf_api.c"
        "hid_dev.c"
        "hid_device_le_prf.c"
    INCLUDE_DIRS "."
    REQUIRES bt esp_wifi nvs_flash usb 
    PRIV_REQUIRES esp_driver_gpio
)