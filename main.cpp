#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/uvpp.h"
#include "NFTimer.h"
#include "common/SharedUniquePtr.h"
#include <string>
#include <assert.h>

#define KEY_STATE(_vk) (GetAsyncKeyState(_vk) & 0x8000)

int printf_flush(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	int result = vfprintf(stdout, fmt, args);
	va_end(args);
	fflush(stdout);
	return result;
}

//////////////////////////////////////////////////////////////////////////
namespace TestUvpp
{
	using namespace uvpp;

	static void TestTimer(UvLoop &loop)
	{
		UvTimer *pTimer = new UvTimer();

		int status = pTimer->Init(loop);
		printf_flush("* Timer.Init: %d\n", status);

		status = pTimer->Start(
			500, 1000,
			[&loop,
			pTimer,
			pCounter = SharedUniquePtr<int>(new int(0))]()
		{
			printf_flush("* Timeout ID: %d\n", *pCounter);
			if (++*pCounter == 5)
			{
				printf_flush("* Timer.Finished\n");
				loop.DelayDelete(pTimer);
			}
		});

		printf_flush("* Timer.Start: %d\n", status);
	}

	static std::string ToAddressName(sockaddr_storage *addr)
	{
		assert(addr->ss_family == AF_INET);
		int port;
		std::string result = UvMisc::ToNameIPv4(reinterpret_cast<sockaddr_in*>(addr), &port);
		result.push_back(':');
		return result + std::to_string(port);
	}

	static void TestTCPServer(UvLoop &loop)
	{
		// 创建服务端
		UvTCP *pServer = new UvTCP;
		int status = pServer->Init(loop);
		printf_flush("* 初始化: %d\n", status);

		// 绑定端口
		sockaddr_in addr;
		UvMisc::ToAddrIPv4("0.0.0.0", 80, &addr);
		status = pServer->Bind(reinterpret_cast<const sockaddr*>(&addr));
		printf_flush("* 绑定: %d\n", status);

		// 开始监听
		status = pServer->Listen([&loop,
			numClients = SharedUniquePtr<int>(new int(0))](UvStream *server, int status)
		{
			if (status != 0)
			{
				printf_flush("* 侦听失败: %d, %s\n", status, UvMisc::ToError(status));
				return;
			}

			// 创建会话
			UvTCP *pClient = new UvTCP;
			pClient->Init(loop);
			status = server->Accept(pClient);
			if (status == 0)
			{
				++*numClients;

				auto addrs = pClient->GetPeerName();
				std::string peerName = ToAddressName(&addrs);
				printf_flush("* 接受连接: [%s], Total: %d\n",
					peerName.c_str(),
					*numClients);

				// 开始接收数据
				UvBuf *pRecvBuf = new UvBuf;
				pClient->ReadStart([&loop, &numClients, pClient, peerName,
					recvBuf = SharedUniquePtr<UvBuf>(pRecvBuf)](UvStream *stream, ssize_t nread, UvBuf *buf)
				{
					if (nread < 0)
					{
						--*numClients;
						printf_flush("* 断开连接: [%s], Total: %d. %Id, %s\n", peerName.c_str(), *numClients, nread, UvMisc::ToError(nread));
						// 停止写入
						pClient->Shutdown([](int) {});
						loop.DelayDelete(stream);
						return;
					}
					// echo
					pClient->Write(buf, 1, [](int status)
					{
						if (status < 0)
							printf_flush("* 写入失败: %d, %s\n", status, UvMisc::ToError(status));
					});
					printf_flush("* 读取: [%s], %Id [%.*s]\n", peerName.c_str(), nread, static_cast<int>(nread), buf->Data);
				}, [pRecvBuf](UvHandle *handle, size_t suggested_size, UvBuf *buf)
				{
					pRecvBuf->Alloc(suggested_size);
					*buf = *pRecvBuf;
				});
			}
			else
			{
				auto addrs = pClient->GetPeerName();
				printf_flush("* 接受连接失败: [%s], %d, %s\n", ToAddressName(&addrs).c_str(), status, UvMisc::ToError(status));
				loop.DelayDelete(pClient);
			}
		});
		printf_flush("* 开始侦听: %d\n", status);
	}

	static void TestEntry()
	{
		printf_flush("===== TestUvpp =====\n");

		UvLoop loop;

		TestTCPServer(loop);
		//TestTimer(loop);

		UvTimer *pStopTmr = new UvTimer;
		pStopTmr->Init(loop);
		pStopTmr->Start(0, 100, [&loop]()
		{
			if (KEY_STATE(VK_F10))
			{
				printf_flush("* 退出\n");

				loop.Walk([&loop](UvHandle *handle)
				{
					printf_flush("* Free: %s\n", handle->GetTypeName());
					loop.DelayDelete(handle);
				});
			}
		});
		loop.Run();
	}
}

namespace TestNF
{
	using namespace NetFoundation;

	static void TestTimer(EventEngine &ee)
	{
		Timer *pTimer = new Timer(ee);
		pTimer->Start(
			500, 1000,
			[pTmr = SharedUniquePtr<Timer>(pTimer),
			pCounter = SharedUniquePtr<int>(new int(0))]() mutable
		{
			printf_flush("* Timeout ID: %d\n", *pCounter);
			if (++*pCounter == 5)
			{
				printf_flush("* Timer.Finished\n");
				pTmr = nullptr;
			}
		});
	}

	static void TestEntry()
	{
		printf_flush("===== TestNF =====\n");

		EventEngine ee;

		TestTimer(ee);

		ee.RunDefault();
	}
}

//////////////////////////////////////////////////////////////////////////
int main()
{
	TestUvpp::TestEntry();
	//TestNF::TestEntry();
	return 0;
}
