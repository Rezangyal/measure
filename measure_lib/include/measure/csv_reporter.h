#include <fstream>
#include <ostream>
#include <iomanip>
#include <string>

namespace dtree
{
    /// @brief print measure report in CSV format
    template<typename Database>
    void CsvReport(std::ostream& os)
    {
        using namespace std;
        std::lock_guard<std::mutex> lock(Database::GetMutex());
        auto records = Database::GetRecords();
        if (!records.empty())
        {
            os << "name,num_calls,total_ns,average_ns\n";
            for (auto measureRecord : records)
            {
                const double totalSec = measureRecord->GetTotalSec();
                if (totalSec < 0 || measureRecord->numCall == 0)
                    os
                        << measureRecord->name << ","
                        << measureRecord->numCall << ","
                        << ",\n";
                else
                    os
                        << measureRecord->name << ","
                        << measureRecord->numCall << ","
                        << fixed << totalSec * 1e9 << ","
                        << fixed << totalSec * 1e9 / double(measureRecord->numCall)
                        << "\n";
            }
        }
    }

    /// @brief print measure report in CSV format to a file
    template<typename Database>
    void CsvReport(std::string filename = "performance_report.csv")
    {
        std::ofstream os(filename);
        CsvReport<Database>(os);
    }
} // namespace dtree
