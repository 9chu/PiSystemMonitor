# 树莓派服务器状态监视器 硬件说明

## 物料清单

| 名称                 | 数量 | 参考购买连接                                                 |
| -------------------- | ---- | ------------------------------------------------------------ |
| 树莓派 Zero 2W | 1    | https://item.taobao.com/item.htm?_u=11lodlan0d5f&id=693613248231&spm=a1z09.2.0.0.8c2a2e8dlMaT7J |
| 微雪 3.5寸 LCD 显示屏 高速 SPI / RPI LCD (C) | 1 | https://item.taobao.com/item.htm?_u=11lodlane72d&id=582719402545&spm=a1z09.2.0.0.8c2a2e8dlMaT7J&skuId=5124253687047 |
| SD 存储卡, 不低于 8GB | 1 |  |
| CPU 纯铜散热片 | 2 | https://item.taobao.com/item.htm?_u=11lodlanc6d7&id=637300256584&spm=a1z09.2.0.0.8c2a2e8dlMaT7J |
| Micro USB 延长线 V8ST-V8F, 0.05m | 1 | https://detail.tmall.com/item.htm?_u=11lodlanb90f&id=669503549747&spm=a1z09.2.0.0.8c2a2e8dlMaT7J |
| 圆柱型磁铁 直径 4mm 厚 2.5mm | 12 | https://item.taobao.com/item.htm?_u=11lodlan1698&id=599078234549&spm=a1z09.2.0.0.8c2a2e8dlMaT7J |

## 外壳文件

- 源文件
	- solidworks/PiSystemMonitor.SLDPRT：壳体源文件
	- solidworks/PiSystemMonitor - Hat.SLDPRT：顶盖源文件
- 壳体模型文件
	- stl/PiSystemMonitor.stl：导出的壳体模型，可以导入切片软件使用
	- stl/PiSystemMonitor - Hat.STL：导出的顶盖模型，可以导入切片软件使用
- 拓竹打印工程
	- PiSystemMonitor.3mf：bamboo studio 工程文件

## 树莓派 zero 2w 基础配置过程

### 准备工作

1. 下载 [Raspberry Pi OS with desktop](https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-64-bit)

2. 编译`waveshare35c.dts`（若使用预编译的`waveshare35c.dtbo`文件可以跳过本步骤）

   ```bash
   dtc -I dts -O dtb -o waveshare35c.dtbo waveshare35c.dts
   ```

### 配置 SD 卡

1. 在 PC 上烧录镜像（略）

2. 重新插入 SD 卡，编辑`bootfs`分区 

3. 在`bootfs`分区根目录创建自动配置文件`custom.toml`，参考下述配置：

   ```toml
   # Raspberry PI OS config.toml
   # This file is used for the initial setup of the system on the first boot, if
   # it's s present in the boot partition of the installation.
   #
   # This file is loaded by firstboot, parsed by init_config and ends up
   # as several calls to imager_custom.
   # The example below has all current fields.
   #
   # References:
   # - https://github.com/RPi-Distro/raspberrypi-sys-mods/blob/master/usr/lib/raspberrypi-sys-mods/firstboot
   # - https://github.com/RPi-Distro/raspberrypi-sys-mods/blob/master/usr/lib/raspberrypi-sys-mods/init_config
   # - https://github.com/RPi-Distro/raspberrypi-sys-mods/blob/master/usr/lib/raspberrypi-sys-mods/imager_custom
   
   # Required:
   config_version = 1
   
   [system]
   hostname = "rpi0"  # 如有需要，修改主机名称
   
   [user]
   # If present, the default "rpi" user gets renamed to this "name"
   name = "rpi"  # 如有需要，修改用户名
   # The password can be encrypted or plain. To encrypt, we can use "openssl passwd -5 raspberry"
   password = "raspberrypi"  # 配置密码
   password_encrypted = false
   
   [ssh]
   # ssh_import_id = "gh:user" # import public keys from github
   enabled = true
   password_authentication = true
   # We can also seed the ssh public keys configured for the default user:
   # authorized_keys = [ "ssh-rsa ... user@host", ... ]
   
   [wlan]
   ssid = "mywifi"  # 配置 WIFI SSID
   password = "12345678910"  # 配置 WIFI 密码
   password_encrypted = false
   hidden = false
   # The country is written to /etc/default/crda
   # Reference: https://wireless.wiki.kernel.org/en/developers/Regulatory
   country = "CN"  # 根据需要调整 WIFI 对应的国家代码
   
   [locale]
   keymap = "gb"
   timezone = "Asia/Shanghai"  # 根据需要调整时区
   ```

4. 拷贝编译好的`waveshare35c.dtbo`到`overlays`文件夹中

5. 修改`config.txt`，参考如下：

   ```txt
   # For more options and information see
   # http://rptl.io/configtxt
   # Some settings may impact device functionality. See link above for details
   
   # Uncomment some or all of these to enable the optional hardware interfaces
   #dtparam=i2c_arm=on
   #dtparam=i2s=on
   #dtparam=spi=on
   
   # Enable audio (loads snd_bcm2835)
   dtparam=audio=on
   
   # Additional overlays and parameters are documented
   # /boot/firmware/overlays/README
   
   # Automatically load overlays for detected cameras
   camera_auto_detect=1
   
   # Automatically load overlays for detected DSI displays
   display_auto_detect=1
   
   # Automatically load initramfs files, if found
   auto_initramfs=1
   
   # Enable DRM VC4 V3D driver
   #dtoverlay=vc4-kms-v3d  # 关闭 3D 加速支持，否则可能进不了桌面
   max_framebuffers=2
   
   # Don't have the firmware create an initial video= setting in cmdline.txt.
   # Use the kernel's default instead.
   disable_fw_kms_setup=1
   
   # Run in 64-bit mode
   arm_64bit=1
   
   # Disable compensation for displays with overscan
   disable_overscan=1
   
   # Run as fast as firmware / board allows
   arm_boost=1
   
   [cm4]
   # Enable host mode on the 2711 built-in XHCI USB controller.
   # This line should be removed if the legacy DWC2 controller is required
   # (e.g. for USB device mode) or if USB support is not required.
   otg_mode=1
   
   [cm5]
   dtoverlay=dwc2,dr_mode=host
   
   [all]
   dtoverlay=waveshare35c  # 添加 LCD 屏的设备描述
   ```

   经过尝试，现有的 DRM 驱动（`tinydrm`/`panel-mipi-dbi-spi`）未能正常点亮该型号 LCD 屏（可能为 vc4 驱动的兼容性问题），故需要关闭 3D 加速防止出现问题。

### 配置系统

插入 SD 卡后等待一段时间，不出意外可以通过 `SSH` 连接到 `Zero 2W`，而后进行下述配置：

1. 通过 `raspi-config` 扩展根文件系统、配置 locale 等

2. 配置并扩展交换空间到 1 GB，修改 `/etc/dphys-swapfile`：

   ```
   CONF_SWAPSIZE=1024
   ```

   由于 zero 2w 内存较小，此举用以防止安装 pyimgui 导致 OOM。

3. 配置`x11`，参考 LCD 屏官方教程，创建文件 `/etc/X11/xorg.conf.d/99-calibration.conf`：

   ```
   Section "Device"
   # WaveShare SpotPear 3.5", framebuffer 1
   Identifier "uga"
   driver "fbdev"
   Option "fbdev" "/dev/fb1"
   Option "ShadowFB" "off"
   EndSection
   
   Section "Monitor"
   # Primary monitor. WaveShare SpotPear 480x320
   Identifier "WSSP"
   EndSection
   
   Section "Screen"
   Identifier "primary"
   Device "uga"
   Monitor "WSSP"
   EndSection
   
   Section "ServerLayout"
   Identifier "default"
   Screen 0 "primary" 0 0
   EndSection
   ```

4. （可选）解决 [WiFi 连接不稳定问题](https://forums.raspberrypi.com/viewtopic.php?t=372254)，创建 `/etc/modprobe.d/brcmfmac.conf`：

   ```conf
   options brcmfmac roamoff=1 feature_disable=0x82000
   ```

## 装配

装配较为简单，部分地方可以涂502（micro usb口、磁铁孔），具体过程略。

需要注意外壳顶盖之间的磁铁正反。