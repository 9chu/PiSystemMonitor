/**
 * @file
 * @author chu
 * @date 2024/11/17
 */
#include <MetricsParser.hpp>

#include <cassert>

using namespace std;

void MetricsParser::Parse(std::string_view content, IListener* callback)
{
    assert(callback);

    enum {
        STATE_LINE_START,
        STATE_EAT_COMMENT_LINE,
        STATE_METRICS_NAME,
        STATE_LABEL_OR_VALUE,
        STATE_LABEL_NAME_START,
        STATE_LABEL_NAME,
        STATE_LABEL_EXPECT_EQUAL,
        STATE_LABEL_EXPECT_QUOTE,
        STATE_LABEL_VALUE,
        STATE_LABEL_NAME_OR_COMMA,
        STATE_WAIT_METRICS_VALUE,
        STATE_METRICS_VALUE,
    } state = STATE_LINE_START;
    size_t pos = 0;

    size_t posStart = 0;
    size_t posEnd = 0;
    size_t posLabelValueStart = 0;
    size_t posLabelValueEnd = 0;

    while (pos <= content.size())
    {
        char ch = (pos >= content.size()) ? '\0' : content[pos];
        switch (state)
        {
            case STATE_LINE_START:
                if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n')
                    break;
                if (ch == '\0')
                    return;
                if (ch == '#')
                {
                    state = STATE_EAT_COMMENT_LINE;
                }
                else
                {
                    state = STATE_METRICS_NAME;
                    posStart = pos;
                    posEnd = pos + 1;
                }
                break;
            case STATE_EAT_COMMENT_LINE:
                if (ch == '\r' || ch == '\n')
                {
                    state = STATE_LINE_START;
                    break;
                }
                if (ch == '\0')
                    return;
                break;
            case STATE_METRICS_NAME:
                if (ch == ' ' || ch == '\t')
                {
                    state = STATE_LABEL_OR_VALUE;
                    callback->OnMetricsBegin(content.substr(posStart, posEnd - posStart));
                    break;
                }
                if (ch == '{')
                {
                    state = STATE_LABEL_NAME_START;
                    callback->OnMetricsBegin(content.substr(posStart, posEnd - posStart));
                    break;
                }
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsBegin(content.substr(posStart, posEnd - posStart));
                    callback->OnMetricsEnd();
                    break;
                }
                posEnd = pos + 1;
                break;
            case STATE_LABEL_OR_VALUE:
                if (ch == '{')
                {
                    state = STATE_LABEL_NAME_START;
                    break;
                }
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == ' ' || ch == '\t')
                    break;
                state = STATE_METRICS_VALUE;
                posStart = pos;
                posEnd = pos + 1;
                break;
            case STATE_LABEL_NAME_START:
                if (ch == ' ' || ch == '\t')
                    break;
                if (ch == '}')
                {
                    state = STATE_WAIT_METRICS_VALUE;
                    break;
                }
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                state = STATE_LABEL_NAME;
                posStart = pos;
                posEnd = pos + 1;
                break;
            case STATE_LABEL_NAME:
                if (ch == ' ' || ch == '\t')
                {
                    state = STATE_LABEL_EXPECT_EQUAL;
                    break;
                }
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == '=')
                {
                    state = STATE_LABEL_EXPECT_QUOTE;
                    break;
                }
                posEnd = pos + 1;
                break;
            case STATE_LABEL_EXPECT_EQUAL:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == '=')
                {
                    state = STATE_LABEL_EXPECT_QUOTE;
                    break;
                }
                break;
            case STATE_LABEL_EXPECT_QUOTE:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == '"')
                {
                    state = STATE_LABEL_VALUE;
                    posLabelValueStart = pos + 1;
                    posLabelValueEnd = pos + 1;
                    break;
                }
                break;
            case STATE_LABEL_VALUE:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == '"')
                {
                    state = STATE_LABEL_NAME_OR_COMMA;
                    callback->OnMetricsLabel(content.substr(posStart, posEnd - posStart),
                        content.substr(posLabelValueStart, posLabelValueEnd - posLabelValueStart));
                    break;
                }
                // TODO: escape sequence
                posLabelValueEnd = pos + 1;
                break;
            case STATE_LABEL_NAME_OR_COMMA:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == ',')
                {
                    state = STATE_LABEL_NAME_START;
                    break;
                }
                if (ch == '}')
                {
                    state = STATE_WAIT_METRICS_VALUE;
                    break;
                }
                break;
            case STATE_WAIT_METRICS_VALUE:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsEnd();
                    break;
                }
                if (ch == ' ' || ch == '\t')
                    break;
                state = STATE_METRICS_VALUE;
                posStart = pos;
                posEnd = pos + 1;
                break;
            case STATE_METRICS_VALUE:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_LINE_START;
                    callback->OnMetricsValue(content.substr(posStart, posEnd - posStart));
                    callback->OnMetricsEnd();
                    break;
                }
                posEnd = pos + 1;
                break;
            default:
                assert(false);
                break;
        }

        ++pos;
    }
}
