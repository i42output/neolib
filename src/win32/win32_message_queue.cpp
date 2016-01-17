#include "neolib.hpp"
#include "win32_message_queue.hpp"

namespace neolib
{
	namespace
	{
		struct scoped_flag
		{
			bool& iFlag;
			scoped_flag(bool& aFlag) : iFlag(aFlag) { iFlag = true; }
			~scoped_flag() { iFlag = false; }
		};
	}

	std::map<UINT_PTR, win32_message_queue*> win32_message_queue::sTimerMap;

	win32_message_queue::win32_message_queue(io_thread& aIoThread, std::function<bool()> aIdleFunction, bool aCreateTimer) :
		iIoThread(aIoThread),
		iIdleFunction(aIdleFunction),
		iInGetMessage(false),
		iInTimerProc(false)
	{
		if (aCreateTimer)
		{
			iTimer = ::SetTimer(NULL, 0, 10, &win32_message_queue::timer_proc);
			sTimerMap[iTimer] = this;
		}
	}

	win32_message_queue::~win32_message_queue()
	{
		for (auto& t : sTimerMap)
			::KillTimer(NULL, t.first);
	}

	bool win32_message_queue::have_message() const
	{
		return ::PeekMessage(NULL, NULL, 0, 0, PM_NOREMOVE) != 0;
	}

	int win32_message_queue::get_message() const
	{
		scoped_flag sf(iInGetMessage);
		MSG msg;
		int result;
		if ((result = ::GetMessage(&msg, NULL, 0, 0)))
		{
			if (result != -1)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		return result;
	}

	void win32_message_queue::bump()
	{
		::PostMessage(NULL, WM_NULL, 0, 0);
	}

	void win32_message_queue::idle()
	{
		if (iIdleFunction)
			iIdleFunction();
	}

	void CALLBACK win32_message_queue::timer_proc(HWND, UINT, UINT_PTR aId, DWORD)
	{
		win32_message_queue& instance = *sTimerMap[aId];
		if (!instance.iInGetMessage || instance.iInTimerProc)
			return;
		scoped_flag sf(instance.iInTimerProc);
		instance.idle();
	}
}