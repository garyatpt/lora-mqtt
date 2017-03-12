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
 * @file	umdk-opt3001.c
 * @brief   umdk-opt3001 message parser
 * @author  Eugeny Ponomarev [ep@unwds.com]
 * @author  Oleg Artamonov [oleg@unwds.com]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unwds-modules.h"
#include "utils.h"

void umdk_opt3001_command(char *param, char *out, int bufsize) {
    if (strstr(param, "set_period ") == param) {
        param += strlen("set period ");    // Skip command

        uint8_t period = atoi(param);
        snprintf(out, bufsize, "0f00%02x", period);
    }
    else if (strstr(param, "get") == param) {
        snprintf(out, bufsize, "0f01");
    }
    else if (strstr(param, "set_i2c ") == param) { 
         param += strlen("set_i2c ");	// Skip command

         uint8_t i2c = atoi(param);

         snprintf(out, bufsize, "0f02%02x", i2c);
    }
}

bool umdk_opt3001_reply(uint8_t *moddata, int moddatalen, char *topic, mqtt_msg_t *mqtt_msg)
{
    char buf[100];
    strcpy(topic, "opt3001");

    if (moddatalen == 1) {
        if (moddata[0] == 0) {
            add_value_pair(mqtt_msg, "msg", "ok");
        } else {
            add_value_pair(mqtt_msg, "msg", "error");
        }
        return true;
    }

	uint16_t lum = 0;
	if (is_big_endian()) {
        lum = (moddata[1] << 8) | moddata[0]; /* We're in big endian there, swap bytes */
    } else {
        lum = (moddata[0] << 8) | moddata[1];
    }

    snprintf(buf, sizeof(buf), "%d", lum);
    add_value_pair(mqtt_msg, "luminocity", buf);

	return true;
}
