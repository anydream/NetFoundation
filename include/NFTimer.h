#pragma once

#include <functional>
#include "NFEventEngine.h"

namespace NetFoundation
{
	class Timer
	{
	public:
		explicit Timer(EventEngine &ee) noexcept;
		Timer(Timer &&other) noexcept;
		~Timer();

		Timer& operator = (Timer &&other) noexcept;

		bool Start(uint64_t timeout, uint64_t repeat, std::function<void()> &&cbTimer);
		bool Stop();
		bool Again();

		void SetRepeat(uint64_t repeat);
		uint64_t GetRepeat() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> Impl_;
	};
}
