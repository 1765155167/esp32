--- 模块功能：MQTT客户端处理框架
-- @author openLuat
-- @module mqtt.mqttTask
-- @license MIT
-- @copyright openLuat
-- @release 2018.03.28

module(...,package.seeall)

require"common"
require"misc"
require"mqtt"
require"sim"
require"utils"
require"pm"
require"duplex"


local base = _G

local SUB_TO_NEWS = "abcall/2222/deviceidrx"
local POSTED_NEWS = "abcall/2222/00000000000000000000000000000000/deviceid"

local telno = "00000000000000000000000000000000"

local TYPE_REQUEST = 102		--请求
local TYPE_RESPONSE = 201		--响应

local SCL_CMD_FORE = "AC03"     --正转
local SCL_CMD_BACK = "NS02"     --反转
local SCL_CMD_STOP = "ST06"		--停止
local SCL_CMD_READ = "RD08"		--读取
local SCL_CMD_BIND = "BD05"		--绑定
local SCL_CMD_LOCK = "LC09"		--锁定
local SCL_CMD_UPDT = "UP04"		--升级
local SCL_CMD_HEART = "HB07"    --心跳
local SCL_CMD_UNBIND = "UB10"   --解绑   

local REG_TIME = 86400       --register上报时间
local T_THR_VAL = 7200        --时长阈值
local SIG_TIME = 3600         --信号强度上传时间

local ssub,schar,smatch,sbyte,slen = string.sub,string.char,string.match,string.byte,string.len
local mqttClient
local ready = false
post_flag = false            --注册后接受信息

Content =
{
	cmd = "command",
	data = "nothing"
}
local jsoncontent = json.encode(Content)

local Register =
{
	DeviceId = "deviceId",
	Imei = "862952021508519",
	Iccid = "iccid",
	Devicetmp = "TempCtrl",
	firmwareRev = tostring(base.VERSION)
}
local jsondRegister = json.encode(Register)


--数据发送的消息队列
local msgQueue = {}

function insertMsg(topic,payload,qos,user)
    table.insert(msgQueue,{t=topic,p=payload,q=qos,user=user})
	--outMsg = table.remove(msgQueue,1)
	--result = mqttClient:publish(outMsg.t,outMsg.p,outMsg.q)
end

--mqtt上传esp32的json信息，回调函数
local function readcb(result)
	log.info("readcb",result)
end

--mqtt上传esp32的json信息
function post_news(data)
	--data = json.encode(data)
	testio.readfile(testio.USER_DIR_PATH.."/user.txt")
	if testio.fileval ~= nil
	then
		telno = testio.fileval
	end
	POSTED_NEWS = "abcall/2222/" .. telno .. "/dev" .. crypto.md5(misc.getImei(),slen(misc.getImei()))
	informations = TYPE_RESPONSE .. "|" .. "dev"..crypto.md5(misc.getImei(),slen(misc.getImei())) .. "|" ..  telno .. "|" .. os.time() .. "000" .. "|" .. data
	insertMsg(POSTED_NEWS,informations,0,{cb=readcb})
end


local function sendcb(result)
	log.info("sendcb",result)
end

function post_new()
	--data = json.encode(data)
	testio.readfile(testio.USER_DIR_PATH.."/user.txt")
	if testio.fileval ~= nil
	then
		telno = testio.fileval
	end
	POSTED_NEWS = "abcall/2222/" .. telno .. "/dev" .. crypto.md5(misc.getImei(),slen(misc.getImei()))
	jsoncontent = json.encode(Content)
	informations = TYPE_RESPONSE .. "|" .. "dev"..crypto.md5(misc.getImei(),slen(misc.getImei())) .. "|" ..  telno .. "|" .. os.time() .. "000" .. "|" .. jsoncontent
	insertMsg(POSTED_NEWS,informations,0,{cb=sendcb})
end

local function pubQos0TestCb(result)
    log.info("mqttOutMsg.pubQos0TestCb",result)
    if result then sys.timerStart(pubQos0Test,60000) end
end

function pubQos0Test()
	POSTED_NEWS = "abcall/2222/" .. telno .. "/dev" .. crypto.md5(misc.getImei(),slen(misc.getImei()))
    insertMsg(POSTED_NEWS,"0",0,{cb=pubQos0TestCb})
end

--air202信号强度回调函数，并一个小时发送一次
local function pubQos1TestCb(result)
    log.info("mqttOutMsg.pubQos1TestCb",result)
    if result then sys.timerStart(pubQos1Test,SIG_TIME * 1000) end
end

--air202信号强度
function pubQos1Test()
	POSTED_NEWS = "abcall/2222/" .. telno .. "/dev" .. crypto.md5(misc.getImei(),slen(misc.getImei()))
	Content["cmd"] = SCL_CMD_HEART
	Content["data"] = "signal:"..net.getRssi()
	jsoncontent = json.encode(Content)
	information = TYPE_RESPONSE .. "|" .. "dev"..crypto.md5(misc.getImei(),slen(misc.getImei())) .. "|" ..  telno .. "|" .. os.time() .. "000" .. "|" .. jsoncontent
    insertMsg(POSTED_NEWS,information,1,{cb=pubQos1TestCb})
end

--air202注册信息回调函数，并一天上报一次
local function registercb(result)
    log.info("mqttOutMsg.pubQos1TestCb",result)
    if result then sys.timerStart(register,REG_TIME * 1000) end
end

--air202注册信息
function register()
	Register["DeviceId"] = "dev"..crypto.md5(misc.getImei(),slen(misc.getImei()))
	Register["Imei"] = tostring(misc.getImei())
	Register["Iccid"] = tostring(sim.getIccid())
	jsondRegister = json.encode(Register)
	POSTED_NEWS = "abcall/2222/" .. telno .. "/dev" .. crypto.md5(misc.getImei(),slen(misc.getImei()))
	Registermation = TYPE_RESPONSE .. "|" .. "dev"..crypto.md5(misc.getImei(),slen(misc.getImei())) .. "|" ..  telno .. "|" .. os.time() .. "000" .. "|" .. jsondRegister
	insertMsg(POSTED_NEWS,Registermation,0,{cb=registercb})
	post_flag = true
end

--- 初始化“MQTT客户端数据发送”
-- @return 无
-- @usage mqttOutMsg.init()
function init()
	register()
    pubQos0Test()
    pubQos1Test()
end

--- 去初始化“MQTT客户端数据发送”
-- @return 无
-- @usage mqttOutMsg.unInit()
function unInit()
    sys.timerStop(pubQos0Test)
    sys.timerStop(pubQos1Test)
    while #msgQueue>0 do
        local outMsg = table.remove(msgQueue,1)
        if outMsg.user and outMsg.user.cb then outMsg.user.cb(false,outMsg.user.para) end
    end
end

--- MQTT客户端是否有数据等待发送
-- @return 有数据等待发送返回true，否则返回false
-- @usage mqttOutMsg.waitForSend()
function waitForSend()
    return #msgQueue > 0
end

--- MQTT客户端数据发送处理
-- @param mqttClient，MQTT客户端对象
-- @return 处理成功返回true，处理出错返回false
-- @usage mqttOutMsg.proc(mqttClient)
function mqttOutMsg(mqttClient)
	log.info("----------------mqtt wait message---------------")
    while #msgQueue>0 do
        local outMsg = table.remove(msgQueue,1)
		log.info("mqtt public: ",outMsg.t)
		log.info("mqtt payload: ",outMsg.p)
        local result = mqttClient:publish(outMsg.t,outMsg.p,outMsg.q)
        if outMsg.user and outMsg.user.cb then outMsg.user.cb(result,outMsg.user.para) end
        if not result then return end --发送失败
    end
    return true
end

--获取esp32的json信息
function mqtt_send()
	local json_data
	while true do
		json_data = duplex.recv()
		log.info("json_data: ",json_data)
		post_news(json_data)
	end
end

--- MQTT客户端数据接收处理
-- @param mqttClient，MQTT客户端对象
-- @return 处理成功返回true，处理出错返回false
-- @usage mqttInMsg.proc(mqttClient)
function mqttInMsg(mqttClient)
    local result,data,res
    while true do
        result,data = mqttClient:receive(2000)
        --接收到数据
        if result then
            log.info("mqtt date: ",data.topic,data.payload)
			if post_flag == true then
				if data.topic == SUB_TO_NEWS then    --收到规定的topic主题信息
					print("message: ", data.payload)
					local protocolType1 = "%d%d%d"
					local protocolType = tonumber(string.sub(data.payload, string.find(data.payload, protocolType1)))
					log.info("protocolType: ",protocolType)
					
					if protocolType == TYPE_REQUEST then     --请求
						local command1 = "\"Cmd\":\"%w%w%w%w\""
						local num1 = nil
						local num2 = nil 
						num1,num2 = string.find(data.payload, command1)
						print("message1")
						if num1 ~= nil then
							print("message2")
							local command2 = string.sub(data.payload,num1,num2)
							local command3 = "%w%w%w%w"
							local command = string.sub(command2, string.find(command2, command3))
							log.info("command: ",command)
							
							if command == SCL_CMD_BIND then      --绑定
								local telno1 = "\"op\":\"%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w\""
								local telno2,telno3 = nil,nil
								telno2,telno3 = string.find(data.payload,telno1)
								if telno2 ~= nil
								then
									--print("ooooooooooooooooooooooo")
									local telno4 = string.sub(data.payload,telno2,telno3)
									local telno5 = "%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w%w"
									local telno6 = string.sub(telno4, string.find(telno4,telno5))
									if rtos.make_dir(testio.USER_DIR_PATH)
									then
										testio.writevalw(testio.USER_DIR_PATH.."/user.txt",telno6)
									end
									Content["cmd"] = "BD05"
									Content["data"] = "bind"
									post_new()
								else
									Content["cmd"] = "BD05"
									Content["data"] = "error"
									post_new()
								end

							elseif command == SCL_CMD_UNBIND then      --解绑
								--print("4444444444444444444444444444")
								local telno7 = "00000000000000000000000000000000"
								if rtos.make_dir(testio.USER_DIR_PATH)
								then
									testio.writevalw(testio.USER_DIR_PATH.."/user.txt",telno7)
								end
								Content["cmd"] = "UB10"
								Content["data"] = "unbind"
								post_new()
							else
								local x = string.sub(data.payload, string.find(data.payload, "{"), string.find(data.payload, "}!$#"))
								print("-----------", x)
								local num = 0
								res = duplex.sendstr(x, 500)
								while not res do
									num = num + 1
									if num >= 4
									then
										log.info("esp32 error")
										break
									end
									res = duplex.sendstr(x, 500)
								end
							end
						elseif num1 == nil then
							local x = string.sub(data.payload, string.find(data.payload, "{"), string.find(data.payload, "}!$#"))
							print("-----------", x)
							local num = 0
							res = duplex.sendstr(x, 500)
							while not res do
								num = num + 1
								if num >= 4
								then
									log.info("esp32 error")
									break
								end
								res = duplex.sendstr(x, 500)
							end
						end
					end
				end
			end
			--send_esp32(data.payload)
			
			
            --write(data.payload)
            --TODO：根据需求自行处理data.payload
            
            --如果mqttOutMsg中有等待发送的数据，则立即退出本循环
            if waitForSend() then return true end
        else
            break
        end
    end
	
    return result or data=="timeout"
end


--- MQTT连接是否处于激活状态
-- @return 激活状态返回true，非激活状态返回false
-- @usage mqttTask.isReady()
function isReady()
    return ready
end

--启动MQTT客户端任务
sys.taskInit(
    function()
        local retryConnectCnt = 0
        while true do
            if not socket.isReady() then
                retryConnectCnt = 0
				log.info("wait net")
				print("wait net")
                --等待网络环境准备就绪，超时时间是5分钟
                sys.waitUntil("IP_READY_IND",300000)
            end
			--订阅主题
			SUB_TO_NEWS = "abcall/2222".."/dev".. crypto.md5(misc.getImei(),slen(misc.getImei())).."rx"
            --print("sub", SUB_TO_NEWS)
			--遗嘱主题
			will = "abcall/3333/" .. telno .. "/dev" .. crypto.md5(misc.getImei(),slen(misc.getImei())).."rx"
			local user1 = "dev" .. crypto.md5(866289034915910,slen(866289034915910))
			print("123456",user1)
			local paw1 = string.sub(user1,-6,string.len(user1))
			print("123456",paw1)
			local password1 = crypto.md5(user1 .. paw1,slen(user1 .. paw1))
			print("123456",password1)
			
			local user = "dev" .. crypto.md5(misc.getImei(),slen(misc.getImei()))
			print("123456",user)
			local paw = string.sub(user,-6,string.len(user))
			print("123456",paw)
			local password = crypto.md5(user .. paw,slen(user .. paw))
			print("123456",password)
            if socket.isReady() then
                local imei = misc.getImei()
                --创建一个MQTT客户端
                mqttClient = mqtt.client(imei,60,user,password,nil,{qos=0, retain=0, topic=will, payload="will_end"})
				log.info("mqtt client")
                --阻塞执行MQTT CONNECT动作，直至成功
                --如果使用ssl连接，打开mqttClient:connect("lbsmqtt.airm2m.com",1884,"tcp_ssl",{caCert="ca.crt"})，根据自己的需求配置
                --mqttClient:connect("lbsmqtt.airm2m.com",1884,"tcp_ssl",{caCert="ca.crt"})
                --if mqttClient:connect("lbsmqtt.airm2m.com",1883,"tcp") then
				if mqttClient:connect("www.abcall.cn",8883,"tcp_ssl",{}) then
					log.info("mqtt connect")
                    retryConnectCnt = 0
                    ready = true
                    --订阅主题: mqttClient:subscribe({["/event0"]=0, ["/中文event1"]=1})
                    if mqttClient:subscribe({[SUB_TO_NEWS]=0}) then
                        init()
                        --循环处理接收和发送的数据
                        while true do
                            if not mqttInMsg(mqttClient) then log.error("mqttTask.mqttInMsg.recv error") break end
                            if not mqttOutMsg(mqttClient) then log.error("mqttTask.mqttOutMsg send error") break end
                        end
                        unInit()
                    end
                    ready = false
                else
                    retryConnectCnt = retryConnectCnt+1
                end
                --断开MQTT连接
                mqttClient:disconnect()
                if retryConnectCnt>=5 then link.shut() retryConnectCnt=0 end
                sys.wait(5000)
            else
                --进入飞行模式，20秒之后，退出飞行模式
                net.switchFly(true)
                sys.wait(20000)
                net.switchFly(false)
            end
        end
    end
)
sys.taskInit(mqtt_send)
