/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#pragma once
#include <thread>
#include <imgui.h>
#include "AppBase.hpp"
#include "MetricsSampleThread.hpp"

class App :
    public AppBase
{
public:
    Result<void> Initialize() noexcept;

protected: // AppBase
    void OnStart() noexcept override;
    void OnFrame(double delta) noexcept override;
    void OnStop() noexcept override;

private:
    static const size_t kHistorySampleCount = 150;

    ImFont* m_pDefaultFont = nullptr;
    ImFont* m_pNumericFont = nullptr;
    ImFont* m_pDefaultTinyFont = nullptr;
    ImFont* m_pNumericTinyFont = nullptr;

    MetricsSampleThread m_stSampleThread;
    std::thread m_stSampleThreadHandle;

    // 采样数据
    MetricsSampleThread::MetricsResult m_stCurrentMetrics;
    std::vector<double> m_stCpuUsageHistory;
    std::vector<double> m_stMemoryUsageHistory;
    std::vector<double> m_stIoReadHistory;
    std::vector<double> m_stIoWriteHistory;
    std::vector<double> m_stNetworkReceiveHistory;
    std::vector<double> m_stNetworkTransmitHistory;
};
