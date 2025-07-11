// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

#pragma once

#ifndef MEASURE_IS_ON
    #define MEASURE_IS_ON 1
#endif

#if defined(_MSC_VER) && (defined(_WIN64) || defined(_WIN32))
    #define RDTSC_MEASURE_IS_SUPPORTED 1
#elif ((defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__))
    #define RDTSC_MEASURE_IS_SUPPORTED 1
#else
    #define RDTSC_MEASURE_IS_SUPPORTED 0
#endif

#ifndef MEASURE_WINDOWS
    #ifdef _MSC_VER
        #define MEASURE_WINDOWS 1
    #else
        #define MEASURE_WINDOWS 0
    #endif
#endif

#include <string>
#include <iomanip>
#include <sstream>
#include <thread>
#include <chrono>

#if RDTSC_MEASURE_IS_SUPPORTED
    #ifdef _MSC_VER
        #include <intrin.h>
    #else
        #include <x86intrin.h>
    #endif
#endif

#if MEASURE_WINDOWS
    #define NOMINMAX
    #include <wtypes.h>
    #ifdef small
        #undef small
        using small = char;
    #endif
#endif

namespace dtree
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    namespace MeasureUtils
    {
        /// @brief Format a number with a thousands separator (default '\'').
        /// Example: 12345678 -> 12'345'678
        inline std::string FormatWithSeparator(uint64_t num, char separator = '\'')
        {
            std::string s = std::to_string(num);
            int n = int(s.length()) - 3;
            while (n > 0)
            {
                s.insert(n, 1, separator);
                n -= 3;
            }
            return s;
        }

        /// @brief Converts a time duration from seconds to nanoseconds.
        /// @param sec Time duration in seconds.
        /// @return A string representing the time in nanoseconds, formatted with thousands separators.
        /// In this format the longer number is bigger, so it's easier to compare different values.
        /// but a little bit harder to read the individual numbers.
        inline std::string TimeToStrNs(const double sec)
        {
            return FormatWithSeparator(uint64_t(sec*1e9));
        }

        /// @brief Converts a time duration from seconds to a human-readable string format.
        /// @param sec Time duration in seconds.
        /// @return A string representing the time duration in the most suitable unit: seconds, milliseconds, microseconds, or nanoseconds.
        /// In this format every number is short and readable,
        /// but because in this format every number has almost the same length,
        /// it's hard to compare values with different units, sometimes the longer number is the smaller.
        inline std::string TimeToStr(const double sec)
        {
            using namespace std;

            ostringstream sstream;
            sstream << fixed << setw(8) << setprecision(3);
            if (sec >= 10)
                sstream << sec << " sec";
            else if (sec >= 1e-2)
                sstream << sec * 1e3 << " ms.";
            else if (sec >= 1e-5)
                sstream << sec * 1e6 << " us "; // console don't like "µ" character
            else
                sstream << sec * 1e9 << " ns ";
            return sstream.str();
        }

        /**
         * @brief Get the processor frequency in Hz.
         * @param measureTimeSeconds The time for which the thread should sleep in order to measure the frequency.
         * @return The processor frequency in Hz.
         *
         * This function measures the processor frequency by sleeping for a given amount of time and
         * counting the number of cycles of the processor's timestamp counter. The measured frequency
         * is then divided by the elapsed time to get the processor frequency in Hz.
         *
         * The function returns a static variable, so it only measures the frequency once.
         *
         * Note that the measured frequency may not be exactly equal to the processor's rated frequency
         * due to various factors such as turbo boost, power saving, and differences between the
         * actual and nominal clock frequencies.
         */
        inline uint64_t GetProcessorFrequency(double measureTimeSeconds = 0.25)
        {
            static uint64_t freq = [measureTimeSeconds]() -> uint64_t
            {
#if RDTSC_MEASURE_IS_SUPPORTED
                auto measureTimeUs = std::chrono::nanoseconds(uint64_t(measureTimeSeconds * 1e9));

                auto t1 = std::chrono::high_resolution_clock::now();
                const uint64_t start = __rdtsc();

                std::this_thread::sleep_for(measureTimeUs); // not accurate

                const uint64_t end = __rdtsc();
                auto t2 = std::chrono::high_resolution_clock::now();

                std::chrono::nanoseconds elapsed = t2 - t1;
                double elapsedSec = double(elapsed.count()) / 1e9;
                // elapsedSec ~ measureTimeSeconds, but not equal :-(
                return uint64_t((end - start) / elapsedSec);
#else
                //static_assert(false, "unknown compiler");
                return 3200000000ull;
#endif
            }();
            return freq;
        }

#if MEASURE_WINDOWS
        inline void SetThreadAffinity(const DWORD_PTR coreIdx = 0)
        {
            const HANDLE hThread = ::GetCurrentThread();
            ::SetThreadAffinityMask(hThread, DWORD_PTR(1) << coreIdx);
        }

        inline unsigned GetCurrentProcessorNumberXp()
        {
            int cpuInfo[4];
            __cpuid(cpuInfo, 1);
            // CPUInfo[3] is EDX, bit 9 = "APIC On-Chip."
            if ((cpuInfo[3] & (1 << 9)) == 0)
                return (uint32_t)-1;  // no APIC on chip
            // CPUInfo[1] is EBX, bits 24-31 are APIC ID
            return (unsigned)cpuInfo[1] >> 24;
        }
#endif
    } // namespace MeasureUtils
} // namespace dtree
