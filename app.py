# -*- coding: utf-8 -*-
import os
import math
import datetime
import dearpygui.dearpygui as dpg
from queue import Queue
from app_base import AppBase
from sample_thread import SampleThread, Metrics


def _uptime_to_dhms(t: float):
    d = t // (24 * 60 * 60)
    t = math.fmod(t, 24 * 60 * 60)
    h = t // (60 * 60)
    t = math.fmod(t, 60 * 60)
    m = t // 60
    t = math.fmod(t, 60)
    s = t // 1
    return d, h, m, s


def _calc_total_cpu_usage(d):
    total = 0
    count = 0
    for id in d:
        value = d[id]
        total += value
        count += 1
    return 0 if count == 0 else int(max(0, min(1, total / count)) * 100)


def _calc_total_value(d):
    total = 0
    for id in d:
        total += d[id]
    return total


def _value_to_gb(value):
    return int(value / 1024 / 1024 / 1024)


def _value_auto_unit(value):
    if value < 1000:
        return value, "B"
    elif value < 1000 * 1000:
        return int(value / 1000), "K"
    elif value < 1000 * 1000 * 1000:
        return int(value / 1000 / 1000), "M"
    elif value < 1000 * 1000 * 1000 * 1000:
        return int(value / 1000 / 1000 / 1000), "G"
    else:
        return int(value / 1000 / 1000 / 1000 / 1000), "T"


FONT_SIZE1 = 26
FONT_SIZE2 = 28

HIST_METRICS = 100


class App(AppBase):
    def __init__(self):
        super().__init__("PiSystemMonitor", 480, 320)

        self._queue_to_thread = Queue(20)
        self._queue_from_thread = Queue(20)
        self._sample_thread = SampleThread(self._queue_to_thread, self._queue_from_thread)

        self._fonts = {}
        self._widgets = {}

        self._current_datetime = datetime.datetime.now()
        self._metrics = Metrics.model_construct()  # type: Metrics
        self._cpu_history = [0 for _ in range(HIST_METRICS)]
        self._mem_history = [0 for _ in range(HIST_METRICS)]
        self._io_read_history = [0 for _ in range(HIST_METRICS)]
        self._io_write_history = [0 for _ in range(HIST_METRICS)]
        self._net_read_history = [0 for _ in range(HIST_METRICS)]
        self._net_write_history = [0 for _ in range(HIST_METRICS)]

    def _layout_plot(self, usage_control_tag, series_control_tag, unit="%", plot_theme="plot_theme"):
        with dpg.group(horizontal=True):
            usage_text = dpg.add_text("%03d" % 0, tag=usage_control_tag)
            unit_text = dpg.add_text(unit, tag=f"{usage_control_tag}_unit")
            dpg.bind_item_font(usage_text, self._fonts["numeric"])
            dpg.bind_item_font(unit_text, self._fonts["default"])

            with dpg.plot(label="", height=FONT_SIZE1, width=270, no_title=True, no_menus=True) as plot:
                dpg.add_plot_axis(dpg.mvXAxis, label="x", no_label=True, no_tick_labels=True, no_tick_marks=True,
                                  no_gridlines=True)
                axis_y = dpg.add_plot_axis(dpg.mvYAxis, label="y", no_label=True, no_tick_labels=True,
                                           no_tick_marks=True, no_gridlines=True, tag=f"{series_control_tag}_y")
                dpg.set_axis_limits(axis_y, 0, 1)

                dpg.add_line_series([], [], parent=axis_y, tag=series_control_tag, shaded=True)

                dpg.bind_item_theme(plot, plot_theme)

    def _update_plot(self, tag, value, max_y=None):
        x = [i for i in range(HIST_METRICS)]
        y = value
        y_min = min(y)
        y_max = max(y)
        if max_y is not None:
            y_max = max_y
        dpg.set_axis_limits(f"{tag}_y", y_min, y_max)
        dpg.set_value(tag, [x, y])

    def on_start(self):
        # 初始化字体
        self.register_font("default", "./res/whitrabt.ttf", FONT_SIZE1)
        self.register_font("numeric", "./res/Segment7-4Gml.otf", FONT_SIZE2)

        self.register_font("default_tiny", "./res/whitrabt.ttf", int(FONT_SIZE1 * 0.6))
        self.register_font("numeric_tiny", "./res/Segment7-4Gml.otf", int(FONT_SIZE2 * 0.6))

        # 初始化采样线程
        metrics_url = os.getenv("METRICS_URL", "http://localhost:9100/metrics")  # 从环境变量获取配置
        self._queue_to_thread.put(["change_url", metrics_url, 1.0])

        # 启动采样线程
        self._sample_thread.start()

    def on_layout(self):
        with dpg.theme(tag="plot_theme"):
            with dpg.theme_component(dpg.mvPlot):
                dpg.add_theme_style(dpg.mvPlotStyleVar_PlotPadding, 0, category=dpg.mvThemeCat_Plots)

        # 标题行
        with dpg.group(horizontal=True):
            title_datetime_text = dpg.add_text("1970-01-01 00:00:00", tag="title_datetime_text")
            dpg.add_spacer(width=55)
            title_uptime_text = dpg.add_text("UP 000 00:00:00", tag="title_uptime_text")

            dpg.bind_item_font(title_datetime_text, self._fonts["default_tiny"])
            dpg.bind_item_font(title_uptime_text, self._fonts["default_tiny"])

        # 图表
        with dpg.table(header_row=False):
            dpg.add_table_column(width_fixed=True)
            dpg.add_table_column()

            # CPU
            with dpg.table_row():
                with dpg.group(horizontal=True):
                    dpg.add_spacer(width=1)
                    plot_cpu_text = dpg.add_text("CPU", tag="cpu_text")
                    dpg.bind_item_font(plot_cpu_text, self._fonts["default"])
                    dpg.add_spacer(width=1)

                self._layout_plot("cpu_usage_text", "cpu_usage_series")

            # MEM
            with dpg.table_row():
                with dpg.group(horizontal=True):
                    dpg.add_spacer(width=1)
                    plot_mem_text = dpg.add_text("MEM", tag="mem_text")
                    dpg.bind_item_font(plot_mem_text, self._fonts["default"])
                    dpg.add_spacer(width=1)

                self._layout_plot("mem_usage_text", "mem_usage_series", "G")

            # I/O
            with dpg.table_row():
                with dpg.group(horizontal=True):
                    dpg.add_spacer(width=1)
                    plot_io_text = dpg.add_text("I/O", tag="io_text")
                    dpg.bind_item_font(plot_io_text, self._fonts["default"])
                    dpg.add_spacer(width=1)

                with dpg.table(header_row=False):
                    dpg.add_table_column(width_fixed=True)
                    dpg.add_table_column()

                    with dpg.table_row():
                        self._layout_plot("io_read_text", "io_read_series", "B")
                    with dpg.table_row():
                        self._layout_plot("io_write_text", "io_write_series", "B")

            # NET
            with dpg.table_row():
                with dpg.group(horizontal=True):
                    dpg.add_spacer(width=1)
                    plot_net_text = dpg.add_text("NET", tag="net_text")
                    dpg.bind_item_font(plot_net_text, self._fonts["default"])
                    dpg.add_spacer(width=1)

                with dpg.table(header_row=False):
                    dpg.add_table_column(width_fixed=True)
                    dpg.add_table_column()

                    with dpg.table_row():
                        self._layout_plot("net_read_text", "net_read_series", "B")
                    with dpg.table_row():
                        self._layout_plot("net_write_text", "net_write_series", "B")

        #dpg.show_implot_demo()

    def on_frame(self):
        # 接收采样线程消息
        while not self._queue_from_thread.empty():
            self._metrics = self._queue_from_thread.get()

            # 收集历史数据
            self._cpu_history.pop(0)
            self._mem_history.pop(0)
            self._io_read_history.pop(0)
            self._io_write_history.pop(0)
            self._net_read_history.pop(0)
            self._net_write_history.pop(0)

            self._cpu_history.append(_calc_total_cpu_usage(self._metrics.cpu_usage_percent))
            self._mem_history.append(self._metrics.memory_total_bytes - self._metrics.memory_available_bytes)
            self._io_read_history.append(_calc_total_value(self._metrics.disk_read_bytes_per_second))
            self._io_write_history.append(_calc_total_value(self._metrics.disk_written_bytes_per_second))
            self._net_read_history.append(_calc_total_value(self._metrics.network_receive_bytes_per_second))
            self._net_write_history.append(_calc_total_value(self._metrics.network_transmit_bytes_per_second))

        self._current_datetime = datetime.datetime.now()

        # 更新标题行
        dpg.set_value("title_datetime_text", self._current_datetime.strftime("%Y-%m-%d %H:%M:%S"))
        dpg.set_value("title_uptime_text", "UP %03d %02d:%02d:%02d" %
                      _uptime_to_dhms(self._metrics.boot_time_seconds))

        # 更新 CPU 占用率
        dpg.set_value("cpu_usage_text", "%03d" % self._cpu_history[-1])
        self._update_plot("cpu_usage_series", self._cpu_history, 100)

        # 更新内存占用
        dpg.set_value("mem_usage_text", "%03d" % _value_to_gb(self._mem_history[-1]))
        self._update_plot("mem_usage_series", self._mem_history, self._metrics.memory_total_bytes)

        # 更新 I/O
        v, u = _value_auto_unit(self._io_read_history[-1])
        dpg.set_value("io_read_text", "%03d" % v)
        dpg.set_value("io_read_text_unit", u)
        self._update_plot("io_read_series", self._io_read_history)
        v, u = _value_auto_unit(self._io_write_history[-1])
        dpg.set_value("io_write_text", "%03d" % v)
        dpg.set_value("io_write_text_unit", u)
        self._update_plot("io_write_series", self._io_write_history)

        # 更新 NET
        v, u = _value_auto_unit(self._net_read_history[-1])
        dpg.set_value("net_read_text", "%03d" % v)
        dpg.set_value("net_read_text_unit", u)
        self._update_plot("net_read_series", self._net_read_history)
        v, u = _value_auto_unit(self._net_write_history[-1])
        dpg.set_value("net_write_text", "%03d" % v)
        dpg.set_value("net_write_text_unit", u)
        self._update_plot("net_write_series", self._net_write_history)

    def on_stop(self):
        # 终止采样线程
        self._queue_to_thread.put(["quit"])
        self._sample_thread.join()
