// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

#pragma once

#include "measure/measure_base.h"
#include "measure/cpp_measure.h"
#include "measure/rdtsc_measure.h"
#include "measure/qpc_measure.h"

#ifndef DEFAULT_MEASURE_TYPE
    #define DEFAULT_MEASURE_TYPE CppMeasure
    //#define DEFAULT_MEASURE_TYPE RdtscMeasure
    //#define DEFAULT_MEASURE_TYPE QPCMeasure
#endif

namespace dtree
{
    using Measure = DEFAULT_MEASURE_TYPE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if MEASURE_IS_ON

    /// @brief basic measure macro, must be placed at the beginning of a scope
    /// @param NAME is a c++ identifier
    #define MEASURE(NAME) \
        static dtree::Measure::Base::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::Measure::Base::Scope measureScope_##NAME(&measureRecord_##NAME)

    /// @brief basic measure macro, must be placed at the beginning of a scope
    /// @param TITTLE is a string literal
    #define MEASURE_S(TITLE) \
        static dtree::Measure::Base::MeasureRecord measureRecord(TITLE); \
        dtree::Measure::Base::Scope measureScope(&measureRecord)

    /// @brief safe measure macro, must be placed at the beginning of a scope
    /// @param NAME is a c++ identifier
    /// @param POLICY is Base, RSafe, TSafe or TRSafe
    ///                RSafe: recursive safe measurement
    ///                TSafe: thread safe measurement
    ///                TRSafe: thread and recursion safe measurement
    #define SAFE_MEASURE(NAME, POLICY) \
        static dtree::Measure::POLICY::MeasureRecord measureRecord_##NAME(""#NAME); \
        dtree::Measure::POLICY::Scope measureScope_##NAME(&measureRecord_##NAME)

    /// @brief safe measure macro, must be placed at the beginning of a scope
    /// @param TITLE is a string literal
    /// @param POLICY is Base, RSafe, TSafe or TRSafe
    ///                RSafe: recursive safe measurement
    ///                TSafe: thread safe measurement
    ///                TRSafe: thread and recursion safe measurement
    #define SAFE_MEASURE_S(TITLE, POLICY) \
        static dtree::Measure::POLICY::MeasureRecord measureRecord(TITLE); \
        dtree::Measure::POLICY::Scope measureScope(&measureRecord)

    /// @brief safe measure macro, must be placed at the beginning of a scope
    /// @param TITLE is a string (not just a string literal, can be a dynamically generated string)
    /// @param POLICY is Base, RSafe, TSafe or TRSafe
    ///                RSafe: recursive safe measurement
    ///                TSafe: thread safe measurement
    ///                TRSafe: thread and recursion safe measurement
    #define SAFE_MEASURE_DYNAMIC_S(TITLE, POLICY) \
        dtree::Measure::POLICY::Scope measureScope(dtree::Measure::TSafe::GetDynamicRecord(TITLE));

    /// @brief print measure report, typically called at the end of program
    namespace dtree {
        inline void PrintMeasure()
            { dtree::Measure::Database::PrintReport(); }
    }
#else
    #define MEASURE(NAME)
    #define MEASURE_S(TITLE)
    #define SAFE_MEASURE(NAME, POLICY)
    #define SAFE_MEASURE_S(TITLE, POLICY)
    #define SAFE_MEASURE_DYNAMIC_S(TITLE, POLICY)
    namespace dtree { inline void PrintMeasure() {} }
#endif
