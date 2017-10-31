
#if defined(__cplusplus)
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "ifconfig.h"

#ifdef __cplusplus

Lib_IfConfig::Lib_IfConfig(void)
{
}

Lib_IfConfig::~Lib_IfConfig(void)
{
}

bool Lib_IfConfig::checkMaskValidity(in_addr_t mask)
{
	in_addr_t little = htonl(mask);

	if(0xFFFFFFFF == ((little - 1) | little))
	{
		return true;
	}

	return false;
}

int Lib_IfConfig::setSocketNonBlock(int fd)
{
	int opts = fcntl(fd, F_GETFL);

	if(0 > opts)
	{
		return -1;
	}

	if(0 > fcntl(fd, F_SETFL, opts | O_NONBLOCK))
	{
		return -1;
	}

	return 0;
}

int Lib_IfConfig::setSocketReUseAddr(int fd)
{
	return Lib_IfConfig::interfaceSetsockopt(fd, SO_REUSEADDR, 1);
}

int Lib_IfConfig::setSocketBroadCast(int fd)
{
	return Lib_IfConfig::interfaceSetsockopt(fd, SO_BROADCAST, 1);
}

int Lib_IfConfig::setInterfaceIpAddr(const char *ifname, in_addr_t data)
{
	return Lib_IfConfig::interfaceIoctl(ifname, (unsigned char *)&data, SIOCSIFADDR);
}

int Lib_IfConfig::setInterfaceMask(const char *ifname, in_addr_t data)
{
	if(true != checkMaskValidity(data))
	{
		return false;
	}

	return Lib_IfConfig::interfaceIoctl(ifname, (unsigned char *)&data, SIOCSIFNETMASK);
}

int Lib_IfConfig::setInterfacePhyAddr(const char *ifname, unsigned char *data)
{
	return Lib_IfConfig::interfaceIoctl(ifname, data, SIOCSIFHWADDR);
}

int Lib_IfConfig::getInterfaceIpAddr(const char *ifname, in_addr_t *data)
{
	return Lib_IfConfig::interfaceIoctl(ifname, (unsigned char *)data, SIOCGIFADDR);
}

int Lib_IfConfig::getInterfaceMask(const char *ifname, in_addr_t *data)
{
	return Lib_IfConfig::interfaceIoctl(ifname, (unsigned char *)data, SIOCGIFNETMASK);
}

int Lib_IfConfig::getInterfacePhyAddr(const char *ifname, unsigned char *data)
{
	return Lib_IfConfig::interfaceIoctl(ifname, data, SIOCGIFHWADDR);
}

int Lib_IfConfig::getInterfaceFlags(const char *ifname, unsigned char *data)
{
	return Lib_IfConfig::interfaceIoctl(ifname, data, SIOCGIFFLAGS);
}

int Lib_IfConfig::getInterfaceIndex(const char *ifname)
{
	return Lib_IfConfig::interfaceIoctl(ifname, NULL, SIOCGIFINDEX);
}

unsigned long long int Lib_IfConfig::getInterfaceRxPackets(const char *ifname)
{
	int fd = -1;

	char buffer[128] = {0};

	unsigned long long int rx_packets = 0;

	if(NULL != ifname)
	{
		snprintf(buffer, sizeof(buffer), "/sys/class/net/%s/statistics/rx_packets", ifname);

		fd = open(buffer, O_RDONLY);

		if(0 <= fd)
		{
			memset(buffer, 0, sizeof(buffer));

			read(fd, buffer, sizeof(buffer));

			close(fd);
			fd = -1;

			rx_packets = strtoull(buffer, NULL, 0);
		}
	}

	return rx_packets;
}

unsigned long long int Lib_IfConfig::getInterfaceTxPackets(const char *ifname)
{
	int fd = -1;

	char buffer[128] = {0};

	unsigned long long int tx_packets = 0;

	if(NULL != ifname)
	{
		snprintf(buffer, sizeof(buffer), "/sys/class/net/%s/statistics/tx_packets", ifname);

		fd = open(buffer, O_RDONLY);

		if(0 <= fd)
		{
			memset(buffer, 0, sizeof(buffer));

			read(fd, buffer, sizeof(buffer));

			close(fd);
			fd = -1;

			tx_packets = strtoull(buffer, NULL, 0);
		}
	}

	return tx_packets;
}

bool Lib_IfConfig::isInterfaceCableConnected(const char *ifname)
{
	unsigned int flags = 0;

	if(0 == Lib_IfConfig::getInterfaceFlags(ifname, (unsigned char *)&flags))
	{
		if(0 != (IFF_UP & flags) && 0 != (IFF_RUNNING & flags))
		{
			return true;
		}
	}

	return false;
}

bool Lib_IfConfig::addRoutingTable(const char *ifname)
{
	return true;
}

bool Lib_IfConfig::getDefaultRoute(const char *ifname, in_addr_t *data)
{
	bool ret = false;

#if defined(__cplusplus)
	std::ifstream ifs("/proc/net/route");

	std::string line, word;

	// 取出每一行
	while(std::getline(ifs, line))
	{
		// 设备路径检测
		if(1024 > line.size())
		{
			std::istringstream stream(line);

			word.clear();

			stream >> word;

			if(0 < word.size() && 0 == strcmp(word.c_str(), ifname))
			{
				unsigned int dst = 0;
				unsigned int gate = 0;
				unsigned int flag = 0;

				for(int i = 0; 3 > i; i++)
				{
					stream >> word;

					switch(i)
					{
						case 0:
							dst = strtoul(word.c_str(), NULL, 16);
							break;
						case 1:
							gate = strtoul(word.c_str(), NULL, 16);
							break;
						case 2:
							flag = strtoul(word.c_str(), NULL, 16);
							break;
						default:
							break;
					}
				}

				if(0 == dst && 0x2 == (flag & 0x2))
				{
					*data = gate;

					ret = true;

					break;
				}
			}
		}
	}
#else
	int retval = 0;

	char devname[64] = {0};
	unsigned long d = 0, g = 0, m = 0;
	int flgs = 0, ref = 0, use = 0, metric = 0, mtu = 0, win = 0, ir = 0;

	FILE *fp = NULL;

	if(NULL == data)
	{
		goto out;
	}

	*data = 0;

	fp = fopen("/proc/net/route", "r");

	if(NULL != fp)
	{
		retval = fscanf(fp, "%*[^\n]\n");

		if(0 > retval)
		{
			goto out;
		}

		while(1)
		{
			if(0 != feof(fp))
			{
				break;
			}

			memset(devname, 0, sizeof(devname));

			retval = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n", devname, &d, &g, &flgs, &ref, &use, &metric, &m, &mtu, &win, &ir);

			if(11 != retval)
			{
				break;
			}

			if(0 == strcmp(ifname, devname))
			{
				if((0 == d) && (0x2 == (flgs & 0x2)))
				{
					*data = g;

					ret = true;

					break;
				}
			}
		}
	}

out:
	if(NULL != fp)
	{
		fclose(fp);
		fp = NULL;
	}
#endif
	return ret;
}

bool Lib_IfConfig::IpNumberToIpString(char *buffer, int size, in_addr_t addr)
{
	if(NULL != buffer && 32 <= size)
	{
		snprintf(buffer, size, "%u.%u.%u.%u", ((unsigned char *)&addr)[0], ((unsigned char *)&addr)[1], ((unsigned char *)&addr)[2], ((unsigned char *)&addr)[3]);

		return true;
	}

	return false;
}

int Lib_IfConfig::interfaceSetsockopt(int fd, int ops, int opt)
{
	return 0 == setsockopt(fd, SOL_SOCKET, ops, &opt, sizeof(opt)) ? 0 : -1;
}

int Lib_IfConfig::interfaceIoctl(const char *ifname, unsigned char *data, int ops)
{
	int ret = -1;

	int sockfd = 0;

	struct ifreq req;
	struct sockaddr_in *p = NULL;

	if(NULL == ifname)
	{
		goto out;
	}

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	if(0 > sockfd)
	{
		goto out;
	}

	memset(&req, 0, sizeof(req));
	strncpy(req.ifr_name, ifname, sizeof(req.ifr_name));

	if(NULL != data)
	{
		switch(ops)
		{
			case SIOCSIFADDR:			// set addr
			case SIOCSIFNETMASK:		// set mask
				p = (struct sockaddr_in *)&(req.ifr_addr);

				p->sin_family = AF_INET;
				p->sin_addr.s_addr = *(in_addr_t *)data;

				ret = ioctl(sockfd, ops, &req);

				break;
			case SIOCSIFHWADDR:			// set phy addr
				for(int i = 0; 6 > i; i++)
				{
					req.ifr_hwaddr.sa_data[i] = data[i];
				}

				ret = ioctl(sockfd, ops, &req);

				break;
			case SIOCGIFADDR:			// get addr
			case SIOCGIFNETMASK:		// get mask
				ret = ioctl(sockfd, ops, &req);

				if(0 == ret)
				{
					for(int i = 2; 6 > i; i++)
					{
						data[i - 2] = req.ifr_addr.sa_data[i];
					}
				}

				break;
			case SIOCGIFHWADDR:			// get phy addr
				ret = ioctl(sockfd, ops, &req);

				if(0 == ret)
				{
					for(int i = 0; 6 > i; i++)
					{
						data[i] = req.ifr_hwaddr.sa_data[i];
					}
				}

				break;
			case SIOCGIFFLAGS:			// get flags
				ret = ioctl(sockfd, ops, &req);

				if(0 == ret)
				{
					*(unsigned int *)data = req.ifr_flags;
				}

				break;
			default:
				break;
		}
	}
	else
	{
		switch(ops)
		{
		case SIOCGIFINDEX:
			ret = ioctl(sockfd, ops, &req);

			if(0 == ret)
			{
				ret = req.ifr_ifindex;
			}
			break;
		default:
			break;
		}
	}

	goto out1;

out1:
	if(0 <= sockfd)
	{
		close(sockfd);
		sockfd = -1;
	}
out:
	return ret;
}

int Lib_IfConfig::socket_listen_on(int type, unsigned short port)
{
	int listen_fd = -1;

	struct sockaddr_in local_addr;

	if(SOCK_STREAM != type && SOCK_DGRAM != type)
	{
		goto out;
	}

	listen_fd = socket(AF_INET, type, 0);

	if(0 > listen_fd)
	{
		goto out;
	}

	if(0 != setSocketReUseAddr(listen_fd))
	{
		goto out1;
	}

	memset(&local_addr, 0, sizeof(local_addr));

	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(port);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	if(0 != bind(listen_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)))
	{
		goto out1;
	}

	if(SOCK_STREAM == type)
	{
		if(0 != listen(listen_fd, 5))
		{
			goto out1;
		}
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
#endif
