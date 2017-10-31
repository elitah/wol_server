/* ’‚ «ANSIŒƒµµ */

#ifndef __LIB__IFCONFIG__H__
#define __LIB__IFCONFIG__H__

#include <netinet/in.h>
#include <net/if.h>

#include <arpa/inet.h>

#ifdef __cplusplus

class Lib_IfConfig
{
public:
	~Lib_IfConfig(void);

	static bool checkMaskValidity(in_addr_t mask);

	static int setSocketNonBlock(int fd);
	static int setSocketReUseAddr(int fd);
	static int setSocketBroadCast(int fd);

	static int setInterfaceIpAddr(const char *ifname, in_addr_t data);
	static int setInterfaceMask(const char *ifname, in_addr_t data);
	static int setInterfacePhyAddr(const char *ifname, unsigned char *data);

	static int getInterfaceIpAddr(const char *ifname, in_addr_t *data);
	static int getInterfaceMask(const char *ifname, in_addr_t *data);
	static int getInterfacePhyAddr(const char *ifname, unsigned char *data);

	static int getInterfaceFlags(const char *ifname, unsigned char *data);
	static int getInterfaceIndex(const char *ifname);

	static unsigned long long int getInterfaceRxPackets(const char *ifname);
	static unsigned long long int getInterfaceTxPackets(const char *ifname);

	static bool isInterfaceCableConnected(const char *ifname);

	static bool addRoutingTable(const char *ifname);

	static bool getDefaultRoute(const char *ifname, in_addr_t *data);

	static bool IpNumberToIpString(char *buffer, int size, in_addr_t addr);

	static int interfaceSetsockopt(int fd, int ops, int opt);
	static int interfaceIoctl(const char *ifname, unsigned char *data, int ops);

	static int socket_listen_on(int type, unsigned short port);

private:
	Lib_IfConfig(void);
};

#endif

#endif
