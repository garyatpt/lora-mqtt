/*
 * Copyright (C) 2016 Unwired Devices [info@unwds.com]
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file	umdk-lps331.c
 * @brief   umdk-lps331 message parser
 * @author  Eugeny Ponomarev [ep@unwds.com]
 * @author  Oleg Artamonov [oleg@unwds.com]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unwds-modules.h"
#include "utils.h"

void umdk_lps331_command(char *param, char *out, int bufsize) {
    if (strstr(param, "set_period ") == param) {
        param += strlen("set_period ");    // Skip command

        uint8_t period = atoi(param);
        snprintf(out, bufsize, "0b00%02x", period);
    }
    else if (strstr(param, "get") == param) {
        snprintf(out, bufsize, "0b01");
    }
    else if (strstr(param, "set_i2c ") == param) { 
         param += strlen("set_i2c ");	// Skip command

         uint8_t i2c = atoi(param);

         snprintf(out, bufsize, "0b02%02x", i2c);
    }
}

bool umdk_lps331_reply(uint8_t *moddata, int moddatalen, char *topic, mqtt_msg_t *mqtt_msg)
{
    char buf[100];
    strcpy(topic, "lps331");

    if (moddatalen == 1) {
        if (moddata[0] == 0) {
            add_value_pair(mqtt_msg, "msg", "ok");
        } else {
            add_value_pair(mqtt_msg, "msg", "error");
        }
        return true;
    }

    /* Extract temperature */
    int16_t temperature = 0;

    if (is_big_endian()) {
        temperature = moddata[0];
        temperature += (moddata[1] << 8);
    }
    else {
        temperature = moddata[1];
        temperature += (moddata[0] << 8);
    }

    /* Extract pressure */
    uint16_t pressure = 0;

    if (is_big_endian()) {
        pressure = moddata[2];
        pressure += (moddata[3] << 8);
    }
    else {
        pressure = moddata[3];
        pressure += (moddata[2] << 8);
    }

    snprintf(buf, sizeof(buf), "%d.%d", temperature/10, abs(temperature%10));
    add_value_pair(mqtt_msg, "temperature", buf);
    snprintf(buf, sizeof(buf), "%d", pressure);
    add_value_pair(mqtt_msg, "pressure", buf);
    
    return true;
}
