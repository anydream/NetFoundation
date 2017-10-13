﻿#include <assert.h>
#include "uvpp.h"

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	const uv_handle_t* UvHandle::GetRawHandle() const
	{
		return const_cast<UvHandle*>(this)->GetRawHandle();
	}

	bool UvHandle::IsActive() const
	{
		return uv_is_active(GetRawHandle()) != 0;
	}

	bool UvHandle::IsClosing() const
	{
		return uv_is_closing(GetRawHandle()) != 0;
	}

	//////////////////////////////////////////////////////////////////////////
	UvLoop::UvLoop()
	{
		int status = uv_loop_init(&Loop_);
		assert(status == 0);
	}

	UvLoop::~UvLoop()
	{
		Stop();
		int status = uv_loop_close(&Loop_);
		assert(status == 0);
	}

	int UvLoop::Run(RunMode mode)
	{
		IsRunning_ = true;
		return uv_run(&Loop_, static_cast<uv_run_mode>(mode));
	}

	void UvLoop::Stop()
	{
		uv_stop(&Loop_);
		IsRunning_ = false;
	}

	bool UvLoop::Alive() const
	{
		return uv_loop_alive(&Loop_) != 0;
	}

	uint64_t UvLoop::Now() const
	{
		return uv_now(&Loop_);
	}

	void UvLoop::DelayDelete(UvHandle *pHandle)
	{
		assert(pHandle);
		if (IsRunning_)
		{
			uv_handle_t *pRawHandle = pHandle->GetRawHandle();
			assert(pRawHandle);

			pRawHandle->data = pHandle;
			uv_close(pRawHandle, [](uv_handle_t *handle)
			{
				UvHandle *pHandle = static_cast<UvHandle*>(handle->data);
				assert(pHandle);
				delete pHandle;
			});
		}
		else
			delete pHandle;
	}

	uv_loop_t* UvLoop::GetRawLoop()
	{
		return &Loop_;
	}

	bool UvLoop::IsRunning() const
	{
		return IsRunning_;
	}

	//////////////////////////////////////////////////////////////////////////
	UvTimer::~UvTimer()
	{
		Stop();
	}

	int UvTimer::Init(UvLoop &loop)
	{
		int status = uv_timer_init(loop.GetRawLoop(), &Timer_);
		Timer_.data = this;
		return status;
	}

	int UvTimer::Start(uint64_t timeout, uint64_t repeat, std::function<void()> &&cbTimer)
	{
		assert(cbTimer);
		Callback_ = cbTimer;
		return uv_timer_start(&Timer_, [](uv_timer_t *handle)
		{
			UvTimer *pHandle = static_cast<UvTimer*>(handle->data);
			assert(pHandle);
			pHandle->Callback_();
		}, timeout, repeat);
	}

	int UvTimer::Stop()
	{
		return uv_timer_stop(&Timer_);
	}

	int UvTimer::Again()
	{
		return uv_timer_again(&Timer_);
	}

	void UvTimer::SetRepeat(uint64_t repeat)
	{
		uv_timer_set_repeat(&Timer_, repeat);
	}

	uint64_t UvTimer::GetRepeat() const
	{
		return uv_timer_get_repeat(&Timer_);
	}

	uv_handle_t* UvTimer::GetRawHandle()
	{
		return reinterpret_cast<uv_handle_t*>(&Timer_);
	}

	//////////////////////////////////////////////////////////////////////////
	int UvStream::Listen(CbConnect &&cbConnect, int backlog)
	{
		assert(cbConnect);
		CallbackConnect_ = cbConnect;
		return uv_listen(GetRawStream(), backlog, [](uv_stream_t *server, int status)
		{
			UvStream *pStream = static_cast<UvStream*>(server->data);
			assert(pStream);
			pStream->CallbackConnect_(pStream, status);
		});
	}

	int UvStream::Accept(UvStream *client)
	{
		return uv_accept(GetRawStream(), client->GetRawStream());
	}

	int UvStream::ReadStart(CbRead &&cbRead, CbAlloc &&cbAlloc)
	{
		assert(cbRead && cbAlloc);
		CallbackRead_ = cbRead;
		CallbackAlloc_ = cbAlloc;
		return uv_read_start(GetRawStream(), [](uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
		{
			UvStream *pStream = static_cast<UvStream*>(handle->data);
			assert(pStream);
			pStream->CallbackAlloc_(pStream, suggested_size, reinterpret_cast<UvBuf*>(buf));
		}, [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
		{
			UvStream *pStream = static_cast<UvStream*>(stream->data);
			assert(pStream);
			pStream->CallbackRead_(pStream, nread, reinterpret_cast<UvBuf*>(const_cast<uv_buf_t*>(buf)));
		});
	}

	//////////////////////////////////////////////////////////////////////////
	int UvTCP::Init(UvLoop &loop)
	{
		int status = uv_tcp_init(loop.GetRawLoop(), &TCP_);
		TCP_.data = this;
		return status;
	}

	int UvTCP::Bind(const sockaddr *addr, uint32_t flags)
	{
		return uv_tcp_bind(&TCP_, addr, flags);
	}

	int UvTCP::GetSockName(sockaddr *name, int *namelen) const
	{
		return uv_tcp_getsockname(&TCP_, name, namelen);
	}

	int UvTCP::GetPeerName(sockaddr *name, int *namelen) const
	{
		return uv_tcp_getpeername(&TCP_, name, namelen);
	}

	sockaddr_storage UvTCP::GetPeerName() const
	{
		sockaddr_storage addrs;
		int len = sizeof(addrs);
		int status = GetPeerName(reinterpret_cast<sockaddr*>(&addrs), &len);
		assert(status == 0);
		return addrs;
	}

	uv_stream_t* UvTCP::GetRawStream()
	{
		return reinterpret_cast<uv_stream_t*>(&TCP_);
	}

	uv_handle_t* UvTCP::GetRawHandle()
	{
		return reinterpret_cast<uv_handle_t*>(&TCP_);
	}

	//////////////////////////////////////////////////////////////////////////
	int UvMisc::ToAddrIPv4(const char *ip, int port, sockaddr_in *addr)
	{
		return uv_ip4_addr(ip, port, addr);
	}

	int UvMisc::ToAddrIPv6(const char *ip, int port, sockaddr_in6 *addr)
	{
		return uv_ip6_addr(ip, port, addr);
	}

	std::string UvMisc::ToNameIPv4(const sockaddr_in *addr, int *port)
	{
		const size_t IpSize = 16;
		char ip[IpSize];
		int status = uv_ip4_name(addr, ip, IpSize);
		if (status == 0)
		{
			if (port)
				*port = ntohs(addr->sin_port);
			return ip;
		}
		return std::string();
	}

	std::string UvMisc::ToNameIPv6(const sockaddr_in6 *addr, int *port)
	{
		const size_t IpSize = 46;
		char ip[IpSize];
		int status = uv_ip6_name(addr, ip, IpSize);
		if (status == 0)
		{
			if (port)
				*port = ntohs(addr->sin6_port);
			return ip;
		}
		return std::string();
	}
}
