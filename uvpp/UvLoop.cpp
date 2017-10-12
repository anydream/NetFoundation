#include <uv.h>
#include "UvLoop.h"

namespace uvpp
{
	struct UvLoop::Impl
	{
		uv_loop_t UVLoop_ = {};

		~Impl()
		{
			Close();
		}

		int Close()
		{
			return uv_loop_close(&UVLoop_);
		}
	};

	UvLoop::UvLoop()
		: Impl_(new Impl)
	{
	}

	UvLoop::UvLoop(UvLoop &&other)
		: Impl_(std::move(other.Impl_))
	{
	}

	UvLoop::~UvLoop()
	{
	}

	UvLoop& UvLoop::operator = (UvLoop &&other)
	{
		if (this != &other)
			std::swap(Impl_, other.Impl_);
		return *this;
	}

	int UvLoop::Init()
	{
		return uv_loop_init(&Impl_->UVLoop_);
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
		return uv_run(&Impl_->UVLoop_, uvMode);
	}

	void UvLoop::Stop()
	{
		uv_stop(&Impl_->UVLoop_);
	}
}
