# -*- coding: utf-8 -*-
import os
import math
import time
import datetime
import logging
import imgui
import numpy as np
from queue import Queue
from sdl2 import *
from app_base import ApplicationBase
from sample_thread import CpuMetrics, Metrics, SampleThread


BG_WINDOW_FLAGS = imgui.WINDOW_NO_TITLE_BAR | imgui.WINDOW_NO_SCROLLBAR | imgui.WINDOW_NO_MOVE | \
                  imgui.WINDOW_NO_RESIZE | imgui.WINDOW_NO_COLLAPSE | imgui.WINDOW_NO_SAVED_SETTINGS


COLOR_DEFAULT_CONTENT = [0xCF / 0xFF, 0xD8 / 0xFF, 0xD5 / 0xFF, 1.0]
COLOR_L1 = [0xFB / 0xFF, 0x85 / 0xFF, 0, 1.0]
COLOR_L2 = [0x21 / 0xFF, 0x9E / 0xFF, 0xBC / 0xFF, 1.0]
COLOR_L3 = [0x02 / 0xFF, 0x30 / 0xFF, 0x47 / 0xFF, 1.0]
COLOR_METRICS_L1 = [0xF3 / 0xFF, 0xF7 / 0xFF, 0xF0 / 0xFF, 1.0]
COLOR_METRICS_L2 = [0xFC / 0xFF, 0xBA / 0xFF, 0x04 / 0xFF, 1.0]
COLOR_METRICS_L3 = [0xF4 / 0xFF, 0x2B / 0xFF, 0x03 / 0xFF, 1.0]


def _uptime_to_dhms(t: float):
    d = t // (24 * 60 * 60)
    t = math.fmod(t, 24 * 60 * 60)
    h = t // (60 * 60)
    t = math.fmod(t, 60 * 60)
    m = t // 60
    t = math.fmod(t, 60)
    s = t // 1
    return d, h, m, s


class Application(ApplicationBase):
    def __init__(self):
        super().__init__()
        self._thread_command_queue = Queue(10)
        self._thread_output_queue = Queue(10)
        self._sample_thread = SampleThread(self._thread_command_queue, self._thread_output_queue)

        # 从环境变量获取配置
        metrics_url = os.getenv("METRICS_URL", "http://localhost:9100/metrics")
        self._thread_command_queue.put(["change_url", metrics_url, 1.0])

        self._metrics = Metrics.model_construct()
        self._last_metrics_tick = SDL_GetTicks64() / 1000.0

        self._current_page = 0
        self._current_page_timer = 0.0
        self._pages = [
            self._draw_load_page,
            self._draw_mem_page,
        ]

        self._hist_load1 = [0.0 for _ in range(100)]
        self._hist_load5 = [0.0 for _ in range(100)]
        self._hist_load15 = [0.0 for _ in range(100)]
        self._hist_mem_used = [0.0 for _ in range(100)]

    def _draw_load_page(self, screen_width):
        cpu_cores = len(self._metrics.cpu_seconds_total)
        if cpu_cores == 0:
            load = 0
            load_str = "N/A"
        else:
            load = int(min(1.0, self._metrics.load1 / cpu_cores) * 100)
            load_str = "%03d" % (load)

        with imgui.font(self.get_font("metrics-big")):
            imgui.text("LOAD")
            imgui.same_line(screen_width - (imgui.calc_text_size(load_str).x + 10))
            if load < 30:
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_METRICS_L1)
            elif load < 70:
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_METRICS_L2)
            else:
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_METRICS_L3)
            imgui.text(load_str)
            imgui.pop_style_color()

        # 绘制负载图
        pos = imgui.get_cursor_screen_pos()
        imgui.push_style_color(imgui.COLOR_FRAME_BACKGROUND, 0, 0, 0, 0)
        imgui.push_style_color(imgui.COLOR_PLOT_LINES, *COLOR_L1)
        imgui.plot_lines("##LOAD1", np.array(self._hist_load1, dtype="float32"), scale_min=0,
                         scale_max=cpu_cores,
                         graph_size=(int(screen_width - 20), imgui.get_content_region_available().y))
        imgui.pop_style_color()
        imgui.set_item_allow_overlap()
        imgui.set_cursor_pos(pos)
        imgui.push_style_color(imgui.COLOR_PLOT_LINES, *COLOR_L2)
        imgui.plot_lines("##LOAD5", np.array(self._hist_load5, dtype="float32"), scale_min=0,
                         scale_max=cpu_cores,
                         graph_size=(int(screen_width - 20), imgui.get_content_region_available().y))
        imgui.pop_style_color()
        imgui.set_item_allow_overlap()
        imgui.set_cursor_pos(pos)
        imgui.push_style_color(imgui.COLOR_PLOT_LINES, *COLOR_L3)
        imgui.plot_lines("##LOAD5", np.array(self._hist_load15, dtype="float32"), scale_min=0,
                         scale_max=cpu_cores,
                         graph_size=(int(screen_width - 20), imgui.get_content_region_available().y))
        imgui.pop_style_color()
        imgui.pop_style_color()

    def _draw_mem_page(self, screen_width):
        total_mem = self._metrics.memory_total_bytes
        if total_mem == 0:
            current_used = 0
            current_used_str = "N/A"
        else:
            current_used = (self._metrics.memory_total_bytes - self._metrics.memory_available_bytes) / total_mem
            current_used = int(min(1.0, max(0.0, current_used)) * 100)
            current_used_str = "%03d" % current_used

        with imgui.font(self.get_font("metrics-big")):
            imgui.text("MEM")
            imgui.same_line(screen_width - (imgui.calc_text_size(current_used_str).x + 10))
            if current_used < 30:
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_METRICS_L1)
            elif current_used < 70:
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_METRICS_L2)
            else:
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_METRICS_L3)
            imgui.text(current_used_str)
            imgui.pop_style_color()

        # 绘制内存使用曲线
        imgui.push_style_color(imgui.COLOR_FRAME_BACKGROUND, 0, 0, 0, 0)
        imgui.push_style_color(imgui.COLOR_PLOT_LINES, *COLOR_L1)
        imgui.plot_lines("##MEM", np.array(self._hist_mem_used, dtype="float32"), scale_min=0,
                         scale_max=total_mem,
                         graph_size=(int(screen_width - 20), imgui.get_content_region_available().y))
        imgui.pop_style_color()
        imgui.pop_style_color()

    def on_start(self):
        self.register_font("titlebar", "FrozenCrystalBold-G21m.otf", 32)
        self.register_font("metrics-big", "FrozenCrystalBold-G21m.otf", 96)

        # 启动采样
        self._sample_thread.start()

    def on_frame(self, delta_time):
        # 检查是否有最新的数据
        while True:
            try:
                m = self._thread_output_queue.get(block=False)
            except Exception:
                m = None
            if m is None:
                break
            self._metrics = m

            # 采集历史数据
            current_tick = self._metrics.tick
            self._hist_load1.pop(0)
            self._hist_load1.append(self._metrics.load1)
            self._hist_load5.pop(0)
            self._hist_load5.append(self._metrics.load5)
            self._hist_load15.pop(0)
            self._hist_load15.append(self._metrics.load15)
            self._hist_mem_used.pop(0)
            self._hist_mem_used.append(self._metrics.memory_total_bytes - self._metrics.memory_available_bytes)
            self._last_metrics_tick = current_tick

        # 渲染
        width, height = self.imgui_io.display_size
        imgui.set_next_window_position(0, 0)
        imgui.set_next_window_size(width, height)
        if imgui.begin("main", False, BG_WINDOW_FLAGS):
            # 第一行：时间，启动时间
            with imgui.font(self.get_font("titlebar")):
                # 时间
                dt_str = datetime.datetime.now().strftime("%H:%M:%S")
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_DEFAULT_CONTENT)
                imgui.text(dt_str)
                imgui.pop_style_color()

                # 启动时间
                boot_time = (0, 0, 0, 0) if self._metrics.boot_time_seconds == 0 else _uptime_to_dhms(time.time() - self._metrics.boot_time_seconds)
                boot_time_str = "UP %02dD %02dH %02dM %02dS  " % boot_time
                imgui.same_line(width - (imgui.calc_text_size(boot_time_str).x + 10))
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_DEFAULT_CONTENT)
                imgui.text("UP")
                imgui.pop_style_color()
                imgui.same_line()
                imgui.text(f"%02d" % boot_time[0])
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_DEFAULT_CONTENT)
                imgui.same_line()
                imgui.text("D")
                imgui.pop_style_color()
                imgui.same_line()
                imgui.text(f"%02d" % boot_time[1])
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_DEFAULT_CONTENT)
                imgui.same_line()
                imgui.text("H")
                imgui.pop_style_color()
                imgui.same_line()
                imgui.text(f"%02d" % boot_time[2])
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_DEFAULT_CONTENT)
                imgui.same_line()
                imgui.text("M")
                imgui.pop_style_color()
                imgui.same_line()
                imgui.text(f"%02d" % boot_time[3])
                imgui.push_style_color(imgui.COLOR_TEXT, *COLOR_DEFAULT_CONTENT)
                imgui.same_line()
                imgui.text("S")
                imgui.pop_style_color()

            # 剩下交由 page 函数处理
            self._current_page_timer += delta_time
            if self._current_page_timer > 5.0:  # 5 秒切换显示
                self._current_page_timer = 0.0
                self._current_page = (self._current_page + 1) % len(self._pages)
            self._pages[self._current_page](width)

            imgui.end()

    def on_stop(self):
        logging.info("Quitting sample thread")
        self._thread_command_queue.put(["quit"])
        self._sample_thread.join()
