#include "serial.h"

#define STD_LOCAL_LOG_LEVEL STD_LOG_INFO

#define EX_UART_NUM UART_NUM_1
#define BAND_RATE 115200
#define MODUS_TX 12
#define MODUS_RX 14
#define RD_BUF_SIZE (2048)
#define RECV_PEROID_MS 10
#define RECV_QUEUE_SIZE 8
#define EVENT_QUENE_LENGTH 10

static xTimerHandle g_recv_timer;
static xQueueHandle g_recv_queue;
static xQueueHandle g_recv_self_queue;
static xQueueHandle g_uart_queue;
static uint8_t g_id;
static xSemaphoreHandle g_send_lock;

static void send_lock()
{
    xSemaphoreTake(g_send_lock, portMAX_DELAY);
}

static void send_unlock()
{
    xSemaphoreGive(g_send_lock);
}

static void uart_event_task(void *pvParameters)
{
    STD_LOGD("uart event task start");
    uart_event_t event;
    for(;;) {
        //Waiting for UART event.
        if(xQueueReceive(g_uart_queue, (void * )&event, (portTickType)portMAX_DELAY)) {
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    if(xTimerStart(g_recv_timer, portMAX_DELAY) == pdFALSE)
                        STD_ASSERT(-1);
                    STD_LOGD("time start");
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    STD_LOGD( "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(g_uart_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    STD_LOGD( "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(EX_UART_NUM);
                    xQueueReset(g_uart_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    STD_LOGD( "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    STD_LOGD( "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    STD_LOGD( "uart frame error");
                    break;
                //UART_PATTERN_DET 
                case UART_PATTERN_DET:
                    break;
                //Othersp
                default:
                    STD_LOGD( "uart event type: %d", event.type);
                    break;
            }
        }
    }

    vTaskDelete(NULL);
}

void empty_recv_queue()
{
    rtu_packet_t *rtu;
    for(;;)
    {
        if(xQueueReceive(g_recv_queue, &rtu, 0) == pdFALSE)
            break;
        else
            free_rtu(rtu);
    }
}

int send_rtu(rtu_packet_t *packet)
{
    send_lock();
    int res;
    res = uart_write_bytes(EX_UART_NUM, (const char *)packet->raw_data, packet->len);
    send_unlock();
    //vTaskDelay(RECV_PEROID_MS / portTICK_PERIOD_MS);
    if(res == -1)
    {
        STD_LOGD( "send rtu error");
        return -1;
    }
    else
    {
        STD_LOGD( "uart send rtu success[%d]", packet->len);
        return 0;
    }
}

rtu_packet_t *wait_rtu(int time_ms)
{
    rtu_packet_t *rtu;
    if(xQueueReceive(g_recv_queue, &rtu, time_ms / portTICK_PERIOD_MS) == pdTRUE)
        return rtu;
    else
        return NULL;
}

rtu_packet_t *wait_self_rtu(int time_ms)
{
    rtu_packet_t *rtu;
    if(xQueueReceive(g_recv_self_queue, &rtu, time_ms / portTICK_PERIOD_MS) == pdTRUE)
        return rtu;
    else
        return NULL;
}

rtu_packet_t *build_rtu(uint8_t *data, int len)
{
    rtu_packet_t *rtu = STD_CALLOC(1, sizeof(rtu_packet_t));
    rtu->raw_data = STD_CALLOC(len, 1);
    rtu->len = len;
    if(data != NULL)
        memcpy(rtu->raw_data, data, len);

    return rtu;
}

void free_rtu(rtu_packet_t *packet)
{
    if(packet == NULL)
        return;
    if(packet->raw_data == NULL)
    {
        STD_FREE(packet);
        return;
    }

    STD_FREE(packet->raw_data);
    STD_FREE(packet);
}

static void recv_cb(TimerHandle_t timer)
{
    size_t recv_size;
    uart_get_buffered_data_len(EX_UART_NUM, &recv_size);
    rtu_packet_t *rtu = build_rtu(NULL, recv_size);
    STD_ASSERT(uart_read_bytes(EX_UART_NUM, rtu->raw_data, rtu->len, 0) == rtu->len);

    STD_LOGD("recv rtu[%d] id[%d]", recv_size, rtu->raw_data[0]);

    if(rtu->raw_data[0] == g_id)
        xQueueSend(g_recv_self_queue, &rtu, 0);
    else
    {
        if(xQueueSend(g_recv_queue, &rtu, portMAX_DELAY) == pdFALSE)
            STD_ASSERT(-1); 
    }

}

int serial_init(uint8_t id)
{
    g_id = id;
    g_send_lock = xSemaphoreCreateBinary();
    xSemaphoreGive(g_send_lock);
    g_recv_timer = xTimerCreate("recv time", RECV_PEROID_MS / portTICK_PERIOD_MS, pdFALSE, NULL, recv_cb);
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = BAND_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(EX_UART_NUM, &uart_config));

    //Set UART pins (using UART0 default pins ie no changes.)
    ESP_ERROR_CHECK(uart_set_pin(EX_UART_NUM, MODUS_TX, MODUS_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    //Install UART driver, and get the queue.
    ESP_ERROR_CHECK(uart_driver_install(EX_UART_NUM, RD_BUF_SIZE, 0, EVENT_QUENE_LENGTH, &g_uart_queue, 0));

    g_recv_queue = xQueueCreate(10, sizeof(rtu_packet_t *));
    g_recv_self_queue = xQueueCreate(1, sizeof(rtu_packet_t *));

    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, RECV_QUEUE_SIZE, NULL);

    STD_LOGI("rtu serial init success");

    return 0;
}
