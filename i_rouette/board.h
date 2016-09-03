#ifndef __BOARD_H__
#define __BOARD_H__

// Pins Assignement
#define _A0_PIN             A0
#define RAIN_PIN            A1
#define LED_BLUE_PIN        A2
#define LED_GREEN_PIN       A3
#define SDA_PIN             A4
#define SCL_PIN             A5
#define REF25_PIN           A6
#define LDR_PIN             A7

#define RX_PIN              0
#define TX_PIN              1
#define WAKEUP_PIN          2
#define LED1_PIN            3
#define VCC_EN_PIN          4
#define _D5_PIN             5
#define DHT_PIN             6
#define CHARGING_PIN        7
#define CHARGED_PIN         8
#define BUZZER_PIN          9
#define LED_TAIL            10
#define LED0_PWM_PIN        11
#define DBG_TX_PIN          12
#define _D13_PIN            13

#define RTC_INT             0

void board_init(void);
void vcc_sensor_enable(bool en);
void radio_enable(bool en);
enum ChargeState get_charge_status(void);
void cpu_8MHZ(void);
void cpu_250KHZ(void);
void beep(uint8_t nb_beep, bool len);
void led_tail(bool en);
void led_white(bool en);
void led_blue(bool en);
void led_green(bool en);
void all_led_off(void);
void all_led_on(void);

#endif // __BOARD_H__
