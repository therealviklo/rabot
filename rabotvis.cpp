#include <cstdio>
#include <chrono>
#include <thread>
#include "raboter.cpp"

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::puts("Användning: rabotvis n bpm");
		return 0;
	}

	const unsigned long n = std::strtoul(argv[1], nullptr, 0);
	const std::chrono::milliseconds qn(60000 / std::strtoul(argv[2], nullptr, 0));

	std::printf("\x1b[;H\x1b[2J");

	std::chrono::high_resolution_clock::time_point nextFrame = std::chrono::high_resolution_clock::now();
	for (unsigned long i = 0; i < n; i++)
	{
		const unsigned x = raboter(i);
		const unsigned x2 = x % 88;
		for (unsigned j = 0; j < x2; j++) std::printf("\u2588");
		if (x != x2)
		{
			std::printf(" %u (%u %% 88)\n", x2, x);
		}
		else
		{
			std::printf(" %u\n", x2);
		}
		std::fflush(stdout);
		std::this_thread::sleep_until(nextFrame += qn);
		// std::this_thread::sleep_for(qn);
	}
}