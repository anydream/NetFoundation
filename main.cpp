#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/uvpp.h"
#include "common/SharedUniquePtr.h"

//////////////////////////////////////////////////////////////////////////
using namespace uvpp;

static void TestTimer(UvLoop &loop)
{
	UvTimer *pTimer = UvTimer::New();

	int status = pTimer->Init(loop);
	printf("* Timer.Init: %d\n", status);

	SharedUniquePtr<int> pCounter(new int(0));
	status = pTimer->Start([&loop, pTimer, pCounter]()
	{
		printf("* Timeout ID: %d\n", *pCounter);
		if (++*pCounter == 3)
		{
			printf("* Timer.Finished\n");
			//pTimer->Stop();
			loop.DelayDelete(pTimer);
		}
	}, 500, 1000);

	printf("* Timer.Start: %d\n", status);
}

//////////////////////////////////////////////////////////////////////////
int main()
{
	UvLoop loop;

	TestTimer(loop);

	loop.Run();

	return 0;
}
