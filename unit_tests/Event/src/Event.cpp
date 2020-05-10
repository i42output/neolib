#include <neolib/event.hpp>
#include <neolib/async_thread.hpp>

template class neolib::event<int>;

class greetings
{
public:
	neolib::event<const std::string&> hello_and_goodbye;
public:
	greetings()
	{
		iSink += hello_and_goodbye([this](const std::string& greeting) { handle_hello_and_goodbye(greeting); });
	}
	~greetings()
	{
		std::cout << "Destroying greetings object..." << std::endl;
	}
public:
	void handle_hello_and_goodbye(const std::string& greeting)
	{
		static int phase;
		std::cout << "In handler, phase = " << phase << ", argument = '" << greeting << "'" << std::endl;
		switch (phase)
		{
		case 0:
			++phase;
			hello_and_goodbye.trigger(", world!");
			break;
		case 1:
			delete this;
			break;
		}
	}
private:
	neolib::sink iSink; // performs the traditional role of slot handler de-registration
};

class counter
{
public:
	neolib::event<int> new_integer;
public:
	void count(int n)
	{
		for (int i = 1; i <= n; ++i)
			new_integer.trigger(i);
	}
};

int main()
{
	neolib::async_task mainTask;
	neolib::async_thread mainThread{ mainTask, "neolib::event unit test(s)", true };

	{
		// we shall use 'new' instead of smart pointer maker as greetings object will delete itself in event handler...
		auto object = new greetings{};
		object->hello_and_goodbye.trigger("Hello");
	}

	{
		counter c;
		{
			neolib::sink localSink;
			localSink += c.new_integer([](int n) { std::cout << n << std::endl; });
			c.count(10);
		}
		c.count(10); // shouldn't print anything as 'localSink' has been destroyed deregistering the handler
	}
}