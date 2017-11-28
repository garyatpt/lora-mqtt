/* Copyright (c) 2017 Unwired Devices LLC [info@unwds.com]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file	umdk-iec61107.c
 * @brief   umdk-iec61107 message parser
 * @author   Mikhail Perkov
 * @author  
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unwds-modules.h"
#include "utils.h"

#define IEC61107_BRACKET_OPEN 0x28 /* '(' */
#define IEC61107_BRACKET_CLOSE 0x29 /* ')' */
#define	IEC61107_WRITE 0x57	/* 'W' - Write */
#define	IEC61107_READ 0x52	/* 'R' - Read */

#define IEC61107_MODE_SP_READ 0x36

typedef enum {   
    IEC61107_CMD_PROPRIETARY_COMMAND = 0xF0,		/* Less this value - single command of IEC61107 */

    IEC61107_CMD_SPECIFIC				= 0x00,		/* Manufacturer Special cmd */
	
	IEC61107_CMD_OPTIONS				= 0x01,		/* Set programming mode without DSTP-button */
    IEC61107_CMD_TIME					= 0x02,		/* Read/write the internal time */
    IEC61107_CMD_DATE					= 0x03,		/* Read/write the internal date */
    IEC61107_CMD_SERIAL					= 0x04,		/* Read/write serial number */
	IEC61107_CMD_ADDRESS				= 0x05,		/* Read/write address of the device */
	IEC61107_CMD_STATUS					= 0x06,		/* Read device status */
		
    IEC61107_CMD_GET_VALUE_TOTAL_ALL	= 0x07,		/* Read the total values of energy after reset */
	IEC61107_CMD_GET_VALUE_TOTAL_MONTH	= 0x08,		/* Read the total values of monthly energy */
    IEC61107_CMD_GET_VALUE_TOTAL_DAY	= 0x09,		/* Read the total values of daily energy */
    IEC61107_CMD_GET_VALUE_END_MONTH	= 0x0A,		/* Read the values of monthly energy */
    IEC61107_CMD_GET_VALUE_END_DAY		= 0x0B,		/* Read the values of daily energy */ 
    IEC61107_CMD_GET_VALUE_DATE_MONTH	= 0x0C,		/* Read the date array of monthly energy savings */
    IEC61107_CMD_GET_VALUE_DATE_DAY		= 0x0D,		/* Read the date array of daily energy savings */
	IEC61107_CMD_ERASAE_VALUES			= 0x0E,		/* Erase all energy  values */
	
	IEC61107_CMD_GET_VOLT				= 0x0F,		/* Read the voltage value */
	IEC61107_CMD_GET_CURR				= 0x10,		/* Read the current value */
	IEC61107_CMD_GET_POWER				= 0x11,		/* Read the power value */
	
	IEC61107_CMD_SCHEDULE				= 0x12,		/* Read/write schedule ot tariffs */
	IEC61107_CMD_HOLIDAYS				= 0x13,		/* Read/write list of holidays */
	// IEC61107_CMD_GET_SAVING_END_MONTH	= 0x14,
} umdk_iec61107_cmd_t;

typedef enum {
	UMDK_IEC61107_ERROR_REPLY 		= 0,
    UMDK_IEC61107_OK_REPLY 			= 1,
    UMDK_IEC61107_NO_RESPONSE_REPLY 	= 2,
	UMDK_IEC61107_INVALID_CMD_REPLY 	= 0xFF,
} iec61107_reply_t;

// static char str_dow[8][4] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Hol" };

void umdk_iec61107_command(char *param, char *out, int bufsize) {
	uint8_t cmd = 0;		
	uint8_t mode = 0;
	uint8_t zz_param = 0;
	if (strstr(param, "set init") == param) {
		param += strlen("set init");    // Skip 
		cmd = IEC61107_CMD_OPTIONS;
		mode = IEC61107_WRITE;
	}
	else if (strstr(param, "get init") == param) {
		param += strlen("get init");    // Skip 
		cmd = IEC61107_CMD_OPTIONS;
		mode = IEC61107_READ;
	}
	else if (strstr(param, "get serial") == param) {
		param += strlen("get serial");    // Skip 
		cmd = IEC61107_CMD_SERIAL;
		mode = IEC61107_READ;
	}
	else if (strstr(param, "get time") == param) { 
		param += strlen("get time");    // Skip command
		cmd = IEC61107_CMD_TIME;	
		mode = IEC61107_READ;		
	}
	else if (strstr(param, "set time") == param) { 
		param += strlen("get time");    // Skip command
		cmd = IEC61107_CMD_TIME;	
		mode = IEC61107_WRITE;		
	}	
	else if (strstr(param, "get date") == param) { 
		param += strlen("get date");    // Skip command
		cmd = IEC61107_CMD_DATE;	
		mode = IEC61107_READ;		
	}
	else if (strstr(param, "set date") == param) { 
		param += strlen("set date");    // Skip command
		cmd = IEC61107_CMD_DATE;	
		mode = IEC61107_WRITE;		
	}
	else if (strstr(param, "get value ") == param) { 
		param += strlen("get value ");    // Skip command
		mode = IEC61107_READ;
		if(strstr(param, "total_sp") == param) {
			param += strlen("total_sp");				// Skip command
			cmd = IEC61107_CMD_SPECIFIC;
			mode = IEC61107_MODE_SP_READ;
		}
		else if(strstr(param, "total_all") == param) { 
			param += strlen("total_all");    // Skip command
			cmd = IEC61107_CMD_GET_VALUE_TOTAL_ALL;
		}
		else if(strstr(param, "total_month") == param) { 
			param += strlen("total_month");    // Skip command
			cmd = IEC61107_CMD_GET_VALUE_TOTAL_MONTH;
		}
		else if(strstr(param, "total_day") == param) { 
			param += strlen("total_day");    // Skip command
			cmd = IEC61107_CMD_GET_VALUE_TOTAL_DAY;
		}
		else if(strstr(param, "end_month") == param) { 
			param += strlen("end_month");    // Skip command
			cmd = IEC61107_CMD_GET_VALUE_END_MONTH;
		}
		else if(strstr(param, "end_day") == param) { 
			param += strlen("end_day");    // Skip command
			cmd = IEC61107_CMD_GET_VALUE_END_DAY;
		}
		else if(strstr(param, "saving ") == param) { 
			param += strlen("saving ");    // Skip command
			if(strstr(param, "end_month ") == param) {
				param += strlen("end_month ");				// Skip command
				
				// cmd = IEC61107_CMD_GET_SAVING_END_MONTH;
				// zz_param = strtol(param, &param, 10);
				
				// if((zz_param < 1) || (zz_param > 12)) {
					// snprintf(out, bufsize, "%02x", UMDK_IEC61107_INVALID_CMD_REPLY);
					// return;					
				// }
			}
			else {
				snprintf(out, bufsize, "%02x", UMDK_IEC61107_INVALID_CMD_REPLY);
				return;
			}
		}		
		else {
			snprintf(out, bufsize, "%02x", UMDK_IEC61107_INVALID_CMD_REPLY);
			return;
		}
	}
	else if (strstr(param, "get schedule") == param) { 
		param += strlen("get schedule");    // Skip command
		cmd = IEC61107_CMD_SCHEDULE;	
		mode = IEC61107_READ;		
		zz_param = 0x01;
	}
	else if (strstr(param, "get holidays") == param) { 
		param += strlen("get holidays");    // Skip command
		cmd = IEC61107_CMD_HOLIDAYS;	
		mode = IEC61107_READ;		
	}
	else {
		snprintf(out, bufsize, "%02x", UMDK_IEC61107_INVALID_CMD_REPLY);
		return;
	}
		
	param += strlen(" ");    						// Skip space
		
	char *parametr_ptr = param;
	char *address_ptr = NULL;
	
	uint8_t symb_addr = 0;
	uint8_t symb_param = 0;
	uint8_t length_total = 0;
	uint8_t length_param = 0;
	uint8_t length_addr = 0;
		
	length_total = (uint8_t)strlen(parametr_ptr);
	
	address_ptr = strstr(param, " ");
	if(address_ptr == NULL) {
		length_param = 0;
		length_addr = length_total;
		address_ptr = parametr_ptr;
	}
	else {
		address_ptr += strlen(" ");    // Skip space
		length_addr = (uint8_t)strlen(address_ptr);		
		length_param = length_total - length_addr - strlen(" ");
	}
	
	if(length_addr == 0) {
		snprintf(out, bufsize, "%02x", UMDK_IEC61107_INVALID_CMD_REPLY);
		return;		
	}
	
	uint16_t num_char;		
	num_char = snprintf(out, bufsize, "%02x%02x%02x%02x", cmd, mode, zz_param, length_addr);
	
	uint8_t i = 0;
	for(i = 0; i < length_addr; i++) {
		symb_addr = *address_ptr;
		num_char += snprintf(out + num_char, bufsize - num_char, "%02x", symb_addr);
		address_ptr++;
	}
	
	num_char += snprintf(out + num_char, bufsize - num_char, "%02x", IEC61107_BRACKET_OPEN);	
	
	for(i = 0; i < length_param; i++) {
		
		symb_param = *parametr_ptr;
		num_char += snprintf(out + num_char, bufsize - num_char, "%02x", symb_param);			
		parametr_ptr++;
	}
	
	num_char += snprintf(out + num_char, bufsize - num_char, "%02x", IEC61107_BRACKET_CLOSE);	
		
}

bool umdk_iec61107_reply(uint8_t *moddata, int moddatalen, mqtt_msg_t *mqtt_msg)
{
	// char buf[100];
    // char strbuf[20];
	char buf_addr[50];
	
	uint8_t ii;
    printf("[iec61107] RX data:  ");
    for(ii = 0; ii < moddatalen; ii++) {
        printf(" %02X ", moddata[ii]);
    }
   puts("\n");
	
   if (moddatalen == 1) {
        if (moddata[0] == UMDK_IEC61107_OK_REPLY) {
            add_value_pair(mqtt_msg, "msg", "ok");
        } else if(moddata[0] == UMDK_IEC61107_ERROR_REPLY){
            add_value_pair(mqtt_msg, "msg", "error");
		}
		else if(moddata[0] == UMDK_IEC61107_INVALID_CMD_REPLY){
			add_value_pair(mqtt_msg, "msg", "invalid command");
		}
        return true;
    }	
	
	umdk_iec61107_cmd_t cmd = moddata[0];	
	uint8_t length_addr = moddata[1];	
	
	if(cmd < IEC61107_CMD_PROPRIETARY_COMMAND) {
		uint8_t *address_ptr = &moddata[2];	
		uint16_t num_char = 0;
		uint8_t i = 0;
		uint8_t symb_addr = 0;
		
		for(i = 0; i < length_addr; i++) {
			symb_addr = *address_ptr;
			num_char += snprintf(buf_addr + num_char, sizeof(buf_addr) - num_char, "%c", symb_addr);
			address_ptr++;
		}
		
		add_value_pair(mqtt_msg, "address", buf_addr);
							
		if (moddatalen == length_addr + 2) {
			if (moddata[0] == UMDK_IEC61107_OK_REPLY) {
				add_value_pair(mqtt_msg, "msg", "ok");
			} else if(moddata[0] == UMDK_IEC61107_ERROR_REPLY){
				add_value_pair(mqtt_msg, "msg", "error");
			} else if(moddata[0] == UMDK_IEC61107_NO_RESPONSE_REPLY){
				add_value_pair(mqtt_msg, "msg", "no response");					
			}
			return true;
		}
	}
  
	// uint8_t i;
	// uint32_t * ptr_value;
	
	switch(cmd) {			
		case IEC61107_CMD_TIME: {
			char time_buf[10] = { };
			
			uint8_t hour = ( moddata[length_addr + 2] - 0x30) * 10 + ( moddata[length_addr + 3] - 0x30);
			uint8_t min =  ( moddata[length_addr + 4] - 0x30) * 10 + ( moddata[length_addr + 5] - 0x30);
			uint8_t sec =  ( moddata[length_addr + 6] - 0x30) * 10 + ( moddata[length_addr + 7] - 0x30);
					
			
			snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", hour, min, sec);	
			add_value_pair(mqtt_msg, "time", time_buf);
			
			return true;
			break;
		}

		// case M200_CMD_GET_VALUE: {
			// uint32_t value[5] = { 0 };
			// for(i = 0; i < 5; i++) {
				// ptr_value = (uint32_t *)(&moddata[4*i + 5]);
				// uint32_to_le(ptr_value);      
				// value[i] = *ptr_value;
			// }
			
			// char tariff[5] = { };
			// for(i = 0; i < 4; i++) {
				// snprintf(tariff, sizeof(tariff), "T%02d", i + 1);
                // int_to_float_str(strbuf, value[i], 2);
				// snprintf(buf, sizeof(buf), "%s", strbuf);
				// add_value_pair(mqtt_msg, tariff, buf);				
			// }
            // int_to_float_str(strbuf, value[4], 2);
			// snprintf(buf, sizeof(buf), "%s", strbuf);
			// add_value_pair(mqtt_msg, "Total", buf);		
	
			// return true;
			// break;
		// }		
				
		default:
			break;
	}
	return true;
}