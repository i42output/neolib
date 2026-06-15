// tsc_clock.hpp
// *** AI (ChatGPT 5.2, Claude Opus 4.8) GENERATED WITH MODIFICATIONS ***
/*
Creative Commons Legal Code

CC0 1.0 Universal

    CREATIVE COMMONS CORPORATION IS NOT A LAW FIRM AND DOES NOT PROVIDE
    LEGAL SERVICES. DISTRIBUTION OF THIS DOCUMENT DOES NOT CREATE AN
    ATTORNEY-CLIENT RELATIONSHIP. CREATIVE COMMONS PROVIDES THIS
    INFORMATION ON AN "AS-IS" BASIS. CREATIVE COMMONS MAKES NO WARRANTIES
    REGARDING THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS
    PROVIDED HEREUNDER, AND DISCLAIMS LIABILITY FOR DAMAGES RESULTING FROM
    THE USE OF THIS DOCUMENT OR THE INFORMATION OR WORKS PROVIDED
    HEREUNDER.

Statement of Purpose

The laws of most jurisdictions throughout the world automatically confer
exclusive Copyright and Related Rights (defined below) upon the creator
and subsequent owner(s) (each and all, an "owner") of an original work of
authorship and/or a database (each, a "Work").

Certain owners wish to permanently relinquish those rights to a Work for
the purpose of contributing to a commons of creative, cultural and
scientific works ("Commons") that the public can reliably and without fear
of later claims of infringement build upon, modify, incorporate in other
works, reuse and redistribute as freely as possible in any form whatsoever
and for any purposes, including without limitation commercial purposes.
These owners may contribute to the Commons to promote the ideal of a free
culture and the further production of creative, cultural and scientific
works, or to gain reputation or greater distribution for their Work in
part through the use and efforts of others.

For these and/or other purposes and motivations, and without any
expectation of additional consideration or compensation, the person
associating CC0 with a Work (the "Affirmer"), to the extent that he or she
is an owner of Copyright and Related Rights in the Work, voluntarily
elects to apply CC0 to the Work and publicly distribute the Work under its
terms, with knowledge of his or her Copyright and Related Rights in the
Work and the meaning and intended legal effect of CC0 on those rights.

1. Copyright and Related Rights. A Work made available under CC0 may be
protected by copyright and related or neighboring rights ("Copyright and
Related Rights"). Copyright and Related Rights include, but are not
limited to, the following:

  i. the right to reproduce, adapt, distribute, perform, display,
     communicate, and translate a Work;
 ii. moral rights retained by the original author(s) and/or performer(s);
iii. publicity and privacy rights pertaining to a person's image or
     likeness depicted in a Work;
 iv. rights protecting against unfair competition in regards to a Work,
     subject to the limitations in paragraph 4(a), below;
  v. rights protecting the extraction, dissemination, use and reuse of data
     in a Work;
 vi. database rights (such as those arising under Directive 96/9/EC of the
     European Parliament and of the Council of 11 March 1996 on the legal
     protection of databases, and under any national implementation
     thereof, including any amended or successor version of such
     directive); and
vii. other similar, equivalent or corresponding rights throughout the
     world based on applicable law or treaty, and any national
     implementations thereof.

2. Waiver. To the greatest extent permitted by, but not in contravention
of, applicable law, Affirmer hereby overtly, fully, permanently,
irrevocably and unconditionally waives, abandons, and surrenders all of
Affirmer's Copyright and Related Rights and associated claims and causes
of action, whether now known or unknown (including existing as well as
future claims and causes of action), in the Work (i) in all territories
worldwide, (ii) for the maximum duration provided by applicable law or
treaty (including future time extensions), (iii) in any current or future
medium and for any number of copies, and (iv) for any purpose whatsoever,
including without limitation commercial, advertising or promotional
purposes (the "Waiver"). Affirmer makes the Waiver for the benefit of each
member of the public at large and to the detriment of Affirmer's heirs and
successors, fully intending that such Waiver shall not be subject to
revocation, rescission, cancellation, termination, or any other legal or
equitable action to disrupt the quiet enjoyment of the Work by the public
as contemplated by Affirmer's express Statement of Purpose.

3. Public License Fallback. Should any part of the Waiver for any reason
be judged legally invalid or ineffective under applicable law, then the
Waiver shall be preserved to the maximum extent permitted taking into
account Affirmer's express Statement of Purpose. In addition, to the
extent the Waiver is so judged Affirmer hereby grants to each affected
person a royalty-free, non transferable, non sublicensable, non exclusive,
irrevocable and unconditional license to exercise Affirmer's Copyright and
Related Rights in the Work (i) in all territories worldwide, (ii) for the
maximum duration provided by applicable law or treaty (including future
time extensions), (iii) in any current or future medium and for any number
of copies, and (iv) for any purpose whatsoever, including without
limitation commercial, advertising or promotional purposes (the
"License"). The License shall be deemed effective as of the date CC0 was
applied by Affirmer to the Work. Should any part of the License for any
reason be judged legally invalid or ineffective under applicable law, such
partial invalidity or ineffectiveness shall not invalidate the remainder
of the License, and in such case Affirmer hereby affirms that he or she
will not (i) exercise any of his or her remaining Copyright and Related
Rights in the Work or (ii) assert any associated claims and causes of
action with respect to the Work, in either case contrary to Affirmer's
express Statement of Purpose.

4. Limitations and Disclaimers.

 a. No trademark or patent rights held by Affirmer are waived, abandoned,
    surrendered, licensed or otherwise affected by this document.
 b. Affirmer offers the Work as-is and makes no representations or
    warranties of any kind concerning the Work, express, implied,
    statutory or otherwise, including without limitation warranties of
    title, merchantability, fitness for a particular purpose, non
    infringement, or the absence of latent or other defects, accuracy, or
    the present or absence of errors, whether or not discoverable, all to
    the greatest extent permissible under applicable law.
 c. Affirmer disclaims responsibility for clearing rights of other persons
    that may apply to the Work or any use thereof, including without
    limitation any person's Copyright and Related Rights in the Work.
    Further, Affirmer disclaims responsibility for obtaining any necessary
    consents, permissions or other rights required for any use of the
    Work.
 d. Affirmer understands and acknowledges that Creative Commons is not a
    party to this document and has no duty or obligation with respect to
    this CC0 or use of the Work.
*/

#pragma once

#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdint>
#include <limits>
#include <thread>
#include <vector>

#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <x86intrin.h>
#if defined(__i386__) || defined(__x86_64__)
#include <cpuid.h> // __cpuid macro lives here, not in <x86intrin.h>
#endif
#endif

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#endif

#if !defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
// nothing
#elif !defined(__SIZEOF_INT128__) && defined(_MSC_VER)
// For Boost fallback (used e.g. on MSVC x86)
#include <boost/multiprecision/cpp_int.hpp>
#endif

namespace neolib::chrono
{
    namespace detail
    {

        // ------------------------ Architecture detection -----------------------------

#if defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64)
#define TSC_AVAILABLE 1
        constexpr bool kTscAvailable = true;
#else
#define TSC_AVAILABLE 0
        constexpr bool kTscAvailable = false;
#endif

        // ------------------------ Spin hint -----------------------------------------
        // Self-contained PAUSE/yield. Deliberately NOT named cpu_relax: Boost.Fiber's
        // <boost/fiber/detail/cpu_relax.hpp> defines cpu_relax() as a function-like macro,
        // which would rewrite this definition and every call site if that header is in the TU.

        inline void spin_pause() noexcept
        {
#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_X64))
            _mm_pause();
#elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
            __yield();
#elif defined(__i386__) || defined(__x86_64__)
            __builtin_ia32_pause();
#elif defined(__aarch64__) || defined(__arm__)
            __asm__ __volatile__("yield" ::: "memory");
#else
            std::this_thread::yield();
#endif
        }

        // ------------------------ Ordered TSC reads ---------------------------------
        // Prefer RDTSCP (partially serializing) for end read.
        // Use CPUID before begin read to serialize prior instructions.

        inline std::uint64_t rdtsc_begin_ordered() noexcept
        {
#if !TSC_AVAILABLE
            return 0;
#elif defined(_MSC_VER)
            int cpuInfo[4];
            __cpuid(cpuInfo, 0);
            return __rdtsc();
#else
            unsigned int eax, ebx, ecx, edx;
            __cpuid(0, eax, ebx, ecx, edx);
            return __rdtsc();
#endif
        }

        inline std::uint64_t rdtscp_end() noexcept
        {
#if !TSC_AVAILABLE
            return 0;
#else
            unsigned aux = 0;
            return __rdtscp(&aux);
#endif
        }

        // RDTSCP returns the counter AND IA32_TSC_AUX atomically. Reading aux here lets
        // callers learn which logical CPU produced the timestamp without a separate query,
        // closing the migration window between "which CPU am I on" and "read the TSC".
        inline std::uint64_t rdtscp_end_aux(unsigned& aux) noexcept
        {
#if !TSC_AVAILABLE
            aux = 0;
            return 0;
#else
            return __rdtscp(&aux);
#endif
        }

        // Cheapest read: plain RDTSC. Unlike RDTSCP it does not wait for prior loads/stores,
        // so the sample may be taken slightly early relative to surrounding code under OOO
        // execution -- fine for a timestamp, and it avoids the aux register entirely.
        inline std::uint64_t rdtsc_now() noexcept
        {
#if !TSC_AVAILABLE
            return 0;
#else
            return __rdtsc();
#endif
        }

        // ------------------------ CPUID invariant TSC --------------------------------
        // CPUID.80000007H:EDX[8] = Invariant TSC (when leaf exists)

        inline bool cpu_has_invariant_tsc() noexcept
        {
#if !TSC_AVAILABLE
            return false;
#elif defined(_MSC_VER)
            int regs[4]{};
            __cpuid(regs, 0x80000000);
            unsigned max_leaf = static_cast<unsigned>(regs[0]);
            if (max_leaf < 0x80000007) return false;
            __cpuid(regs, 0x80000007);
            return (regs[3] & (1 << 8)) != 0;
#else
            unsigned int eax, ebx, ecx, edx;
            __cpuid(0x80000000, eax, ebx, ecx, edx);
            if (eax < 0x80000007) return false;
            __cpuid(0x80000007, eax, ebx, ecx, edx);
            return (edx & (1u << 8)) != 0;
#endif
        }

        // ------------------------ Fixed-point ticks->ns scaling ----------------------
        // dns = (dticks * mul) >> shift
        // where mul ≈ (ns_per_tick * 2^shift) computed once at calibration.

        inline std::uint64_t scale_ticks_to_ns(std::uint64_t ticks, std::uint64_t mul, unsigned shift) noexcept
        {
#if defined(__SIZEOF_INT128__)
            __uint128_t p = static_cast<__uint128_t>(ticks) * static_cast<__uint128_t>(mul);
            return static_cast<std::uint64_t>(p >> shift);
#elif defined(_MSC_VER) && defined(_M_X64)
            std::uint64_t hi = 0;
            std::uint64_t lo = _umul128(ticks, mul, &hi);

            if (shift == 0)
                return lo;

            if (shift < 64)
            {
                // (hi:lo) >> shift
                return (lo >> shift) | (hi << (64 - shift));
            }
            else
            {
                const unsigned s = shift - 64;
                return (s >= 64) ? 0ull : (hi >> s);
            }
#else
            using boost::multiprecision::uint128_t;
            uint128_t p = uint128_t(ticks) * uint128_t(mul);
            return static_cast<std::uint64_t>(p >> shift);
#endif
        }

        // ------------------------ Platform CPU id + affinity ------------------------

        struct CpuHandle
        {
#if defined(_WIN32)
            GROUP_AFFINITY ga{};
#else
            int cpu = -1;
#endif
        };

        inline int cpu_count_online() noexcept
        {
#if defined(_WIN32)
            // Count all active processors across groups.
            WORD groups = GetActiveProcessorGroupCount();
            int total = 0;
            for (WORD g = 0; g < groups; ++g) {
                total += static_cast<int>(GetActiveProcessorCount(g));
            }
            return total;
#else
            long n = ::sysconf(_SC_NPROCESSORS_ONLN);
            return (n > 0) ? static_cast<int>(n) : 0;
#endif
        }

        // Enumerate logical CPUs into a list of handles we can pin to.
        // Windows: one handle per logical processor across groups.
        // Linux: cpu indices [0..N-1].
        inline bool enumerate_cpus(std::vector<CpuHandle>& out)
        {
            out.clear();
            const int n = cpu_count_online();
            if (n <= 0) return false;

#if defined(_WIN32)
            WORD groups = GetActiveProcessorGroupCount();
            for (WORD g = 0; g < groups; ++g)
            {
                DWORD count = GetActiveProcessorCount(g);
                for (DWORD i = 0; i < count; ++i)
                {
                    CpuHandle h;
                    h.ga.Group = g;
                    h.ga.Mask = (KAFFINITY(1) << i);
                    out.push_back(h);
                }
            }
            return !out.empty();
#else
            out.reserve(static_cast<std::size_t>(n));
            for (int c = 0; c < n; ++c)
            {
                CpuHandle h;
                h.cpu = c;
                out.push_back(h);
            }
            return true;
#endif
        }

        // Pin current thread to one CPU; return previous affinity state for restore.
        struct PrevAffinity
        {
#if defined(_WIN32)
            GROUP_AFFINITY prev{};
            bool valid = false;
#else
            cpu_set_t prev{};
            bool valid = false;
#endif
        };

        inline bool pin_this_thread(const CpuHandle& cpu, PrevAffinity& prev_out) noexcept
        {
#if defined(_WIN32)
            prev_out.valid = SetThreadGroupAffinity(GetCurrentThread(), &cpu.ga, &prev_out.prev) != 0;
            return prev_out.valid;
#else
            CPU_ZERO(&prev_out.prev);
            if (pthread_getaffinity_np(pthread_self(), sizeof(prev_out.prev), &prev_out.prev) != 0) {
                prev_out.valid = false;
                return false;
            }
            cpu_set_t set;
            CPU_ZERO(&set);
            CPU_SET(cpu.cpu, &set);
            prev_out.valid = (pthread_setaffinity_np(pthread_self(), sizeof(set), &set) == 0);
            return prev_out.valid;
#endif
        }

        inline void restore_affinity(const PrevAffinity& prev) noexcept
        {
#if defined(_WIN32)
            if (!prev.valid) return;
            GROUP_AFFINITY ignored{};
            (void)SetThreadGroupAffinity(GetCurrentThread(), &prev.prev, &ignored);
#else
            if (!prev.valid) return;
            (void)pthread_setaffinity_np(pthread_self(), sizeof(prev.prev), &prev.prev);
#endif
        }

    } // namespace detail

    // ============================================================================
    // tsc_clock
    // ============================================================================

    struct tsc_clock
    {
        using rep = std::int64_t;
        using period = std::nano;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<tsc_clock, duration>;

        using steady_clock = std::conditional_t<
            std::chrono::high_resolution_clock::is_steady,
            std::chrono::high_resolution_clock,
            std::chrono::steady_clock>;

        static constexpr bool is_steady = true;

        struct options
        {
            std::chrono::milliseconds calibration_window{ 200 };          // larger => better ratio stability
            int                       validation_rounds{ 64 };            // more rounds => better confidence
            std::chrono::nanoseconds  max_allowed_skew{ std::chrono::microseconds(5) }; // if worse, fall back
            bool                      enable_per_cpu_offsets{ true };     // makes cross-thread comparisons stronger
            std::chrono::nanoseconds  offset_apply_threshold{ std::chrono::nanoseconds(100) }; // don't pay the per-call correction cost unless measured offsets exceed this
        };

        // Calibration is expensive and largely one-off: it spends ~calibration_window
        // (default 200 ms) measuring the TSC/steady ratio, then runs a multi-threaded
        // validation pass (~50 ms lead-in + validation_rounds * 200 us, plus spawning
        // one thread per logical CPU). It is STRONGLY recommended to call init() once
        // explicitly at start-up; otherwise the very first now() pays this whole cost
        // lazily (and serialises any other thread that calls now() concurrently).
        static void init(options opt = {}) noexcept
        {
            (void)calibrate_once(opt);
        }

        static time_point now() noexcept
        {
            if (!sReady.load(std::memory_order_acquire))
            {
                options opt{};
                calibrate_once(opt);
            }

            if (!sUseTsc.load(std::memory_order_relaxed) || !detail::kTscAvailable)
            {
                auto s = steady_clock::now().time_since_epoch();
                auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(s).count();
                return time_point(duration(static_cast<rep>(ns)));
            }

            unsigned aux = 0;
            const std::uint64_t base_tsc = sBase_tsc;
            const std::uint64_t base_ns = sBase_ns;
            const std::uint64_t mul = sNsPerTickMul;
            const unsigned      shift = sNsPerTickShift;

            // Fast path: when no per-CPU correction is in effect, a plain RDTSC (cheaper than
            // RDTSCP, and no per-call CPU query) plus the fixed-point scale is all we need.
            if (!sPerCpuEnabled)
            {
                const std::uint64_t t = detail::rdtsc_now();
                const std::uint64_t dticks = (t >= base_tsc) ? (t - base_tsc) : 0;
                const std::uint64_t ns = base_ns + detail::scale_ticks_to_ns(dticks, mul, shift);
                return time_point(duration(static_cast<rep>(ns)));
            }

            // Correction path: we need the CPU that produced this timestamp, so use RDTSCP to
            // read the counter and IA32_TSC_AUX atomically (see current_cpu_index_dense()).
            const std::uint64_t t = detail::rdtscp_end_aux(aux);

            const std::uint64_t dticks = (t >= base_tsc) ? (t - base_tsc) : 0;
            const std::uint64_t dns = detail::scale_ticks_to_ns(dticks, mul, shift);
            std::uint64_t ns = base_ns + dns;

            const int cpu = current_cpu_index_dense(aux);
            const std::uint32_t n = sOffsetsCount;
            if (cpu >= 0 && static_cast<std::uint32_t>(cpu) < n)
            {
                const std::int64_t corr = sOffsets_ns[static_cast<std::uint32_t>(cpu)];
                ns = static_cast<std::uint64_t>(static_cast<std::int64_t>(ns) + corr);
            }

            return time_point(duration(static_cast<rep>(ns)));
        }

    private:
        // Current CPU index (dense [0..cpu_count-1]) for indexing offset arrays.
        //
        // Linux: the kernel programs IA32_TSC_AUX = (node << 12) | (cpu & 0xfff), so the
        // dense cpu index falls straight out of the aux value returned by the same RDTSCP
        // that produced the timestamp -- no extra query and no migration window.
        //
        // Windows: the IA32_TSC_AUX encoding is not a documented stable ABI, so decode the
        // current processor freshly here. This is read on every now() (it must reflect the
        // calling thread's CPU, not the calibrating thread's), and is a cheap user-mode read.
        static int current_cpu_index_dense(unsigned aux) noexcept
        {
#if defined(_WIN32)
            (void)aux;
            PROCESSOR_NUMBER pn{};
            GetCurrentProcessorNumberEx(&pn);

            const int groups = sGroupCount;
            const int g = static_cast<int>(pn.Group);
            if (g < 0 || g >= groups)
                return -1;

            return sGroupBase[g] + static_cast<int>(pn.Number);
#else
            return static_cast<int>(aux & 0xfffu);
#endif
        }

        // Sample a (TSC, steady) anchor with the steady read bracketed between two TSC
        // reads, taking the TSC midpoint. This keeps the two clock domains near-simultaneous
        // at the anchor, instead of biasing by one steady-read latency.
        static void sample_anchor(std::uint64_t& tsc_out, std::uint64_t& ns_out) noexcept
        {
            const std::uint64_t a = detail::rdtsc_begin_ordered();
            const auto s = steady_clock::now();
            const std::uint64_t b = detail::rdtscp_end();
            tsc_out = a + (b - a) / 2; // midpoint, overflow-safe
            ns_out = static_cast<std::uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(s.time_since_epoch()).count());
        }

#if defined(_WIN32)
        static void precompute_group_bases() noexcept
        {
            const WORD groupsW = GetActiveProcessorGroupCount();
            const int groups = (groupsW <= kMaxProcessorGroups) ? static_cast<int>(groupsW) : kMaxProcessorGroups;

            int base = 0;
            for (int g = 0; g < groups; ++g)
            {
                sGroupBase[g] = base;

                const DWORD count = GetActiveProcessorCount(static_cast<WORD>(g));
                base += static_cast<int>(count);
            }

            sGroupCount = groups;
        }
#endif

        static bool calibrate_once(const options& opt) noexcept
        {
            bool expected = false;
            if (!sCalibrating.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
            {
                while (!sReady.load(std::memory_order_acquire))
                {
                    detail::spin_pause();
                }
                return sUseTsc.load(std::memory_order_relaxed);
            }

            sUseTsc.store(false, std::memory_order_relaxed);
            sPerCpuEnabled = false;
            sOffsetsCount = 0;

            // Basic arch + feature gating
            if constexpr (!detail::kTscAvailable)
            {
                sReady.store(true, std::memory_order_release);
                sCalibrating.store(false, std::memory_order_release);
                return false;
            }
            if (!detail::cpu_has_invariant_tsc())
            {
                sReady.store(true, std::memory_order_release);
                sCalibrating.store(false, std::memory_order_release);
                return false;
            }

#if defined(_WIN32)
            precompute_group_bases();
#endif

            // Warm up
            for (int i = 0; i < 2000; ++i)
                (void)__rdtsc();

            // 1) Global calibration vs chrono steady_clock clock.
            //    Each anchor is a bracketed (TSC, steady) pair so c0/ns0 and c1/ns1 are
            //    each near-simultaneous; dt_ns and dc therefore span the same interval.
            std::uint64_t c0 = 0, ns0 = 0, c1 = 0, ns1 = 0;
            sample_anchor(c0, ns0);

            const auto window_start = steady_clock::now();
            while (steady_clock::now() - window_start < opt.calibration_window)
            {
                detail::spin_pause();
            }

            sample_anchor(c1, ns1);

            const std::int64_t dt_ns = static_cast<std::int64_t>(ns1) - static_cast<std::int64_t>(ns0);
            const std::uint64_t dc = (c1 >= c0) ? (c1 - c0) : 0;

            if (dt_ns <= 0 || dc == 0)
            {
                sReady.store(true, std::memory_order_release);
                sCalibrating.store(false, std::memory_order_release);
                return false;
            }

            // Anchor epoch mapping at end of calibration window
            const std::uint64_t base_tsc = c1;
            const std::uint64_t base_ns = ns1;

            sBase_tsc = base_tsc;
            sBase_ns = base_ns;

            // Fixed-point multiplier (computed once; used in now() without division)
            // dns = (dticks * mul) >> SHIFT
            // mul = round((dt_ns * 2^SHIFT) / dc)
            constexpr unsigned SHIFT = 32;

#if defined(__SIZEOF_INT128__)
            __uint128_t numer = (static_cast<__uint128_t>(static_cast<std::uint64_t>(dt_ns)) << SHIFT)
                + (static_cast<__uint128_t>(dc) >> 1); // rounding
            const std::uint64_t mul = static_cast<std::uint64_t>(numer / dc);
#elif defined(_MSC_VER) && defined(_M_X64)
            // Calibration-only 128/64 division is acceptable here.
            std::uint64_t hi = 0, lo = 0;
            const std::uint64_t pow2 = (1ull << SHIFT);
            lo = _umul128(static_cast<std::uint64_t>(dt_ns), pow2, &hi);

            // + dc/2 for rounding
            const std::uint64_t add = (dc >> 1);
            const std::uint64_t lo2 = lo + add;
            hi += (lo2 < lo) ? 1 : 0;
            lo = lo2;

            std::uint64_t rem = 0;
            const std::uint64_t mul = _udiv128(hi, lo, dc, &rem);
#else
            using boost::multiprecision::uint128_t;
            uint128_t numer = (uint128_t(static_cast<std::uint64_t>(dt_ns)) << SHIFT)
                + (uint128_t(dc) >> 1);
            const std::uint64_t mul = static_cast<std::uint64_t>(numer / dc);
#endif

            sNsPerTickMul = mul;
            sNsPerTickShift = SHIFT;

            // 2) Cross-thread/cross-core validation + optional per-CPU offsets.
            //    This section allocates and spawns threads, both of which can throw; the
            //    whole thing is wrapped so calibrate_once stays noexcept and a failure here
            //    degrades cleanly to steady_clock rather than terminating the process.
            try
            {
                std::vector<detail::CpuHandle> cpus;
                if (!detail::enumerate_cpus(cpus) || cpus.empty() || cpus.size() > kMaxCpus)
                {
                    // Can't enumerate CPUs => without validation, TSC mode is risky, so fall back.
                    sReady.store(true, std::memory_order_release);
                    sCalibrating.store(false, std::memory_order_release);
                    return false;
                }

                const int ncpu = static_cast<int>(cpus.size());
                const int rounds = (opt.validation_rounds > 0 ? opt.validation_rounds : 1);

                std::barrier sync_point{ ncpu };

                // Launch protocol: workers only touch the barrier once every worker has been
                // spawned (launch=true). If thread creation fails part-way, we set abort and
                // the already-spawned workers return WITHOUT entering the barrier -- otherwise a
                // barrier sized for ncpu would deadlock the join with fewer participants.
                std::atomic<bool> launch{ false };
                std::atomic<bool> abort_workers{ false };

                struct Sample { std::uint64_t tsc = 0; std::int64_t steady_ns = 0; };
                std::vector<Sample> samples(static_cast<std::size_t>(ncpu) * static_cast<std::size_t>(rounds));
                std::atomic<bool> ok{ true };

                std::vector<std::thread> threads;
                threads.reserve(static_cast<std::size_t>(ncpu));

                auto join_all = [&threads]() noexcept
                    {
                        for (auto& th : threads)
                            if (th.joinable())
                                th.join();
                    };

                // Worker per CPU: wait for launch, pin, then barrier-aligned rdtscp.
                try
                {
                    for (int i = 0; i < ncpu; ++i)
                    {
                        threads.emplace_back([&, i]
                            {
                                while (!launch.load(std::memory_order_acquire))
                                {
                                    if (abort_workers.load(std::memory_order_acquire))
                                        return; // spawn failed elsewhere; never touch the barrier
                                    detail::spin_pause();
                                }

                                detail::PrevAffinity prev{};
                                if (!detail::pin_this_thread(cpus[static_cast<std::size_t>(i)], prev))
                                {
                                    ok.store(false, std::memory_order_relaxed);
                                    sync_point.arrive_and_drop();
                                    return;
                                }

                                // All workers start rounds on the same steady_clock schedule.
                                // A modest lead-in helps everyone reach the first target.
                                constexpr auto kLeadIn = std::chrono::milliseconds(50);
                                constexpr auto kPeriod = std::chrono::microseconds(200); // larger => less jitter, slower

                                sync_point.arrive_and_wait();

                                const auto start_tp = steady_clock::now() + kLeadIn;

                                for (int r = 0; r < rounds; ++r)
                                {
                                    const auto target = start_tp + kPeriod * r;

                                    // Busy-wait until target time (avoid sleep jitter)
                                    while (steady_clock::now() < target)
                                        detail::spin_pause();

                                    // Take paired sample: TSC and steady_clock time near-simultaneously
                                    const std::uint64_t tsc = detail::rdtscp_end();
                                    const auto st = steady_clock::now();

                                    const std::int64_t st_ns =
                                        std::chrono::duration_cast<std::chrono::nanoseconds>(st.time_since_epoch()).count();

                                    samples[static_cast<std::size_t>(r) * static_cast<std::size_t>(ncpu) + static_cast<std::size_t>(i)] =
                                        Sample{ tsc, st_ns };
                                }

                                detail::restore_affinity(prev);
                            });
                    }
                }
                catch (...)
                {
                    // Could not spawn the full set. Release the spawned workers (they exit
                    // before the barrier), join them, then propagate to the outer handler.
                    abort_workers.store(true, std::memory_order_release);
                    join_all();
                    throw;
                }

                // Full set spawned: release everyone into the barrier-aligned measurement.
                launch.store(true, std::memory_order_release);
                join_all();

                if (!ok.load(std::memory_order_relaxed))
                {
                    sReady.store(true, std::memory_order_release);
                    sCalibrating.store(false, std::memory_order_release);
                    return false;
                }

                // Capture mul/shift locally for validation to avoid atomic loads in the lambda.
                const std::uint64_t mul_local = mul;
                const unsigned      shift_local = SHIFT;

                auto tsc_to_ns_i64 = [&](std::uint64_t t) -> std::int64_t
                    {
                        const std::uint64_t dticks = (t >= base_tsc) ? (t - base_tsc) : 0;
                        const std::uint64_t dns = detail::scale_ticks_to_ns(dticks, mul_local, shift_local);
                        const std::uint64_t ns = base_ns + dns;
                        return static_cast<std::int64_t>(ns);
                    };

                // Compute skew stats relative to CPU 0 samples
                std::int64_t max_abs_skew = 0;
                std::vector<std::int64_t> avg_skew(static_cast<std::size_t>(ncpu), 0);
                std::vector<std::int64_t> count(static_cast<std::size_t>(ncpu), 0);

                for (int r = 0; r < rounds; ++r)
                {
                    const auto& refS = samples[static_cast<std::size_t>(r) * static_cast<std::size_t>(ncpu) + 0];
                    const std::int64_t ref_tsc_ns = tsc_to_ns_i64(refS.tsc);
                    const std::int64_t ref_err = ref_tsc_ns - refS.steady_ns; // TSC vs steady error on CPU0

                    for (int i = 0; i < ncpu; ++i)
                    {
                        const auto& S = samples[static_cast<std::size_t>(r) * static_cast<std::size_t>(ncpu) + static_cast<std::size_t>(i)];
                        const std::int64_t tsc_ns = tsc_to_ns_i64(S.tsc);
                        const std::int64_t err = tsc_ns - S.steady_ns;         // TSC vs steady error on CPU i

                        // Compare errors => cancels out "what time did this actually happen" (mostly),
                        // leaving cross-core mapping differences.
                        const std::int64_t skew = err - ref_err;

                        avg_skew[static_cast<std::size_t>(i)] += skew;
                        count[static_cast<std::size_t>(i)] += 1;

                        const std::int64_t abs_skew = (skew < 0) ? -skew : skew;
                        if (abs_skew > max_abs_skew)
                            max_abs_skew = abs_skew;
                    }
                }

                if (max_abs_skew > static_cast<std::int64_t>(opt.max_allowed_skew.count()))
                {
                    // Too much inter-core skew => do not use TSC clock.
                    sReady.store(true, std::memory_order_release);
                    sCalibrating.store(false, std::memory_order_release);
                    return false;
                }

                // Optional per-CPU offsets:
                // correction[i] = -(mean skew[i]) so corrected_ns = raw_ns + correction aligns to CPU 0.
                if (opt.enable_per_cpu_offsets)
                {
                    std::int64_t max_abs_offset = 0;
                    for (int i = 0; i < ncpu; ++i)
                    {
                        const std::int64_t mean = avg_skew[static_cast<std::size_t>(i)] / count[static_cast<std::size_t>(i)];
                        sOffsets_ns[static_cast<std::uint32_t>(i)] = -mean;

                        const std::int64_t a = (mean < 0) ? -mean : mean;
                        if (a > max_abs_offset)
                            max_abs_offset = a;
                    }

                    // Only commit to the correction path (RDTSCP + per-call CPU query) if the
                    // offsets are large enough to be worth it. On a well-synchronised
                    // invariant-TSC system they are usually noise, so we stay on the cheaper
                    // RDTSC fast path and skip the lookup entirely.
                    if (max_abs_offset >= static_cast<std::int64_t>(opt.offset_apply_threshold.count()))
                    {
                        sOffsetsCount = static_cast<std::uint32_t>(ncpu);
                        sPerCpuEnabled = true;
                    }
                }

                // Enable TSC mode
                sUseTsc.store(true, std::memory_order_relaxed);

                sReady.store(true, std::memory_order_release);
                sCalibrating.store(false, std::memory_order_release);
                return true;
            }
            catch (...)
            {
                // Allocation or thread-creation failure during validation: degrade to steady_clock.
                sUseTsc.store(false, std::memory_order_relaxed);
                sPerCpuEnabled = false;
                sOffsetsCount = 0;
                sReady.store(true, std::memory_order_release);
                sCalibrating.store(false, std::memory_order_release);
                return false;
            }
        }

        // Shared state.
        //
        // Publication is via sReady: calibrate_once writes every scalar below (plain, not
        // atomic) and then performs a release store to sReady; now() performs an acquire
        // load of sReady before reading any of them. The acquire/release pair establishes
        // happens-before, so plain reads in now() observe the calibrated values.
        static inline std::atomic<bool>     sReady{ false };
        static inline std::atomic<bool>     sCalibrating{ false };
        static inline std::atomic<bool>     sUseTsc{ false };

        static inline std::uint64_t sBase_tsc{ 0 };
        static inline std::uint64_t sBase_ns{ 0 };

        // Fixed-point scaling parameters (used in now())
        static inline std::uint64_t sNsPerTickMul{ 0 };
        static inline unsigned      sNsPerTickShift{ 0 };

        static inline bool     sPerCpuEnabled{ false };

        static constexpr std::uint32_t kMaxCpus = 512; // across Windows groups or big Linux boxes
        static inline std::uint32_t sOffsetsCount{ 0 };
        static inline std::int64_t  sOffsets_ns[kMaxCpus]{};

#if defined(_WIN32)
        // Precomputed mapping: dense_index = group_base[group] + processor_number.
        // The processor *number* itself is read freshly per now() via GetCurrentProcessorNumberEx;
        // only the per-group base offsets are cached here (fixed for process lifetime).
        static constexpr int kMaxProcessorGroups = 64; // Windows supports up to 64 groups.
        static inline int sGroupCount{ 0 };
        static inline int sGroupBase[kMaxProcessorGroups]{};
#endif
    };
}
