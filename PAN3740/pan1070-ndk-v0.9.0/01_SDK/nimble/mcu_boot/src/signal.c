#include "soc_api.h"
#include "app_log.h"
#include "signal.h"
#include "flash_manager.h"
#include "prf_ota.h"
#include "comm_prf.h"
#include "usb_dfu.h"

bool sig_key1_push_down(void)
{
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P0, BIT6);
    SYS_delay_10nop(10000);

    for (uint16_t i = 0; i < 1000; i++) {
        if (P06 == 1) {
            return false;
        }
    }
    return true;
}

bool sig_key2_push_down(void)
{
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_EnablePullupPath(P1, BIT2);
    SYS_delay_10nop(10000);

    for (uint16_t i = 0; i < 1000; i++) {
        if (P12 == 1) {
            return false;
        }
    }

    return true;
}

bool sig_usb_dfu_enter_check(void)
{
    return is_dfu_flag_valid();
}

bool sig_ota_start_received(void)
{
    return panchip_prf_ota_start();
}

bool sig_back_up_is_completed_image(void)
{
    APP_LOG_INFO("Check valid image in App Backup Partition..\n");

    return fm_image_completed_check(FLASH_AREA_BACK_UP_START);
}

/* recovery gpio status that you used to trigger signal */
void sig_hardware_recovery(void)
{
    // Reset P06 (KEY1) to default state
    GPIO_DisablePullupPath(P0, BIT6);
    GPIO_SetMode(P0, BIT6, GPIO_MODE_INPUT);
    GPIO_DisableDigitalPath(P0, BIT6);

    // Reset P12 (KEY2) to default state
    GPIO_DisablePullupPath(P1, BIT2);
    GPIO_SetMode(P1, BIT2, GPIO_MODE_INPUT);
    GPIO_DisableDigitalPath(P1, BIT2);
}
