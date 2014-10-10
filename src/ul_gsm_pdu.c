/*
 * gsm_pdu.c
 *
 *  Created on: Sep 15, 2014
 *      Author: qwer
 */

#include "ul_gsm_pdu.h"

u8 pdu_7to8(u8 *a, u8 len){
	//'123456789abc' -> 31 D9 8C 56 B3 DD 70 B9 B0 78 OC
	//'hellohello'   -> E8 32 9B FD 46 97 D9 EC 37
	//'Hello!'       -> C8 32 9B FD 0E 01
	if (len == 0) len = s_len(a,0);
	a[len] = 0; len++;
	if (len > 0){
		for (u8 i1=0; i1<(len-2); i1++){
			for (u8 i2=(len-1); i2>i1; i2--){
				a[i2-1] = a[i2-1] | ((a[i2] & 1) ? 0x80 : 0);
				a[i2] >>= 1;
			}
		}
	}
	len--;
	return len - (len >> 3);
}
u8 pdu_8to7(u8 *a, u8 len){
	u8 len1 = len + (len >> 3);
	while (len < len1) a[len++] = 0;
	if (len > 0){
		for (u8 i1=0; i1<(len1-1); i1++)
			for (u8 i2=(len1-1); i2>i1; i2--)
				a[i2] = (a[i2] << 1) | !!(a[i2-1] & 0x80);
	}
	for (u8 i=0; i<len1; i++) a[i] &= 0x7F;
	return len1;
}

//u16 pdu_utf8_to_ucs2(u8 *in, u16 in_len, u8 *out){
//	u16 out_len = 0;
//	while (in_len){
//		u8 n = utf8_to_ucs2(in, out);
//		if (n == 0) in_len = 0;
//		else {
//			in += n;
//			out++; out_len++;
//			in_len = (in_len >= n) ? (in_len - n) : 0;
//		}
//	}
//	return out_len;
//}


u8  swap8 (u8 b){ return (b << 4)|(b >> 4); }
u16 swap16(u16 w){ return (w << 8)|(w >> 8); }

void pdu_hex2bytes(u8 *h, u16 len, u8 *b){
	if (len == 0) return;
	len >>= 1;
	while (len--){
		*b++ = (s_h2b4(*h) << 4) | s_h2b4(*(h+1));
		h += 2;
	}
}
void pdu_bytes2hex(u8 *b, u16 len, u8 *h){
	if (len == 0) return;
	b += len-1; h += ((u16)len << 1) - 1;
	while (len--){
		*h-- = s_b2h4(*b & 0xF);
		*h-- = s_b2h4(*b-- >> 4);
	}
}

void pdu_in_decode(u8 *in, u16 len, struct pdu_struct *out){
	if (len == 0) len = s_len(in, 0);

	pdu_hex2bytes(in, len, in);
	len >>= 1;

	out->smsc.len = in[0];
	out->smsc.bytes = out->smsc.len - 1;
	if (in[0] == 7){
		out->smsc.type = in[1];
		for (u8 i=0; i<(out->smsc.bytes); i++) out->smsc.data[i] = in[i+2];
	}

	in += out->smsc.len + 1;

	out->first        = *in++;
	out->sender.len   = *in++;
	out->sender.bytes = (out->sender.len + (out->sender.len & 1)) >> 1;
	out->sender.type  = *in++;
	for (u8 i=0; i<((out->sender.len + (out->sender.len & 1)) >> 1); i++) out->sender.data[i] = *in++;

	out->tp_pid       = *in++;
	out->tp_dcs       = *in++;

	out->year         = swap8( *in++ );
	out->month        = swap8( *in++ );
	out->date         = swap8( *in++ );
	out->hour         = swap8( *in++ );
	out->min          = swap8( *in++ );
	out->sec          = swap8( *in++ );
	out->tz           = swap8( *in++ );

	out->msg.len = out->msg.bytes = *in++;
	// correction for 7-bit encoding
	if (out->tp_dcs == PDU_DCS_7) out->msg.bytes -= (out->msg.bytes >> 3);

	// decode message text (if output buffer is given)
	out->msg.data = in;
}
u16 pdu_in_decode_text(u8 *in, u16 in_bytes, u8 in_dcs, u8 *out){
	if (in_dcs == PDU_DCS_UCS2){
		in_bytes = s_ucs2_to_utf8( (u16 *)in, in_bytes >> 1, out, 1 );
		out += in_bytes;
	} else {
		if (in_dcs == PDU_DCS_7) {
			in_bytes += in_bytes >> 3;
			pdu_8to7(in, in_bytes);
		}
		for (u8 i=0; i<in_bytes; i++) *out++ = *in++;
	}
	*out = 0;
	return in_bytes;
}

u16 pdu_out_encode(struct pdu_struct *in, u8 *out){
	u16 len = 1;
	*out++ = in->smsc.len;
	// fill SMSC data
	if (in->smsc.len){
		*out++ = in->smsc.type;
		for (u8 i=0; i<(in->smsc.len-1); i++) *out++ = in->smsc.data[i];
		len = in->smsc.len;
	}
	*out++ = in->first;       // 1st octet (0x11)
	*out++ = in->tp_msg_ref;  // TP-Message-Reference. The "00" value here lets the phone set the message  reference number itself

	in->sender.bytes = pdu_phone_bytes(in->sender.data);
	in->sender.len   = pdu_phone_digits(in->sender.data, in->sender.bytes);
	*out++ = in->sender.len;  // address length, digits
	*out++ = in->sender.type; // type-of-address
	for (u8 i=0; i<in->sender.bytes; i++) *out++ = in->sender.data[i];

	*out++ = in->tp_pid;      // TP-PID. Protocol identifier

	if (in->tp_dcs == PDU_DCS_AUTO) in->tp_dcs = s_is_7bit(in->msg.data, 0) ? PDU_DCS_7 : PDU_DCS_UCS2;
	*out++ = in->tp_dcs;      // TP-DCS. Data coding scheme

	*out++ = in->tp_vp;       // TP-Validity-Period. "AA" means 4 days
	len += in->sender.bytes + 8;

	// add message
	in->msg.len = s_len(in->msg.data,0);

	if (in->tp_dcs == PDU_DCS_UCS2){ // UCS2 ? Convert from UTF8
		in->msg.bytes = in->msg.len = (s_utf8_to_ucs2(in->msg.data, in->msg.len, (u16 *)(out+1), 1) << 1);
		*out++ = in->msg.len;
	} else {
		*out++ = in->msg.len;
		s_copy(in->msg.data, 0, out);
		if (in->tp_dcs == PDU_DCS_7) in->msg.bytes = pdu_7to8(out, in->msg.len);
		else in->msg.bytes = in->msg.len;
	}
	out -= len;
	len += in->msg.bytes;
	pdu_bytes2hex(out, len, out);
	in->len_bytes      = len << 1;
	in->len_cmgs       = len - in->smsc.len - 1;
	out[in->len_bytes] = 0;

	return in->len_bytes;
}

u16 pdu_out_encode_simple(struct pdu_struct *pdu, u8 *out, void *sender, void *msg, u8 tp_vp){
	pdu->first       = 0x11;
	pdu->tp_msg_ref  = 0;
	pdu->tp_pid      = 0;
	pdu->tp_vp       = (tp_vp == 0) ? 0xAA : tp_vp;

	pdu->smsc.len    = 0;

	pdu->sender.type = PDU_TYPE_INTERNATIONAL;
	if (pdu_phone_is_packed((u8 *)sender)) pdu_phone_copy((u8 *)sender, pdu->sender.data);
	else pdu_phone_pack((u8 *)sender, pdu->sender.data);

	pdu->tp_dcs      = PDU_DCS_AUTO;
	pdu->msg.data    = (u8 *)msg;

	pdu_out_encode(pdu, out);

	return pdu->len_bytes;
}

u8 pdu_phone_pack(u8 *in, u8 *out){
	u8 n = 0;
	if (*in == '+') in++;
	while ((*in >= '0')&&(*in <= '9')){
		*out = s_h2b4(*in++);
		if (*in){
			*out |= s_h2b4(*in) << 4;
			in++; n++;
		} else *out |= 0xF0;
		n++; out++;
	}
	*out = 0xFF;
	return n;
}
u8 pdu_phone_unpack(u8 *in, u8 len, u8 *out, u8 type){
	u8 n = 0;
	if (type == PDU_TYPE_INTERNATIONAL) *out++ = '+';
	while (len--){
		*out++ = s_b2h4(*in & 0xF); n++;
		if ((*in & 0xF0) != 0xF0) {
			*out++ = s_b2h4(*in >> 4);
			n++;
		}
		in++;
	}
	*out = 0;
	return n;
}
u8 pdu_phone_is_packed(u8 *phone){ return *phone != '+'; }

u8 pdu_phone_digits_bytes(u8 digits){ return (digits + (digits & 1)) >> 1; }
u8 pdu_phone_bytes(u8 *in){
	u8 len = 0;
	while (*in++ != 0xFF) len++;
	return len;
}
u8 pdu_phone_digits(u8 *in, u8 bytes){
	u8 len = bytes << 1;
	if ((in[bytes-1] & 0xF0) == 0xF0) len--;
	return len;
}
u8 pdu_phone_cmp(u8 *in1, u8 in1_bytes, u8 *in2, u8 in2_bytes){
	if ((in1_bytes != in2_bytes)||(in1_bytes == 0)) return 0;
	while (in1_bytes--) if (*in1++ != *in2++) return 0;
	return 1;
}
void pdu_phone_copy(u8 *in, u8 *out){
	if (in == out) return;
	u8 eop = pdu_phone_is_packed(in) ? 0xFF : 0;
	while (*in != eop) *out++ = *in++;
	*out = eop;
}
