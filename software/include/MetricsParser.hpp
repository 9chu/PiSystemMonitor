/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#pragma once
#include <string>
#include <string_view>

class MetricsParser
{
public:
    class IListener
    {
    public:
        virtual ~IListener() = default;

    public:
        virtual void OnMetricsBegin(std::string_view name) = 0;
        virtual void OnMetricsLabel(std::string_view name, std::string_view value) = 0;
        virtual void OnMetricsValue(std::string_view value) = 0;
        virtual void OnMetricsEnd() = 0;
    };

    static void Parse(std::string_view content, IListener* callback);
};
