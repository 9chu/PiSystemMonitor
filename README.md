# 树莓派系统状态监视器

基于`Python`开发，通过读取`node_exporter`导出信息来展示系统状态。

适用于 480x320 分辨率的屏幕。

## 安装

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

## 启动

```bash
export METRICS_URL='http://your-ip:9100/metrics'
python3 main.py
```

### 树莓派自动启动

```bash
cp startup.sh.default startup.sh
chmod +x startup.sh
# modify startup.sh as you need
mkdir -p ~/.config/autostart
cat <<EOF > ~/.config/autostart/PiSystemMonitor.desktop
[Desktop Entry]
Type=Application
Name=PiSystemMonitor
Exec=/where_this_project_stores/startup.sh
EOF
```

## 使用字体

- [FrozenCrystalBold](https://www.fontspace.com/frozen-crystal-font-f33002)
