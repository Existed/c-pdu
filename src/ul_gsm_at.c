/*
 * gsm_at.c
 *
 *  Created on: Sep 15, 2014
 *      Author: qwer
 */

#include "ul_s.h"
#include "ul_gsm_at.h"

u8 at_reply_ok(u8 *s, u16 len){
	if (len == 0) len = s_len(s,0);
	return ((len >= 4)&&(s[len-2] == '\r')&&(s[len-1] == '\n'));
}

u8 *at_status_start(u8 *s, u16 len){
	if (len == 0) len = s_len(s, 0);
	u8 lines = s_lines(s, len, '\r');
	if (lines >= 1) return s_skip_crlf(s_line_start(s, len, lines-1, '\r'));
	return NULL;
}
u8 at_status_len(u8 *s){ return s_len(s, '\r'); }

u8 at_status_cmp(u8 *s, char *cmp, u16 len){
	if (len == 0){
		s = at_status_start(s,0);
		len = s_len(s, '\r');
	}
	u8 len1 = s_len((u8 *)cmp,0);
	return ((len == len1)&&(s_cmp_upcase(s, (u8 *)cmp, len)));
}

u8 at_status_ok(u8 *s, u16 len){ return at_status_cmp(s,"OK",len); }

u8 at_reply_check_ok(u8 *s){ return at_reply_ok(s, at_reply_len(s)) && at_status_ok(s,0); }

void at_tx_cmd(u8 n, const char *cmd){
	uart_rx_reset(n);
	uart_tx_c(n, 'A');
	uart_tx_c(n, 'T');
	uart_tx_s(n, cmd);
	uart_tx_c(n, '\r');
}
