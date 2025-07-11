// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

// Time measurment based on std::crono.

#pragma once

#include "measure_base.h"
#include <chrono>

namespace dtree
{

    struct CppBackend
    {
        inline static const char* GetMeasureTitle() { return "cpp times"; }

        inline static std::chrono::high_resolution_clock::time_point GetTick() noexcept
        {
            return std::chrono::high_resolution_clock::now();
        }

        inline static double TimeDiffToSec(std::chrono::nanoseconds time)
        {
            return std::chrono::duration<double>(time).count();
        }
    };

    using CppMeasure = TMeasure<CppBackend>;

} // namespace dtree

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if MEASURE_IS_ON
    #define CPP_MEASURE(NAME) \
        static dtree::CppMeasure::Base::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::CppMeasure::Base::Scope measureScope_##NAME(&measureRecord_##NAME)
    #define CPP_MEASURE_S(TITLE) \
        static dtree::CppMeasure::Base::MeasureRecord measureRecord(TITLE); \
        dtree::CppMeasure::Base::Scope measureScope(&measureRecord)
    #define CPP_SAFE_MEASURE(NAME, POLICY) \
        static dtree::CppMeasure::POLICY::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::CppMeasure::POLICY::Scope measureScope_##NAME(&measureRecord_##NAME)
    #define CPP_SAFE_MEASURE_S(TITLE, POLICY) \
        static dtree::CppMeasure::POLICY::MeasureRecord measureRecord(TITLE); \
        dtree::CppMeasure::POLICY::Scope measureScope(&measureRecord)
#else
    #define CPP_MEASURE(NAME)
    #define CPP_MEASURE_S(TITLE)
    #define CPP_SAFE_MEASURE(NAME, POLICY)
    #define CPP_SAFE_MEASURE_S(TITLE, POLICY)
#endif
