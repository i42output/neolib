#include <neolib/task/event.hpp>
#include <neolib/task/async_thread.hpp>
#include <neolib/task/timer.hpp>

namespace test
{
	struct thread : neolib::async_task, neolib::async_thread
	{
		thread() : async_task{ "test::task" }, async_thread{ *this, "test::thread" }
		{
			start();
		}
		void exec_preamble() override
		{
			neolib::async_thread::exec_preamble();
			timer.emplace(*this, [&](neolib::callback_timer&) { end = std::chrono::steady_clock::now(); }, std::chrono::milliseconds{ 100 });
		}
		std::atomic<std::optional<std::chrono::steady_clock::time_point>> end;
		std::optional<neolib::callback_timer> timer;
	};
}

int main()
{
	std::optional<std::pair<double, double>> stats;
	for (int32_t i = 1; i <= 200; ++i)
	{
		test::thread thread;
		while (thread.state() != neolib::thread_state::Started)
			std::this_thread::yield();
		std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
		while (thread.end.load() == std::nullopt)
			std::this_thread::yield();
		auto time = std::chrono::duration<double>(*thread.end.load() - start).count();
		if (stats == std::nullopt)
			stats.emplace(time, time);
		else
		{
			stats->first = std::min(stats->first, time);
			stats->second = std::max(stats->second, time);
		}
		if (i % 20 == 0)
			std::cout << "Iteration #" << i << " time: " << time << " s" << ", min: " << stats->first << " s, max: " << stats->second << " s" << std::endl;
		if (time > 0.11)
		{
			std::cout << "Iteration #" << i << " FAILED, time: " << time << " s" << std::endl;
			throw std::logic_error("failed");
		}
	}
}