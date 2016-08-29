#ifndef __BOARD_H__
#define __BOARD_H__

void board_init(void);
void sensor_enable(bool en);
void radio_enable(bool en);
enum ChargeState get_charge_status(void);
void cpu_8MHZ(void);
void cpu_250KHZ(void);
void beep(uint8_t nb_beep, bool len);
void led_tail(bool en);
void all_led_off(void);


#endif //__BOARD_H__
