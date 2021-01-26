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

#include "esp_common.h"

#include "user_config.h"

#include "gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "ssl/ssl_crypto.h"

#include "json/cJSON.h"

static uint32 *global_flag = NULL;

bool ICACHE_FLASH_ATTR
generateKey(unsigned char *key, unsigned int size)
{
	MD5_CTX md5_ctx;

	unsigned char buffer[32] = {0};

	unsigned char mac[6] = {0};

	if(NULL != key && 16 <= size)
	{
		if(true == wifi_get_macaddr(STATION_IF, mac))
		{
			MD5_Init(&md5_ctx);

			MD5_Update(&md5_ctx, buffer, \
								snprintf(buffer, sizeof(buffer), \
								"%08X%08X%02X%02X%02X%02X%02X%02X", \
								system_get_chip_id(), spi_flash_get_id(), \
								mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

			MD5_Final(buffer, &md5_ctx);

			snprintf(key, size, "%02X%02X%02X%02X%02X%02X", \
								buffer[1], buffer[3], buffer[5], \
								buffer[7], buffer[9], buffer[11]);

			return true;
		}
	}

	return false;
}

bool ICACHE_FLASH_ATTR
get_server_ip(const char *domain, char *ip, unsigned int size)
{
	if(NULL != domain && NULL != ip)
	{
		if(0 < strlen(domain))
		{
			int retry = 0;

			struct hostent *hptr = NULL;

			do
			{
				os_printf("%s: gethostbyname(%s)\n", __func__, domain);

				hptr = gethostbyname(domain);
			} while(NULL == hptr && 30 > retry++);

			if(NULL != hptr && 4 == hptr->h_length)
			{
				memset(ip, 0x0, size);

				snprintf(ip, size - 1, "%u.%u.%u.%u", hptr->h_addr_list[0][0], hptr->h_addr_list[0][1], hptr->h_addr_list[0][2], hptr->h_addr_list[0][3]);

				os_printf("%s: gethostbyname(%s) => %s\n", __func__, domain, ip);

				return true;
			}
		}
	}

	return false;
}

bool ICACHE_FLASH_ATTR
json_get_value_array(cJSON *pRoot, const char *name, unsigned char *buf, unsigned int size)
{
	unsigned int i = 0;

	cJSON *pNode = pRoot->child;

	while(NULL != pNode)
	{
		if(cJSON_Array == pNode->type)
		{
			if(0 == strcmp(name, pNode->string))
			{
				cJSON *pArray = pNode->child;

				for(i = 0; size > i && NULL != pArray; i++, pArray = pArray->next)
				{
					buf[i] = (unsigned char)(pArray->valueint & 0xFF);
				}

				return true;
			}
		}

		pNode = pNode->next;
	}

	return false;
}

bool ICACHE_FLASH_ATTR
json_get_value_string(cJSON *pRoot, const char *name, char **value)
{
	cJSON *pNode = pRoot->child;

	while(NULL != pNode)
	{
		if(cJSON_String == pNode->type)
		{
			if(0 == strcmp(name, pNode->string))
			{
				*value = pNode->valuestring;

				return true;
			}
		}

		pNode = pNode->next;
	}

	return false;
}

bool ICACHE_FLASH_ATTR
json_get_value_boolean(cJSON *pRoot, const char *name)
{
	cJSON *pNode = pRoot->child;

	while(NULL != pNode)
	{
		if(cJSON_True == pNode->type)
		{
			if(0 == strcmp(name, pNode->string))
			{
				return true;
			}
		}

		pNode = pNode->next;
	}

	return false;
}

bool ICACHE_FLASH_ATTR
wol_send(unsigned char *mac)
{
	bool ret = false;

	unsigned char *packet = NULL;

	unsigned int i = 0;

	int sockfd = -1;

	struct sockaddr_in sock_addr;

	packet = (unsigned char *)malloc(0x6C);

	if(NULL != packet)
	{
		memset(packet, 0, 0x6C);

		sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		if(0 <= sockfd)
		{
			memset(&sock_addr, 0, sizeof(sock_addr));

			sock_addr.sin_family = AF_INET;
			sock_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
			sock_addr.sin_port = htons(60069);

			packet[0] = 0xFF;
			packet[1] = 0xFF;
			packet[2] = 0xFF;
			packet[3] = 0xFF;
			packet[4] = 0xFF;
			packet[5] = 0xFF;

			for(i = 0x6; 0x66 > i; i++)
			{
				packet[i] = mac[(i - 0x6) % 0x6];
			}

			sendto(sockfd, packet, 0x6C, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));

			os_printf("%s: ok\n", __func__);

			ret = true;

			close(sockfd);
			sockfd = -1;
		}

		free(packet);
		packet = NULL;
	}

	return ret;
}

bool ICACHE_FLASH_ATTR
shutdown_send(unsigned char *mac)
{
	bool ret = false;

	int sockfd = -1;

	struct sockaddr_in sock_addr;

	char buffer[128] = {0};

	unsigned int len = 0;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(0 <= sockfd)
	{
		memset(&sock_addr, 0, sizeof(sock_addr));

		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
		sock_addr.sin_port = htons(4003);

		len = snprintf(buffer, sizeof(buffer), \
				"{\"cmd\":\"shutdown\",\"mac\":\"%02X-%02X-%02X-%02X-%02X-%02X\"}", \
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

		sendto(sockfd, buffer, len, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));

		os_printf("%s: ok\n", __func__);

		ret = true;

		close(sockfd);
		sockfd = -1;
	}

	return ret;
}

bool ICACHE_FLASH_ATTR
wifi_check_connect_status(void)
{
	struct ip_info info;

	if(true == wifi_get_ip_info(STATION_IF, &info))
	{
		if(0 != info.ip.addr)
		{
			return true;
		}
	}

	return false;
}

bool ICACHE_FLASH_ATTR
wifi_check_connect_able(void)
{
	struct station_config sta_conf;

	if(true == wifi_station_get_config_default(&sta_conf))
	{
		if(0 < sta_conf.ssid[0])
		{
			os_printf("SSID: %s\n", sta_conf.ssid);
			os_printf("password: %s\n", sta_conf.password);

			return true;
		}
	}

	return false;
}

void ICACHE_FLASH_ATTR
reset_delay_task(void *pvParameters)
{
	vTaskDelay(3000 / portTICK_RATE_MS);

	//system_soft_wdt_stop();

	system_restore();
	system_restart();

	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
wol_client_task(void *pvParameters)
{
	unsigned char key[16] = {0};

	int sockfd = -1;

	int timeout = 1000;

	struct sockaddr_in sock_addr;

	unsigned int retry_count = 0;

#if defined(IO_WARN_TO_PINID)
	unsigned int io_warn_count = 0;
#endif

	char buffer[256] = {0};

	int ret = 0;

	uint32 timestamp_now = 0;

	uint32 timestamp_send_bind = 0;
	uint32 timestamp_send_beat = 0;

	bool response_not_recv = false;

	unsigned int response_lost_count = 0;

	struct ip_info info;
	struct station_config sta_con;

	char ip[32] = {0};

	cJSON *pRoot = NULL;

	if(true == generateKey(key, sizeof(key)))
	{
		while(1)
		{
			if(true == wifi_check_connect_status())
			{
				sockfd = socket(AF_INET, SOCK_STREAM, 0);

				if(0 <= sockfd)
				{
					if(0 == setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)))
					{
						if(true != get_server_ip(SERVICE_ADDRESS, ip, sizeof(ip)))
						{
							os_printf("%s: unable get server ip address by: %s\n", __func__, SERVICE_ADDRESS);

							vTaskDelay(1000 / portTICK_RATE_MS);

							continue;
						}

						memset(&sock_addr, 0, sizeof(sock_addr));

						sock_addr.sin_family = AF_INET;
						sock_addr.sin_addr.s_addr = inet_addr(ip);
						sock_addr.sin_port = htons(SERVICE_PORT);

						timestamp_send_bind = 0;
						timestamp_send_beat = 0;

						response_not_recv = false;

						response_lost_count = 0;

						retry_count++;

						if(0 == connect(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)))
						{
#if defined(IO_WARN_TO_PINID)
							// 设置报警引脚为输入
							GPIO_DIS_OUTPUT(IO_WARN_TO_PINID);
#endif
							os_printf("connect to %s:%u ok...\n", inet_ntoa(sock_addr.sin_addr), ntohs(sock_addr.sin_port));

							retry_count = 0;

							if(NULL != global_flag)
							{
								*global_flag = 1;
							}

							while(1)
							{
								timestamp_now = system_get_time() / 1000000;

								if(timestamp_send_bind < timestamp_now)
								{
									if(true == response_not_recv)
									{
										if(3 <= response_lost_count)
										{
											os_printf("no response! break\n");

											break;
										}
									}

									timestamp_send_bind = timestamp_now + 60;
									timestamp_send_beat = timestamp_now + 6;

									os_printf("send bind command(key: %s)\n", key);

									if(true == wifi_get_ip_info(STATION_IF, &info))
									{
										snprintf(ip, sizeof(ip), "%u.%u.%u.%u", \
													((unsigned char *)&info.ip.addr)[0], ((unsigned char *)&info.ip.addr)[1], \
													((unsigned char *)&info.ip.addr)[2], ((unsigned char *)&info.ip.addr)[3]);
									}
									else
									{
										ip[0] = 0;
									}

									if(true != wifi_station_get_config(&sta_con))
									{
										sta_con.ssid[0] = 0;
									}

									ret = snprintf(buffer, sizeof(buffer), \
													"{\"cmd\":\"bind\",\"key\":\"%s\",\"ssid\":\"%s\",\"ip\":\"%s\"}", \
													key, sta_con.ssid, ip);

									if(0 < ret)
									{
										ret = send(sockfd, buffer, ret, 0);

										if(0 < ret)
										{
											response_not_recv = true;

											response_lost_count++;
										}
									}
								}
								else if(timestamp_send_beat < timestamp_now)
								{
									if(true == response_not_recv)
									{
										if(3 <= response_lost_count)
										{
											os_printf("no response! break\n");

											break;
										}
									}

									timestamp_send_beat = timestamp_now + 6;

									os_printf("send beat command\n");

									ret = snprintf(buffer, sizeof(buffer), "{\"cmd\":\"beat\"}");

									if(0 < ret)
									{
										ret = send(sockfd, buffer, ret, 0);

										if(0 < ret)
										{
											response_not_recv = true;

											response_lost_count++;
										}
									}
								}

								// 1秒超时
								ret = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

								if(0 < ret)
								{
									buffer[ret] = 0;

									pRoot = cJSON_Parse(buffer);

									if(NULL != pRoot)
									{
										char *cmd = NULL;

										if(true == json_get_value_boolean(pRoot, "status"))
										{
											if(true == response_not_recv)
											{
												response_not_recv = false;

												response_lost_count = 0;
											}
										}
										else if(true == json_get_value_string(pRoot, "cmd", &cmd))
										{
											bool cmd_status = false;

											if(0 == strcmp(cmd, "wol_send"))
											{
												unsigned char mac[6] = {0};

												if(true == json_get_value_array(pRoot, "mac", mac, sizeof(mac) / sizeof(mac[0])))
												{
													cmd_status = wol_send(mac);
												}
											}
											else if(0 == strcmp(cmd, "shutdown_send"))
											{
												unsigned char mac[6] = {0};

												if(true == json_get_value_array(pRoot, "mac", mac, sizeof(mac) / sizeof(mac[0])))
												{
													cmd_status = shutdown_send(mac);
												}
											}
											else if(0 == strcmp(cmd, "reset"))
											{
												cmd_status = true;

												xTaskCreate(reset_delay_task, "reset_delay_task", 256, NULL, 2, NULL);
											}

											ret = snprintf(buffer, sizeof(buffer), \
															"{\"cmd\":\"response\",\"response\":{\"status\":%s,\"key\":\"%s\"}}", \
															true == cmd_status ? "true" : "false", key);

											if(0 < ret)
											{
												send(sockfd, buffer, ret, 0);
											}
										}

										cJSON_Delete(pRoot);
										pRoot = NULL;
									}
								}
								else if(0 == ret)
								{
									os_printf("recv: failed(%d), break\n", ret);

									break;
								}
								else
								{
									if(EAGAIN != errno)
									{
										os_printf("recv: failed(%d), break\n", ret);

										break;
									}
#if defined(IO_WARN_TO_PINID)
									if(0 == (GPIO_INPUT_GET(IO_WARN_TO_PINID)))
									{
										io_warn_count++;

										os_printf("io_warn: warn %u\n", io_warn_count);

										if(3 <= io_warn_count)
										{
											ret = snprintf(buffer, sizeof(buffer), "{\"cmd\":\"warn\"}");

											if(0 < ret)
											{
												ret = send(sockfd, buffer, ret, 0);

												if(0 < ret)
												{
													os_printf("io_warn: warn send ok\n");
												}
												else
												{
													os_printf("io_warn: warn send failed\n");
												}
											}

											io_warn_count = 0;
										}
									} else {
										io_warn_count = 0;
									}
#endif
								}
							}

							if(NULL != global_flag)
							{
								*global_flag = 0;
							}

							os_printf("lost connect to %s:%u\n", inet_ntoa(sock_addr.sin_addr), ntohs(sock_addr.sin_port));
						}
						else
						{
							os_printf("connect to %s:%u failed...\n", inet_ntoa(sock_addr.sin_addr), ntohs(sock_addr.sin_port));

							if(5 <= retry_count)
							{
								wifi_station_disconnect();
								wifi_station_connect();

								retry_count = 0;
							}
						}
					}
					else
					{
						os_printf("setsockopt failed!\n");
					}

					close(sockfd);
					sockfd = -1;
				}
				else
				{
					os_printf("socket failed!\n");
				}
			}

			vTaskDelay(1000 / portTICK_RATE_MS);
		}
	}

	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
send_key_task(void *pvParameters)
{
	int ret = 0;

	unsigned int i = 0;

	int sockfd = -1;

	struct sockaddr_in sock_addr;

	unsigned char key[16] = {0};

	unsigned char buffer[64] = {0};

	if(true == generateKey(key, sizeof(key)))
	{
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);

		if(0 <= sockfd)
		{
			memset(&sock_addr, 0, sizeof(sock_addr));

			sock_addr.sin_family = AF_INET;
			sock_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
			sock_addr.sin_port = htons(60069);

			ret = snprintf(buffer, sizeof(buffer), "{\"type\":\"send_key\",\"key\":\"%s\"}", key);

			for(i = 0; 10 > i; i++)
			{
				sendto(sockfd, buffer, ret, 0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));

				vTaskDelay(1000 / portTICK_RATE_MS);
			}

			close(sockfd);
			sockfd = -1;
		}
	}

	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
led_task(void *pvParameters)
{
#if defined(IO_LED_TO_PINID)
	uint32 gpio_mask = 0x0;

	uint32 count_led = 6;

	// 常亮
	GPIO_OUTPUT_SET(IO_LED_TO_PINID, 0);

	while(1)
	{
		if(NULL != global_flag)
		{
			if(0 == *global_flag)
			{
				count_led++;

				if(0 == count_led % 8)
				{
					// 当前是否处于亮状态
					if(0 == GPIO_INPUT_GET(IO_LED_TO_PINID))
					{
						// 是，灭灯
						GPIO_OUTPUT_SET(IO_LED_TO_PINID, 1);

						count_led = 0;
					}
					else
					{
						// 否，点亮
						GPIO_OUTPUT_SET(IO_LED_TO_PINID, 0);

						count_led = 6;
					}
				}

				vTaskDelay(100 / portTICK_RATE_MS);
			}
			else
			{
				// 常亮
				GPIO_OUTPUT_SET(IO_LED_TO_PINID, 0);
			}
		}
		else
		{
			if(0 == GPIO_INPUT_GET(IO_LED_TO_PINID))
			{
				GPIO_OUTPUT_SET(IO_LED_TO_PINID, 1);
			}
			else
			{
				GPIO_OUTPUT_SET(IO_LED_TO_PINID, 0);
			}

			vTaskDelay(50 / portTICK_RATE_MS);
		}
	}
#endif
	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
smartconfig_callback(sc_status status, void *pdata)
{
	switch(status)
	{
	case SC_STATUS_WAIT:
		os_printf("### SC_STATUS_WAIT\n");
		break;
	case SC_STATUS_FIND_CHANNEL:
		os_printf("### SC_STATUS_FIND_CHANNEL\n");
		break;
	case SC_STATUS_GETTING_SSID_PSWD:
		os_printf("### SC_STATUS_GETTING_SSID_PSWD\n");
		if(NULL != pdata)
		{
			if(SC_TYPE_ESPTOUCH == *((sc_type *)pdata))
			{
				os_printf("SC_TYPE: SC_TYPE_ESPTOUCH\n");
			}
			else
			{
				os_printf("SC_TYPE: SC_TYPE_AIRKISS\n");
			}
		}
		break;
	case SC_STATUS_LINK:
		os_printf("### SC_STATUS_LINK\n");
		wifi_station_disconnect();
		wifi_station_set_config((struct station_config *)pdata);
		wifi_station_connect();
		break;
	case SC_STATUS_LINK_OVER:
		os_printf("### SC_STATUS_LINK_OVER\n");
		if(NULL != pdata)
		{
			os_printf("Phone ip: %d.%d.%d.%d\n", ((uint8 *)pdata)[0], ((uint8 *)pdata)[1], ((uint8 *)pdata)[2], ((uint8 *)pdata)[3]);
		}
		smartconfig_stop();
		break;
	default:
		os_printf("### default\n");
		break;
	}
}

void ICACHE_FLASH_ATTR
smartconfig_task(void *pvParameters)
{
	smartconfig_start(smartconfig_callback);

	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
button_task(void *pvParameters)
{
#if defined(IO_RESET_TO_PINID)
	uint32 count_btn = 0;

	GPIO_DIS_OUTPUT(IO_RESET_TO_PINID);

	while(1)
	{
		if(0 == GPIO_INPUT_GET(IO_RESET_TO_PINID))
		{
			if(0 == count_btn)
			{
				os_printf("Button press down\n");
			}

			count_btn++;
		}
		else
		{
			count_btn = 0;
		}

		if(2 == (count_btn / 10))
		{
			os_printf("reset wait: %u\n", 30 - count_btn);
		}

		if(3 <= (count_btn / 10))
		{
			//system_soft_wdt_stop();

			system_restore();
			system_restart();

			break;
		}

		vTaskDelay(100 / portTICK_RATE_MS);
	}

	os_printf("Reset Config, Reboot...\n");
#endif
	vTaskDelete(NULL);
}

void ICACHE_FLASH_ATTR
wifi_event_callback(System_Event_t *event)
{
	static bool task_client_enable = false;

	if(NULL == event)
	{
		return;
	}

	switch (event->event_id)
	{
		case EVENT_STAMODE_SCAN_DONE:
			os_printf("### EVENT_STAMODE_SCAN_DONE\n");
			break;
		case EVENT_STAMODE_CONNECTED:
			os_printf("### EVENT_STAMODE_CONNECTED\n");

			if(NULL == global_flag)
			{
				global_flag = (uint32 *)malloc(sizeof(*global_flag));

				if(NULL != global_flag)
				{
					*global_flag = 0;
				}
			}
			break;
		case EVENT_STAMODE_DISCONNECTED:
			os_printf("### EVENT_STAMODE_DISCONNECTED\n");
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			os_printf("### EVENT_STAMODE_AUTHMODE_CHANGE\n");
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("### EVENT_STAMODE_GOT_IP\n");
			if(true != task_client_enable)
			{
				task_client_enable = true;

				xTaskCreate(send_key_task, "send_key_task", 4096, NULL, 2, NULL);
				xTaskCreate(wol_client_task, "wol_client_task", 4096, NULL, 2, NULL);
			}
			break;
		case EVENT_STAMODE_DHCP_TIMEOUT:
			os_printf("### EVENT_STAMODE_DHCP_TIMEOUT\n");
			break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			os_printf("### EVENT_SOFTAPMODE_STACONNECTED\n");
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			os_printf("### EVENT_SOFTAPMODE_STADISCONNECTED\n");
			break;
		case EVENT_SOFTAPMODE_PROBEREQRECVED:
			os_printf("### EVENT_SOFTAPMODE_PROBEREQRECVED\n");
			break;
		default:
			os_printf("### default\n");
			break;
	}
}

unsigned int ICACHE_FLASH_ATTR
user_get_flash_size(void)
{
	unsigned int addr = 0x0;

	char buffer[32] = {0};

	while(SPI_FLASH_RESULT_OK == spi_flash_read(addr, (unsigned int *)buffer, sizeof(buffer)))
	{
		addr += 0x1000;
	}

	return addr;
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
	flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map) {
		case FLASH_SIZE_4M_MAP_256_256:
			rf_cal_sec = 128 - 5;
			break;

		case FLASH_SIZE_8M_MAP_512_512:
			rf_cal_sec = 256 - 5;
			break;

		case FLASH_SIZE_16M_MAP_512_512:
		case FLASH_SIZE_16M_MAP_1024_1024:
			rf_cal_sec = 512 - 5;
			break;

		case FLASH_SIZE_32M_MAP_512_512:
		case FLASH_SIZE_32M_MAP_1024_1024:
			rf_cal_sec = 1024 - 5;
			break;
		case FLASH_SIZE_64M_MAP_1024_1024:
			rf_cal_sec = 2048 - 5;
			break;
		case FLASH_SIZE_128M_MAP_1024_1024:
			rf_cal_sec = 4096 - 5;
			break;
		default:
			rf_cal_sec = 0;
			break;
	}

	return rf_cal_sec;
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	unsigned int flash_size = user_get_flash_size();

#if defined(IO_INIT_FUNC)
	IO_INIT_FUNC();
#endif

	os_printf("SDK version:%s\n", system_get_sdk_version());

	if(0x100000 > flash_size)
	{
		os_printf("Flash size: %uKByte\n", flash_size / 1024);
	}
	else
	{
		os_printf("Flash size: %uMByte\n", flash_size / 1048576);
	}

	wifi_set_event_handler_cb(wifi_event_callback);

	wifi_set_opmode(STATION_MODE);

	xTaskCreate(button_task, "button_task", 512, NULL, 2, NULL);

	if(true != wifi_check_connect_able())
	{
		xTaskCreate(smartconfig_task, "smartconfig_task", 512, NULL, 2, NULL);
	}
	else
	{
		global_flag = (uint32 *)malloc(sizeof(*global_flag));

		if(NULL != global_flag)
		{
			*global_flag = 0;
		}
	}

#if defined(IO_LED_TO_PINID)
	xTaskCreate(led_task, "led_task", 512, NULL, 2, NULL);
#endif
}
