/*
 * gsm_pdu.h
 *
 *  Created on: Sep 15, 2014
 *      Author: qwer
 */

#ifndef GSM_PDU_H_
#define GSM_PDU_H_


// data coding scheme
enum {PDU_DCS_7=0, PDU_DCS_8=0x4, PDU_DCS_UCS2=0x08, PDU_DCS_AUTO=0xFF};
// number format
enum {PDU_TYPE_NATIONAL=161, PDU_TYPE_INTERNATIONAL=145, PDU_TYPE_NETWORK=177};

struct pdu_struct {
	// SMSC number
	struct{ u8 len, bytes, type, data[14]; } smsc;
	// caller/sender number
	struct{	u8 len, bytes, type, data[14]; } sender;
	// input/output zero-terminated message (7bit/8bit/UTF8)
	struct{ u16 len, bytes; u8 *data; } msg;

	u8 first;      // 1st octet of the SMS-SUBMIT message (0x11)
	u8 tp_msg_ref; // TP-Message-Reference. The "00" value here lets the phone set the message  reference number itself
	u8 tp_pid;     // Protocol identifier (0)
	u8 tp_dcs;     // Data coding scheme.This message is coded  according to the 7bit default alphabet. Having "02" instead of "00" here, would  indicate that the TP-User-Data field of this message should be interpreted as  8bit rather than 7bit (used in e.g. smart messaging, OTA provisioning etc)
	u8 tp_vp;      // TP-Validity-Period. "AA" means 4 days. Note: This  octet is optional, see bits 4 and 3 of the first  octet

	// incoming SMS timestamp
	u8 year, month, date; // date
	u8 hour, min, sec;    // time
	u8 tz;                // zone

	// PDU length in bytes and for CMGS command
	u16 len_bytes, len_cmgs;
};


// pack 7-bit array to 8-bit
u8 pdu_7to8(u8 *a, u8 len);
// unpack 8-bit array to 7-bit
u8 pdu_8to7(u8 *a, u8 len);

// convert HEX line to bytes, len - length of input buffer
void pdu_hex2bytes(u8 *h, u16 len, u8 *b);
// convert bytes to HEX line, len - length of input buffer
void pdu_bytes2hex(u8 *b, u16 len, u8 *h);

// decode incoming PDU to out structure, out->msg.data points to start of incoming text
void pdu_in_decode(u8 *in, u16 len, struct pdu_struct *out);
// decode text PDU, in - start of incoming text, in_bytes - its length in bytes, in_dcs - Data Coding Scheme, out - output buffer, returns number of chars
u16 pdu_in_decode_text(u8 *in, u16 in_bytes, u8 in_dcs, u8 *out);

// encode outcoming PDU
u16 pdu_out_encode(struct pdu_struct *in, u8 *out);
// encode outcoming PDU, simple interface
u16 pdu_out_encode_simple(struct pdu_struct *pdu, u8 *out, void *sender, void *msg, u8 tp_vp);

// pack text phone number to bytes, returns number of packed digits (11/12), not including terminating F
u8 pdu_phone_pack(u8 *in, u8 *out);
// unpack phone number to string, returns number of unpacked digits
u8 pdu_phone_unpack(u8 *in, u8 len, u8 *out, u8 type);
// check if phone is packed
u8 pdu_phone_is_packed(u8 *phone);

// get number of bytes from number of digits
u8 pdu_phone_digits_bytes(u8 digits);
// get number of bytes of packed phone
u8 pdu_phone_bytes(u8 *in);
// count number of digits
u8 pdu_phone_digits(u8 *in, u8 bytes);

// compare 2 packed phones
u8 pdu_phone_cmp(u8 *in1, u8 in1_bytes, u8 *in2, u8 in2_bytes);
// copy packed phone
void pdu_phone_copy(u8 *in, u8 *out);

#endif /* GSM_PDU_H_ */
