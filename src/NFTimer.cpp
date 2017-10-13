#include "NFTimer.h"
#include "uvpp.h"

namespace NetFoundation
{
	using namespace uvpp;

	struct Timer::Impl
	{
		EventEngine &EE_;
		UvTimer *Timer_ = nullptr;

		explicit Impl(EventEngine &pEE)
			: EE_(pEE)
		{
			Timer_ = new UvTimer;
			Timer_->Init(EE_.InternalGetLoop());
		}

		~Impl()
		{
			EE_.InternalDelayDelete(Timer_);
		}
	};

	Timer::Timer(EventEngine &ee) noexcept
		: Impl_(new Impl(ee))
	{
	}

	Timer::Timer(Timer &&other) noexcept
		: Impl_(std::move(other.Impl_))
	{
	}

	Timer::~Timer()
	{
	}

	Timer& Timer::operator = (Timer &&other) noexcept
	{
		if (this != &other)
			Impl_ = std::move(other.Impl_);
		return *this;
	}

	bool Timer::Start(uint64_t timeout, uint64_t repeat, std::function<void()> &&cbTimer)
	{
		int status = Impl_->Timer_->Start(timeout, repeat, std::move(cbTimer));
		if (status == 0)
			return true;
		return false;
	}

	bool Timer::Stop()
	{
		int status = Impl_->Timer_->Stop();
		if (status == 0)
			return true;
		return false;
	}

	bool Timer::Again()
	{
		int status = Impl_->Timer_->Again();
		if (status == 0)
			return true;
		return false;
	}

	void Timer::SetRepeat(uint64_t repeat)
	{
		Impl_->Timer_->SetRepeat(repeat);
	}

	uint64_t Timer::GetRepeat() const
	{
		return Impl_->Timer_->GetRepeat();
	}
}
