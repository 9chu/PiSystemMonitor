/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#pragma once
#include <map>
#include <optional>
#include <variant>
#include <vector>
#include <concurrentqueue/concurrentqueue.h>

class MetricsSampleThread
{
public:
    struct QuitCommand
    {
    };

    struct ChangeUrlCommand
    {
        std::string Url;
        double RefreshIntervalMs = 1000;
    };

    using Command = std::variant<QuitCommand, ChangeUrlCommand>;

    struct MetricsResult
    {
        uint64_t Tick = 0;
        double BootTimeSeconds = 0;
        double Load1 = 0;
        double Load5 = 0;
        double Load15 = 0;
        uint64_t MemoryAvailableBytes = 0;
        uint64_t MemoryTotalBytes = 0;
        uint64_t MemoryFreeBytes = 0;
        std::vector<double> CpuUsage;
        std::map<std::string, double> DiskReadBytesPerSecond;
        std::map<std::string, double> DiskWrittenBytesPerSecond;
        std::map<std::string, double> NetworkReceiveBytesPerSecond;
        std::map<std::string, double> NetworkTransmitBytesPerSecond;
    };

    using Result = std::variant<std::monostate, MetricsResult>;

    struct RawCpuMetrics
    {
        double Idle = 0;
        double IoWait = 0;
        double Irq = 0;
        double Nice = 0;
        double SoftIrq = 0;
        double Steal = 0;
        double System = 0;
        double User = 0;

        double TotalSeconds() const noexcept
        {
            return Idle + IoWait + Irq + Nice + SoftIrq + Steal + System + User;
        }
    };

    struct RawMetrics
    {
        uint64_t Tick = 0;
        uint64_t BootTimestamp = 0;
        double Load1 = 0;
        double Load5 = 0;
        double Load15 = 0;
        uint64_t MemoryAvailableBytes = 0;
        uint64_t MemoryTotalBytes = 0;
        uint64_t MemoryFreeBytes = 0;
        std::map<int, RawCpuMetrics> CpuSecondsTotal;
        std::map<std::string, double> DiskIoTimeSecondsTotal;
        std::map<std::string, double> DiskReadTimeSecondsTotal;
        std::map<std::string, double> DiskWriteTimeSecondsTotal;
        std::map<std::string, double> DiskReadBytesTotal;
        std::map<std::string, double> DiskWrittenBytesTotal;
        std::map<std::string, double> NetworkReceiveBytesTotal;
        std::map<std::string, double> NetworkTransmitBytesTotal;
    };

public:
    void Run();
    void EnqueueCommand(Command&& cmd);
    bool TryDequeueResult(Result& result);

private:
    void RefreshMetrics();

private:
    moodycamel::ConcurrentQueue<Command> m_stCommandQueue;
    moodycamel::ConcurrentQueue<Result> m_stResultQueue;
    bool m_bStopped = false;
    double m_dRefreshTimerMs = 0.;
    std::optional<RawMetrics> m_stLastRawMetrics;

    std::string m_stUrl;
    double m_dRefreshIntervalMs = 1000.;
};
