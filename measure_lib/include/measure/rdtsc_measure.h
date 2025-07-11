// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

// Time measurment based on x86 processor timestamp (rdtsc instruction).
// Maybe this is not exact in some rare configuration, but most case its ok, and this is the most accurate.

#pragma once

#if RDTSC_MEASURE_IS_SUPPORTED

#include "measure/measure_base.h"

#ifdef _MSC_VER
    #include <intrin.h>
#else
    #include <x86intrin.h>
#endif

namespace dtree
{
    struct RdtscBackend
    {
        inline static const char* GetMeasureTitle() { return "rdtsc times"; }

        inline static uint64_t GetTick() noexcept
        {
            return __rdtsc();
        }

        inline static double TimeDiffToSec(uint64_t time)
        {
            const uint64_t freq = MeasureUtils::GetProcessorFrequency();
            const double sec = double(time) / double(freq);
            return sec;
        }
    };

    using RdtscMeasure = TMeasure<RdtscBackend>;

} // namespace dtree

#endif // RDTSC_MEASURE_IS_SUPPORTED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if MEASURE_IS_ON && RDTSC_MEASURE_IS_SUPPORTED
    #define RDTSC_MEASURE(NAME) \
        static dtree::RdtscMeasure::Base::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::RdtscMeasure::Base::Scope measureScope_##NAME(&measureRecord_##NAME)
    #define RDTSC_MEASURE_S(TITLE) \
        static dtree::RdtscMeasure::Base::MeasureRecord measureRecord(TITLE); \
        dtree::RdtscMeasure::Base::Scope measureScope(&measureRecord)
    #define RDTSC_SAFE_MEASURE(NAME, POLICY) \
        static dtree::RdtscMeasure::POLICY::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::RdtscMeasure::POLICY::Scope measureScope_##NAME(&measureRecord_##NAME)
    #define RDTSC_SAFE_MEASURE_S(TITLE, POLICY) \
        static dtree::RdtscMeasure::POLICY::MeasureRecord measureRecord(TITLE); \
        dtree::RdtscMeasure::POLICY::Scope measureScope(&measureRecord)
#else
    #define RDTSC_MEASURE(NAME)
    #define RDTSC_MEASURE_S(TITLE)
    #define RDTSC_SAFE_MEASURE(NAME, POLICY)
    #define RDTSC_SAFE_MEASURE_S(TITLE, POLICY)
#endif