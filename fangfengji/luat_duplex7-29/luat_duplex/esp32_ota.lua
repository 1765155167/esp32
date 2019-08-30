require "sim"
require "misc"
require "utils"
require "pm"
require "duplex"
require "httpv2"


esp32_ota = {}
esp32_len = 0
-- cut_url = "http://lumoschen.cn:2201/get_cut"
cut_url = "http://101.132.42.189:8070"
-- cut_url = "https//:www.baidu.com"
download_bin = "hello-world.bin"

local function wait_socket()
    while true do
        if socket.isReady() then 
            break
        end
        sys.wait(3000)
    end
end

local flage = true
local num = 0
--http 回调函数
function myhttpcb(str, len)
    if(flage)then
        flage = false
        esp32_len = len
    end
    table.insert(esp32_ota, str)
    -- log.info("data str",string.format("%x%x %x%x %x%x %x%x",str[0],str[1],str[2],str[3],str[#str - 4],str[#str - 3],str[#str - 2],str[#str - 1]))
    esp32_len = esp32_len - #str
    log.info("len_str:",#str,esp32_len)
end


function http_post(arg)
    sys.waitUntil("GET_BODY", 100)
    log.info("post arg: ", arg)
    a,b,c = httpv2.request("GET", cut_url.."/"..download_bin, nil, nil, nil, nil, nil, nil, nil, myhttpcb)
    print("return:................................",a,b,c)
    res = sys.waitUntil("GET_BODY", 15000)
end

local function send_ota()
    while true do
        a = table.remove(esp32_ota,1)
        local num = 0
        if a ~= nil then
            res = duplex.sendbin(a,500)
            while not res do
                num = num + 1
                if num >= 4 then
                    log.info("send bin error")
                    break
                end
                res = duplex.sendbin(a,500)
            end
        else
            sys.wait(500)
        end
    end
end

local function loop_task()
    wait_socket()
    while true do
        local str = duplex.recv_ota()
        if str then
            http_post(str) 
        end
    end
end
sys.taskInit(send_ota)
sys.taskInit(loop_task)
