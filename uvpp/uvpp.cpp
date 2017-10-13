#include <assert.h>
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
}
