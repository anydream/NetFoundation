#include <assert.h>
#include "uvpp.h"

namespace uvpp
{
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

	uv_loop_t* UvLoop::GetRawHandle()
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
		int status = uv_timer_init(loop.GetRawHandle(), &Timer_);
		Timer_.data = this;
		return status;
	}

	int UvTimer::Start(std::function<void()> &&cbTimer, uint64_t timeout, uint64_t repeat)
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

	UvTimer* UvTimer::New()
	{
		return new UvTimer;
	}
}
