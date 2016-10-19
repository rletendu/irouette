#ifndef __BOARD_H__
#define __BOARD_H__

// Pins Assignement
#define LED_BLINK_PIN       A0
#define RAIN_PIN            A1
#define LED_BLUE_HEAD_PIN   A2
#define LED_GREEN_HEAD_PIN  A3
#define SDA_PIN             A4
#define SCL_PIN             A5
#define REF25_PIN           A6
#define LDR_PIN             A7

#define RX_PIN              0
#define TX_PIN              1
#define WAKEUP_PIN          2
#define LED_GREEN_LEFT_PIN  3
#define VCC_EN_PIN          4
#define RPM_CAPTURE_PIN     5
#define DHT_PIN             6
#define LED_GREEN_RIGHT_PIN 7
#define LED_RED_RIGHT_PIN   8
#define BUZZER_PIN          9
#define LED_TAIL_PIN        10
#define LEDS_WHITE_PIN      11
#define DBG_TX_PIN          12
#define LED_RED_LEFT_PIN    13

#define RTC_INT             0

void board_init(void);
void vcc_sensor_enable(bool en);
enum ChargeState get_charge_status(void);
void cpu_8MHZ(void);
void cpu_250KHZ(void);
void beep(uint8_t nb_beep, bool len);
void led_blink(bool en);
void led_tail(bool en);
void led_white(bool en);
void led_blue_head(bool en);
void led_green_head(bool en);

void led_red_left(bool en);
void led_red_right(bool en);
void led_green_right(bool en);
void led_green_left(bool en);

void led_error_code(uint8_t code);

void all_led_off(void);
void all_led_on(void);

#endif // __BOARD_H__
