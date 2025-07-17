# C++ measure

Header only measure library based on std::chrono and RDTSC instruction.
You can choose thread or recursion safe implementation.
Human readable or csv reporting is available.

Optionally QureryPerformanceCounter support (MSVC only).
Additional backends and reporters can be defined.

Easy to use, high precision, can work in the live environment.

Requirements: C++11, CMake for unittest

MIT License, see LICENSE.txt

[![test clang](https://github.com/Rezangyal/measure/actions/workflows/cmake-clang.yml/badge.svg)](https://github.com/Rezangyal/measure/actions/workflows/cmake-clang.yml)
[![test gcc](https://github.com/Rezangyal/measure/actions/workflows/cmake-gcc.yml/badge.svg)](https://github.com/Rezangyal/measure/actions/workflows/cmake-gcc.yml)
[![test msvc](https://github.com/Rezangyal/measure/actions/workflows/cmake-msvc.yml/badge.svg)](https://github.com/Rezangyal/measure/actions/workflows/cmake-msvc.yml)
[![test macos-latest](https://github.com/Rezangyal/measure/actions/workflows/macos_test.yml/badge.svg)](https://github.com/Rezangyal/measure/actions/workflows/macos_test.yml)

## Usage

### Basic usage
``` c++
#include "measure/measure.h"
uint64_t MyFunction()
{
    MEASURE(MyFunction);
    // or MEASURE_S("title+with-special characters");

    volatile uint64_t sum = 0;
    for (uint64_t i = 0; i < 123456; ++i)
    {
        MEASURE(Loop);
        // or RDTSC_MEASURE(Loop); if the precision is important
        sum += i;
    }
    return sum;
}

int main()
{
    MyFunction();
    ...
    dtree::PrintMeasure();
    return 0;
}
```
output:
```
------------------------------------- cpp times --------------------------------------
                                    Name       Calls       Total (ns)     Average (ns)
--------------------------------------------------------------------------------------
                              MyFunction           1        4'294'900        4'294'900
                                    Loop      123456        2'137'100               17
```


### Recursive function
If your function is recursive and/or called from a multithread environment you must use an other form
``` c++
void MyFunction()
{
    // the second parameter:
    //   RSafe: recursion safe,
    //   TSafe: thread safe,
    //   TRSafe: thread and recursion safe implementation
    SAFE_MEASURE_S("Measure with thread and recursion safe implementation", TRSafe);
    ....
}
```
or the same in expanded form
``` c++
void MyFunction()
{
    // the "static" is important!
    static dtree::Measure::TRSafe::MeasureRecord record("Measure with scope");
    {
        dtree::Measure::TRSafe::Scope scope(&record);
        ....
    }
}
```

### Dynamic title
If the measurement title is dynamically generated then the expanded form is always recommended
``` c++
void MyFunction(int i)
{
    // GetDynamicRecord is very slow, don't call it from time critical code!
    const std::string dinamicGeneratedTitle = std::string("MyFunction_") + std::to_string(i);
    const auto record = dtree::Measure::TSafe::GetDynamicRecord(dinamicGeneratedTitle.c_str());
    for (int j=0; j<i; ++j)
    {
        dtree::Measure::TSafe::Scope scope(record);
        ...
    }
}
```

### Change the default measure technology
Currently three measure techniques are supported: `CppMeasure`, `QPCMeasure` and `RdtscMeasure`.
You can choose one of them by defining `DEFAULT_MEASURE_TYPE` before include `measure.h`,
the default is the `CppMeasure`, which based on `std::chrono::high_resolution_clock`

### Cpp measure
This is based on `std::chrono::high_resolution_clock` and it is available in any c++11 compiler.
As you can see in `measure.h`, this is the default measure technology,
but you can use the CppMeasure functions explicitly if you want:
``` c++
void MyFunction()
{
    CPP_MEASURE(MyFunctionTime);
    ....
}
```
or
``` c++
void MyFunction()
{
    static dtree::CppMeasure::TRSafe::MeasureRecord record("Measure with scope");
    {
        dtree::CppMeasure::TRSafe::Scope scope(&record);
        ....
    }
}
```

### RDTSC measure
This is the lowest level measurement service,
based on the `Read Time-Stamp Counter` instruction of the processor,
with a typical frequency of 2-4 Ghz.
This is the fastest option, but only available in x86/x64 platform.
``` c++
void MyFunction()
{
    RDTSC_MEASURE(MyFunctionTime);
    ....
}
```
or
``` c++
void MyFunction()
{
    static dtree::RdtscMeasure::TRSafe::MeasureRecord record("Measure with scope");
    {
        dtree::RdtscMeasure::TRSafe::Scope scope(&record);
        ....
    }
}
```

### QPC measure
If you are using Visual Studio, you can use `QPCMeasure`,
which is based on `QueryPerformanceCounter` and `QueryPerformanceFrequency`.
The typical frequency of the QPC is 10 Mhz. Every function has a QPC equivalent:
``` c++
void MyFunction()
{
    QPC_MEASURE(MyFunctionTime);
    // or QPC_MEASURE_S("title+with-special characters");
    ....
}
```
or
``` c++
void MyFunction()
{
    static dtree::QPCMeasure::TRSafe::MeasureRecord record("Measure with scope");
    {
        dtree::QPCMeasure::TRSafe::Scope scope(&record);
        ....
    }
}
```

## Notes

- Measurement is never perfectly accurate
- Always close all applications before measuring
- Always measure the optimised code
- Before measuring, turn off the computer's power saving mode
- If your processor has "Efficient-core", it may be slower than the normal core,
  and this will confuse the measurement.
- Some processors may vary the frequency of the processor, this will confuse the measurement.
- In a multithreaded environment, not using thread-safe functions
  is acceptable in most cases, because collisions are very rare. (this is the smallest problem...)
- If your function is recursive, you __MUST__ use recursion safe (and thread and recursion safe)
  functions, the regular functions are __NOT__ working!
- Usually, it is not the specific numerical values that should be considered,
  but their ratio and which code fragments are the most costly.






