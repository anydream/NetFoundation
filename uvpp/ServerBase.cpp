#include <string>
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
			NotifyError(status, StateNewSession);
			return;
		}

		std::unique_ptr<SessionBase> pSession(NewSession(*this));
		status = pSession->Accept();
		if (status == 0)
		{
			status = pSession->Start();
			if (status == 0)
			{
				AddSession(std::move(pSession));
			}
			else
				NotifyError(status, StateStartRead);
		}
		else
			NotifyError(status, StateAccept);
	}

	void ServerBase::NotifyError(int errCode, ErrorState state)
	{
		OnError(errCode, state);
	}

	void ServerBase::AddSession(std::unique_ptr<SessionBase> &&pSession)
	{
		SessionBase *ptr = pSession.get();
		SessionMap_[ptr] = std::move(pSession);
		OnAddSession(ptr);
	}

	void ServerBase::DelSession(SessionBase *ptr)
	{
		OnDelSession(ptr);
		SessionMap_.erase(ptr);
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

	const std::string& SessionBase::GetPeerAddress() const
	{
		if (PeerAddr_.empty())
		{
			auto addrs = SessionPtr_->GetPeerName();
			int port;
			std::string strAddr = UvMisc::ToNameIPv4(reinterpret_cast<const sockaddr_in*>(&addrs), &port);
			strAddr.push_back('|');
			strAddr += std::to_string(port);
			PeerAddr_ = strAddr;
		}
		return PeerAddr_;
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
