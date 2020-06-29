#include <neolib/neolib.hpp>
#include <iostream>
#include <set>
#include <neolib/task/thread.hpp>
#include <neolib/task/thread_pool.hpp>

void benchmark_thread_pool()
{
	neolib::thread_pool threadPool;
	
	std::vector<int> v;
	const int ITERATIONS = 100000;
	v.resize(ITERATIONS);
	
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	
	for (int i = 0; i < ITERATIONS; ++i)
		threadPool.run([i, &v]()
	{ 
		v[i] = i;
	});
	threadPool.wait();
	
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	
	std::set<int> s;
	for (auto n : v)
		s.insert(n);
	std::cout << "\ncheck: " << s.size() << "\ntime: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "ms" << std::endl;
}

