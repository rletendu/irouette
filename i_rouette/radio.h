#ifndef __RADIO_H__
#define __RADIO_H__


bool radio_connect(void );
bool radio_disconnect(void );
bool radio_send(char *data );
bool radio_get_param(char *data);
bool radio_sleep(void );


#endif //__RADIO_H__
