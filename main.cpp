#if defined(_DEBUG) && defined(_WIN32) && !defined(_WIN64)
#include <vld.h>
#endif

#include <stdio.h>
#include "uvpp/UvLoop.h"

int main()
{
	using namespace uvpp;

	UvLoop loop;
	int status = loop.Init();
	printf("* Init: %d\n", status);

	UvLoop loop2 = std::move(loop);
	loop = std::move(loop2);

	status = loop.Run();
	printf("* Run: %d\n", status);

	return 0;
}
