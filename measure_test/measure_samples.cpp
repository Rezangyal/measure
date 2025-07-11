#include "measure_test.h"
#include "measure/measure.h"
#include <cstdlib>

constexpr static uint64_t defaultLoopNum = 1llu << 26;

void Solution_recomended()
{
    // Solution with macro:
    // - not threadsafe
    // - not recursion safe
    // + exception safe
    // + fast
    // + easy to use
    // in most cases this is the best solution
    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum = 0;
    {
        MEASURE(Measure_with_macro_scope);
        // or if you want special character in the title:
        // MEASURE_S("Measure+with macro*scope:")

        // very long calculation ...
        for (uint64_t i = 0; i < loopNum; ++i)
            sum += i;
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}

void Solution1()
{
    // Solution 1, naive solution:
    // - not threadsafe
    // - not recursion safe
    // - not exception safe
    // + fast

    constexpr uint64_t loopNum = defaultLoopNum / 10;
    // without volatile gcc and clang calculate the whoole loop as constant expression!
    volatile uint64_t sum = 0;
    // the "static" is important!
    static dtree::Measure::Base::MeasureRecord record("Naive solution");
    {
        auto start = record.Now();
        // very long calculation ...
        for (uint64_t i = 0; i < loopNum; ++i)
            sum += i;
        record.StopMeasure(start);
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}

void Solution2()
{
    // Solution 2 with scope:
    // - not threadsafe
    // - not recursion safe
    // + exception safe
    // + fast
    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum = 0;
    static dtree::Measure::Base::MeasureRecord record("Measure with scope");
    {
        dtree::Measure::Base::Scope scope(&record);
        // very long calculation ...
        for (uint64_t i = 0; i < loopNum; ++i)
            sum += i;
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}

void Solution3()
{
    // Solution 3, buletproof version:
    // + threadsafe
    // + recursion safe
    // + exception safe
    // - more overhead
    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum = 0;
    {
        // RSafe: recursion safe, TSafe: thread safe, TRSafe: thread and recursion safe implementations
        static dtree::Measure::TRSafe::MeasureRecord record("Measure with thread and recursion safe scope");
        {
            dtree::Measure::TRSafe::Scope scope(&record);
            // very long calculation ...
            for (uint64_t i = 0; i < loopNum; ++i)
                sum += i;
        }
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}

void Solution4()
{
    // Solution 3, buletproof version with macro:
    // + threadsafe
    // + recursion safe
    // + exception safe
    // + easy to use
    // - more overhead
    constexpr uint64_t loopNum = defaultLoopNum / 10;
    volatile uint64_t sum = 0;
    {
        // RSafe: recursion safe, TSafe: thread safe, TRSafe: thread and recursion safe implementations
        SAFE_MEASURE_S("Measure with thread and recursion safe macro", TRSafe);

        // very long calculation ...
        for (uint64_t i = 0; i < loopNum; ++i)
            sum += i;
    }
    ENSURE(sum == loopNum * (loopNum - 1) / 2);
}

void Solution5()
{
    // Solution 5, buletproof version with dynamic generated title:
    // + threadsafe
    // + recursion safe
    // + exception safe
    // + dynamically generated title
    // - most overhead
    constexpr uint64_t loopNum = defaultLoopNum / 20;
    volatile uint64_t sum = 0;
    {
        // a dangerous situation: measure with dynamic name
        for (int j = 0; j < 5; ++j)
        {
            // GetOrAddDynamicRecord is very slow, don't call it from time critical code!
            const std::string dynamicGeneratedTitle = std::string("DynamicMeasure_") + std::to_string(j);
            const auto record = dtree::Measure::TRSafe::GetDynamicRecord(dynamicGeneratedTitle.c_str());

            sum = 0;
            {
                // very long calculation ...
                for (uint64_t i = 0; i < loopNum; ++i)
                {
                    dtree::Measure::TRSafe::Scope scope(record); // the GetDynamicRecord() is not called here!
                    sum += i;
                }
            }
            ENSURE(sum == loopNum * (loopNum - 1) / 2);
        }
    }
}

uint64_t RecursiveFunction(int level)
{
    SAFE_MEASURE(RecursiveFunctionMacroR, RSafe); // it's ok

    SAFE_MEASURE(RecursiveFunctionMacroTR, TRSafe); // it's ok

    static dtree::Measure::Base::MeasureRecord rec1("RecursiveFunction_1");
    dtree::Measure::Base::Scope scope1(&rec1); // BAD! not recursion safe!

    static dtree::Measure::RSafe::MeasureRecord rec2("RecursiveFunction_2");
    dtree::Measure::RSafe::Scope scope2(&rec2); // it's ok, the policy is RSafe

    static dtree::Measure::TSafe::MeasureRecord rec3("RecursiveFunction_3");
    dtree::Measure::TSafe::Scope scope3(&rec3); // BAD! not recursion safe!

    static dtree::Measure::TRSafe::MeasureRecord rec4("RecursiveFunction_4");
    dtree::Measure::TRSafe::Scope scope4(&rec4); // it's ok, the policy is TRSafe

    volatile uint64_t sum = 0;
    if (level > 0)
        sum += RecursiveFunction(level - 1);
    else
    {
        for (uint64_t i = 0; i < defaultLoopNum; i++)
            sum += i;
    }
    return sum;
}

void RecursionTest()
{
    if (!MEASURE_IS_ON)
        return;

    volatile uint64_t sum = 0;
    {
        MEASURE(RecursionTest_Main);

        sum = RecursiveFunction(9);
    }
    ENSURE(sum == defaultLoopNum * (defaultLoopNum - 1) / 2);

    const double expectedTotalTime = dtree::Measure::Database::FindMeasureRecord("RecursionTest_Main")->GetTotalSec();
    const double recursiveTimeMacro = dtree::Measure::Database::FindMeasureRecord("RecursiveFunctionMacroR")->GetTotalSec();
    const double recursiveTimeMacroTR = dtree::Measure::Database::FindMeasureRecord("RecursiveFunctionMacroTR")->GetTotalSec();
    const double recursiveTime1 = dtree::Measure::Database::FindMeasureRecord("RecursiveFunction_1")->GetTotalSec();
    const double recursiveTime_R = dtree::Measure::Database::FindMeasureRecord("RecursiveFunction_2")->GetTotalSec();
    const double recursiveTime_T = dtree::Measure::Database::FindMeasureRecord("RecursiveFunction_3")->GetTotalSec();
    const double recursiveTime_TR = dtree::Measure::Database::FindMeasureRecord("RecursiveFunction_4")->GetTotalSec();

    std::cout
        << "Recursion test, sum: " << sum << '\n'
        << "  total time:            " << expectedTotalTime*1000 << " ms. (reference value)\n"
        << "  RSafe Macro time:      " << recursiveTimeMacro*1000 << " ms. (ok)\n"
        << "  TRSafe Macro time:     " << recursiveTimeMacroTR*1000 << " ms. (ok)\n"
        << "  Basic recursive time:  " << recursiveTime1*1000   << " ms. (bad)\n"
        << "  RSafe recursive time:  " << recursiveTime_R*1000  << " ms. (ok)\n"
        << "  TSafe recursive time:  " << recursiveTime_T*1000  << " ms. (bad)\n"
        << "  TRSafe recursive time: " << recursiveTime_TR*1000 << " ms. (ok)\n"
        << std::endl;

    ENSURE(abs(expectedTotalTime - recursiveTimeMacro) < 0.01); // ok
    ENSURE(abs(expectedTotalTime - recursiveTimeMacroTR) < 0.01); // ok
    ENSURE(abs(recursiveTime1 > expectedTotalTime * 10) < 0.01); // bad, recursiveTime1 is 10* bigger than expected
    ENSURE(abs(expectedTotalTime - recursiveTime_R) < 0.01); // ok
    ENSURE(abs(recursiveTime_T > expectedTotalTime * 10) < 0.01); // bad, recursiveTime_T is 10* bigger than expected
    ENSURE(abs(expectedTotalTime - recursiveTime_TR) < 0.01); // ok
}

void RunSamples()
{
    MEASURE(RunSamples);

    std::cout << "Run samples\n";

    Solution_recomended();
    Solution1();
    Solution2();
    Solution3();
    Solution4();
    Solution5();

    std::cout << "RecursionTest\n";
    RecursionTest();

}