/*
 ============================================================================
 Name        : gsm.c
 Author      : Iverson Wang
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "ul_gsm_at.h"
#include "ul_gsm_pdu.h"
#include "ul_s.h"

/* AT+CMGS
 * "08  91  683110801105F0  11          00      0D  91  XXXXXXXXXXXXX  00      00      AA          06              E8329BFD0E01"
 *            SMSC          TP-MTI/VFP  TP-MR       =                   TP-PID  TP-DCS  TP-VP       TP-UDL          TP-UD
 *  =   =                   ?           =       addr len                =       encode  valid time  user data len   user data
 */
#define PDU_DATA "0891683110801105F011000D91XXXXXXXXXXXXX0000AA06E8329BFD0E01"


int main(void) {
    u8 out[1024] = {0};
    int len = 0, i;
	struct pdu_struct pdu;
//	struct sender_struct sender;
//	struct msg_struct mesg;

	memset(&pdu, 0, sizeof(struct pdu_struct));

	len = pdu_out_encode_simple(&pdu, out, "+XXXXXXXXXXXXX", "hello!", 0);

	for (i = 0; i < len; i++) {
	    printf("%c", ((unsigned char *)out)[i]);
	}

	printf("\n");


	return EXIT_SUCCESS;
}
