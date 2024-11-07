# -*- coding: utf-8 -*-
import time
import logging
import threading
import requests
from queue import Queue
from typing import List, Dict, Any, Union, Optional, Tuple
from pydantic import BaseModel
from prometheus_client.parser import text_string_to_metric_families


def _get_first_metrics(metrics, name):
    if name in metrics:
        return metrics[name][0][0]
    return None


class RawCpuMetrics(BaseModel):
    idle: float = 0
    iowait: float = 0
    irq: float = 0
    nice: float = 0
    softirq: float = 0
    steal: float = 0
    system: float = 0
    user: float = 0


def _total_cpu_seconds(cpu_metrics: RawCpuMetrics):
    return cpu_metrics.idle + cpu_metrics.iowait + cpu_metrics.irq + cpu_metrics.nice + cpu_metrics.softirq + \
        cpu_metrics.steal + cpu_metrics.system + cpu_metrics.user


class RawMetrics(BaseModel):
    tick: float = 0
    boot_timestamp: float = 0
    load1: float = 0
    load5: float = 0
    load15: float = 0
    memory_available_bytes: float = 0
    memory_total_bytes: float = 0
    memory_free_bytes: float = 0
    cpu_seconds_total: Dict[int, RawCpuMetrics] = {}
    disk_io_time_seconds_total: Dict[str, float] = {}
    disk_read_time_seconds_total: Dict[str, float] = {}
    disk_write_time_seconds_total: Dict[str, float] = {}
    disk_read_bytes_total: Dict[str, float] = {}
    disk_written_bytes_total: Dict[str, float] = {}
    network_receive_bytes_total: Dict[str, float] = {}
    network_transmit_bytes_total: Dict[str, float] = {}


class Metrics(BaseModel):
    tick: float = 0
    boot_time_seconds: float = 0
    load1: float = 0
    load5: float = 0
    load15: float = 0
    memory_available_bytes: float = 0
    memory_total_bytes: float = 0
    memory_free_bytes: float = 0
    cpu_usage_percent: Dict[int, float] = {}
    disk_read_bytes_per_second: Dict[str, float] = {}
    disk_written_bytes_per_second: Dict[str, float] = {}
    network_receive_bytes_per_second: Dict[str, float] = {}
    network_transmit_bytes_per_second: Dict[str, float] = {}
    raw: RawMetrics


class SampleThread(threading.Thread):
    def __init__(self, q: Queue, q_out: Queue):
        super().__init__()
        self._q = q
        self._q_out = q_out
        self._event_dispatcher = {
            "quit": self._event_quit,
            "change_url": self._event_change_url
        }

        self._url = "http://localhost:9100/metrics"
        self._refresh_interval = 1.0

        self._stopped = False
        self._refresh_timer = 0.0
        self._last_metrics = None  # type: Optional[RawMetrics]

    def _event_quit(self):
        logging.info("Stopping sample thread")
        self._stopped = True

    def _event_change_url(self, url: str, refresh_interval: float):
        logging.info(f"Changing URL to {url}, refresh interval to {refresh_interval}")
        self._url = url
        self._refresh_interval = refresh_interval

    def _refresh_metrics(self):
        raw_metrics = {}  # type: Dict[str, List[Tuple[float, Dict[str, str]]]]
        try:
            text = requests.get(self._url, timeout=1).text
            for family in text_string_to_metric_families(text):
                for sample in family.samples:
                    if sample[0] not in raw_metrics:
                        raw_metrics[sample[0]] = []
                    raw_metrics[sample[0]].append((sample[2], sample[1]))
        except Exception:
            logging.exception(f"Failed to fetch metrics from {self._url}")

        metrics = RawMetrics.model_construct()
        metrics.tick = time.time()

        # 单项参数
        metrics.boot_timestamp = _get_first_metrics(raw_metrics, "node_boot_time_seconds") or 0
        metrics.load1 = _get_first_metrics(raw_metrics, "node_load1") or 0
        metrics.load5 = _get_first_metrics(raw_metrics, "node_load5") or 0
        metrics.load15 = _get_first_metrics(raw_metrics, "node_load15") or 0
        metrics.memory_available_bytes = _get_first_metrics(raw_metrics, "node_memory_MemAvailable_bytes") or 0
        metrics.memory_total_bytes = _get_first_metrics(raw_metrics, "node_memory_MemTotal_bytes") or 0
        metrics.memory_free_bytes = _get_first_metrics(raw_metrics, "node_memory_MemFree_bytes") or 0

        # 获取 CPU 状态
        cpu_seconds_total = raw_metrics.get("node_cpu_seconds_total", [])
        for e in cpu_seconds_total:
            value, tags = e
            cpu_id = tags.get("cpu", None)
            mode = tags.get("mode", None)
            if not cpu_id or not mode:
                continue
            cpu_id = int(cpu_id)
            cpu_metrics = metrics.cpu_seconds_total.get(cpu_id, None)
            if not cpu_metrics:
                cpu_metrics = RawCpuMetrics.model_construct()
                metrics.cpu_seconds_total[cpu_id] = cpu_metrics
            if mode == "idle":
                cpu_metrics.idle = value
            elif mode == "iowait":
                cpu_metrics.iowait = value
            elif mode == "irq":
                cpu_metrics.irq = value
            elif mode == "nice":
                cpu_metrics.nice = value
            elif mode == "softirq":
                cpu_metrics.softirq = value
            elif mode == "steal":
                cpu_metrics.steal = value
            elif mode == "system":
                cpu_metrics.system = value
            elif mode == "user":
                cpu_metrics.user = value

        # 获取磁盘状态
        disk_io_time_seconds_total = raw_metrics.get("node_disk_io_time_seconds_total", [])
        for e in disk_io_time_seconds_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.disk_io_time_seconds_total[device] = value

        disk_read_time_seconds_total = raw_metrics.get("node_disk_read_time_seconds_total", [])
        for e in disk_read_time_seconds_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.disk_read_time_seconds_total[device] = value

        disk_write_time_seconds_total = raw_metrics.get("node_disk_write_time_seconds_total", [])
        for e in disk_write_time_seconds_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.disk_write_time_seconds_total[device] = value

        disk_read_bytes_total = raw_metrics.get("node_disk_read_bytes_total", [])
        for e in disk_read_bytes_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.disk_read_bytes_total[device] = value

        disk_written_bytes_total = raw_metrics.get("node_disk_written_bytes_total", [])
        for e in disk_written_bytes_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.disk_written_bytes_total[device] = value

        network_receive_bytes_total = raw_metrics.get("node_network_receive_bytes_total", [])
        for e in network_receive_bytes_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.network_receive_bytes_total[device] = value

        network_transmit_bytes_total = raw_metrics.get("node_network_transmit_bytes_total", [])
        for e in network_transmit_bytes_total:
            value, tags = e
            device = tags.get("device", None)
            if not device:
                continue
            metrics.network_transmit_bytes_total[device] = value

        self._update_metrics(metrics)

    def _update_metrics(self, metrics: RawMetrics):
        if self._last_metrics is None:
            self._last_metrics = metrics
            return

        m = Metrics.model_construct()
        m.tick = metrics.tick

        # 计算启动时间
        m.boot_time_seconds = 0 if metrics.boot_timestamp == 0 else time.time() - metrics.boot_timestamp

        # 单项参数
        m.load1 = metrics.load1
        m.load5 = metrics.load5
        m.load15 = metrics.load15
        m.memory_available_bytes = metrics.memory_available_bytes
        m.memory_total_bytes = metrics.memory_total_bytes
        m.memory_free_bytes = metrics.memory_free_bytes

        # 计算 CPU 占用
        for cpu_id, cpu_metrics in metrics.cpu_seconds_total.items():
            last_cpu_metrics = self._last_metrics.cpu_seconds_total.get(cpu_id, None)
            if last_cpu_metrics is None:
                continue
            delta_total = _total_cpu_seconds(cpu_metrics) - _total_cpu_seconds(last_cpu_metrics)
            delta_idle = cpu_metrics.idle - last_cpu_metrics.idle
            m.cpu_usage_percent[cpu_id] = 100.0 * (delta_total - delta_idle) / delta_total

        # 计算磁盘占用
        for device, value in metrics.disk_read_bytes_total.items():
            last_value = self._last_metrics.disk_read_bytes_total.get(device, None)
            if last_value is None:
                continue
            m.disk_read_bytes_per_second[device] = (value - last_value) / (metrics.tick - self._last_metrics.tick)

        for device, value in metrics.disk_written_bytes_total.items():
            last_value = self._last_metrics.disk_written_bytes_total.get(device, None)
            if last_value is None:
                continue
            m.disk_written_bytes_per_second[device] = (value - last_value) / (metrics.tick - self._last_metrics.tick)

        # 计算网络占用
        for device, value in metrics.network_receive_bytes_total.items():
            last_value = self._last_metrics.network_receive_bytes_total.get(device, None)
            if last_value is None:
                continue
            m.network_receive_bytes_per_second[device] = (value - last_value) / (metrics.tick - self._last_metrics.tick)

        for device, value in metrics.network_transmit_bytes_total.items():
            last_value = self._last_metrics.network_transmit_bytes_total.get(device, None)
            if last_value is None:
                continue
            m.network_transmit_bytes_per_second[device] = (value - last_value) / (metrics.tick - self._last_metrics.tick)

        self._q_out.put(m)
        self._last_metrics = metrics

    def run(self):
        last_time = time.time()
        while not self._stopped:
            # 检查事件
            try:
                event = self._q.get(block=False)
            except Exception:
                event = None
            while event is not None:
                try:
                    if event[0] in self._event_dispatcher:
                        self._event_dispatcher[event[0]](*event[1:])
                except Exception:
                    logging.exception(f"Failed to handle event: {event[0]}")
                try:
                    event = self._q.get(block=False)
                except Exception:
                    event = None

            # 计算时间
            current_time = time.time()
            delta_time = current_time - last_time
            last_time = current_time

            # 刷新数据
            self._refresh_timer += delta_time
            if self._refresh_timer >= self._refresh_interval:
                self._refresh_timer = 0.0
                self._refresh_metrics()

            # 等待时间（10 ticks/s）
            time.sleep(0.1)
        self._q.task_done()
