﻿#pragma once

#include <functional>
#include <uv.h>

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	struct UvBuf
	{
		char *Data = nullptr;
		size_t Length = 0;

		UvBuf()
		{
		}

		UvBuf(char *data, size_t len)
			: Data(data)
			, Length(len)
		{
		}
	};
	static_assert(sizeof(UvBuf) == sizeof(uv_buf_t), "sizeof uv_buf_t");

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

		uv_loop_t* GetRawLoop();
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
		int Start(uint64_t timeout, uint64_t repeat, std::function<void()> &&cbTimer);
		int Stop();
		int Again();

		void SetRepeat(uint64_t repeat);
		uint64_t GetRepeat() const;

		uv_handle_t* GetRawHandle() override;

	private:
		~UvTimer();

	private:
		uv_timer_t Timer_ = {};
		std::function<void()> Callback_;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvStream : public UvHandle
	{
	public:
		using CbConnect = std::function<void(UvStream *server, int status)>;
		using CbAlloc = std::function<void(UvHandle *handle, size_t suggested_size, UvBuf *buf)>;
		using CbRead = std::function<void(UvStream *stream, ssize_t nread, const UvBuf *buf)>;
		using CbWrite = std::function<void(int status)>;
		using CbShutdown = std::function<void(int status)>;

	public:
		int Listen(CbConnect &&cbConnect, int backlog = 128);
		int Accept(UvStream *client);
		int ReadStart(CbRead &&cbRead, CbAlloc &&cbAlloc);
		int Write(const UvBuf bufs[], uint32_t nbufs, CbWrite &&cbWrite);
		int Shutdown(CbShutdown &&cbShutdown);

		virtual uv_stream_t* GetRawStream() = 0;

	private:
		CbConnect CallbackConnect_;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvTCP : public UvStream
	{
	public:
		int Init(UvLoop &loop);
		int Bind(const sockaddr *addr, uint32_t flags = 0);

		uv_stream_t* GetRawStream() override;
		uv_handle_t* GetRawHandle() override;

	private:
		uv_tcp_t TCP_ = {};
	};

	//////////////////////////////////////////////////////////////////////////
	class UvMisc
	{
	public:
		static int ToAddrIPv4(const char *ip, int port, sockaddr_in *addr);
	};
}
