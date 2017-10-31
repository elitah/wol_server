/* ����ANSI�ĵ� */

#ifndef __LIB__EPOLL__H__
#define __LIB__EPOLL__H__

#ifdef __cplusplus

#include <sys/epoll.h>

class Lib_Epoll
{
public:
	// TODO: �������������������빫��
	~Lib_Epoll(void);

	bool checkListen(int fd);

	void setPtr(void *ptr);
	void *getPtr(void);

	unsigned int size(void);

	bool ctl(int fd, bool is_add);
	bool add(int fd, bool is_listen = false);
	bool del(int fd, bool is_close = true);

	void quit(void);
	bool loop(void);

	void wait(int timeout, void (*handle_call)(void *arg, int fd), void *arg);

	int waitSingle(int timeout);

	static Lib_Epoll *getObject(int count = 512);

private:
	// TODO: ˽���๹�캯����˽���Ƿ�ֱֹ�ӹ�������
	// TODO: ֻ��ͨ��getObject��������
	// TODO: ��ΪgetObject�����жϲ����Ƿ�Ϸ�
	Lib_Epoll(void);

	int m_fd;

	struct epoll_event *m_events;

	int m_count;

	unsigned int m_nums;

	int m_listen;

	void *m_ptr;
};

#endif

#endif
