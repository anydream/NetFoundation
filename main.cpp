#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/UvLoop.h"

using namespace uvpp;

static void IdleCallback(UvIdle &idle, int &count)
{
	++count;
	if (count >= 900000)
		idle.Stop();
}

int main()
{
	UvLoop loop;
	int status = loop.Init();
	printf("* UvLoop.Init: %d\n", status);

	UvLoop loop2 = std::move(loop);
	loop = std::move(loop2);

	UvIdle idle;
	status = idle.Init(loop);
	printf("* UvIdle.Init: %d\n", status);
	int idleCount = 0;
	status = idle.Start(std::bind(&IdleCallback, std::ref(idle), std::ref(idleCount)));
	printf("* UvIdle.Start: %d\n", status);

	status = loop.Run();
	printf("* UvLoop.Run: %d\n", status);

	printf("* IdleCount: %d\n", idleCount);

	return 0;
}
