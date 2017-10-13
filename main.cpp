#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/uvpp.h"
#include "NFTimer.h"
#include "common/SharedUniquePtr.h"

//////////////////////////////////////////////////////////////////////////
namespace TestUvpp
{
	using namespace uvpp;

	static void TestTimer(UvLoop &loop)
	{
		UvTimer *pTimer = new UvTimer();

		int status = pTimer->Init(loop);
		printf("* Timer.Init: %d\n", status);

		status = pTimer->Start(
			500, 1000,
			[&loop,
			pTimer,
			pCounter = SharedUniquePtr<int>(new int(0))]()
		{
			printf("* Timeout ID: %d\n", *pCounter);
			if (++*pCounter == 5)
			{
				printf("* Timer.Finished\n");
				loop.DelayDelete(pTimer);
			}
		});

		printf("* Timer.Start: %d\n", status);
	}

	static void TestTCPServer(UvLoop &loop)
	{
		// 创建服务端
		UvTCP *pServer = new UvTCP;
		int status = pServer->Init(loop);
		printf("* UvTCP.Init: %d\n", status);

		// 绑定端口
		sockaddr_in addr;
		UvMisc::ToAddrIPv4("0.0.0.0", 12334, &addr);
		status = pServer->Bind(reinterpret_cast<const sockaddr*>(&addr));
		printf("* UvTCP.Bind: %d\n", status);

		// 开始监听
		status = pServer->Listen([&loop, pServer,
			numClients = SharedUniquePtr<int>(new int(0))](UvStream *server, int status)
		{
			if (status < 0)
			{
				printf("* IncomingConnect Error: %d\n", status);
				return;
			}
			printf("* IncomingConnect: %d\n", status);
			// 创建会话
			UvTCP *pClient = new UvTCP;
			pClient->Init(loop);
			status = server->Accept(pClient);
			if (status == 0)
			{
				++*numClients;
				printf("* Accepted: Total: %d\n", *numClients);
				// 开始接收数据
				UvBuf *pRecvBuf = new UvBuf;
				pClient->ReadStart([&loop, &numClients, pServer,
					pRecvBuf](UvStream *stream, ssize_t nread, UvBuf *buf)
				{
					if (nread < 0)
					{
						--*numClients;
						printf("* Disconnected: %Id, Total: %d\n", nread, *numClients);
						delete pRecvBuf;
						loop.DelayDelete(stream);
						if (*numClients == 0)
							loop.DelayDelete(pServer);
						return;
					}
					printf("* Read: %Id, [%.*s]\n", nread, nread, buf->Data);
				}, [pRecvBuf](UvHandle *handle, size_t suggested_size, UvBuf *buf)
				{
					pRecvBuf->Alloc(suggested_size);
					*buf = *pRecvBuf;
				});
			}
			else
			{
				printf("* Accept Error: %d\n", status);
				loop.DelayDelete(pClient);
			}
		});
		printf("* UvTCP.Listen: %d\n", status);
	}

	static void TestEntry()
	{
		printf("===== TestUvpp =====\n");

		UvLoop loop;

		TestTCPServer(loop);
		//TestTimer(loop);

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
			printf("* Timeout ID: %d\n", *pCounter);
			if (++*pCounter == 5)
			{
				printf("* Timer.Finished\n");
				pTmr = nullptr;
			}
		});
	}

	static void TestEntry()
	{
		printf("===== TestNF =====\n");

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
