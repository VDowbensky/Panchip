#ifndef __LCD_UI_H
#define __LCD_UI_H
#include "PANSeries.h"
#include "comm_prf.h"

#define UI_PARAM_INDEX_FREQ                 0
#define UI_PARAM_INDEX_POWER                1
#define UI_PARAM_INDEX_PHY                  2
#define UI_PARAM_INDEX_LENGTH               3
#define UI_PARAM_INDEX_DEVIATION            4
#define UI_PARAM_INDEX_TEST_MODE            5
#define UI_PARAM_INDEX_START_KEY            6
#define UI_PARAM_INDEX_MAX                  7

extern uint16_t ui_prf_channel;
extern int8_t ui_prf_power;
extern uint8_t ui_prf_phy;

void init_led(void);
void init_lcd(void);
void delay_us(uint32_t nus);

void prf_config_updated_from_ui(pan_prf_config_t* config);
void prf_packet_updated_from_ui(panchip_prf_payload_t* config);

bool ui_get_ready_test(void);
void update_test_result_ui(void);

bool is_b250k_test_mode(void);
bool is_297_test_mode(void);

typedef enum {NORMAL_TEST_MODE = 0, XN297_TEST_MODE, B250K_TEST_MODE, TEST_MODE_MAX} test_mode_t;

#endif
