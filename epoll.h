/* 这是ANSI文档 */

#ifndef __LIB__EPOLL__H__
#define __LIB__EPOLL__H__

#ifdef __cplusplus

#include <sys/epoll.h>

class Lib_Epoll
{
public:
	// TODO: 公有类析构函数，必须公有
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
	// TODO: 私有类构造函数，私有是防止直接构建对象
	// TODO: 只能通过getObject创建对象
	// TODO: 因为getObject可以判断参数是否合法
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
