/*
 * gsm_at.h
 *
 *  Created on: Sep 15, 2014
 *      Author: qwer
 */

#ifndef GSM_AT_H_
#define GSM_AT_H_

#include "ul_other.h"

extern u16 s_len (u8 *s, u8 eol);
extern u8  s_lines (u8 *s, u16 len, u8 eol);

// get length of reply
inline u16 at_reply_len(u8 *s){ return s_len(s,0); }
// get number of lines in reply
inline u8 at_reply_lines(u8 *s, u16 len){ return s_lines(s,len,'\r'); }
// check reply validity
u8 at_reply_ok(u8 *s, u16 len);

// find start of status message
u8 *at_status_start(u8 *s, u16 len);
// find length of status message
u8 at_status_len(u8 *s);

// compare status to cmp
u8 at_status_cmp(u8 *s, char *cmp, u16 len);
// check if status is OK
u8 at_status_ok(u8 *s, u16 len);

// check if AT reply and status is OK
u8 at_reply_check_ok(u8 *s);

// reset RX buffer, TX "AT" + cmd + "\r"
void at_tx_cmd(u8 n, const char *cmd);

// TX command with delay
#define AT_CMD(n, cmd, delay_after)\
	do {                           \
		at_tx_cmd(n, cmd);         \
		PTX_DELAY_MS(delay_after); \
		uart_rx_buf_append(n, 0);  \
	} while (0);

#endif /* GSM_AT_H_ */
