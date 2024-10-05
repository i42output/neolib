// system.hpp
/*
 *  Copyright (c) 2018, 2020 Leigh Johnston.
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

#pragma once

#include <neolib/neolib.hpp>
#include <atomic>
#include <vector>
#include <numeric>
#include <chrono>
#include <neolib/core/set.hpp>
#include <neolib/core/allocator.hpp>
#include <neolib/task/thread.hpp>
#include <neolib/task/async_task.hpp>
#include <neolib/task/async_thread.hpp>
#include <neolib/app/i_power.hpp>
#include <neolib/ecs/i_system.hpp>
#include <neolib/ecs/entity_info.hpp>

namespace neolib::ecs
{
    class i_ecs;

    template <typename... ComponentData>
    class system : public i_system
    {
        typedef system<ComponentData...> self_type;
    private:
        class thread : public async_task, public async_thread
        {
        public:
            thread(self_type& aOwner) : async_task{ "neolib::ecs::system::thread" }, async_thread{ *this, "neolib::ecs::system::thread" }, iOwner{ aOwner }
            {
                start();
            }
            ~thread()
            {
                set_destroying();
                if (iOwner.waiting())
                    iOwner.signal();
            }
        public:
            bool do_work(neolib::yield_type aYieldType = neolib::yield_type::NoYield) final
            {
                bool didWork = async_task::do_work(aYieldType);
                if (iOwner.can_apply())
                    didWork = iOwner.apply() || didWork;
                iOwner.yield();
                if (iOwner.paused() && !iOwner.waiting())
                    iOwner.wait();
                return didWork;
            }
        private:
            self_type& iOwner;
        };
        typedef neolib::set<component_id, std::less<component_id>, neolib::fast_pool_allocator<component_id>> component_list;
        struct performance_metrics
        {
            std::vector<std::chrono::microseconds> updateTimes;
            std::size_t updateCounter = 0;
            std::chrono::high_resolution_clock::time_point updateStartTime;
        };
    public:
        struct no_thread : std::logic_error { no_thread() : std::logic_error{ "neolib::ecs::system::no_thread" } {} };
    public:
        system(i_ecs& aEcs) :
            iEcs{ aEcs }, iComponents{ ComponentData::meta::id()... }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
        }
        system(const system& aOther) :
            iEcs{ aOther.iEcs }, iComponents{ aOther.iComponents }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
        }
        system(system&& aOther) :
            iEcs{ aOther.iEcs }, iComponents{ std::move(aOther.iComponents) }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
        }
        template <typename ComponentIdIter>
        system(i_ecs& aEcs, ComponentIdIter aFirstComponent, ComponentIdIter aLastComponent) :
            iEcs{ aEcs }, iComponents{ aFirstComponent, aLastComponent }, iPaused{ 0u }
        {
            (ecs().template component<ComponentData>(), ...);
            if (ecs().all_systems_paused())
                pause();
        }
        ~system()
        {
        }
    public:
        i_ecs& ecs() const final
        {
            return iEcs;
        }
    public:
        const i_set<component_id>& components() const final
        {
            return iComponents;
        }
        i_set<component_id>& components() final
        {
            return iComponents;
        }
    public:
        const i_component& component(component_id aComponentId) const final
        {
            return ecs().component(aComponentId);
        }
        i_component& component(component_id aComponentId) final
        {
            return ecs().component(aComponentId);
        }
    public:
        bool can_apply() const final
        {
            return !paused() && (!have_thread() || (have_thread() && get_thread().in()));
        }
        bool paused() const final
        {
            return iPaused != 0u;
        }
        void pause() final
        {
            ++iPaused;
        }
        void resume() final
        {
            if (--iPaused == 0 && waiting())
                signal();
        }
        void terminate() final
        {
            iThread = nullptr;
        }
        bool waiting() const final
        {
            return iWaiting;
        }
        void wait() final
        {
            if (!have_thread())
                throw no_thread();
            if (!get_thread().in())
                throw wrong_thread();
            if (!get_thread().is_alive())
                return;
            std::unique_lock<std::mutex> lock{ iMutex };
            iWaiting = true;
            iCondVar.wait(lock, [&]() { return !iWaiting; });
        }
        void wait_for(scalar aDuration) final
        {
            if (!have_thread())
                throw no_thread();
            if (!get_thread().in())
                throw wrong_thread();
            if (!get_thread().is_alive())
                return;
            std::unique_lock<std::mutex> lock{ iMutex };
            iWaiting = true;
            iCondVar.wait_for(lock, std::chrono::duration<double>(aDuration), [&](){ return !iWaiting; });
        }
        void signal() final
        {
            if (have_thread() && get_thread().in())
                throw wrong_thread();
            bool doIt = false;
            {
                std::unique_lock<std::mutex> lock{ iMutex };
                if (iWaiting)
                {
                    iWaiting = false;
                    doIt = true;
                }
            }
            if (doIt)
                iCondVar.notify_one();
        }
    public:
        void start_thread_if() final
        {
            if (ecs().run_threaded(id()))
                start_thread();
        }
        void start_thread() final
        {
            iThread = std::make_unique<thread>(*this);
        }
    public:
        bool debug() const final
        {
            return iDebug;
        }
        void set_debug(bool aDebug) final
        {
            if (iDebug != aDebug)
            {
                iDebug = aDebug;
                iPerformanceMetrics.clear();
            }
        }
        std::chrono::microseconds update_time(std::size_t aMetricsIndex = 0) const final
        {
            if (iPerformanceMetrics.size() <= aMetricsIndex || iPerformanceMetrics[aMetricsIndex].updateTimes.empty())
                return std::chrono::microseconds{ 0 };
            return std::accumulate(iPerformanceMetrics[aMetricsIndex].updateTimes.begin(), iPerformanceMetrics[aMetricsIndex].updateTimes.end(), std::chrono::microseconds{}) / iPerformanceMetrics[aMetricsIndex].updateTimes.size();
        }
    protected:
        bool have_thread() const
        {
            return iThread != nullptr;
        }
        thread& get_thread() const
        {
            if (have_thread())
                return *iThread;
            throw no_thread();
        }
        void yield(bool aSleep = false)
        {
            if (service<neolib::i_power>().green_mode_active() || aSleep)
                neolib::thread::sleep(std::chrono::milliseconds{ 1 });
            else
                neolib::thread::yield();
        }
        std::mutex& waiting_mutex()
        {
            return iMutex;
        }
        void start_update(std::size_t aMetricsIndex = 0)
        {
            if (debug())
            {
                if (iPerformanceMetrics.size() <= aMetricsIndex)
                    iPerformanceMetrics.resize(aMetricsIndex + 1);
                iPerformanceMetrics[aMetricsIndex].updateStartTime = std::chrono::high_resolution_clock::now();
            }
        }
        void end_update(std::size_t aMetricsIndex = 0)
        {
            if (debug())
            {
                if (iPerformanceMetrics.size() > aMetricsIndex)
                {
                    std::size_t const updateQueueSize = 100;
                    auto const time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - iPerformanceMetrics[aMetricsIndex].updateStartTime);
                    if (iPerformanceMetrics[aMetricsIndex].updateTimes.size() < updateQueueSize)
                        iPerformanceMetrics[aMetricsIndex].updateTimes.push_back(time);
                    else
                    {
                        iPerformanceMetrics[aMetricsIndex].updateTimes[iPerformanceMetrics[aMetricsIndex].updateCounter++] = time;
                        iPerformanceMetrics[aMetricsIndex].updateCounter %= updateQueueSize;
                    }
                }
            }
        }
    private:
        i_ecs& iEcs;
        component_list iComponents;
        std::atomic<uint32_t> iPaused = 0u;
        std::mutex iMutex;
        std::condition_variable iCondVar;
        std::atomic<bool> iWaiting = false;
        std::atomic<bool> iDebug = false;
        std::vector<performance_metrics> iPerformanceMetrics;
        std::unique_ptr<thread> iThread;
    };
}