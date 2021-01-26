/*
 * ESPRSSIF MIT License
 *
 * Copyright (c) 2015 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "gpio.h"

#define SERVICE_ADDRESS			"example.com"
#define SERVICE_PORT			8888

#define IO_MODE_INPUT			0
#define IO_MODE_OUTPUT			1

#define IO_PULLUP_EN			1
#define IO_PULLUP_DIS			0

inline void io_init_at(int id, int mode, int up, int def)
{
	int mux = 0, func = 0;

	switch (id) {
	case 0:
		mux = PERIPHS_IO_MUX_GPIO0_U;
		func = FUNC_GPIO0;
		break;
	case 1:
		mux = PERIPHS_IO_MUX_U0TXD_U;
		func = FUNC_GPIO1;
		break;
	case 2:
		mux = PERIPHS_IO_MUX_GPIO2_U;
		func = FUNC_GPIO2;
		break;
	case 3:
		mux = PERIPHS_IO_MUX_U0RXD_U;
		func = FUNC_GPIO3;
		break;
	case 4:
		mux = PERIPHS_IO_MUX_GPIO4_U;
		func = FUNC_GPIO4;
		break;
	case 5:
		mux = PERIPHS_IO_MUX_GPIO5_U;
		func = FUNC_GPIO5;
		break;
	case 6:
		mux = PERIPHS_IO_MUX_SD_CLK_U;
		func = FUNC_GPIO6;
		break;
	case 7:
		mux = PERIPHS_IO_MUX_SD_DATA0_U;
		func = FUNC_GPIO7;
		break;
	case 8:
		mux = PERIPHS_IO_MUX_SD_DATA1_U;
		func = FUNC_GPIO8;
		break;
	case 9:
		mux = PERIPHS_IO_MUX_SD_DATA2_U;
		func = FUNC_GPIO9;
		break;
	case 10:
		mux = PERIPHS_IO_MUX_SD_DATA3_U;
		func = FUNC_GPIO10;
		break;
	case 11:
		mux = PERIPHS_IO_MUX_SD_CMD_U;
		func = FUNC_GPIO11;
		break;
	case 12:
		mux = PERIPHS_IO_MUX_MTDI_U;
		func = FUNC_GPIO12;
		break;
	case 13:
		mux = PERIPHS_IO_MUX_MTCK_U;
		func = FUNC_GPIO13;
		break;
	case 14:
		mux = PERIPHS_IO_MUX_MTMS_U;
		func = FUNC_GPIO14;
		break;
	case 15:
		mux = PERIPHS_IO_MUX_MTDO_U;
		func = FUNC_GPIO15;
		break;
	default:
		return;
	}

	// 输入模式
	if (0 == mode)
	{
		if (0 == up)
		{
			PIN_PULLUP_DIS(mux);
		}
		else
		{
			PIN_PULLUP_EN(mux);
		}

		GPIO_DIS_OUTPUT(GPIO_ID_PIN(id));
	}
	else
	{
		PIN_PULLUP_DIS(mux);

		GPIO_OUTPUT_SET(GPIO_ID_PIN(id), 0 == def ? 0 : 1);
	}

	PIN_FUNC_SELECT(mux, func);
}

inline void io_init_all()
{
};

#define IO_INIT_FUNC()			io_init_all()

#endif

