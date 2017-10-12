#pragma once

#include <memory>

namespace uvpp
{
	class UvLoop;
	class UvHandle;
}

namespace NetFoundation
{
	class EventEngine
	{
	public:
		EventEngine() noexcept;
		EventEngine(EventEngine &&other) noexcept;
		~EventEngine();

		EventEngine& operator = (EventEngine &&other) noexcept;

		bool RunDefault();
		bool RunOnce();
		bool RunNoWait();
		void Stop();

		uvpp::UvLoop& InternalGetLoop() const;
		void InternalDelayDelete(uvpp::UvHandle *pInternalHandle);

	private:
		struct Impl;
		std::unique_ptr<Impl> Impl_;
	};
}
