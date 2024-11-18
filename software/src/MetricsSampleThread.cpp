/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#include <MetricsSampleThread.hpp>

#include <ada.h>
#include <httplib.h>
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include <double-conversion/string-to-double.h>
#include <MetricsParser.hpp>

using namespace std;
using namespace double_conversion;

namespace
{
    double ToDouble(std::string_view input) noexcept
    {
        auto flags = StringToDoubleConverter::ALLOW_LEADING_SPACES | StringToDoubleConverter::ALLOW_TRAILING_SPACES |
            StringToDoubleConverter::ALLOW_SPACES_AFTER_SIGN;
        StringToDoubleConverter converter(flags, 0.0, 0.0, "inf", "nan", 0);

        int processed = 0;
        return converter.StringToDouble(input.data(), static_cast<int>(input.size()), &processed);
    }

    int64_t ToInteger(std::string_view input) noexcept
    {
        return static_cast<int64_t>(ToDouble(input));
    }

    class MetricsParseListener :
        public MetricsParser::IListener
    {
    public:
        MetricsParseListener(MetricsSampleThread::RawMetrics& raw)
            : m_stRawMetrics(raw) {}

    protected: // MetricsParseListener
        void OnMetricsBegin(std::string_view name) override
        {
            m_stCurrentMetricsName = name;
        }

        void OnMetricsLabel(std::string_view name, std::string_view value) override
        {
            if (m_stCurrentMetricsName == "node_cpu_seconds_total")
            {
                if (name == "cpu")
                    m_iCurrentCpuIndex = static_cast<int>(ToInteger(value));
                else if (name == "mode")
                    m_stCurrentCpuMode = value;
            }
            else if (m_stCurrentMetricsName == "node_disk_io_time_seconds_total" ||
                m_stCurrentMetricsName == "node_disk_read_time_seconds_total" ||
                m_stCurrentMetricsName == "node_disk_write_time_seconds_total" ||
                m_stCurrentMetricsName == "node_disk_read_bytes_total" ||
                m_stCurrentMetricsName == "node_disk_written_bytes_total" ||
                m_stCurrentMetricsName == "node_network_receive_bytes_total" ||
                m_stCurrentMetricsName == "node_network_transmit_bytes_total")
            {
                if (name == "device")
                    m_stCurrentDeviceName = value;
            }
        }

        void OnMetricsValue(std::string_view value) override
        {
            if (m_stCurrentMetricsName == "node_boot_time_seconds")
            {
                m_stRawMetrics.BootTimestamp = ToInteger(value);
            }
            else if (m_stCurrentMetricsName == "node_load1")
            {
                m_stRawMetrics.Load1 = ToDouble(value);
            }
            else if (m_stCurrentMetricsName == "node_load5")
            {
                m_stRawMetrics.Load5 = ToDouble(value);
            }
            else if (m_stCurrentMetricsName == "node_load15")
            {
                m_stRawMetrics.Load15 = ToDouble(value);
            }
            else if (m_stCurrentMetricsName == "node_memory_MemAvailable_bytes")
            {
                m_stRawMetrics.MemoryAvailableBytes = ToInteger(value);
            }
            else if (m_stCurrentMetricsName == "node_memory_MemTotal_bytes")
            {
                m_stRawMetrics.MemoryTotalBytes = ToInteger(value);
            }
            else if (m_stCurrentMetricsName == "node_memory_MemFree_bytes")
            {
                m_stRawMetrics.MemoryFreeBytes = ToInteger(value);
            }
            else if (m_stCurrentMetricsName == "node_cpu_seconds_total")
            {
                auto& cpuMetrics = m_stRawMetrics.CpuSecondsTotal[m_iCurrentCpuIndex];
                if (m_stCurrentCpuMode == "idle")
                    cpuMetrics.Idle = ToDouble(value);
                else if (m_stCurrentCpuMode == "iowait")
                    cpuMetrics.IoWait = ToDouble(value);
                else if (m_stCurrentCpuMode == "irq")
                    cpuMetrics.Irq = ToDouble(value);
                else if (m_stCurrentCpuMode == "nice")
                    cpuMetrics.Nice = ToDouble(value);
                else if (m_stCurrentCpuMode == "softirq")
                    cpuMetrics.SoftIrq = ToDouble(value);
                else if (m_stCurrentCpuMode == "steal")
                    cpuMetrics.Steal = ToDouble(value);
                else if (m_stCurrentCpuMode == "system")
                    cpuMetrics.System = ToDouble(value);
                else if (m_stCurrentCpuMode == "user")
                    cpuMetrics.User = ToDouble(value);
                m_iCurrentCpuIndex = -1;
                m_stCurrentCpuMode = {};
            }
            else if (m_stCurrentMetricsName == "node_disk_io_time_seconds_total")
            {
                m_stRawMetrics.DiskIoTimeSecondsTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
            else if (m_stCurrentMetricsName == "node_disk_read_time_seconds_total")
            {
                m_stRawMetrics.DiskReadTimeSecondsTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
            else if (m_stCurrentMetricsName == "node_disk_write_time_seconds_total")
            {
                m_stRawMetrics.DiskWriteTimeSecondsTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
            else if (m_stCurrentMetricsName == "node_disk_read_bytes_total")
            {
                m_stRawMetrics.DiskReadBytesTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
            else if (m_stCurrentMetricsName == "node_disk_written_bytes_total")
            {
                m_stRawMetrics.DiskWrittenBytesTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
            else if (m_stCurrentMetricsName == "node_network_receive_bytes_total")
            {
                m_stRawMetrics.NetworkReceiveBytesTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
            else if (m_stCurrentMetricsName == "node_network_transmit_bytes_total")
            {
                m_stRawMetrics.NetworkTransmitBytesTotal[m_stCurrentDeviceName] = ToDouble(value);
                m_stCurrentDeviceName = {};
            }
        }

        void OnMetricsEnd() override
        {
            m_stCurrentMetricsName = {};
        }

    private:
        MetricsSampleThread::RawMetrics& m_stRawMetrics;

        std::string_view m_stCurrentMetricsName;
        int m_iCurrentCpuIndex = -1;
        std::string m_stCurrentCpuMode;
        std::string m_stCurrentDeviceName;
    };
}

void MetricsSampleThread::Run()
{
    auto lastTick = ::SDL_GetTicks64();
    while (!m_bStopped)
    {
        while (true)
        {
            Command cmd;
            bool hasEvent = m_stCommandQueue.try_dequeue(cmd);
            if (!hasEvent)
                break;

            if (std::holds_alternative<QuitCommand>(cmd))
            {
                spdlog::info("Stopping sample thread");
                m_bStopped = true;
            }
            else if (std::holds_alternative<ChangeUrlCommand>(cmd))
            {
                auto& changeUrlCmd = std::get<ChangeUrlCommand>(cmd);
                spdlog::info("Changing URL to {}, refresh interval {}ms", changeUrlCmd.Url, changeUrlCmd.RefreshIntervalMs);
                m_stUrl = changeUrlCmd.Url;
                m_dRefreshIntervalMs = changeUrlCmd.RefreshIntervalMs;
            }
        }

        auto currentTick = ::SDL_GetTicks64();
        auto deltaTimeMs = static_cast<double>(currentTick - lastTick);
        lastTick = currentTick;

        m_dRefreshTimerMs += deltaTimeMs;
        if (m_dRefreshTimerMs >= m_dRefreshIntervalMs)
        {
            m_dRefreshTimerMs = 0.;
            RefreshMetrics();
        }

        ::SDL_Delay(100);  // 10 FPS
    }
}

void MetricsSampleThread::EnqueueCommand(Command&& cmd)
{
    m_stCommandQueue.enqueue(std::move(cmd));
}

bool MetricsSampleThread::TryDequeueResult(Result& result)
{
    return m_stResultQueue.try_dequeue(result);
}

void MetricsSampleThread::RefreshMetrics()
{
    RawMetrics rawMetrics;

    // 发起 HTTP 请求
    if (!m_stUrl.empty())
    {
        auto url = ada::parse(m_stUrl);
        if (!url)
        {
            spdlog::error("Failed to parse URL: {}", m_stUrl);
        }
        else
        {
            httplib::Client client(fmt::format("{}//{}", url->get_protocol(), url->get_host()));
            client.set_connection_timeout(5);
            auto res = client.Get(fmt::format("{}", url->get_pathname()));
            if (!res)
            {
                spdlog::error("Failed to get URL: {}, error: {}", m_stUrl, static_cast<int>(res.error()));
            }
            else
            {
                auto status = res->status;
                if (status != 200)
                {
                    spdlog::error("Failed to get URL: {}, status: {}", m_stUrl, status);
                }
                else
                {
                    const auto& body = res->body;

                    MetricsParseListener listener(rawMetrics);
                    MetricsParser::Parse(body, &listener);
                    rawMetrics.Tick = ::SDL_GetTicks64();
                }
            }
        }
    }

    // 从 rawMetrics 产生处理后的结果
    if (!m_stLastRawMetrics)
    {
        m_stLastRawMetrics = std::move(rawMetrics);
        return;
    }

    MetricsResult metrics;
    metrics.Tick = rawMetrics.Tick;
    metrics.BootTimeSeconds = rawMetrics.BootTimestamp == 0 ? 0 : static_cast<double>(::time(nullptr) - rawMetrics.BootTimestamp);
    metrics.Load1 = rawMetrics.Load1;
    metrics.Load5 = rawMetrics.Load5;
    metrics.Load15 = rawMetrics.Load15;
    metrics.MemoryAvailableBytes = rawMetrics.MemoryAvailableBytes;
    metrics.MemoryTotalBytes = rawMetrics.MemoryTotalBytes;
    metrics.MemoryFreeBytes = rawMetrics.MemoryFreeBytes;

    // 计算 CPU 占用
    size_t lastFillIndex = 0;
    for (const auto& [cpuIndex, cpuMetrics] : rawMetrics.CpuSecondsTotal)
    {
        // 由于 CpuSecondsTotal 使用 map 存储，因此保证从小到大排序
        while (lastFillIndex < cpuIndex)
        {
            metrics.CpuUsage.push_back(0);
            ++lastFillIndex;
        }

        // 查找上次的 CPU 时间
        auto it = m_stLastRawMetrics->CpuSecondsTotal.find(cpuIndex);
        if (it == m_stLastRawMetrics->CpuSecondsTotal.end())
            continue;

        // 计算时间
        auto cpuTotalSecondsDelta = cpuMetrics.TotalSeconds() - it->second.TotalSeconds();
        auto cpuIdleDelta = cpuMetrics.Idle - it->second.Idle;
        auto cpuUsage = std::min(100., std::max(0., 100.0 * (cpuTotalSecondsDelta - cpuIdleDelta) / cpuTotalSecondsDelta));

        // 记录
        metrics.CpuUsage.push_back(cpuUsage);
        ++lastFillIndex;
    }

    // 计算磁盘占用
    for (const auto& [device, value] : rawMetrics.DiskReadBytesTotal)
    {
        auto it = m_stLastRawMetrics->DiskReadBytesTotal.find(device);
        if (it == m_stLastRawMetrics->DiskReadBytesTotal.end())
            continue;

        auto delta = value - it->second;
        auto deltaTickMs = static_cast<double>(rawMetrics.Tick - m_stLastRawMetrics->Tick);
        metrics.DiskReadBytesPerSecond[device] = static_cast<double>(delta) / static_cast<double>(deltaTickMs / 1000);
    }
    for (const auto& [device, value] : rawMetrics.DiskWrittenBytesTotal)
    {
        auto it = m_stLastRawMetrics->DiskWrittenBytesTotal.find(device);
        if (it == m_stLastRawMetrics->DiskWrittenBytesTotal.end())
            continue;

        auto delta = value - it->second;
        auto deltaTickMs = static_cast<double>(rawMetrics.Tick - m_stLastRawMetrics->Tick);
        metrics.DiskWrittenBytesPerSecond[device] = static_cast<double>(delta) / static_cast<double>(deltaTickMs / 1000);
    }

    for (const auto& [device, value] : rawMetrics.NetworkReceiveBytesTotal)
    {
        auto it = m_stLastRawMetrics->NetworkReceiveBytesTotal.find(device);
        if (it == m_stLastRawMetrics->NetworkReceiveBytesTotal.end())
            continue;

        auto delta = value - it->second;
        auto deltaTickMs = static_cast<double>(rawMetrics.Tick - m_stLastRawMetrics->Tick);
        metrics.NetworkReceiveBytesPerSecond[device] = static_cast<double>(delta) / static_cast<double>(deltaTickMs / 1000);
    }
    for (const auto& [device, value] : rawMetrics.NetworkTransmitBytesTotal)
    {
        auto it = m_stLastRawMetrics->NetworkTransmitBytesTotal.find(device);
        if (it == m_stLastRawMetrics->NetworkTransmitBytesTotal.end())
            continue;

        auto delta = value - it->second;
        auto deltaTickMs = static_cast<double>(rawMetrics.Tick - m_stLastRawMetrics->Tick);
        metrics.NetworkTransmitBytesPerSecond[device] = static_cast<double>(delta) / static_cast<double>(deltaTickMs / 1000);
    }

    m_stLastRawMetrics = std::move(rawMetrics);

    // 推送 Metrics
    m_stResultQueue.enqueue(std::move(metrics));
}
