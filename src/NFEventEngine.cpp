#include "NFEventEngine.h"
#include "uvpp.h"

namespace NetFoundation
{
	using namespace uvpp;

	struct EventEngine::Impl
	{
		UvLoop Loop_;
	};

	EventEngine::EventEngine() noexcept
		: Impl_(new Impl)
	{
	}

	EventEngine::EventEngine(EventEngine &&other) noexcept
		: Impl_(std::move(other.Impl_))
	{
	}

	EventEngine::~EventEngine()
	{
		Stop();
	}

	EventEngine& EventEngine::operator = (EventEngine &&other) noexcept
	{
		if (this != &other)
			Impl_ = std::move(other.Impl_);
		return *this;
	}

	bool EventEngine::RunDefault()
	{
		int status = Impl_->Loop_.Run(UvLoop::Default);
		if (status == 0)
			return true;
		return false;
	}

	bool EventEngine::RunOnce()
	{
		int status = Impl_->Loop_.Run(UvLoop::Once);
		if (status == 0)
			return true;
		return false;
	}

	bool EventEngine::RunNoWait()
	{
		int status = Impl_->Loop_.Run(UvLoop::NoWait);
		if (status == 0)
			return true;
		return false;
	}

	void EventEngine::Stop()
	{
		Impl_->Loop_.Stop();
	}

	UvLoop& EventEngine::InternalGetLoop() const
	{
		return Impl_->Loop_;
	}

	void EventEngine::InternalDelayDelete(UvHandle *pInternalHandle)
	{
		Impl_->Loop_.DelayDelete(pInternalHandle);
	}
}
