#pragma once

#include <memory>

namespace uvpp
{
	class UvLoop
	{
	public:
		enum RunMode
		{
			Default,
			Once,
			NoWait
		};

	public:
		UvLoop();
		UvLoop(UvLoop &&other);
		~UvLoop();

		UvLoop& operator = (UvLoop &&other);

		int Init();
		int Close();
		int Run(RunMode mode = Default);
		void Stop();

	private:
		struct Impl;
		std::unique_ptr<Impl> Impl_;
	};
}
