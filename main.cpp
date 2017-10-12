#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/uvpp.h"
#include "common/SharedUniquePtr.h"

using namespace uvpp;

//////////////////////////////////////////////////////////////////////////
static void IdleCallback(SharedUniquePtr<UvIdle> &pIdle, SharedUniquePtr<int> &pCount)
{
	if (++*pCount >= 600000)
	{
		printf("* UvIdle.Stop: %d\n", *pCount);
		pIdle->Stop();
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
int main()
{
	TestSharedUnique();

	UvLoop loop;
	int status = loop.Init();
	printf("* UvLoop.Init: %d\n", status);

	UvLoop loop2 = std::move(loop);
	loop = std::move(loop2);

	TestIdle(loop);

	status = loop.Run();
	printf("* UvLoop.Run: %d\n", status);

	return 0;
}
