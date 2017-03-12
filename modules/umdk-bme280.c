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
 * @file	umdk-bme280.c
 * @brief   umdk-bme280 message parser
 * @author  Oleg Artamonov [oleg@unwds.com]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unwds-modules.h"
#include "utils.h"

void umdk_bme280_command(char *param, char *out, int bufsize) {
    if (strstr(param, "set_period ") == param) {
        param += strlen("set_period ");    // Skip command

        uint8_t period = atoi(param);
        snprintf(out, bufsize, "1100%02x", period);
    }
    else if (strstr(param, "get") == param) {
        snprintf(out, bufsize, "1101");
    }
}

bool umdk_bme280_reply(uint8_t *moddata, int moddatalen, char *topic, mqtt_msg_t *mqtt_msg)
{
    char buf[100];
    strcpy(topic, "bme280");

    if (moddatalen == 1) {
        if (moddata[0] == 0) {
            add_value_pair(mqtt_msg, "msg", "ok");
        } else {
            add_value_pair(mqtt_msg, "msg", "error");
        }
        return true;
    }

    int16_t temp = 0;
    int16_t hum = 0;
    uint16_t press = 0;
    
    if (is_big_endian()) {
        /* We're in big endian here, swap bytes */
        temp = (moddata[1] << 8) | moddata[0];
        hum = (moddata[3] << 8) | moddata[2];
        press = (moddata[5] << 8) | moddata[4];
    }
    else {
        temp = (moddata[0] << 8) | moddata[1];
        hum = (moddata[2] << 8) | moddata[3];
        press = (moddata[4] << 8) | moddata[5];
    }

    snprintf(buf, sizeof(buf), "%d.%d", temp/10, abs(temp%10));
    add_value_pair(mqtt_msg, "temperature", buf);
    
    snprintf(buf, sizeof(buf), "%d.%d", hum/10, hum%10);
    add_value_pair(mqtt_msg, "humidity", buf);
    
    snprintf(buf, sizeof(buf), "%d", press);
    add_value_pair(mqtt_msg, "pressure", buf);
    
    return true;
}
