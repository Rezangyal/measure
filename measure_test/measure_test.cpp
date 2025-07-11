// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

#include "measure_test.h"
#include "measure/measure_base.h"
#include "measure/cpp_measure.h"
#include "measure/rdtsc_measure.h"
#include "measure/measure.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#if MEASURE_WINDOWS
    #include "measure/qpc_measure.h"
#endif

constexpr static uint64_t defaultLoopNum = 1llu << 26;

void CppTest()
{
    CPP_MEASURE(CppMeasureBasicTests);

    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum;

    sum = 0;
    {
        CPP_MEASURE(CppMeasureTest_1);
        for (uint64_t i = 0; i < loopNum; ++i)
        {
            CPP_MEASURE(CppMeasureTest_Core);
            sum += i;
        }
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);

    {
        CPP_MEASURE(CppMeasureDynamicTest_1);
        for (int i = 0; i < 5; ++i)
        {
            // this could be very slow in a time critical code!
            const std::string dinamicGeneratedTitle = std::string("dyn_measure_") + std::to_string(i);
            const auto record = dtree::Measure::TSafe::GetDynamicRecord(dinamicGeneratedTitle.c_str());
            dtree::Measure::TSafe::Scope scope(record);

            sum = 0;
            for (uint64_t j = 0; j < loopNum; ++j)
                sum += j;
        }
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}

#if RDTSC_MEASURE_IS_SUPPORTED
void RDTSCTest()
{
    RDTSC_MEASURE(RDTSCBasicTests);

    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum;

    sum = 0;
    {
        RDTSC_MEASURE(RDTSCMeasureTest_1);
        for (uint64_t i = 0; i < loopNum; ++i)
        {
            RDTSC_MEASURE(RDTSCMeasureTest_Core);
            sum += i;
        }
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}
#endif

#if MEASURE_WINDOWS
void QPCTest()
{
    QPC_MEASURE(QPCTest);

    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum;
    sum = 0;
    {
        QPC_MEASURE(QPCMeasureTest_1);
        for (uint64_t i = 0; i < loopNum; ++i)
        {
            QPC_MEASURE(QPCMeasureTest_Core);
            sum += i;
        }
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}
#endif

template<typename TestedMeasure>
void PerformanceTestTemplate(const char *title)
{
    using namespace std;

    constexpr uint64_t loopNum = defaultLoopNum;
    constexpr double freqDetectTime = 1.0;
    const uint64_t freq = dtree::MeasureUtils::GetProcessorFrequency(freqDetectTime);
    const auto targetRecord = dtree::CppMeasure::Base::GetDynamicRecord(title);
    std::string referenceTitle = title + std::string("_reference");
    const auto referenceRecord = dtree::CppMeasure::Base::GetDynamicRecord(referenceTitle.c_str());
    volatile uint64_t sum;

#if MEASURE_WINDOWS
    dtree::MeasureUtils::SetThreadAffinity(0);
#endif

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // warm up
    sum = 0;
    {
        for (uint64_t i = 0; i < loopNum; i++)
        {
            static typename TestedMeasure::MeasureRecord record("warm up");
            typename TestedMeasure::Scope subScope(&record);
            sum += i;
        }
    }

    // base time
    sum = 0;
    {
        dtree::CppMeasure::Base::Scope scope(referenceRecord);
        for (uint64_t i = 0; i < loopNum; i++)
        {
            sum += i;
        }
    }

    // test
    sum = 0;
    {
        dtree::CppMeasure::Base::Scope scope(targetRecord);
        for (uint64_t i = 0; i < loopNum; i++)
        {
            static typename TestedMeasure::MeasureRecord record("SubMeasure_3");
            typename TestedMeasure::Scope subScope(&record);
            sum += i;
        }
    }

    const double baseTime = referenceRecord->GetTotalSec();
    const double withMeasureTime = targetRecord->GetTotalSec();
    const double measureTime = withMeasureTime - baseTime;

    cout
        << setw(22) << title << ": "
        << "~ " << fixed << setw(6) << setprecision(3) << measureTime * 1000000000 / loopNum << " ns/call, "
        << "~ " << fixed << setw(5) << setprecision(1) << measureTime * freq / loopNum << " clock/call, "
        << "~ " << fixed << 1.0 / (measureTime / loopNum) << " call/sec"
        << endl;
}

void PerformanceTest()
{
    std::cout << "time consumptions: \n";

    // the dummy measure always returns 0 as run time
    using DummyMeasure = dtree::TMeasure<dtree::MeasureBackend>;
    PerformanceTestTemplate<DummyMeasure::Base>("DummyMeasure::Base");
    PerformanceTestTemplate<DummyMeasure::TSafe>("DummyMeasure::TSafe");
    PerformanceTestTemplate<DummyMeasure::RSafe>("DummyMeasure::RSafe");
    PerformanceTestTemplate<DummyMeasure::TRSafe>("DummyMeasure::TRSafe");

    PerformanceTestTemplate<dtree::CppMeasure::Base>("CppMeasure::Base");
    PerformanceTestTemplate<dtree::CppMeasure::TSafe>("CppMeasure::TSafe");
    PerformanceTestTemplate<dtree::CppMeasure::RSafe>("CppMeasure::RSafe");
    PerformanceTestTemplate<dtree::CppMeasure::TRSafe>("CppMeasure::TRSafe");

#if MEASURE_WINDOWS
    PerformanceTestTemplate<dtree::QPCMeasure::Base>("QPCMeasure::Base");
    PerformanceTestTemplate<dtree::QPCMeasure::TSafe>("QPCMeasure::TSafe");
    PerformanceTestTemplate<dtree::QPCMeasure::RSafe>("QPCMeasure::RSafe");
    PerformanceTestTemplate<dtree::QPCMeasure::TRSafe>("QPCMeasure::TRSafe");
#endif

#if RDTSC_MEASURE_IS_SUPPORTED
    PerformanceTestTemplate<dtree::RdtscMeasure::Base>("RdtscMeasure::Base");
    PerformanceTestTemplate<dtree::RdtscMeasure::TSafe>("RdtscMeasure::TSafe");
    PerformanceTestTemplate<dtree::RdtscMeasure::RSafe>("RdtscMeasure::RSafe");
    PerformanceTestTemplate<dtree::RdtscMeasure::TRSafe>("RdtscMeasure::TRSafe");
#endif
}

int main()
{
    using namespace std;

    cout << "GetProcessorFrequency ...\n";
    uint64_t freq = dtree::MeasureUtils::GetProcessorFrequency(2);
    cout << "processor frquency= " << fixed << setprecision(2) << double(freq) / double(1000000) << " MHz\n";

#if MEASURE_WINDOWS
    uint64_t qpcFreq = dtree::QPCBackend::GetFrequency();
    cout << "QPC frequency=" << fixed << setprecision(2) << double(qpcFreq) / double(1000000) << " MHz\n";
#endif

    void RunSamples();
    RunSamples();

    cout << "CppTest\n";
    CppTest();

#if RDTSC_MEASURE_IS_SUPPORTED
    cout << "RDTSCTest\n";
    RDTSCTest();
#endif

#if MEASURE_WINDOWS
    cout << "QPCTest\n";
    QPCTest();
#endif

    cout << "PerformanceTest\n";
    PerformanceTest();

    cout << "reports:\n";
    dtree::CppMeasure::Database::PrintReport();

#if RDTSC_MEASURE_IS_SUPPORTED
    dtree::RdtscMeasure::Database::PrintReport();
#endif

#if MEASURE_WINDOWS
    dtree::QPCMeasure::Database::PrintReport();
#endif

    cout << "test Ok.\n";
    return 0;
}

/*
time consumptions, core i5 @ 3.2GHz, VS2022:
    DummyMeasure::Base: ~ -0.003 ns/call, ~  -0.0 clock/call, ~ ??? call/sec
   DummyMeasure::TSafe: ~ 16.530 ns/call, ~  52.9 clock/call, ~ 60496941.5 call/sec
   DummyMeasure::RSafe: ~  1.179 ns/call, ~   3.8 clock/call, ~ 848047272.1 call/sec
  DummyMeasure::TRSafe: ~ 62.771 ns/call, ~ 200.8 clock/call, ~ 15930940.5 call/sec
      CppMeasure::Base: ~ 31.490 ns/call, ~ 100.7 clock/call, ~ 31756495.2 call/sec
     CppMeasure::TSafe: ~ 51.182 ns/call, ~ 163.7 clock/call, ~ 19538144.3 call/sec
     CppMeasure::RSafe: ~ 30.757 ns/call, ~  98.4 clock/call, ~ 32512774.7 call/sec
    CppMeasure::TRSafe: ~ 97.524 ns/call, ~ 311.9 clock/call, ~ 10253889.3 call/sec
      QPCMeasure::Base: ~ 24.475 ns/call, ~  78.3 clock/call, ~ 40857818.0 call/sec
     QPCMeasure::TSafe: ~ 44.064 ns/call, ~ 140.9 clock/call, ~ 22694032.5 call/sec
     QPCMeasure::RSafe: ~ 25.006 ns/call, ~  80.0 clock/call, ~ 39990870.7 call/sec
    QPCMeasure::TRSafe: ~ 90.124 ns/call, ~ 288.2 clock/call, ~ 11095829.0 call/sec
    RdtscMeasure::Base: ~ 10.533 ns/call, ~  33.7 clock/call, ~ 94941924.7 call/sec
   RdtscMeasure::TSafe: ~ 30.258 ns/call, ~  96.8 clock/call, ~ 33049643.3 call/sec
   RdtscMeasure::RSafe: ~ 10.546 ns/call, ~  33.7 clock/call, ~ 94821272.1 call/sec
  RdtscMeasure::TRSafe: ~ 76.112 ns/call, ~ 243.4 clock/call, ~ 13138504.5 call/sec
*/
