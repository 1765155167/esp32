# esp32
myproject-esp32
# git指令
## 添加
- git add .
- git commit -m "v2.0 add key http mesh etc."
## 上传
- git push origin master
## 历史记录
- git log
# 目录结构
1. espnow
2. fangfengji 方风机代码
    - [[ESP32程序]](./fangfengji/esp32)
    - [[Air202程序]](./fangfengji/luat_duplex7-29)
    - [[Stm32程序]](./fangfengji/Stm32)
3. gpio　
4. http_request
5. key
6. led_control_mconfig　灯控(mesh app配网，MQTT, MUPGRADE升级)
7. mconfig (mesh app　配网历程)
8. mdf-mesh　(mesh组网)
9. mqtt_example　(MQTT)
10. mupgrade
    1. mupgrade1 (mupgrade 升级　串口接受固件方式)
    2. mupgrade2 (mupgrate 升级　连接路由器并将固件通过串口发送出去)
11. no_router (无路由器mesh组网方式)
12. onOff 开关
    1. on_off_mconfig  (开关esp-mesh app配网)
    2. on_off_smconfig　(开关+smconfig配网)
13. README.md (本文件)
14. serial (串口)
15. smart_config (smartconfig 配网)
16. tcp
17. wifi