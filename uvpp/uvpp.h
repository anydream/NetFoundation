#pragma once

#include <functional>
#include <uv.h>

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	class UvHandle
	{
	public:
		virtual ~UvHandle() {}

		virtual uv_handle_t* GetRawHandle() = 0;

		const uv_handle_t* GetRawHandle() const;
		bool IsActive() const;
		bool IsClosing() const;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvLoop
	{
	public:
		enum RunMode
		{
			Default = UV_RUN_DEFAULT,
			Once = UV_RUN_ONCE,
			NoWait = UV_RUN_NOWAIT
		};

	public:
		UvLoop();
		UvLoop(const UvLoop&) = delete;
		~UvLoop();

		UvLoop& operator = (const UvLoop&) = delete;

		int Run(RunMode mode = Default);
		void Stop();
		bool Alive() const;
		uint64_t Now() const;

		void DelayDelete(UvHandle *pHandle);

		uv_loop_t* GetRawHandle();
		bool IsRunning() const;

	private:
		uv_loop_t Loop_ = {};
		bool IsRunning_ = false;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvTimer : public UvHandle
	{
	public:
		int Init(UvLoop &loop);
		int Start(std::function<void()> &&cbTimer, uint64_t timeout, uint64_t repeat);
		int Stop();
		int Again();

		void SetRepeat(uint64_t repeat);
		uint64_t GetRepeat() const;

		uv_handle_t* GetRawHandle() override;

	public:
		static UvTimer* New();

	private:
		UvTimer() {}
		~UvTimer();

	private:
		uv_timer_t Timer_ = {};
		std::function<void()> Callback_;
	};
}
