#include <memory>
#include "ServerBase.h"

namespace uvpp
{
	//////////////////////////////////////////////////////////////////////////
	ServerBase::ServerBase(UvLoop &loop)
		: Loop_(loop)
		, ServerPtr_(new UvTCP)
	{
		ServerPtr_->Init(loop);
	}

	ServerBase::~ServerBase()
	{
		Stop();
		Loop_.DelayDelete(ServerPtr_);
	}

	int ServerBase::Bind(int port, const char *ip) const
	{
		sockaddr_in addr;
		UvMisc::ToAddrIPv4(ip, port, &addr);
		return ServerPtr_->Bind(reinterpret_cast<const sockaddr*>(&addr));
	}

	int ServerBase::Start(int backlog)
	{
		return ServerPtr_->Listen(std::bind(&ServerBase::NotifyNewSession, this, std::placeholders::_1), backlog);
	}

	void ServerBase::Stop() const
	{
		ServerPtr_->ReadStop();
		ServerPtr_->Shutdown([](int) {});
	}

	void ServerBase::NotifyNewSession(int status)
	{
		if (status != 0)
		{
			NotifyError(status);
			return;
		}

		std::unique_ptr<SessionBase> pSession(new SessionBase(*this));
		status = pSession->Accept();
		if (status == 0)
		{
			status = pSession->Start();
			if (status == 0)
			{
				AddSession(pSession.get(), std::move(pSession));
				return;
			}
		}
		NotifyError(status);
	}

	void ServerBase::NotifyError(int errCode)
	{
		OnError(errCode, UvMisc::ToError(errCode));
	}

	void ServerBase::AddSession(void *id, std::unique_ptr<SessionBase> &&pSession)
	{
		SessionMap_[id] = std::move(pSession);
	}

	void ServerBase::DelSession(void *id)
	{
		SessionMap_.erase(id);
	}

	//////////////////////////////////////////////////////////////////////////
	SessionBase::SessionBase(ServerBase &owner)
		: Owner_(owner)
		, SessionPtr_(new UvTCP)
	{
		SessionPtr_->Init(owner.Loop_);
	}

	SessionBase::~SessionBase()
	{
		Stop();
		Owner_.Loop_.DelayDelete(SessionPtr_);
		ReadBuf_.Free();
	}

	int SessionBase::Accept() const
	{
		return Owner_.ServerPtr_->Accept(SessionPtr_);
	}

	void SessionBase::Stop() const
	{
		SessionPtr_->ReadStop();
	}

	int SessionBase::Start()
	{
		return SessionPtr_->ReadStart(
			std::bind(&SessionBase::NotifyRead, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&SessionBase::NotifyAlloc, this, std::placeholders::_1, std::placeholders::_2));
	}

	void SessionBase::NotifyRead(ssize_t nread, UvBuf *buf)
	{
		if (nread < 0)
		{
			NotifyDisconnected(static_cast<int>(nread));
			Owner_.DelSession(this);
			return;
		}

		OnRead(buf->Data, nread);
	}

	void SessionBase::NotifyAlloc(size_t suggested_size, UvBuf *buf)
	{
		ReadBuf_.Alloc(suggested_size);
		*buf = ReadBuf_;
	}

	void SessionBase::NotifyDisconnected(int status)
	{
		OnDisconnected(status);
	}
}
