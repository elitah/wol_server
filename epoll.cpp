
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/epoll.h>

#include "ifconfig.h"
#include "epoll.h"

#ifdef __cplusplus

Lib_Epoll::Lib_Epoll(void)
	: m_fd(-1)
	, m_events(NULL)
	, m_count(0)
	, m_nums(0)
	, m_listen(-1)
	, m_ptr(NULL)
{
}

Lib_Epoll::~Lib_Epoll(void)
{
	if(0 <= m_fd)
	{
		close(m_fd);
		m_fd = -1;
	}

	if(NULL != m_events)
	{
		free(m_events);
		m_events = NULL;
	}

	m_count = 0;
	m_nums = 0;
	m_listen = -1;
	m_ptr = NULL;
}

bool Lib_Epoll::checkListen(int fd)
{
	return m_listen == fd;
}

void Lib_Epoll::setPtr(void *ptr)
{
	m_ptr = ptr;
}

void *Lib_Epoll::getPtr(void)
{
	return m_ptr;
}

unsigned int Lib_Epoll::size(void)
{
	return m_nums;
}

bool Lib_Epoll::ctl(int fd, bool is_add)
{
	struct epoll_event ev;

	if(0 > fd)
	{
		return false;
	}

	if(true == is_add)
	{
		if(0 != Lib_IfConfig::setSocketNonBlock(fd))
		{
			return false;
		}
	}

	ev.data.fd = fd;
	ev.events = EPOLLIN;

	if(0 == epoll_ctl(m_fd, true == is_add ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, fd, &ev))
	{
		if(true == is_add)
		{
			m_nums++;
		}
		else
		{
			m_nums--;
		}

		return true;
	}

	return false;
}

bool Lib_Epoll::add(int fd, bool is_listen)
{
	bool ret = false;

	ret = ctl(fd, true);

	if(true == ret)
	{
		if(true == is_listen)
		{
			m_listen = fd;
		}
	}

	return ret;
}

bool Lib_Epoll::del(int fd, bool is_close)
{
	bool ret = false;

	ret = ctl(fd, false);

	if(true == ret)
	{
		if(m_listen == fd)
		{
			m_listen = -1;
		}

		if(true == is_close)
		{
			close(fd);
			fd = -1;
		}
	}

	return ret;
}

void Lib_Epoll::quit(void)
{
	m_nums = 0;
}

bool Lib_Epoll::loop(void)
{
	return 0 < m_nums;
}

void Lib_Epoll::wait(int timeout, void (*handle_call)(void *arg, int fd), void *arg)
{
	int i = 0, epoll_events_count = 0;

	epoll_events_count = epoll_wait(m_fd, m_events, m_count, timeout);

	if(0 < epoll_events_count)
	{
		for(i = 0; epoll_events_count > i; i++)
		{
			handle_call(arg, m_events[i].data.fd);
		}
	}
}

int Lib_Epoll::waitSingle(int timeout)
{
	if(0 < epoll_wait(m_fd, m_events, 1, timeout))
	{
		return m_events[0].data.fd;
	}

	return -1;
}

Lib_Epoll *Lib_Epoll::getObject(int count)
{
	Lib_Epoll *obj = NULL;

	// TODO: 检查参数
	// TODO: 如果参数不合法，直接返回NULL到调用处
	if(0 >= count || 1024 < count)
	{
		goto out;
	}

	obj = new Lib_Epoll();

	if(NULL == obj)
	{
		goto out;
	}

	obj->m_fd = epoll_create1(0);

	if(0 > obj->m_fd)
	{
		goto out1;
	}

	obj->m_events = (struct epoll_event *)malloc(sizeof(*obj->m_events) * count);

	if(NULL == obj->m_events)
	{
		goto out2;
	}

	obj->m_count = count;

	goto out;

out2:
	if(0 <= obj->m_fd)
	{
		close(obj->m_fd);
		obj->m_fd = -1;
	}
out1:
	if(NULL != obj)
	{
		delete obj;
		obj = NULL;
	}
out:
	return obj;
}

#endif
