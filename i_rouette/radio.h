#ifndef __RADIO_H__
#define __RADIO_H__

enum radio_status {
  RADIO_OK,
  RADIO_FAIL,
  RADIO_WAIT
};

bool radio_connect(void );
bool radio_disconnect(void );
bool radio_send(char *data );
bool radio_get_param(char *data);
bool radio_sleep(void );
bool radio_ping(void);
bool radio_sync(void);
enum radio_status get_radio_status(long timeout, char *data);


#endif //__RADIO_H__
