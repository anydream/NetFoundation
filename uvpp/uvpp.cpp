#include <uv.h>
#include "uvpp.h"

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	struct UvLoop::Impl
	{
		uv_loop_t Loop_ = {};

		~Impl()
		{
			Stop();
			Close();
		}

		void Stop()
		{
			uv_stop(&Loop_);
		}

		int Close()
		{
			return uv_loop_close(&Loop_);
		}
	};

	UvLoop::UvLoop()
		: Impl_(new Impl)
	{
	}

	UvLoop::UvLoop(UvLoop &&other) noexcept
		: Impl_(std::move(other.Impl_))
	{
	}

	UvLoop::~UvLoop()
	{
	}

	UvLoop& UvLoop::operator = (UvLoop &&other) noexcept
	{
		if (this != &other)
			std::swap(Impl_, other.Impl_);
		return *this;
	}

	int UvLoop::Init()
	{
		return uv_loop_init(&Impl_->Loop_);
	}

	int UvLoop::Close()
	{
		return Impl_->Close();
	}

	int UvLoop::Run(RunMode mode)
	{
		uv_run_mode uvMode = UV_RUN_DEFAULT;
		switch (mode)
		{
		case Default:
			uvMode = UV_RUN_DEFAULT;
			break;
		case Once:
			uvMode = UV_RUN_ONCE;
			break;
		case NoWait:
			uvMode = UV_RUN_NOWAIT;
			break;
		}
		return uv_run(&Impl_->Loop_, uvMode);
	}

	void UvLoop::Stop()
	{
		Impl_->Stop();
	}

	//////////////////////////////////////////////////////////////////////////
	struct UvIdle::Impl
	{
		uv_idle_t Idle_ = {};
		std::function<void()> Callback_;

		void UpdateDataPtr()
		{
			Idle_.data = this;
		}
	};

	UvIdle::UvIdle()
		: Impl_(new Impl)
	{
	}

	UvIdle::UvIdle(UvIdle &&other) noexcept
		: Impl_(std::move(other.Impl_))
	{
	}

	UvIdle::~UvIdle()
	{
	}

	UvIdle& UvIdle::operator = (UvIdle &&other) noexcept
	{
		if (this != &other)
			std::swap(Impl_, other.Impl_);
		return *this;
	}

	int UvIdle::Init(UvLoop &loop)
	{
		return uv_idle_init(&loop.Impl_->Loop_, &Impl_->Idle_);
	}

	int UvIdle::Start(std::function<void()> &&cbIdle)
	{
		Impl_->Callback_ = std::move(cbIdle);
		Impl_->UpdateDataPtr();

		return uv_idle_start(&Impl_->Idle_, [](uv_idle_t *handle)
		{
			if (UvIdle::Impl *self = static_cast<UvIdle::Impl*>(handle->data))
				self->Callback_();
		});
	}

	int UvIdle::Stop()
	{
		return uv_idle_stop(&Impl_->Idle_);
	}

	//////////////////////////////////////////////////////////////////////////
	struct UvTimer::Impl
	{
		uv_timer_t Timer_ = {};
		std::function<void()> Callback_;

		void UpdateDataPtr()
		{
			Timer_.data = this;
		}
	};

	UvTimer::UvTimer()
		: Impl_(new Impl)
	{
	}

	UvTimer::UvTimer(UvTimer &&other) noexcept
		: Impl_(std::move(other.Impl_))
	{
	}

	UvTimer::~UvTimer()
	{
	}

	UvTimer& UvTimer::operator = (UvTimer &&other) noexcept
	{
		if (this != &other)
			std::swap(Impl_, other.Impl_);
		return *this;
	}

	int UvTimer::Init(UvLoop &loop)
	{
		return uv_timer_init(&loop.Impl_->Loop_, &Impl_->Timer_);
	}

	int UvTimer::Start(std::function<void()> &&cbTimer, uint64_t timeout, uint64_t repeat)
	{
		Impl_->Callback_ = std::move(cbTimer);
		Impl_->UpdateDataPtr();

		return uv_timer_start(&Impl_->Timer_, [](uv_timer_t *handle)
		{
			if (UvTimer::Impl *self = static_cast<UvTimer::Impl*>(handle->data))
				self->Callback_();
		}, timeout, repeat);
	}

	int UvTimer::Stop()
	{
		return uv_timer_stop(&Impl_->Timer_);
	}

	int UvTimer::Again()
	{
		return uv_timer_again(&Impl_->Timer_);
	}

	void UvTimer::SetRepeat(uint64_t repeat)
	{
		uv_timer_set_repeat(&Impl_->Timer_, repeat);
	}

	uint64_t UvTimer::GetRepeat() const
	{
		return uv_timer_get_repeat(&Impl_->Timer_);
	}
}
