﻿#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
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
		UvTimer *pTimer = UvTimer::New();

		int status = pTimer->Init(loop);
		printf("* Timer.Init: %d\n", status);

		status = pTimer->Start(
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
		}, 500, 1000);

		printf("* Timer.Start: %d\n", status);
	}

	static void TestEntry()
	{
		printf("===== TestUvpp =====\n");

		UvLoop loop;

		TestTimer(loop);

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
			[pTmr = SharedUniquePtr<Timer>(pTimer),
			pCounter = SharedUniquePtr<int>(new int(0))]() mutable
		{
			printf("* Timeout ID: %d\n", *pCounter);
			if (++*pCounter == 5)
			{
				printf("* Timer.Finished\n");
				pTmr = nullptr;
			}
		}, 500, 1000);
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
	TestNF::TestEntry();
	return 0;
}
