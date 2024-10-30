# 树莓派系统状态监视器

基于`Python`开发，通过读取`node_exporter`导出信息来展示系统状态。

使用`SDL2` + `imgui`渲染`UI`，允许在`kiosk`模式下运行。

适用于 480x320 分辨率的屏幕。

## 安装

```bash
pip install -r requirements.txt
```

## 启动

```bash
export METRICS_URL='http://your-ip:9100/metrics'
python3 main.py
```

## 使用字体

- [FrozenCrystalBold](https://www.fontspace.com/frozen-crystal-font-f33002)
