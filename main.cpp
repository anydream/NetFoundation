#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/uvpp.h"
#include "common/SharedUniquePtr.h"

//////////////////////////////////////////////////////////////////////////
struct MyCls
{
	MyCls()
	{
		printf("MyCls\n");
	}

	~MyCls()
	{
		printf("~MyCls\n");
	}

	void Foo()
	{
		printf("data %d, %p\n", data, this);
	}
	int data = 1234;
};

static void TestSharedUnique()
{
	SharedUniquePtr<MyCls> myptr(new MyCls);
	SharedUniquePtr<MyCls> myptr2 = myptr;
	SharedUniquePtr<MyCls> myptr3;
	myptr3 = myptr2;
	myptr3->Foo();
	(*myptr3).Foo();
	myptr3 = nullptr;
}

//////////////////////////////////////////////////////////////////////////
using namespace uvpp;

static void IdleCallback(SharedUniquePtr<UvIdle> &pIdle, SharedUniquePtr<int> &pCount)
{
	if (++*pCount == 600000)
	{
		printf("* UvIdle.Stop: %d\n", *pCount);
		pIdle->Stop();
		pIdle->Close();
		pIdle = nullptr;
	}
}

static void TestIdle(UvLoop &loop)
{
	UvIdle idle;
	int status = idle.Init(loop);
	printf("* UvIdle.Init: %d\n", status);

	UvIdle idle2 = std::move(idle);
	idle = std::move(idle2);

	UvIdle *pIdle = new UvIdle(std::move(idle));
	status = pIdle->Start(std::bind(
		&IdleCallback,
		SharedUniquePtr<UvIdle>(pIdle),
		SharedUniquePtr<int>(new int(0))));
	printf("* UvIdle.Start: %d\n", status);
}

static void TimerCallback(SharedUniquePtr<UvTimer> &pTmr, SharedUniquePtr<int> &pCount)
{
	++*pCount;
	printf("* TimerCallback %d\n", *pCount);
	if (*pCount == 2)
	{
		pTmr->Stop();
		pTmr->Close();
		pTmr = nullptr;
	}
}

static void TestTimer(UvLoop &loop)
{
	UvTimer tmr;
	int status = tmr.Init(loop);
	printf("* UvTimer.Init: %d\n", status);

	UvTimer tmr2 = std::move(tmr);
	tmr = std::move(tmr2);

	UvTimer *pTmr = new UvTimer(std::move(tmr));
	status = pTmr->Start(std::bind(
		&TimerCallback,
		SharedUniquePtr<UvTimer>(pTmr),
		SharedUniquePtr<int>(new int(0))),
		500, 1000);
	printf("* UvTimer.Start: %d\n", status);
}

//////////////////////////////////////////////////////////////////////////
int main()
{
	TestSharedUnique();

	UvLoop loop;
	int status = loop.Init();
	printf("* UvLoop.Init: %d\n", status);

	UvLoop loop2 = std::move(loop);
	loop = std::move(loop2);

	TestIdle(loop);
	TestTimer(loop);

	status = loop.Run();
	printf("* UvLoop.Run: %d\n", status);

	return 0;
}
