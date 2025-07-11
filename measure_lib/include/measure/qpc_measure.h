// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

// Time measurment based on QueryPerformanceCounter. MSVC only.

#pragma once

#include "measure/measure_base.h"

#if MEASURE_WINDOWS

#define NOMINMAX
#include <winnt.h>
#include <profileapi.h>

namespace dtree
{
    struct QPCBackend
    {
        inline static const char* GetMeasureTitle() { return "QueryPerformanceCounter times"; }

        inline static int64_t GetTick()
        {
            LARGE_INTEGER counter = {};
            QueryPerformanceCounter(&counter);
            return counter.QuadPart;
        }

        inline static double TimeDiffToSec(int64_t time)
        {
            return double(time) / double(GetFrequency());
        }

        inline static int64_t GetFrequency()
        {
            static int64_t frequency = []()
            {
                LARGE_INTEGER performanceFrequency = {};
                QueryPerformanceFrequency(&performanceFrequency);
                return performanceFrequency.QuadPart;
            }();
            return frequency;
        }
    };

    using QPCMeasure = TMeasure<QPCBackend>;
} // namespace dtree

#endif // MEASURE_WINDOWS

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if MEASURE_IS_ON && MEASURE_WINDOWS
    #define QPC_MEASURE(NAME) \
        static dtree::QPCMeasure::Base::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::QPCMeasure::Base::Scope measureScope_##NAME(&measureRecord_##NAME)
    #define QPC_MEASURE_S(TITLE) \
        static dtree::QPCMeasure::Base::MeasureRecord measureRecord(TITLE); \
        dtree::QPCMeasure::Base::Scope measureScope(&measureRecord)
    #define QPC_SAFE_MEASURE(NAME, POLICY) \
        static dtree::QPCMeasure::POLICY::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::QPCMeasure::POLICY::Scope measureScope_##NAME(&measureRecord_##NAME)
    #define QPC_SAFE_MEASURE_S(TITLE, POLICY) \
        static dtree::QPCMeasure::POLICY::MeasureRecord measureRecord(TITLE); \
        dtree::QPCMeasure::POLICY::Scope measureScope(&measureRecord)
#else
    #define QPC_MEASURE(NAME)
    #define QPC_MEASURE_S(TITLE)
    #define QPC_SAFE_MEASURE(NAME, POLICY)
    #define QPC_SAFE_MEASURE_S(TITLE, POLICY)
#endif

