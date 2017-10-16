#pragma once

#include <memory>
#include <unordered_map>
#include "uvpp.h"

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	class ServerBase
	{
		friend class SessionBase;
	public:
		enum ErrorState
		{
			StateNewSession,
			StateStartRead,
			StateAccept
		};

	public:
		explicit ServerBase(UvLoop &loop);
		ServerBase(const ServerBase&) = delete;
		virtual ~ServerBase();

		int Bind(int port, const char *ip = "0.0.0.0") const;

		int Start(int backlog = 128);
		void Stop() const;

	private:
		void NotifyNewSession(int status);
		void NotifyError(int errCode, ErrorState state);

		void AddSession(std::unique_ptr<SessionBase> &&pSession);
		void DelSession(SessionBase *ptr);

	protected:
		virtual SessionBase* NewSession(ServerBase &owner) = 0;
		virtual void OnAddSession(SessionBase *pSession) {}
		virtual void OnDelSession(SessionBase *pSession) {}
		virtual void OnError(int errCode, ErrorState state) {}

	protected:
		UvLoop &Loop_;
		UvTCP *ServerPtr_ = nullptr;
		std::unordered_map<void*, std::unique_ptr<SessionBase>> SessionMap_;
	};

	//////////////////////////////////////////////////////////////////////////
	class SessionBase
	{
		friend class ServerBase;
	public:
		explicit SessionBase(ServerBase &owner);
		SessionBase(const SessionBase&) = delete;
		virtual ~SessionBase();

		int Write(const char *data, size_t len);
		void Disconnect(int status = 0);

		const std::string& GetPeerAddress() const;

	private:
		int Accept() const;
		int Start();
		void Stop() const;

		void NotifyRead(ssize_t nread, UvBuf *buf);
		void NotifyAlloc(size_t suggested_size, UvBuf *buf);
		void NotifyWrite(int status);
		void NotifyDisconnected(int status);

	protected:
		virtual void OnRead(const char *data, size_t len) {}
		virtual void OnWrite(int status) {}
		virtual void OnDisconnected(int status) {}

	protected:
		ServerBase &Owner_;
		UvTCP *SessionPtr_ = nullptr;
		UvBuf ReadBuf_;

	private:
		mutable std::string PeerAddr_;
	};
}
