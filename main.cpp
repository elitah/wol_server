
#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ifconfig.h"
#include "epoll.h"
#include "json.h"

#define JSON_RET_STR	"{\"status\":%s,\"response\":%s}"

struct client_info
{
	bool enable;

	bool esp8266;

	unsigned long long int timestamp;

	char key[16];
	char ssid[32];
	char lan_ip[16];
	char global_ip[16];
};

static bool exit_flag = false;

static unsigned long long int getTimestamp(void)
{
	struct timespec ts = {0, 0};

	// 获取系统运行总时长
	clock_gettime(CLOCK_MONOTONIC, &ts);

	// 返回微秒级时间戳
	return (((unsigned long long int)ts.tv_sec) * 1000000ULL) + (((unsigned long long int)ts.tv_nsec) / 1000ULL);
}

static void send_udp(const char *ip, unsigned short port, char *data, unsigned int size)
{
	if(NULL != ip && 0 < port && NULL != data && 0 < size)
	{
		int sock_fd = -1;

		sock_fd = socket(AF_INET, SOCK_DGRAM, 0);

		if(0 <= sock_fd)
		{
			int ret = -1;

			struct sockaddr_in addr_serv;

			memset(&addr_serv, 0, sizeof(addr_serv));

			addr_serv.sin_family = AF_INET;
			addr_serv.sin_addr.s_addr = inet_addr(ip);
			addr_serv.sin_port = htons(port);

			ret = sendto(sock_fd, data, size, 0, (struct sockaddr *)&addr_serv, sizeof(addr_serv));

			if(0 < ret)
			{
				printf("%s: send ok\n", __func__);
			}
			else
			{
				printf("%s: send failed\n", __func__);
			}

			close(sock_fd);
		}
	}
}

static void handle_socket(Lib_Epoll *epoll, int fd, void *arg)
{
	struct client_info *client_list = (struct client_info *)arg;

	if(true == epoll->checkListen(fd))
	{
		int tmp_fd = -1;

		struct sockaddr_in remote_addr;
		socklen_t len = sizeof(remote_addr);

		tmp_fd = accept(fd, (struct sockaddr *)&remote_addr, &len);

		if(0 <= tmp_fd)
		{
			if(NULL != client_list)
			{
				if(true != epoll->add(tmp_fd))
				{
					close(tmp_fd);
					tmp_fd = -1;
				}
				else
				{
					printf("[Client %d] new\n", tmp_fd);

					client_list[tmp_fd].enable = true;
					client_list[tmp_fd].esp8266 = false;
					client_list[tmp_fd].timestamp = getTimestamp() + 10000000;

					memset(client_list[tmp_fd].key, 0, sizeof(client_list[tmp_fd].key));
					memset(client_list[tmp_fd].ssid, 0, sizeof(client_list[tmp_fd].ssid));
					memset(client_list[tmp_fd].lan_ip, 0, sizeof(client_list[tmp_fd].lan_ip));

					snprintf(client_list[tmp_fd].global_ip, sizeof(client_list[tmp_fd].global_ip), "%s", inet_ntoa(remote_addr.sin_addr));
				}
			}
		}
	}
	else
	{
		int ret = 0;

		char buffer[32 * 1024] = {0};

		std::string str_recv, str_send;

		bool ret_to_client = false;
		bool ret_to_ignore = false;

		unsigned long long int recv_start = getTimestamp();

		// 尝试接收一个完整的包
		while(1)
		{
			ret = ::recv(fd, buffer, sizeof(buffer) - 1, 0);

			// 收到数据
			if(0 < ret)
			{
				buffer[ret] = 0;

				str_recv += buffer;
			}
			// 对端已关闭
			else if(0 == ret)
			{
				printf("[Client %d] close\n", fd);

				if(NULL != client_list)
				{
					client_list[fd].enable = false;
				}

				epoll->del(fd);
				fd = -1;

				break;
			}
			// 没有收到数据（已有数据已接收完成）
			else
			{
				break;
			}

			usleep(1);
		}

		if(0 <= fd)
		{
			if(0 < str_recv.size())
			{
				printf("recv: %s\n", str_recv.c_str());

				if(NULL != client_list)
				{
					Lib_Json *json_recv = Lib_Json::getObject(str_recv.c_str(), str_recv.size());

					if(NULL != json_recv)
					{
						char cmd[32] = {0};

						//json_recv->dumpObject();

						if(true == json_recv->getValueString("cmd", cmd, sizeof(cmd)))
						{
							printf("cmd: %s, ", cmd);

							if(0 == strcmp(cmd, "response"))
							{
								ret_to_ignore = true;
							}
							else if(0 == strcmp(cmd, "bind"))
							{
								char key[32] = {0};
								char ssid[32] = {0};
								char ip[32] = {0};

								if(true == json_recv->getValueString("key", key, sizeof(key)) \
									&& true == json_recv->getValueString("ssid", ssid, sizeof(ssid)) \
									&& true == json_recv->getValueString("ip", ip, sizeof(ip)))
								{
									if(0 < strlen(key) && 0 < strlen(ssid) && 0 < strlen(ip))
									{
										client_list[fd].esp8266 = true;

										snprintf(client_list[fd].key, sizeof(client_list[fd].key), "%s", key);
										snprintf(client_list[fd].ssid, sizeof(client_list[fd].ssid), "%s", ssid);
										snprintf(client_list[fd].lan_ip, sizeof(client_list[fd].lan_ip), "%s", ip);

										ret_to_client = true;
									}
								}
							}
							else if(0 == strcmp(cmd, "client_list"))
							{
								unsigned int count = 0;

								str_send += "[";

								for(unsigned int i = 0; 1024 > i; i++)
								{
									if(true == client_list[i].enable)
									{
										count++;

										if(true == client_list[i].esp8266)
										{
											ret = snprintf(buffer, sizeof(buffer), \
																	"{"
																	"\"key\":\"%s\","
																	"\"ssid\":\"%s\","
																	"\"lan_ip\":\"%s\","
																	"\"global_ip\":\"%s\""
																	"}", \
																	client_list[i].key, \
																	client_list[i].ssid, \
																	client_list[i].lan_ip, \
																	client_list[i].global_ip);

											if(1 < str_send.size())
											{
												str_send += ",";
											}

											str_send += buffer;
										}
									}
								}

								str_send += "]";

								snprintf(buffer, sizeof(buffer), "{\"count\":%u,\"list\":", count);

								str_send = buffer + str_send + "}";

								ret_to_client = true;
							}
							else if(0 == strcmp(cmd, "wol"))
							{
								char key[32] = {0};
								char mac[32] = {0};

								if(true == json_recv->getValueString("key", key, sizeof(key)) \
									&& true == json_recv->getValueString("mac", mac, sizeof(mac)))
								{
									if(0 < strlen(key) && 0 < strlen(mac))
									{
										unsigned int macval_int[6] = {0};

										if(6 == sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", &macval_int[0], &macval_int[1], &macval_int[2], &macval_int[3], &macval_int[4], &macval_int[5])
											|| 6 == sscanf(mac, "%02x-%02x-%02x-%02x-%02x-%02x", &macval_int[0], &macval_int[1], &macval_int[2], &macval_int[3], &macval_int[4], &macval_int[5]))
										{
											unsigned char macval_char[6] = {0};

											macval_char[0] = (unsigned char)(macval_int[0] & 0xFF);
											macval_char[1] = (unsigned char)(macval_int[1] & 0xFF);
											macval_char[2] = (unsigned char)(macval_int[2] & 0xFF);
											macval_char[3] = (unsigned char)(macval_int[3] & 0xFF);
											macval_char[4] = (unsigned char)(macval_int[4] & 0xFF);
											macval_char[5] = (unsigned char)(macval_int[5] & 0xFF);

											for(unsigned int i = 0; 1024 > i; i++)
											{
												if(true == client_list[i].enable && true == client_list[i].esp8266)
												{
													if(0 == strcmp(client_list[i].key, key))
													{
														ret = snprintf(buffer, sizeof(buffer), \
																		"{\"cmd\":\"wol_send\",\"mac\":[%u,%u,%u,%u,%u,%u]}", \
																		macval_char[0], macval_char[1], macval_char[2], \
																		macval_char[3], macval_char[4], macval_char[5]);

														if(0 < ret)
														{
															send(i, buffer, ret, 0);

															ret_to_client = true;
														}

														break;
													}
												}
											}
										}
									}
								}
							}
							else if(0 == strcmp(cmd, "shutdown"))
							{
								char key[32] = {0};
								char mac[32] = {0};

								if(true == json_recv->getValueString("key", key, sizeof(key)) \
									&& true == json_recv->getValueString("mac", mac, sizeof(mac)))
								{
									if(0 < strlen(key) && 0 < strlen(mac))
									{
										unsigned int macval_int[6] = {0};

										if(6 == sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", &macval_int[0], &macval_int[1], &macval_int[2], &macval_int[3], &macval_int[4], &macval_int[5])
											|| 6 == sscanf(mac, "%02x-%02x-%02x-%02x-%02x-%02x", &macval_int[0], &macval_int[1], &macval_int[2], &macval_int[3], &macval_int[4], &macval_int[5]))
										{
											unsigned char macval_char[6] = {0};

											macval_char[0] = (unsigned char)(macval_int[0] & 0xFF);
											macval_char[1] = (unsigned char)(macval_int[1] & 0xFF);
											macval_char[2] = (unsigned char)(macval_int[2] & 0xFF);
											macval_char[3] = (unsigned char)(macval_int[3] & 0xFF);
											macval_char[4] = (unsigned char)(macval_int[4] & 0xFF);
											macval_char[5] = (unsigned char)(macval_int[5] & 0xFF);

											for(unsigned int i = 0; 1024 > i; i++)
											{
												if(true == client_list[i].enable && true == client_list[i].esp8266)
												{
													if(0 == strcmp(client_list[i].key, key))
													{
														ret = snprintf(buffer, sizeof(buffer), \
																		"{\"cmd\":\"shutdown_send\",\"mac\":[%u,%u,%u,%u,%u,%u]}", \
																		macval_char[0], macval_char[1], macval_char[2], \
																		macval_char[3], macval_char[4], macval_char[5]);

														if(0 < ret)
														{
															send(i, buffer, ret, 0);

															ret_to_client = true;
														}

														break;
													}
												}
											}
										}
									}
								}
							}
							else if(0 == strcmp(cmd, "warn"))
							{
								if(true == client_list[fd].enable && true == client_list[fd].esp8266)
								{
									void *x = epoll->getPtr();

									if(NULL != x)
									{
										unsigned short port = *(unsigned short *)x;

										if(0 < port)
										{
											ret = snprintf(buffer, sizeof(buffer), "{\"cmd\":\"send_notice\",\"key\":\"%s\"}", client_list[fd].key);

											if(0 < ret)
											{
												send_udp("127.0.0.1", port, buffer, ret);
											}
										}
									}
								}
							}
							else if(0 == strcmp(cmd, "reset"))
							{
								char key[32] = {0};

								if(true == json_recv->getValueString("key", key, sizeof(key)))
								{
									if(0 < strlen(key))
									{
										for(unsigned int i = 0; 1024 > i; i++)
										{
											if(true == client_list[i].enable && true == client_list[i].esp8266)
											{
												if(0 == strcmp(client_list[i].key, key))
												{
													ret = snprintf(buffer, sizeof(buffer), "{\"cmd\":\"reset\"}");

													if(0 < ret)
													{
														send(i, buffer, ret, 0);

														ret_to_client = true;
													}

													break;
												}
											}
										}
									}
								}
							}
							else if(0 == strcmp(cmd, "beat"))
							{
								ret_to_client = true;
							}
						}

						delete json_recv;
						json_recv = NULL;
					}
				}
			}

			if(true != ret_to_ignore)
			{
				ret = snprintf(buffer, sizeof(buffer), JSON_RET_STR, ret_to_client ? "true" : "false", 0 < str_send.size() ? str_send.c_str() : "null");

				if(0 < ret)
				{
					if(true == ret_to_client)
					{
						client_list[fd].timestamp = getTimestamp() + 10000000;

						printf("Response Success! ");
					}
					else
					{
						printf("Response Failed! ");
					}

					send(fd, buffer, ret, 0);

					printf("Time consumption: %llu us\n", getTimestamp() - recv_start);
				}
			}
		}
	}
}

static int listen_on(unsigned short port)
{
	int listen_fd = -1;

	struct sockaddr_in local_addr;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(0 > listen_fd)
	{
		goto out;
	}

	if(0 != Lib_IfConfig::setSocketReUseAddr(listen_fd))
	{
		goto out1;
	}

	memset(&local_addr, 0, sizeof(local_addr));

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(port);

	if(0 != bind(listen_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)))
	{
		goto out1;
	}

	if(0 != listen(listen_fd, 5))
	{
		goto out1;
	}

	goto out;

out1:
	if(0 <= listen_fd)
	{
		close(listen_fd);
		listen_fd = -1;
	}
out:
	return listen_fd;
}

static void sigHandleFunc(int sig)
{
	switch(sig)
	{
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			exit_flag = true;
			break;
		default:
			break;
	}
}

static int deamon_create(bool print = true)
{
	int i = 0;

	int fd[3] = {0, 0, 0};

	if(0 == fork())
	{
		setsid();

		if(0 == fork())
		{
			setsid();

			chdir("/");
			umask(0);

			if(true == print)
			{
				return 0;
			}
			else
			{
				for(i = 0; i < 1024; i++)
				{
					close(i);
				}

				fd[0] = open("/dev/null", O_RDWR);
				fd[1] = dup(0);
				fd[2] = dup(0);

				if(0 == fd[0] && 1 == fd[1] && 2 == fd[2])
				{
					return 0;
				}
			}
		}
	}

	return -1;
}

int doMainFunc(unsigned short port, unsigned short port_warn)
{
	Lib_Epoll *epoll = NULL;

	struct client_info *client_list = NULL;

	int listen_fd = -1;

	unsigned long long int timestamp = 0;
	unsigned long long int timestamp_checktimeout = 0;

	signal(SIGINT, sigHandleFunc);
	signal(SIGQUIT, sigHandleFunc);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, sigHandleFunc);

	printf("[Main] start to create epoll\n");

	epoll = Lib_Epoll::getObject(512);

	if(NULL == epoll)
	{
		goto out;
	}

	client_list = (struct client_info *)malloc(sizeof(*client_list) * 1024);

	if(NULL == client_list)
	{
		goto out1;
	}

	for(unsigned int i = 0; 1024 > i; i++)
	{
		client_list[i].enable = false;
	}

	printf("[Main] start to listen: port: %u\n", port);

	listen_fd = listen_on(port);

	if(0 > listen_fd)
	{
		goto out1;
	}

	printf("[Main] create socket ok(%d)\n", listen_fd);

	if(true != epoll->add(listen_fd, true))
	{
		goto out2;
	}

	printf("[Main] add to epoll ok\n");

	epoll->setPtr(&port_warn);

	while(true != exit_flag && epoll->loop())
	{
		timestamp = getTimestamp();

		if(timestamp_checktimeout < timestamp)
		{
			timestamp_checktimeout = timestamp + 10000000;

			for(unsigned int i = 0; 1024 > i; i++)
			{
				if(true == client_list[i].enable)
				{
					if(client_list[i].timestamp < timestamp)
					{
						printf("[Client %d] timeout\n", i);

						epoll->del(i);

						client_list[i].enable = false;
						client_list[i].timestamp = 0;
					}
				}
			}
		}

		epoll->wait(1000, handle_socket, client_list);

		printf("[Main] client total: %u\n", epoll->size());
	}

out2:
	close(listen_fd);
	listen_fd = -1;
out1:
/*
	{
		void *ptr = epoll->getPtr();

		if(NULL != ptr)
		{
			free(ptr);
			ptr = NULL;
		}

		epoll->setPtr(NULL);
	}
*/
	delete epoll;
	epoll = NULL;
out:
	printf("[Main] Exit\n");

	return 0;
}

int main(int argc, char **argv)
{
	int oc = 0;

	unsigned short port = 50173;
	unsigned short port_warn = 50174;

	bool foreground = false;

	if(1 < argc)
	{
		while(0 < (oc = getopt(argc, argv, "fp:w:")))
		{
			switch(oc)
			{
				case 'f':
					foreground = true;
					break;
				case 'p':
					port = (unsigned short)strtoul(optarg, NULL, 0);
					if(0 == port)
					{
						goto out;
					}
					break;
				case 'w':
					port_warn = (unsigned short)strtoul(optarg, NULL, 0);
					break;
				case '?':
					goto out;
					break;
				case ':':
					goto out;
					break;
			}
		}
	}

	if(true != foreground)
	{
		if(0 != deamon_create(foreground))
		{
			return 0;
		}
	}

	return doMainFunc(port, port_warn);

out:
	return 0;
}
