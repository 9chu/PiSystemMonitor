/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#include <App.hpp>

#include <implot.h>

#include <whitrabt.ttf.inl>
#include <Segment7-4Gml.otf.inl>

using namespace std;

static const float kFontSize1 = 40.f;
static const float kFontSize2 = 42.f;

namespace
{
    double CalculateTotalCpuUsage(std::vector<double>& cpuUsage) noexcept
    {
        double total = 0;
        for (auto& usage : cpuUsage)
            total += usage;
        if (cpuUsage.empty())
            return 0;
        return total / static_cast<double>(cpuUsage.size());
    }

    double SumAllMetricsValue(std::map<std::string, double>& metrics) noexcept
    {
        double total = 0;
        for (const auto& [_, value] : metrics)
            total += value;
        return total;
    }

    std::tuple<int, const char*> AutoUnit(double bytes) noexcept
    {
        if (bytes < 1000.)
            return {bytes, "B"};
        if (bytes < 1000. * 1000)
            return {static_cast<int>(bytes / 1000), "K"};
        if (bytes < 1000. * 1000 * 1000)
            return {static_cast<int>(bytes / 1000 / 1000), "M"};
        if (bytes < 1000. * 1000 * 1000 * 1000)
            return {static_cast<int>(bytes / 1000 / 1000 / 1000), "G"};
        return {static_cast<int>(bytes / 1000 / 1000 / 1000 / 1000), "T"};
    }

    std::tuple<int, int, int, int> UptimeToDHMS(double t) noexcept
    {
        auto d = floor(t / (24 * 60 * 60));
        t = fmod(t, 24 * 60 * 60);
        auto h = floor(t / (60 * 60));
        t = fmod(t, 60 * 60);
        auto m = floor(t / 60);
        t = fmod(t, 60);
        auto s = floor(t / 1);
        return {d, h, m, s};
    }
}

Result<void> App::Initialize() noexcept
{
    bool fullScreen = false;
    ::SDL_DisplayMode displayMode;
    if (::SDL_GetCurrentDisplayMode(0, &displayMode) == 0)
    {
        if (displayMode.w == 480 && displayMode.h == 320)
            fullScreen = true;
    }

    AppBaseConfig config;
    config.Title = "PiSystemMonitor";
    config.InitialWidth = 480;
    config.InitialHeight = 320;
    config.FullScreen = fullScreen;
    config.TargetFPS = 5;  // 5fps is enough
    return AppBase::Initialize(config);
}

void App::OnStart() noexcept
{
    auto& io = ImGui::GetIO();

    // 加载字体
    io.Fonts->AddFontDefault();
    m_pDefaultFont = io.Fonts->AddFontFromMemoryCompressedTTF(kFontWhitrabt_compressed_data,
        static_cast<int>(kFontWhitrabt_compressed_size), kFontSize1);
    m_pNumericFont = io.Fonts->AddFontFromMemoryCompressedTTF(kFontSegment7_compressed_data,
        static_cast<int>(kFontSegment7_compressed_size), kFontSize2);
    m_pDefaultTinyFont = io.Fonts->AddFontFromMemoryCompressedTTF(kFontWhitrabt_compressed_data,
        static_cast<int>(kFontWhitrabt_compressed_size), kFontSize1 / 2.5f);
    m_pNumericTinyFont = io.Fonts->AddFontFromMemoryCompressedTTF(kFontSegment7_compressed_data,
        static_cast<int>(kFontSegment7_compressed_size), kFontSize2 / 2.5f);

    // 启动采样线程
    const char* url = ::getenv("METRICS_URL");
    if (!url)
        url = "http://localhost:9100/metrics";
    m_stSampleThread.EnqueueCommand(MetricsSampleThread::ChangeUrlCommand { url });
    m_stSampleThreadHandle = thread([this]() { m_stSampleThread.Run(); });

    // 填充数据
    m_stCpuUsageHistory.resize(kHistorySampleCount);
    m_stMemoryUsageHistory.resize(kHistorySampleCount);
    m_stIoReadHistory.resize(kHistorySampleCount);
    m_stIoWriteHistory.resize(kHistorySampleCount);
    m_stNetworkReceiveHistory.resize(kHistorySampleCount);
    m_stNetworkTransmitHistory.resize(kHistorySampleCount);

    // 隐藏鼠标
    SDL_ShowCursor(SDL_DISABLE);
}

void App::OnFrame(double delta) noexcept
{
    try
    {
        // 从采样线程接收数据
        MetricsSampleThread::Result sampleThreadResult;
        while (m_stSampleThread.TryDequeueResult(sampleThreadResult))
        {
            if (std::holds_alternative<MetricsSampleThread::MetricsResult>(sampleThreadResult))
            {
                auto& metrics = std::get<MetricsSampleThread::MetricsResult>(sampleThreadResult);
                m_stCurrentMetrics = std::move(metrics);

                // 记录历史数据
                m_stCpuUsageHistory.push_back(CalculateTotalCpuUsage(m_stCurrentMetrics.CpuUsage));
                m_stMemoryUsageHistory.push_back(static_cast<double>(m_stCurrentMetrics.MemoryTotalBytes -
                    m_stCurrentMetrics.MemoryAvailableBytes));
                m_stIoReadHistory.push_back(SumAllMetricsValue(m_stCurrentMetrics.DiskReadBytesPerSecond));
                m_stIoWriteHistory.push_back(SumAllMetricsValue(m_stCurrentMetrics.DiskWrittenBytesPerSecond));
                m_stNetworkReceiveHistory.push_back(SumAllMetricsValue(m_stCurrentMetrics.NetworkReceiveBytesPerSecond));
                m_stNetworkTransmitHistory.push_back(SumAllMetricsValue(m_stCurrentMetrics.NetworkTransmitBytesPerSecond));
                if (m_stCpuUsageHistory.size() > kHistorySampleCount)
                    m_stCpuUsageHistory.erase(m_stCpuUsageHistory.begin());
                if (m_stMemoryUsageHistory.size() > kHistorySampleCount)
                    m_stMemoryUsageHistory.erase(m_stMemoryUsageHistory.begin());
                if (m_stIoReadHistory.size() > kHistorySampleCount)
                    m_stIoReadHistory.erase(m_stIoReadHistory.begin());
                if (m_stIoWriteHistory.size() > kHistorySampleCount)
                    m_stIoWriteHistory.erase(m_stIoWriteHistory.begin());
                if (m_stNetworkReceiveHistory.size() > kHistorySampleCount)
                    m_stNetworkReceiveHistory.erase(m_stNetworkReceiveHistory.begin());
                if (m_stNetworkTransmitHistory.size() > kHistorySampleCount)
                    m_stNetworkTransmitHistory.erase(m_stNetworkTransmitHistory.begin());
            }
        }

        // 绘制界面
        auto& io = ImGui::GetIO();
        const auto& displaySize = io.DisplaySize;
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize(displaySize);
        if (ImGui::Begin("main", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
        {
            // 标题行
            ImGui::BeginGroup();
            {
                ::tm timeInfo;
                ::time_t now = ::time(nullptr);
                ::localtime_r(&now, &timeInfo);
                char timeStr[64];
                ::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo);

                auto dhms = UptimeToDHMS(m_stCurrentMetrics.BootTimeSeconds);
                char uptimeStr[64];
                ::snprintf(uptimeStr, sizeof(uptimeStr), "UP %03d %02d:%02d:%02d", std::get<0>(dhms), std::get<1>(dhms), std::get<2>(dhms),
                    std::get<3>(dhms));

                ImGui::PushFont(m_pDefaultTinyFont);
                ImGui::Text("%s", timeStr);
                ImGui::SameLine(0, 60);
                ImGui::Text("%s", uptimeStr);
                ImGui::PopFont();
            }
            ImGui::EndGroup();

            // 图表
            if (ImGui::BeginTable("metrics_table", 4, ImGuiTableFlags_SizingFixedFit))
            {
                auto drawMetricRow = [&](const char* label, int value, const char* unit, const char* plotCanvasName, const char* plotName,
                    double* data, size_t count, optional<double> maxY = {}, optional<ImVec4> lineColor = {},
                    optional<ImVec4> fillColor = {}) {
                    ImGui::TableNextRow();
                    ImGui::PushFont(m_pDefaultFont);

                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150 / 255., 150 / 255., 150 / 255., 1.f));
                    ImGui::Text("%s", label);
                    ImGui::PopStyleColor();

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushFont(m_pNumericFont);
                    ImGui::Text("%03d", value);
                    ImGui::PopFont();

                    ImGui::TableSetColumnIndex(2);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(150 / 255., 150 / 255., 150 / 255., 1.f));
                    ImGui::Text("%s", unit);
                    ImGui::PopStyleColor();

                    ImGui::TableSetColumnIndex(3);
                    auto plotWidth = displaySize.x - ImGui::GetCursorScreenPos().x;
                    ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0, 0));
                    ImPlot::PushStyleColor(ImPlotCol_PlotBorder, ImVec4(0, 0, 0, 0));
                    if (lineColor)
                        ImPlot::PushStyleColor(ImPlotCol_Line, *lineColor);
                    if (fillColor)
                        ImPlot::PushStyleColor(ImPlotCol_Fill, *fillColor);
                    if (ImPlot::BeginPlot(plotCanvasName, {plotWidth, kFontSize1}, ImPlotFlags_CanvasOnly))
                    {
                        if (!maxY)
                        {
                            for (size_t i = 0; i < count; ++i)
                                maxY = maxY ? std::max(*maxY, data[i]) : data[i];
                        }

                        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_NoDecorations);
                        ImPlot::SetupAxesLimits(0, kHistorySampleCount, 0, *maxY, ImPlotCond_Always);
                        ImPlot::PlotShaded(plotName, data, static_cast<int>(count));
                        ImPlot::PlotLine(plotName, data, static_cast<int>(count));
                        ImPlot::EndPlot();
                    }
                    ImPlot::PopStyleColor();
                    if (fillColor)
                        ImPlot::PopStyleColor();
                    if (lineColor)
                        ImPlot::PopStyleColor();
                    ImPlot::PopStyleVar();

                    ImGui::PopFont();
                };

                static const ImVec4 kCpuPlotColor = ImVec4{255 / 255.f, 140 / 255.f, 140 / 255.f, 255 / 255.f};
                static const ImVec4 kCpuPlotColorFill = ImVec4{255 / 255.f, 140 / 255.f, 140 / 255.f, 50 / 255.f};
                static const ImVec4 kMemoryPlotColor = ImVec4{135 / 255.f, 206 / 255.f, 250 / 255.f, 255 / 255.f};
                static const ImVec4 kMemoryPlotColorFill = ImVec4{135 / 255.f, 206 / 255.f, 250 / 255.f, 50 / 255.f};
                static const ImVec4 kIoReadPlotColor = ImVec4{255 / 255.f, 215 / 255.f, 0 / 255.f, 255 / 255.f};
                static const ImVec4 kIoReadPlotColorFill = ImVec4{255 / 255.f, 215 / 255.f, 0 / 255.f, 50 / 255.f};
                static const ImVec4 kIoWritePlotColor = ImVec4{255 / 255.f, 215 / 255.f, 0 / 255.f, 255 / 255.f};
                static const ImVec4 kIoWritePlotColorFill = ImVec4{255 / 255.f, 215 / 255.f, 0 / 255.f, 50 / 255.f};
                static const ImVec4 kNetworkReceivePlotColor = ImVec4{0 / 255.f, 255 / 255.f, 127 / 255.f, 255 / 255.f};
                static const ImVec4 kNetworkReceivePlotColorFill = ImVec4{0 / 255.f, 255 / 255.f, 127 / 255.f, 50 / 255.f};
                static const ImVec4 kNetworkTransmitPlotColor = ImVec4{0 / 255.f, 255 / 255.f, 127 / 255.f, 255 / 255.f};
                static const ImVec4 kNetworkTransmitPlotColorFill = ImVec4{0 / 255.f, 255 / 255.f, 127 / 255.f, 50 / 255.f};

                drawMetricRow("CPU", static_cast<int>(m_stCpuUsageHistory.back()), "%", "cpu_plot_c", "cpu_plot",
                    m_stCpuUsageHistory.data(), m_stCpuUsageHistory.size(), 100, kCpuPlotColor, kCpuPlotColorFill);

                auto memAutoUnit = AutoUnit(m_stMemoryUsageHistory.back());
                drawMetricRow("MEM", std::get<0>(memAutoUnit), std::get<1>(memAutoUnit), "mem_plot_c", "mem_plot",
                    m_stMemoryUsageHistory.data(), m_stMemoryUsageHistory.size(), m_stCurrentMetrics.MemoryTotalBytes,
                    kMemoryPlotColor, kMemoryPlotColorFill);

                auto ioReadAutoUnit = AutoUnit(m_stIoReadHistory.back());
                drawMetricRow("I/O", std::get<0>(ioReadAutoUnit), std::get<1>(ioReadAutoUnit), "io_read_plot_c", "io_read_plot",
                    m_stIoReadHistory.data(), m_stIoReadHistory.size(), {}, kIoReadPlotColor, kIoReadPlotColorFill);

                auto ioWriteAutoUnit = AutoUnit(m_stIoWriteHistory.back());
                drawMetricRow("   ", std::get<0>(ioWriteAutoUnit), std::get<1>(ioWriteAutoUnit), "io_write_plot_c", "io_write_plot",
                    m_stIoWriteHistory.data(), m_stIoWriteHistory.size(), {}, kIoWritePlotColor, kIoWritePlotColorFill);

                auto networkReceiveAutoUnit = AutoUnit(m_stNetworkReceiveHistory.back());
                drawMetricRow("NET", std::get<0>(networkReceiveAutoUnit), std::get<1>(networkReceiveAutoUnit), "network_receive_plot_c",
                    "network_receive_plot", m_stNetworkReceiveHistory.data(), m_stNetworkReceiveHistory.size(), {},
                    kNetworkReceivePlotColor, kNetworkReceivePlotColorFill);

                auto networkTransmitAutoUnit = AutoUnit(m_stNetworkTransmitHistory.back());
                drawMetricRow("   ", std::get<0>(networkTransmitAutoUnit), std::get<1>(networkTransmitAutoUnit), "network_transmit_plot_c",
                    "network_transmit_plot", m_stNetworkTransmitHistory.data(), m_stNetworkTransmitHistory.size(), {},
                    kNetworkTransmitPlotColor, kNetworkTransmitPlotColorFill);

                ImGui::EndTable();
            }

            ImGui::End();
        }
    }
    catch (...)
    {
    }
}

void App::OnStop() noexcept
{
    // 等待采样线程结束
    m_stSampleThread.EnqueueCommand(MetricsSampleThread::QuitCommand {});
    m_stSampleThreadHandle.join();
}
