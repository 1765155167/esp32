# esp32
myproject-esp32
# 添加
git add .
git commit -m "v2.0 add key http mesh etc."
# 上传
git push origin master
# 历史记录
git log
# 目录结构
- espnow
- fangfengji 方风机代码
- gpio　
- http_request
- key
- led_contrl 灯控
- led_control_mconfig　灯控(mesh app配网，MQTT, MUPGRADE升级)
- mconfig (mesh app　配网历程)
- mdf-mesh　(mesh组网)
- mqtt_example　(MQTT)
- mupgrade1 (mupgrade 升级　串口接受固件方式)
- mupgrade2 (mupgrate 升级　连接路由器并将固件通过串口发送出去)
- no_router (无路由器mesh组网方式)
- on_off (开关)
- on_off_smconfig　(开关+smconfig配网)
- README.md (本文件)
- serial (串口)
- smart_config (smartconfig 配网)
- tcp
- wifi