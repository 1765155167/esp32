[[EN]](./README.md)

# Mupgrade 示例

## 介绍
本示例将介绍如何快速使用 `Mupgrade` 进行 ESP-MESH 设备的升级。

[Mesh Upgrade 介绍](https://docs.espressif.com/projects/esp-mdf/en/latest/api-guides/mupgrade.html)

## 工作流程

### 步骤 1：连接到路由器

将主机 PC 连接到 ESP-MESH 网络所在的路由器。

### 步骤 2：运行 HTTP 服务器

### 步骤 3：构建 OTA 示例

切换回 OTA 示例目录，然后键入 `make menuconfig`(Make) 或者 `make menuconfig`(CMake) 以配置 OTA 示例。在 “Example Configuration” 子菜单下，填写以下详细信息：

* ESP-MESH 网络的配置信息
* 固件升级 URL。URL 将如下所示：

```
https://<host-ip-address>:<host-port>/<firmware-image-filename>

for e.g,
https://101.132.42.189:8070/hello-world.bin
```
保存更改，然后键入 `make`(Make) 或者 `idf.py`(CMake) 来构建示例。

## 故障排除

* 检查您的 PC 是否能够 ping 通 ESP32，同时检查 IP，路由器及 menuconfig 中的其他配置是否正确
* 检查是否有防火墙软件阻止 PC 上的连接操作
* 通过查看以下命令日志，检查是否能够看到配置文件 (default hello-world.bin)

 ```
 curl -v https：// <host-ip-address>：<host-port> / <firmware-image-filename>
 ```

* 如果您有其他 PC 或手机，尝试浏览不同主机所列文件

### errors “ota_begin error err = 0x104”

如果您遇到此项报错，请检查分区表中配置的（及实际的）flash 分区大小是否满足要求。默认的“双 OTA 分区”分区表仅提供 4 MB flash 分区。若 flash 分区大小不能满足 OTA 升级要求，您可以创建自定义分区表 CSV （查看 components/partition_table），并在 menuconfig 中进行配置。

## 注意

为了让更多的节点加入到mesh网络中进行固件升级，在示例中我们将在 10 秒后开始进行固件升级。

# MQTT
1. ESP-MESH 设备每隔三秒会向 Topic:"/topic/subdev/MAC/send"（MAC：节点的 MAC 地址）推送设备信息
2. 当 MESH 网络中路由表发生变化时，会向 Topic:"/topic/gateway/MAC/update"（MAC：为根节点的 MAC 地址）推送变化的节点相关信息
3. 可以从 Topic:"/topic/subdev/MAC/recv" 接收来自服务器的数据

例如：
- MQTT 测试工具中订阅 `/topic/subdev/240ac4085480/send` topic 将收到来自设备的数据
- 在 MQTT 测试工具中向 `/topic/subdev/240ac4085480/recv` topic 发送数据，MAC 地址为 240ac4085480 的设备将收到数据

## 订阅消息
1. mosquitto_sub -h 101.132.42.189 -t "/topic/subdev/30aea4ddb020/send" -v
- 使用通配符一次订阅多个消息
2. mosquitto_sub -h 101.132.42.189 -t "/topic/subdev/+/send" -v

## 发布消息
- mosquitto_pub -h 101.132.42.189 -t "/topic/subdev/30aea4ddb020/recv" -m "{\"ID\": 1, \"Cmd\": \"ONLED\"}"

## 上传灯的状态信息
```
{"layer":1,"Devs":[{"ID":1,"Flag":"On"},{"ID":2,"Flag":"On"},{"ID":3,"Flag":"On"},{"ID":4,"Flag":"On"}]}
```

# 擦除配网信息
- 同时按下4个按键超过10s,则擦除配网信息，并重启设备

# JSON 指令
## OTA升级
```json
{"ID":1,"Cmd":"OTA"} 
```
- 注意要发给根节点设备,才能进行升级
## 开灯
```json
{"ID": 1, "Cmd": "ONLED"}
{"ID": 1, "Cmd": "OFFLED"}
```
## 定时开灯
```json
{"ID": 1, "Cmd": "ONLEDT", "Time":60}
{"ID": 1, "Cmd": "OFFLEDT", "Time":60}
```
# 项目介绍
1. 此设备现在可以通过4个按键来(独立)控制4个灯,
2. 通过MQTT指令来控制4个灯
3. 通过MQTT指令进行OTA升级 发送指令后设备直接向服务器(前面已经配置好)下载固件并升级
4. 通过乐鑫的Esp-mesh 安卓app来配置网络信息(路由器SSID，路由器密码，MESH ID)，配好后将这些信息保存在flash中,下次开机无需配置，如果需要重新配置,请同时按下4个按键超过10s时间,设备将会重启。
5. 固件位置 http服务器:http://101.132.42.189:8070/hello-world.bin
6. MQTT服务器:101.132.42.189 端口1883