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
		UvTCP *pServer = new UvTCP;

		int status = pServer->Init(loop);
		printf("* UvTCP.Init: %d\n", status);

		sockaddr_in addr;
		UvMisc::ToAddrIPv4("0.0.0.0", 12334, &addr);
		status = pServer->Bind(reinterpret_cast<const sockaddr*>(&addr));
		printf("* UvTCP.Bind: %d\n", status);

		status = pServer->Listen([](UvStream *server, int status)
		{
			printf("* OnConnected: %d\n", status);
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
