#pragma once

#include <functional>
#include <uv.h>

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	struct UvBuf
	{
		size_t Length = 0;
		char *Data = nullptr;

		UvBuf();
		UvBuf(size_t len, char *data);

		void Alloc(size_t len);
		void Free();

		static void Deleter(UvBuf *pBuf);
	};
	static_assert(sizeof(UvBuf) == sizeof(uv_buf_t), "sizeof uv_buf_t");

	//////////////////////////////////////////////////////////////////////////
	class UvHandle
	{
		friend class UvLoop;
	public:
		virtual uv_handle_t* GetRawHandle() = 0;
		virtual const char* GetTypeName() = 0;

		const uv_handle_t* GetRawHandle() const;
		bool IsActive() const;
		bool IsClosing() const;

	protected:
		virtual ~UvHandle() {}
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

		using CbWalk = std::function<void(UvHandle *handle)>;

	public:
		UvLoop();
		UvLoop(const UvLoop&) = delete;
		~UvLoop();

		UvLoop& operator = (const UvLoop&) = delete;

		int Run(RunMode mode = Default);
		void Stop();
		bool Alive() const;
		uint64_t Now() const;

		void Walk(CbWalk &&cbWalk);

		void DelayDelete(UvHandle *pHandle);

		uv_loop_t* GetRawLoop();
		bool IsRunning() const;

	private:
		uv_loop_t Loop_ = {};
		CbWalk CallbackWalk_;
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
		const char* GetTypeName() override;

	private:
		~UvTimer();

	private:
		uv_timer_t Timer_ = {};
		std::function<void()> CallbackTimer_;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvStream : public UvHandle
	{
	public:
		using CbConnect = std::function<void(UvStream *server, int status)>;
		using CbAlloc = std::function<void(size_t suggested_size, UvBuf *buf)>;
		using CbRead = std::function<void(ssize_t nread, UvBuf *buf)>;
		using CbWrite = std::function<void(int status)>;
		using CbShutdown = std::function<void(int status)>;

	protected:
		~UvStream() {}

	public:
		int Listen(CbConnect &&cbConnect, int backlog = 128);
		int Accept(UvStream *client);
		int ReadStart(CbRead &&cbRead, CbAlloc &&cbAlloc);
		int ReadStop();
		int Write(const UvBuf *bufs, uint32_t nbufs, CbWrite &&cbWrite);
		int Shutdown(CbShutdown &&cbShutdown);

		virtual uv_stream_t* GetRawStream() = 0;

	private:
		CbConnect CallbackConnect_;
		CbRead CallbackRead_;
		CbAlloc CallbackAlloc_;
		CbWrite CallbackWrite_;
		CbShutdown CallbackShutdown_;
	};

	//////////////////////////////////////////////////////////////////////////
	class UvTCP : public UvStream
	{
	public:
		int Init(UvLoop &loop);
		int Bind(const sockaddr *addr, uint32_t flags = 0);
		int GetSockName(sockaddr *name, int *namelen) const;
		int GetPeerName(sockaddr *name, int *namelen) const;
		sockaddr_storage GetPeerName() const;

		uv_stream_t* GetRawStream() override;
		uv_handle_t* GetRawHandle() override;
		const char* GetTypeName() override;

	private:
		~UvTCP() {}

	private:
		uv_tcp_t TCP_ = {};
	};

	//////////////////////////////////////////////////////////////////////////
	class UvMisc
	{
	public:
		static const char* ToError(int errCode);
		static int ToAddrIPv4(const char *ip, int port, sockaddr_in *addr);
		static int ToAddrIPv6(const char *ip, int port, sockaddr_in6 *addr);
		static std::string ToNameIPv4(const sockaddr_in *addr, int *port = nullptr);
		static std::string ToNameIPv6(const sockaddr_in6 *addr, int *port = nullptr);
	};
}
