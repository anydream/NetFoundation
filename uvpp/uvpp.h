#pragma once

#include <memory>
#include <functional>

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	class UvLoop
	{
		friend class UvIdle;
		friend class UvTimer;
	public:
		enum RunMode
		{
			Default,
			Once,
			NoWait
		};

	public:
		UvLoop();
		UvLoop(UvLoop &&other) noexcept;
		~UvLoop();

		UvLoop& operator = (UvLoop &&other) noexcept;

		int Init();
		int Close();
		int Run(RunMode mode = Default);
		void Stop();

	private:
		struct Impl;
		std::unique_ptr<Impl> Impl_;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvIdle
	{
	public:
		UvIdle();
		UvIdle(UvIdle &&other) noexcept;
		~UvIdle();

		UvIdle& operator = (UvIdle &&other) noexcept;

		int Init(UvLoop &loop);
		void Close();
		int Start(std::function<void()> &&cbIdle);
		int Stop();

	private:
		struct Impl;
		std::unique_ptr<Impl> Impl_;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvTimer
	{
	public:
		UvTimer();
		UvTimer(UvTimer &&other) noexcept;
		~UvTimer();

		UvTimer& operator = (UvTimer &&other) noexcept;

		int Init(UvLoop &loop);
		void Close();
		int Start(std::function<void()> &&cbTimer, uint64_t timeout, uint64_t repeat);
		int Stop();
		int Again();
		void SetRepeat(uint64_t repeat);
		uint64_t GetRepeat() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> Impl_;
	};
}
