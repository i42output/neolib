// async_task.cpp
/*
 *  Copyright (c) 2007, 2020 Leigh Johnston.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 *     * Neither the name of Leigh Johnston nor the names of any
 *       other contributors to this software may be used to endorse or
 *       promote products derived from this software without specific prior
 *       written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <neolib/neolib.hpp>
#include <boost/asio.hpp>
#include <neolib/core/scoped.hpp>
#include <neolib/app/i_module_services.hpp>
#include <neolib/task/thread.hpp>
#include <neolib/task/async_task.hpp>
#include <neolib/task/timer_object.hpp>
#include <neolib/task/event.hpp>

#ifdef _WIN32
#include "../win32/task/win32_message_queue.hpp"
#endif

namespace neolib
{
    class NEOLIB_EXPORT io_service : public reference_counted<i_async_service>
    {
        // types
    public:
        typedef boost::asio::io_service native_io_service_type;
        // construction
    public:
        io_service(i_async_task& aTask, bool aMultiThreaded = false);
        ~io_service();
        // operations
    public:
        bool poll(bool aProcessEvents = true, std::size_t aMaximumPollCount = kDefaultPollCount) override;
        void* native_object() override;
        // attributes
    private:
        i_async_task& iTask;
        native_io_service_type iNativeIoService;
    };

    void io_service_factory(i_async_task& aTask, bool aMultiThreaded, i_ref_ptr<i_async_service>& aResult)
    {
        aResult = neolib::make_ref<io_service>(aTask, aMultiThreaded);
    }

    io_service::io_service(i_async_task& aTask, bool aMultiThreaded) :
        iTask{ aTask },
        iNativeIoService{ aMultiThreaded ? BOOST_ASIO_CONCURRENCY_HINT_DEFAULT : BOOST_ASIO_CONCURRENCY_HINT_1 }
    {
    }

    io_service::~io_service()
    {
        iNativeIoService.stop();
    }

    bool io_service::poll(bool aProcessEvents, std::size_t aMaximumPollCount)
    {
        std::size_t iterationsLeft = aMaximumPollCount;
        bool didSome = false;
        iNativeIoService.restart();
        do
        {
            if (iTask.halted())
                return didSome;
            bool didSomeThisIteration = false;
            if (aProcessEvents)
                didSomeThisIteration = (iTask.pump_messages() || didSomeThisIteration);
            didSomeThisIteration = ((aMaximumPollCount == 0 ? iNativeIoService.poll() : iNativeIoService.poll_one()) != 0 || didSomeThisIteration);
            if (!didSomeThisIteration)
                break;
            didSome = true;
        } while (aMaximumPollCount != 0 && --iterationsLeft > 0);
        return didSome;
    }

    void* io_service::native_object()
    {
        return &iNativeIoService;
    }

    timer_service::timer_service(async_task& aTask, bool aMultiThreaded) :
        iTask{ aTask },
        iTaskDestroying{ aTask }
    {
    }

    bool timer_service::poll(bool aProcessEvents, std::size_t aMaximumPollCount)
    {
        std::size_t iterationsLeft = aMaximumPollCount;
        bool didSome = false;
        do
        {
            if (iTask.halted())
                return didSome;
            bool didSomeThisIteration = false;
            if (aProcessEvents)
                didSomeThisIteration = (iTask.pump_messages() || didSomeThisIteration);

            typedef std::vector<std::pair<decltype(iObjects)::value_type, destroyed_flag>> work_list_t;
            thread_local std::vector<std::unique_ptr<work_list_t>> workListStack;
            thread_local std::size_t stack;
            scoped_counter<std::size_t> stackCounter{ stack };
            if (workListStack.size() < stack)
                workListStack.push_back(std::make_unique<work_list_t>());
            work_list_t& workList = *workListStack[stack - 1];

            std::unique_lock lock{ iMutex };
            iObjects.erase(std::remove_if(iObjects.begin(), iObjects.end(), [](auto const& o) { return o == nullptr; }), iObjects.end());
            std::transform(iObjects.begin(), iObjects.end(), std::back_inserter(workList), [](auto const& s) { return std::make_pair(s, destroyed_flag{ *s }); });
            lock.unlock();
            for (auto const& o : workList)
            {
                if (!o.second.is_alive())
                    continue;
                auto& object = *o.first;
                if (object.poll())
                {
                    didSomeThisIteration = true;
                    if (aMaximumPollCount != 0 && --iterationsLeft == 0)
                        break;
                }
            }
            lock.lock();
            workList.clear();
            if (!didSomeThisIteration)
                break;
            didSome = true;
        } while (aMaximumPollCount != 0 && iterationsLeft > 0);
        return didSome;
    }

    void* timer_service::native_object()
    {
        return nullptr;
    }

    i_timer_object& timer_service::create_timer_object()
    {
        if (iTaskDestroying)
            throw task_destroying();
        std::unique_lock lock{ iMutex };
        iObjects.push_back(make_ref<timer_object>(*this));
        return *iObjects.back();
    }

    void timer_service::remove_timer_object(i_timer_object& aObject)
    {
        std::unique_lock lock{ iMutex };
        auto existing = std::find_if(iObjects.begin(), iObjects.end(), [&aObject](auto&& o) { return o == &aObject; });
        if (existing != iObjects.end())
        {
            auto existingRef = std::move(*existing);
            iObjects.erase(existing);
        }
    }

    async_task::async_task(const std::string& aName) :
        task{ aName }, iThread{ nullptr }, iState{ async_task_state::Init }
    {
    }

    async_task::async_task(i_thread& aThread, const std::string& aName) :
        task{ aName }, iThread{ &aThread }, iState{ async_task_state::Init }
    {
    }

    async_task::~async_task()
    {
        cancel();
        set_destroying();
        if (joined())
            thread().abort();
    }

    i_thread& async_task::thread() const
    {
        if (iThread != nullptr)
            return *iThread;
        throw no_thread();
    }

    bool async_task::joined() const
    {
        return iThread != nullptr;
    }

    void async_task::join(i_thread& aThread)
    {
        iThread = &aThread;
    }

    void async_task::detach()
    {
        iThread = nullptr;
    }

    timer_service& async_task::timer_service()
    {
        if (!iTimerService)
            iTimerService.emplace(*this);
        return *iTimerService;
    }

    i_async_service& async_task::io_service(i_module_services& aModuleServices)
    {
        auto& moduleIoService = iIoServices[&aModuleServices];
        if (moduleIoService == nullptr)
            moduleIoService.reset(aModuleServices.io_service_factory(*this).release());
        return *moduleIoService;
    }

    void async_task::cancel_io_service(i_module_services& aModuleServices)
    {
        iIoServices[&aModuleServices].reset();
    }

    bool async_task::do_work(yield_type aYieldIfNoWork)
    {
        if (halted())
            return false;
        bool didSome = pump_events();
        didSome = (pump_messages() || didSome);
        if (iTimerService)
            didSome = (iTimerService->poll() || didSome);
        for (auto& service : iIoServices)
            didSome = (service.second->poll() || didSome);
        if (!didSome && aYieldIfNoWork != yield_type::NoYield)
        {
            if (aYieldIfNoWork == yield_type::Yield)
                this_thread::yield();
            else if (aYieldIfNoWork == yield_type::Sleep)
                this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        }
        return didSome;
    }

    bool async_task::have_message_queue() const
    {
        return iMessageQueue != nullptr;
    }

    bool async_task::have_messages() const
    {
        return have_message_queue() && message_queue().have_message();
    }

    i_message_queue& async_task::create_message_queue(std::function<bool()> aIdleFunction)
    {
        #ifdef _WIN32
        iMessageQueue = std::make_unique<win32_message_queue>(*this, aIdleFunction);
        #endif
        return message_queue();
    }

    const i_message_queue& async_task::message_queue() const
    {
        if (iMessageQueue == nullptr)
            throw no_message_queue();
        return *iMessageQueue;
    }

    i_message_queue& async_task::message_queue()
    {
        if (iMessageQueue == nullptr)
            throw no_message_queue();
        return *iMessageQueue;
    }

    void async_task::register_event_queue(i_async_event_queue& aQueue)
    {
        std::scoped_lock lock{ iMutex };
        auto existing = std::find(iEventQueues.begin(), iEventQueues.end(), &aQueue);
        if (existing == iEventQueues.end())
            iEventQueues.push_back(&aQueue);
    }

    void async_task::unregister_event_queue(i_async_event_queue& aQueue)
    {
        std::scoped_lock lock{ iMutex };
        auto existing = std::find(iEventQueues.begin(), iEventQueues.end(), &aQueue);
        if (existing != iEventQueues.end())
            iEventQueues.erase(existing);
    }

    bool async_task::pump_events()
    {
        bool didSome = false;
        {
            std::scoped_lock lock{ iMutex };
            for (auto eventQueue : iEventQueues)
                didSome = (eventQueue->pump_events() || didSome);
        }
        return didSome;
    }

    bool async_task::pump_messages()
    {
        bool didWork = false;
        while (have_messages())
        {
            if (halted())
                return didWork;
            if (have_message_queue())
                message_queue().get_message();
            idle();
            didWork = true;
        }
        idle();
        return didWork;
    }

    bool async_task::running() const noexcept
    {
        return iState == async_task_state::Running;
    }

    bool async_task::halted() const noexcept
    {
        return iState == async_task_state::Halted;
    }

    void async_task::halt()
    {
        iState = async_task_state::Halted;
    }

    bool async_task::finished() const noexcept
    {
        return iState == async_task_state::Finished;
    }

    void async_task::wait() const noexcept
    {
        while (!finished() && !cancelled())
            std::this_thread::yield();
    }

    void async_task::set_destroying()
    {
        if (is_alive())
        {
            Destroying.trigger();
            lifetime::set_destroying();
        }
    }

    void async_task::set_destroyed()
    {
        if (!is_destroyed())
        {
            Destroyed.trigger();
            lifetime::set_destroyed();
        }
    }

    void async_task::run(yield_type aYieldType)
    {
        iState = async_task_state::Running;
        while (!finished() && !cancelled())
            do_work(aYieldType);
        detach();
        iState = async_task_state::Finished;
    }

    void async_task::cancel() noexcept
    {
        base_type::cancel();
        while (running())
            std::this_thread::sleep_for(std::chrono::milliseconds{ 1 });
        iTimerService.reset();
        cancel_io_service();
    }

    void async_task::idle()
    {
        IdleWork.trigger();
        if (have_message_queue())
            message_queue().idle();
    }
} // namespace neolib
