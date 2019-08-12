## 发布说明

##### 固件版本：

​	0.0.5

##### 修改日期：

​	2019-5-21


## 设备功能

- 一个mesh中可以有多个设备，不过只能有一个主设备，主设备和其他设备的区别是安装有gprs模块，可以提供广域网数据的进出口，除此之外，主设备提供的功能和一般设备相同。
- 每个设备拥有两个风口控制系统，相互独立。
- 同一mesh内的每个设备上的风口有自己的唯一id，不同设备间的风口id不会相同



每个风口控制系统实现以下功能

| 设备功能         | 说明                                                         |
| ---------------- | ------------------------------------------------------------ |
| 自动控温         | 在自动模式下，根据实时温度和设定温度上下限来进行温度控制     |
| 设备按键操作     | 使用按键来设置风口的开启和关闭，选择手动或自动控温模式       |
| 设备远程控制     | 配置参数，控制模式切换，手动控制风口，温度校准，风口校准，触发实时数据上传 |
| 实时状态屏幕显示 | 显示风口控制系统相关信息                                     |



### 自动控温

当温度处于设定温度的上下限之间时，保持风口开启程度不变，否则将控制风口打开或关闭来调节温度（增量式）



### 设备按键操作

##### 当风口控制系统处于手动模式时，可以操作以下按键

按键1短按：风口打开（电机正转）

按键2短按：风口关闭（电机反转）

按键3短按：风口保持不动（电机停止）



##### 按键4手动和自动模式都可用：

按键4短按：切换操作风口

按键4长按：切换风口控制模式（手动或自动）



### 设备远程控制

见通讯协定



### 实时状态屏幕显示

| 信息           | 说明                               |
| -------------- | ---------------------------------- |
| 风口id         | 用以区分当前的显示信息或操作的风口 |
| 风口开度       | 0-100,表示风口打开程度百分比       |
| 当前温度       | ℃                                  |
| 设定温度上下限 | ℃                                  |
| 电机状态       | 正转，反转，停止                   |
| 控制模式       | 手动，自动                         |



## 通讯协定

### 服务器下发

#### 参数配置

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "cfg",
            "Params": {
                "AlarmTempMax": 30,
                "AlarmTempMin": 15,
                "SetTempMax": 30,
                "SetTempMin": 20,
                "TotalTime": 600
            }
        },
        
        {
            "ID": 2,
            "Cmd": "cfg",
            "Params": {
                "AlarmTempMax": 30,
                "AlarmTempMin": 15,
                "SetTempMax": 20,
                "SetTempMin": 30,
				"TotalTime": 100
            }
        }
    ]
}

{"Devs":[{"ID":1,"Cmd":"cfg","Params":{"AlarmTempMax":30,"AlarmTempMin":15,"SetTempMax":31,"SetTempMin":20,"TotalTime":600}},{"ID":3,"Cmd":"cfg","Params":{"AlarmTempMax":30,"AlarmTempMin":15,"SetTempMax":31,"SetTempMin":20,"TotalTime":100}}]}
```


> - `Devices` 目标设备对象数组，可以为有多个目标id
> - `id` 目标设备id，为-1时代表对所有设备生效
> - `AlarmTempMax` 报警高温
> - `AlarmTempMin` 报警低温
> - `SetTempMax ` 设定控制温度上限
> - `SetTempMin` 设定控制温度下限
> - `TotalTime` 风口完整开启或关闭一次所需时间，单位s



#### 手动控制

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "conMan",
            "Params": {
                "Sta": "forward",
                "TimeS": 23
            }
        },
    ]
}

{"Devs":[{"ID":-1,"Cmd":"conMan","Params":{"Sta":"forward","TimeS":60}}]}
{"Devs":[{"ID":1,"Cmd":"conMan","Params":{"Sta":"reverse","TimeS":60}}]}
```

> - `Devices` 目标设备对象数组，可以为有多个目标id
> - `id` 目标设备id，为-1时代表对所有设备生效
>- `MotorSta` 设置的电机状态
>   - `forward` 正转
>   - `reverse` 反转
>   - `stop` 停止
> - `TimeS` 持续时间




#### 控制模式

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "conMode",
            "Params": {
                "Sta": "auto"
            }
        }
    ]
}

{"Devs":[{"ID":3,"Cmd":"conMode","Params":{"Sta":"auto"}}]}
{"Devs":[{"ID":-1,"Cmd":"conMode","Params":{"Sta":"auto"}}]}
{"Devs":[{"ID":-1,"Cmd":"conMode","Params":{"Sta":"manual"}}]}
```

> - `Devices` 目标设备对象数组，可以为有多个目标id
> - `id` 目标设备id，为-1时代表对所有设备生效
>
> - `ControlSta`  当前控制模式
>   - `auto` 自动模式
>   - `manual` 手动模式



#### 风口校准

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "openAdjust",
            "Params": {
                "Openper": 0
            }
        }
    ]
}
{"Devs":[{"ID":-1,"Cmd":"openAdjust","Params":{"Openper":15}}]}
```

> `Devices` 目标设备对象数组，可以为有多个目标id
>
> `id` 目标设备id，为-1时代表对所有设备生效
>
> `Openper`实际风口打开程度百分比




#### 温度校准

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "tempAdjust",
            "Params": {
                "Temp": 25
            }
        }
    ]
}
{"Devs":[{"ID":3,"Cmd":"tempAdjust","Params":{"Temp":18}}]}
```

> `Devices` 目标设备对象数组，可以为有多个目标id
>
> `id` 目标设备id，为-1时代表对所有设备生效
>
> `temp` 实际温度（整数）




#### 触发实时数据上传

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "getInfo",
            "Params": {
            }
        }
    ]
}

{"Devs":[{"ID":-1,"Cmd":"getInfo","Params":{}}]}
```

> `Devices` 目标设备对象数组，可以为有多个目标id
>
> `id` 目标设备id，为-1时代表对所有设备生效


#### ota升级

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "OAT",
            "Params": {
				"table_size": 1024,
				"name": "helloworld.bin"
            }
        }
    ]
}

{"Devs":[{"ID": 1,"Cmd": "OAT","Params": {"table_size": 1024, "name": "helloworld.bin"}}]}
```

### 设备上传

#### 实时数据

```json
{
    "Typ":"fan",
    "ID": 1,
    "Cmd": "Info",
    "Params": {
        "NTemp": 22,
        "OpenPer": 50,
        "ConSta": "manual",
        "MoSta": "stop"
    }
}
{"Devs":[{"Typ":"fan","ID": 1,"Cmd": "Info","Params":{"NTemp": 15,"OpenPer": 38,"ConSta": "manual","MoSta": "stop"}},{"Typ":"fan","ID": 2,"Cmd": "Info","Params": {"NTemp": 15,"OpenPer": 100,"ConSta": "manual","MoSta": "stop"}},{"Typ":"fan","ID": 5,"Cmd": "Info","Params": {"NTemp": 15,"OpenPer": 0,"ConSta": "manual","MoSta": "stop"}},{"Typ":"fan","ID": 6,"Cmd": "Info","Params": {"NTemp": 15,"OpenPer": 0,"ConSta": "manual","MoSta": "stop"}}]}

{"Devs":[{"Typ":"fan","ID": 1,"Cmd": "Info","Params": {"NTemp": 15,"OpenPer": 38,"ConSta": "manual","MoSta": "stop"}},{"Typ":"fan","ID": 2,"Cmd": "Info","Params": {"NTemp": 15,"OpenPer": 100,"ConSta": "manual","MoSta": "stop"}}]}

```

> - `Typ` 设备类型
>   - `fan` 放风机
> - `NTemp`实时温度
> - `OpenPer`风口当前打开程度百分比
> - `ConSta`当前控制模式
>   - `auto` 自动模式
>   - `manual` 手动模式
> - `MoSta`电机状态
>   - `forward` 正转
>   - `reverse` 反转
>   - `stop` 停止

#### 温度报警

```json
{
    "Devs": [
        {
            "ID": 1,
            "Cmd": "tempAlarm",
            "Params": {
                "Temp": 25
            }
        }
    ]
}

{"Devs":[{"ID": 6,"Cmd": "tempAlarm","Params": {"Temp": 25}}]}
{"Devs":[{"ID": 1,"Cmd": "tempAlarm","Params": {"Temp": 18}}]}
{"Devs":[{"ID": 1,"Cmd": "OAT","Params": {"table_size": 1024, "name": "helloworld.bin"}}]}

```


> `Devices` 目标设备对象数组，可以为有多个目标id
>
> `id` 目标设备id，为-1时代表对所有设备生效
