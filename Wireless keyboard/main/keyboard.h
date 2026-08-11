#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>   // <-- Add this line

void keyboard_init(void);
void keyboard_set_connection(uint16_t conn_id);
void keyboard_send_key(uint8_t key);
void keyboard_send_report(uint8_t modifier, const uint8_t *keys);
void keyboard_send_consumer(uint8_t key, bool pressed);

#endif