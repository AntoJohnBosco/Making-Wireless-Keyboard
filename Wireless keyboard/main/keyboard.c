#include "keyboard.h"
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_hidd_prf_api.h"

static uint16_t hid_conn_id;

void keyboard_set_connection(uint16_t conn_id)
{
    hid_conn_id = conn_id;
}

void keyboard_init(void)
{
    /* Nothing to initialize yet */
}

void keyboard_send_key(uint8_t key)
{
    esp_hidd_send_keyboard_value(hid_conn_id, 0, &key, 1);

    vTaskDelay(pdMS_TO_TICKS(100));

    esp_hidd_send_keyboard_value(hid_conn_id, 0, NULL, 0);
}

void keyboard_send_report(uint8_t modifier, const uint8_t *keys)
{
    esp_hidd_send_keyboard_value(
        hid_conn_id,
        modifier,
        (uint8_t *)keys,
        6
    );
}
void keyboard_send_consumer(uint8_t key, bool pressed)
{
    esp_hidd_send_consumer_value(
        hid_conn_id,
        key,
        pressed
    );
}