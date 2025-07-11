// Copyright 2025 Gurzó Péter.
// Licensed under the MIT License. See LICENSE.txt.

#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <mutex>
#include <map>
#include <memory>
#include <string>
#include <cstring>
#include <map>

#include "measure/measure_utils.h"

namespace dtree
{
    struct MeasureBackend;
    template<typename MeasureBackend> struct TMeasureRecord;
    template<typename MeasureRecord> struct MeasureDatabase;
    template<typename MeasureRecord> struct DynamicMeasureDatabase;
    template<typename MeasureRecord> struct MeassureRecordTSafe;
    template<typename MeasureRecord> struct MeassureRecordRSafe;
    template<typename MeasureRecord> struct MeassureRecordTRSafe;
    template<typename MeasureRecord> struct TMeasureScope;
    template<typename MeasureRecord> struct MeasureScopeRSafe;
    template<typename MeasureBackend> struct TMeasure;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // MeasureBackend interface / concept
    // For performance reasons, we don't use any virtual functions in the measurement library,
    // so this is not a real base class, just a concept
    // but every MeasureRecord based on a MeasureBackend implementation
    // see: CppBackend, RdtscBackend, QPCBackend
    struct MeasureBackend
    {
        inline static const char* GetMeasureTitle()
        {
            return "VeryPreciseMeasure";
        }

        inline static int64_t GetTick() noexcept
        {
            // return VeryPreciseClock();
            return 0;
        }

        inline static double TimeDiffToSec(int64_t time)
        {
            int64_t frequency = 3200000000ull;
            return double(time) / double(frequency);
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief a MeasureRecord store the total time and number of calls
    template <typename InMeasureBackend>
    struct TMeasureRecord
    {
    public:
        using MeasureBackend = InMeasureBackend;
        using TimePoint = decltype(MeasureBackend::GetTick());
        using TimeDiff = decltype(TimePoint() - TimePoint());
        using Database = MeasureDatabase<TMeasureRecord<MeasureBackend>>;

        inline TMeasureRecord(const char* name, const bool autoRegister = true)
            : name(name)
            , totalTime(0)
            , numCall(0)
        {
            if (autoRegister)
                Database::AddRecord(this);
        }

        inline TimePoint Now() const
        {
            return MeasureBackend::GetTick();
        }

        inline void StopMeasure(TimePoint start)
        {
            totalTime += MeasureBackend::GetTick() - start;
            numCall++;
        }

        inline double GetTotalSec() const
        {
            return MeasureBackend::TimeDiffToSec(totalTime);
        }

        std::string name;
        TimeDiff totalTime;
        uint64_t numCall;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /**
     * @brief A database of MeasureRecords
     *
     * For each MeasureBackend type there is a MeasureDatabase instance,
     * but because the most applications only use one MeasureBackend type,
     * there is only one MeasureDatabase instance.
     * All the MeasureRecords are registered here.
     * This class is responsible for adding and finding MeasureRecords,
     * and for printing the report at the end of the program.
     */
    template<typename InMeasureRecord>
    struct MeasureDatabase
    {
    public:
        using MeasureRecord = InMeasureRecord;

        // add MeasureRecord or MeasureRecordSafe
        static void AddRecord(MeasureRecord* record);

        // find MeasureRecord by name, slow
        static MeasureRecord* FindMeasureRecord(const char* name);

        // print report at the end of program
        static void PrintReport(double frequencyMeasureTimeSeconds = 1.0);

        // set all numCall and totalClock to 0. this can be useful if you want to print more than one report
        static void ResetAll();

    private:

        static MeasureDatabase<MeasureRecord>* Instance()
        {
            static MeasureDatabase<MeasureRecord> instance;
            return &instance;
        }

        std::vector<MeasureRecord*> records;
        std::mutex mutex;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /**
     *  @brief a container for dynamic MeasureRecords
     *
     * For each (MeasureBackend type, Policy) pair there is a DynamicMeasureDatabase instance,
     * so in a typical application there is 4 DynamicMeasureDatabase instance.
     * All measure records are registered into the MeasureDatabase,
     * but for dynamic measure records we need an additional container,
     * because the static records are stored in static variables,
     * but the dynamic records are created at runtime,
     * so they need to be stored in a container.
     */
    template<typename InMeasureRecord>
    struct DynamicMeasureDatabase
    {
    public:

        using MeasureRecord = InMeasureRecord;

        // measures with dynamic title
        static MeasureRecord* GetOrAddDynamicRecord(const char* name);

    private:
        static DynamicMeasureDatabase<MeasureRecord>* Instance()
        {
            static DynamicMeasureDatabase<MeasureRecord> instance;
            return &instance;
        }

        std::map<std::string, std::unique_ptr<MeasureRecord>> dynamicRecords;
        std::mutex mutex;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief this class is wrapper around a MeassureRecord, it adds thread safety to it
    template <typename MeasureRecord>
    struct MeassureRecordTSafe : public MeasureRecord
    {
    public:
        using MeasureRecord::MeasureRecord;

        inline void StopMeasure(typename MeasureRecord::TimePoint start)
        {
            const std::lock_guard<std::mutex> lock(mutex);
            MeasureRecord::StopMeasure(start);
        }

        std::mutex mutex;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief this class is wrapper around a MeassureRecord, it adds recursion safety to it
    template <typename MeasureRecord>
    struct MeasureRecordRSafe : public MeasureRecord
    {
        using MeasureRecord::MeasureRecord;

        bool IncrementDepth()
        {
            return ++depth == 1;
        }

        bool DecrementDepth()
        {
            return --depth == 0;
        }

        int32_t GetDepth() const
        {
            return depth;
        }

    protected:
        int32_t depth = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief this class is wrapper around a MeassureRecord, it adds thread and recursion safety to it
    template <typename MeasureRecord>
    struct MeasureRecordTRSafe : public MeassureRecordTSafe<MeasureRecord>
    {
        using MeassureRecordTSafe<MeasureRecord>::MeassureRecordTSafe;

        bool IncrementDepth()
        {
            const std::lock_guard<std::mutex> lock(this->mutex);
            return ++Depth() == 1;
        }

        bool DecrementDepth()
        {
            const std::lock_guard<std::mutex> lock(this->mutex);
            return --Depth() == 0;
        }

        int32_t GetDepth() const
        {
            const std::lock_guard<std::mutex> lock(this->mutex);
            return Depth();
        }

    protected:
        int32_t& Depth()
        {
            static thread_local std::map<std::pair<std::thread::id, void*>, int32_t> depths;
            return depths[std::make_pair(std::this_thread::get_id(), this)];
        }
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief a simple measure scope
    /// it starts and stops a MeasureRecord in the constructor and destructor
    template<typename MeasureRecord>
    struct TMeasureScope
    {
        inline TMeasureScope(MeasureRecord* record_)
#if MEASURE_IS_ON
            : record{ record_ }
            , start{ record->Now() }
        {
        }

        inline ~TMeasureScope()
        {
            record->StopMeasure(start);
        }

        MeasureRecord* record;
        typename MeasureRecord::TimePoint start;
#else
        {}
#endif
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief a measure scope for recursive measurement
    /// it starts and stops a MeasureRecord in the constructor and destructor
    template<typename MeasureRecord>
    struct MeasureScopeRSafe
    {
        inline MeasureScopeRSafe(MeasureRecord* record_)
#if MEASURE_IS_ON
            : record{ record_ }
        {
            if (record->IncrementDepth())
                start = record->Now();
        }

        inline ~MeasureScopeRSafe()
        {
            if (record->DecrementDepth())
                record->StopMeasure(start);
        }

        MeasureRecord* record;
        typename MeasureRecord::TimePoint start;
#else
        {}
#endif
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief the main measure class
    template <typename InMeasureBackend>
    struct TMeasure
    {
        using MeasureBackend = InMeasureBackend;
        using MeasureRecordBase = TMeasureRecord<MeasureBackend>;
        using Database = MeasureDatabase<MeasureRecordBase>;

        // basic measure, not threadsafe, not handle recursive function calls, but fast
        struct Base
        {
            using MeasureRecord = MeasureRecordBase;
            using Scope = TMeasureScope<MeasureRecord>;
            using DynamicDatabase = DynamicMeasureDatabase<MeasureRecord>;
            static MeasureRecord* GetDynamicRecord(const char* name)
                { return DynamicDatabase::GetOrAddDynamicRecord(name); }
        };

        // threadsafe measure, not handle recursive function calls, slower than base
        struct TSafe
        {
            using MeasureRecord = MeassureRecordTSafe<MeasureRecordBase>;
            using Scope = TMeasureScope<MeasureRecord>;
            using DynamicDatabase = DynamicMeasureDatabase<MeasureRecord>;
            static MeasureRecord* GetDynamicRecord(const char* name)
                { return DynamicDatabase::GetOrAddDynamicRecord(name); }
        };

        // not threadsafe, handle recursive function calls
        struct RSafe
        {
            using MeasureRecord = MeasureRecordRSafe<MeasureRecordBase>;
            using Scope = MeasureScopeRSafe<MeasureRecord>;
            using DynamicDatabase = DynamicMeasureDatabase<MeasureRecord>;
            static MeasureRecord* GetDynamicRecord(const char* name)
                { return DynamicDatabase::GetOrAddDynamicRecord(name); }
        };

        // threadsafe and handle recursive function calls, slowest
        struct TRSafe
        {
            using MeasureRecord = MeasureRecordTRSafe<MeasureRecordBase>;
            using Scope = MeasureScopeRSafe<MeasureRecord>;
            using DynamicDatabase = DynamicMeasureDatabase<MeasureRecord>;
            static MeasureRecord* GetDynamicRecord(const char* name)
                { return DynamicDatabase::GetOrAddDynamicRecord(name); }
        };
    };

} // namespace dtree

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename MeasureRecord>
void dtree::MeasureDatabase<MeasureRecord>::AddRecord(MeasureRecord* record)
{
#if MEASURE_IS_ON
    const std::lock_guard<std::mutex> lock(Instance()->mutex);
    Instance()->records.push_back(record);
#endif
}

template<typename MeasureRecord>
auto dtree::MeasureDatabase<MeasureRecord>::FindMeasureRecord(const char* name) -> MeasureRecord*
{
#if MEASURE_IS_ON
    for (MeasureRecord* mr : Instance()->records)
        if (mr->name == name)
            return mr;
#endif
    return nullptr;
}

template<typename MeasureRecord>
void dtree::MeasureDatabase<MeasureRecord>::PrintReport(double frequencyMeasureTimeSeconds)
{
#if MEASURE_IS_ON
    using namespace std;

    if (!Instance()->records.empty())
    {
        //const uint64_t freq = MeasureUtils::GetProcessorFrequency(frequencyMeasureTimeSeconds);
        //cout << " freq: " << setw(8) << setprecision(6) << double(freq) / 1000000.0 << " MHz\n";
        constexpr size_t width = 86;
        const char* title = MeasureRecord::MeasureBackend::GetMeasureTitle();
        const size_t titleLen = strlen(title);

        const std::string pad((width-titleLen)/2 - 1, '-');
        cout << pad << " " << title << " " << pad << (titleLen%2?"-":"") << "\n";

        cout<< setw(40) << "Name"
            << setw(12) << "Calls"
            << setw(17) << "Total (ns)"
            << setw(17) << "Average (ns)"
            << "\n";
        cout << std::string(width, '-') << std::endl;
        for (MeasureRecord* measureRecord : Instance()->records)
        {
            const double totalSec = measureRecord->GetTotalSec();
            if (totalSec < 0 || measureRecord->numCall == 0)
                cout<< setw(40) << measureRecord->name
                    << setw(12) << measureRecord->numCall
                    << "\n";
            else
                cout<< setw(40) << measureRecord->name
                    << setw(12) << measureRecord->numCall
                    << setw(17) << MeasureUtils::TimeToStrNs(totalSec)
                    << setw(17) << MeasureUtils::TimeToStrNs(totalSec / double(measureRecord->numCall))
                    << "\n";
        }
        cout << std::string(width, '-') << std::endl;
    }
#endif
}

template<typename MeasureRecord>
void dtree::MeasureDatabase<MeasureRecord>::ResetAll()
{
#if MEASURE_IS_ON
    for (MeasureRecord* mr : Instance()->records)
    {
        mr->numCall = 0;
        mr->totalTime = {};
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename MeasureRecord>
auto dtree::DynamicMeasureDatabase<MeasureRecord>::GetOrAddDynamicRecord(const char* name) -> MeasureRecord*
{
    const std::lock_guard<std::mutex> lock(Instance()->mutex);
    auto& dynamicRecords = Instance()->dynamicRecords;

    auto recordIter = dynamicRecords.find(name);
    if (recordIter != dynamicRecords.end())
        return recordIter->second.get();

    typename std::remove_reference<decltype(dynamicRecords)>::type::iterator iter;
    bool newItemCreated;
    std::tie(iter, newItemCreated) = dynamicRecords.emplace(name, nullptr);

    std::unique_ptr<MeasureRecord>& uniqueRecord{ iter->second };
    //if (!uniqueRecord)
        uniqueRecord = std::unique_ptr<MeasureRecord>(new MeasureRecord(name));
    return uniqueRecord.get();
}