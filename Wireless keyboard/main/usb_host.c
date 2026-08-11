#include "usb_host.h"
#include "esp_log.h"
#include "usb/usb_host.h"

#include "usb/hid_host.h"
#include "usb/hid_usage_keyboard.h"
#include "usb/hid_usage_mouse.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "keyboard.h"
#include "hid_dev.h"

static const char *TAG = "USB_HOST";

static void usb_lib_task(void *arg);
static void usb_client_task(void *arg);
static void client_event_cb(const usb_host_client_event_msg_t *event_msg, void *arg);

static void hid_host_device_callback(
    hid_host_device_handle_t hid_device_handle,
    const hid_host_driver_event_t event,
    void *arg);

static void hid_host_interface_callback(
    hid_host_device_handle_t hid_device_handle,
    const hid_host_interface_event_t event,
    void *arg);

void usb_host_start(void)
{
    ESP_LOGI(TAG, "USB Host module started");

    usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags = ESP_INTR_FLAG_LEVEL1,
    };

    esp_err_t err = usb_host_install(&host_config);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "USB Host installation failed: %s",
                 esp_err_to_name(err));
        return;
    }
    
    //hid_host_dev_params_t dev_params;



    ESP_LOGI(TAG, "USB Host installed successfully");

    xTaskCreate(
        usb_lib_task,
        "usb_events",
        4096,
        NULL,
        20,
        NULL);

    ESP_LOGI(TAG, "USB Host event task started");

    const hid_host_driver_config_t hid_config = {
        .create_background_task = true,
        .task_priority = 5,
        .stack_size = 4096,
        .core_id = 0,
        .callback = hid_host_device_callback,
        .callback_arg = NULL,
    };

    ESP_ERROR_CHECK(hid_host_install(&hid_config));


    ESP_LOGI(TAG, "HID Host Installed");

    // Leave this disabled for now
    // xTaskCreate(usb_client_task, "usb_client", 4096, NULL, 20, NULL);

    ESP_LOGI(TAG, "USB Client task disabled");
}

    // Keep this for now
   //xTaskCreate(usb_client_task,"usb_client",4096,NULL,20,NULL);

  

static void usb_lib_task(void *arg)
{
    uint32_t event_flags;

    while (1)
    {
        esp_err_t err = usb_host_lib_handle_events(
                            portMAX_DELAY,
                            &event_flags);

        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "USB Host Event Error: %s",
                     esp_err_to_name(err));
        }

        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS)
        {
            ESP_LOGI(TAG, "No USB clients");
        }

        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE)
        {
            ESP_LOGI(TAG, "All USB devices freed");
        }
    }
}

static void usb_client_task(void *arg)
{
    usb_host_client_config_t client_config = {
        .is_synchronous = false,
        .max_num_event_msg = 5,
        .async = {
            .client_event_callback = client_event_cb,
            .callback_arg = NULL,
        },
    };

    usb_host_client_handle_t client_hdl;

    esp_err_t err = usb_host_client_register(
        &client_config,
        &client_hdl);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Client register failed: %s",
                 esp_err_to_name(err));
        vTaskDelete(NULL);
    }

    ESP_LOGI(TAG, "USB Client Registered");

    while (1)
    {
        usb_host_client_handle_events(client_hdl,
                                      portMAX_DELAY);
    }
}

static void client_event_cb(
    const usb_host_client_event_msg_t *event_msg,
    void *arg)
{
    switch (event_msg->event)
    {
        case USB_HOST_CLIENT_EVENT_NEW_DEV:
            ESP_LOGI(TAG,
                     "USB Device Connected! Address = %d",
                     event_msg->new_dev.address);
            break;

        case USB_HOST_CLIENT_EVENT_DEV_GONE:
            ESP_LOGI(TAG,
                     "USB Device Removed!");
            break;

        default:
            break;
    }
}

static void hid_host_device_callback(
    hid_host_device_handle_t hid_device_handle,
    const hid_host_driver_event_t event,
    void *arg)
{
    hid_host_dev_params_t dev_params;

    ESP_ERROR_CHECK(
        hid_host_device_get_params(
            hid_device_handle,
            &dev_params));
          
            ESP_LOGI(TAG, "USB Address : %d", dev_params.addr);
            ESP_LOGI(TAG, "Interface   : %d", dev_params.iface_num);
            ESP_LOGI(TAG, "Subclass    : %d", dev_params.sub_class);
            ESP_LOGI(TAG, "Protocol    : %d", dev_params.proto);
            ESP_LOGI(TAG, "Driver Event: %d", event);

           ESP_LOGI(TAG,
         "Opening interface %d protocol %d",
         dev_params.iface_num,
         dev_params.proto);

    switch (event)
    {
        case HID_HOST_DRIVER_EVENT_CONNECTED:

            ESP_LOGI(TAG,"HID Driver Event = %d",event);

            const hid_host_device_config_t dev_config = {
                .callback = hid_host_interface_callback,
                .callback_arg = NULL,
            };

            ESP_ERROR_CHECK(hid_host_device_open(hid_device_handle,&dev_config));

            size_t desc_len = 0;

uint8_t *desc = hid_host_get_report_descriptor(
    hid_device_handle,
    &desc_len);

if (desc != NULL)
{
    ESP_LOGI(TAG, "========== HID REPORT DESCRIPTOR ==========");
    ESP_LOGI(TAG, "Descriptor Length = %d", desc_len);

    for (size_t i = 0; i < desc_len; i++)
    {
        printf("%02X ", desc[i]);

        if ((i + 1) % 16 == 0)
        {
            printf("\n");
        }
    }

    printf("\n");
}
else
{
    ESP_LOGW(TAG, "Failed to get report descriptor");
}

            if (dev_params.sub_class == HID_SUBCLASS_BOOT_INTERFACE)
            {
             // esp_err_t err = hid_class_request_set_protocol(hid_device_handle,HID_REPORT_PROTOCOL_BOOT);

            //if (err != ESP_OK)
              //{
             //    ESP_LOGW(TAG, "Set Protocol failed: %s",  esp_err_to_name(err));
              //}

if (dev_params.proto == HID_PROTOCOL_KEYBOARD)
{
    esp_err_t err = hid_class_request_set_idle(hid_device_handle,0,0);

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "Set Idle failed: %s",
                 esp_err_to_name(err));
    }
}
            }

            ESP_ERROR_CHECK(
                hid_host_device_start(
                    hid_device_handle));

            ESP_LOGI(TAG,
                     "Keyboard Ready");

            break;

        default:
            break;
    }
}

static void hid_host_interface_callback(
    hid_host_device_handle_t hid_device_handle,
    const hid_host_interface_event_t event,
    void *arg)
{
    switch (event)
    {
        case HID_HOST_INTERFACE_EVENT_INPUT_REPORT:
        {
            uint8_t report[64];
            size_t report_len = 0;

            esp_err_t err = hid_host_device_get_raw_input_report_data(
                hid_device_handle,
                report,
                sizeof(report),
                &report_len);

            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "Failed to read report: %s",
                         esp_err_to_name(err));
                return;
            }

            hid_host_dev_params_t dev_params;

ESP_ERROR_CHECK(
    hid_host_device_get_params(
        hid_device_handle,
        &dev_params));

ESP_LOGI(TAG,
         "INPUT REPORT -> Interface %d  Protocol %d",
         dev_params.iface_num,
         dev_params.proto);

            ESP_LOGI(TAG, "Report length: %d", report_len);

printf("RAW: ");
for (size_t i = 0; i < report_len; i++)
{
    printf("%02X ", report[i]);
}
printf("\n");


           if (dev_params.proto == HID_PROTOCOL_KEYBOARD && report_len == 8)
{
    // Normal keyboard
    keyboard_send_report(report[0], &report[2]);

    ESP_LOGI(TAG,
             "BLE <- USB Keyboard  Mod=%02X Keys=%02X %02X %02X %02X %02X %02X",
             report[0],
             report[2],
             report[3],
             report[4],
             report[5],
             report[6],
             report[7]);
}
else if (dev_params.proto == HID_PROTOCOL_MOUSE && report_len == 3)
{
    ESP_LOGI(TAG,
             "Consumer report: %02X %02X %02X",
             report[0],
             report[1],
             report[2]);

    switch (report[1])
    {
        case 0xE9:   // Volume Up
            keyboard_send_consumer(HID_CONSUMER_VOLUME_UP, true);
            keyboard_send_consumer(HID_CONSUMER_VOLUME_UP, false);
            break;

        case 0xEA:   // Volume Down
            keyboard_send_consumer(HID_CONSUMER_VOLUME_DOWN, true);
            keyboard_send_consumer(HID_CONSUMER_VOLUME_DOWN, false);
            break;

        case 0xE2:   // Mute
            keyboard_send_consumer(HID_CONSUMER_MUTE, true);
            keyboard_send_consumer(HID_CONSUMER_MUTE, false);
            break;

        default:
            ESP_LOGI(TAG, "Unknown Consumer Key: %02X", report[1]);
            break;
    }
}
 
            break;
        }

        case HID_HOST_INTERFACE_EVENT_TRANSFER_ERROR:
            ESP_LOGW(TAG, "Transfer Error");
            break;

        case HID_HOST_INTERFACE_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Keyboard Disconnected");
            break;

        default:
            break;
    }
}