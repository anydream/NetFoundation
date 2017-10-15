#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <malloc.h>
#include <string>
#include "DateTime.h"
#include "StringConvert.h"
#include "uvpp.h"
#include "common/SharedUniquePtr.h"

#pragma execution_character_set("utf-8")

class Logger
{
public:
	~Logger()
	{
		fclose(FilePtr_);
	}

	void Init(const char *fname)
	{
		assert(!FilePtr_);
		FilePtr_ = fopen(fname, "ab");
	}

	void LogBuffer(size_t len, const char *data) const
	{
		fwrite(data, 1, len, FilePtr_);
	}

	void Log(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);

		int bufSize = _vscprintf(fmt, args) + 1;
		char *str = static_cast<char*>(alloca(bufSize));
		bufSize = vsnprintf(str, bufSize, fmt, args);

		va_end(args);

		std::string strTimestamp = "[" + RCSoft::Utils::DateTime::Now().ToString("%Y-%m-%d %H:%M:%S") + "] ";
		LogBuffer(strTimestamp.size(), strTimestamp.data());
		LogBuffer(bufSize, str);

		IsDirty_ = true;
	}

	template <class ...TArgs>
	void operator () (const char *fmt, TArgs &&...args)
	{
		Log(fmt, std::forward<TArgs>(args)...);
	}

	void TryFlush()
	{
		if (IsDirty_)
		{
			IsDirty_ = false;
			fflush(FilePtr_);
		}
	}

private:
	FILE *FilePtr_ = nullptr;
	bool IsDirty_ = false;
};

static Logger Log;
using namespace uvpp;

#define KEY_STATE(_vk) (GetAsyncKeyState(_vk) & 0x8000)

static std::string ToAddressName(sockaddr_storage *addr)
{
	assert(addr->ss_family == AF_INET);
	int port;
	std::string result = UvMisc::ToNameIPv4(reinterpret_cast<sockaddr_in*>(addr), &port);
	result.push_back(':');
	return result + std::to_string(port);
}

static void ServerLoop(int port)
{
	UvLoop loop;

	// 创建服务端
	UvTCP *pServer = new UvTCP;
	int status = pServer->Init(loop);
	Log("* 初始化: %d\n", status);

	// 绑定端口
	sockaddr_in addr;
	UvMisc::ToAddrIPv4("0.0.0.0", port, &addr);
	status = pServer->Bind(reinterpret_cast<const sockaddr*>(&addr));
	Log("* 绑定: %d\n", status);

	// 开始监听
	status = pServer->Listen([&loop, pServer,
		numClients = SharedUniquePtr<int>(new int(0))](int status)
	{
		if (status != 0)
		{
			Log("* 侦听失败: %d, %s\n", status, UvMisc::ToError(status));
			return;
		}

		// 创建会话
		UvTCP *pClient = new UvTCP;
		pClient->Init(loop);
		status = pServer->Accept(pClient);
		if (status == 0)
		{
			++*numClients;

			auto addrs = pClient->GetPeerName();
			std::string peerName = ToAddressName(&addrs);
			Log("* 接受连接: [%s], Total: %d\n",
				peerName.c_str(),
				*numClients);

			// 开始接收数据
			UvBuf *pRecvBuf = new UvBuf;
			pClient->ReadStart([&loop, &numClients, pClient, peerName,
				recvBuf = SharedUniquePtr<UvBuf, std::function<void(UvBuf*)>>(pRecvBuf, UvBuf::Deleter)]
				(ssize_t nread, UvBuf *buf)
			{
				if (nread < 0)
				{
					--*numClients;
					Log("* 断开连接: [%s], Total: %d. %Id, %s\n", peerName.c_str(), *numClients, nread, UvMisc::ToError(static_cast<int>(nread)));
					// 停止写入
					pClient->Shutdown([](int) {});
					loop.DelayDelete(pClient);
					return;
				}
				// echo
				UvBuf bufRange(nread, buf->Data);
				pClient->Write(&bufRange, 1, [&loop, &numClients, pClient, peerName](int status)
				{
					// eof
					--*numClients;
					if (status < 0)
						Log("* 写入失败: [%s], Total: %d. %d, %s\n", peerName.c_str(), *numClients, status, UvMisc::ToError(status));
					else
						Log("* 写入成功: [%s], Total: %d\n", peerName.c_str(), *numClients);
					pClient->ReadStop();
					loop.DelayDelete(pClient);
				});
				Log("* 读取: [%s], %Id\n[", peerName.c_str(), bufRange.Length);
				Log.LogBuffer(bufRange.Length, bufRange.Data);
				const char strLogEnd[] = "]\n";
				Log.LogBuffer(sizeof(strLogEnd) - 1, strLogEnd);
			}, [pRecvBuf](size_t suggested_size, UvBuf *buf)
			{
				pRecvBuf->Alloc(suggested_size);
				*buf = *pRecvBuf;
			});
		}
		else
		{
			auto addrs = pClient->GetPeerName();
			Log("* 接受连接失败: [%s], %d, %s\n", ToAddressName(&addrs).c_str(), status, UvMisc::ToError(status));
			loop.DelayDelete(pClient);
		}
	});
	Log("* 开始侦听: %d\n", status);

	//////////////////////////////////////////////////////////////////////////
	UvTimer *pFlushTmr = new UvTimer;
	pFlushTmr->Init(loop);
	pFlushTmr->Start(0, 5000, []()
	{
		Log.TryFlush();
	});

	UvTimer *pStopTmr = new UvTimer;
	pStopTmr->Init(loop);
	pStopTmr->Start(0, 100, [&loop]()
	{
		if (KEY_STATE(VK_F10))
		{
			const size_t bufSize = 16;
			char buf[bufSize];
			printf("* Exit? ");
			fgets(buf, bufSize, stdin);
			if (buf[0] != 'y')
				return;

			Log("* 退出\n");
			loop.Walk([&loop](UvHandle *handle)
			{
				Log("* 释放: %s\n", handle->GetTypeName());
				loop.DelayDelete(handle);
			});
		}
	});
	loop.Run();
}

//////////////////////////////////////////////////////////////////////////
int TestServerMain(int argc, const char **argv)
{
	const char *strPort = "80";
	const char *strLogFile = "netlog.txt";

	if (argc >= 2)
		strPort = argv[1];
	if (argc >= 3)
		strLogFile = argv[2];

	Log.Init(strLogFile);

	ServerLoop(RCSoft::Utils::ParseInt(strPort));

	return 0;
}
