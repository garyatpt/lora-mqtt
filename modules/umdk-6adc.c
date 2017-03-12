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
 * @file	umdk-6adc.c
 * @brief   umdk-6adc message parser
 * @author  Eugeny Ponomarev [ep@unwds.com]
 * @author  Oleg Artamonov [oleg@unwds.com]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unwds-modules.h"
#include "utils.h"

void umdk_6adc_command(char *param, char *out, int bufsize) {
    if (strstr(param, "set_period ") == param) {
        param += strlen("set_period ");    // Skip command

        uint8_t period = atoi(param);
        snprintf(out, bufsize, "0a00%02x", period);
    }
    else if (strstr(param, "get") == param) {
        snprintf(out, bufsize, "0a01");
    }
    else if (strstr(param, "set_gpio ") == param) {
        param += strlen("set_gpio "); // Skip command

        uint8_t gpio = atoi(param);

        snprintf(out, bufsize, "0a02%02x", gpio);
    }
    else if (strstr(param, "set_lines ") == param) {
        param += strlen("set_lines ");    // Skip command

        uint8_t lines_en = 0;
        uint8_t line = 0;
        while ( (line = (uint8_t)strtol(param, &param, 10)) ) {
            if (line > 0 && line <= 7) {
                lines_en |= 1 << (line - 1);
            }
        }

        snprintf(out, bufsize, "0a03%02x", lines_en);
    }
}

bool umdk_6adc_reply(uint8_t *moddata, int moddatalen, char *topic, mqtt_msg_t *mqtt_msg)
{
    char buf[100];
    strcpy(topic, "6adc");

    if (moddatalen == 1) {
        if (moddata[0] == 0) {
            add_value_pair(mqtt_msg, "msg", "ok");
        } else {
            add_value_pair(mqtt_msg, "msg", "error");
        }
        return true;
    }

    int i;
    for (i = 0; i < 16; i += 2) {
        uint16_t sensor = 0;
        if (is_big_endian()) {
            sensor = (moddata[i + 1] << 8) | moddata[i]; /* We're in big endian there, swap bytes */
        }
        else {
            sensor = (moddata[i] << 8) | moddata[i + 1];
        }

        char ch[6] = {};
        snprintf(ch, sizeof(ch), "adc%d", (i / 2) + 1);

        if (sensor == 0xFFFF) {
            add_value_pair(mqtt_msg, ch, "null");
        }
        else {
            snprintf(buf, sizeof(buf), "%d", sensor);
            add_value_pair(mqtt_msg, ch, buf);
        }
    }
    return true;
}
