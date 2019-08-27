
#ifndef __STD_COMMON_H__
#define __STD_COMMON_H__

#include <stdlib.h>
#include <stdarg.h>
#include "errno.h"
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include "esp_err.h"
#include "esp_log.h"
#include "mdf_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#ifdef __cplusplus
extern "C" {
#endif /**< _cplusplus */

#define ___PASTE(a,b) a##b
#define __PASTE(a,b) ___PASTE(a,b)

#define STD_TIME 0
#define PRODUCT_TEST 1

#define STD_LOG_MARK        1
#define STD_LOG_ERROR		2
#define STD_LOG_WARN		3
#define STD_LOG_INFO		4
#define STD_LOG_DEBUG		5
#define STD_LOG_VERBOSE     6

extern void std_rdebug_log(int level, char *file, int line, char *format, ...);
extern int STD_GLOBAL_LOG_LEVEL;

// #define LOG_SPECFIC(level, format, ...) std_rdebug_log(level, __FILE__, __LINE__, format"\033[0m\n",  ##__VA_ARGS__)
#define LOG_SPECFIC(level, format, ...) \
esp_log_write(ESP_LOG_INFO, "TAG", MDF_LOG_FORMAT(I, format), esp_log_timestamp(), "TAG", __LINE__, ##__VA_ARGS__);

#define _STD_LOGM(format, ...) do{\
    if (STD_LOCAL_LOG_LEVEL >= STD_LOG_MARK)\
    LOG_SPECFIC(STD_LOG_MARK, format, ##__VA_ARGS__);\
}while(0) 

#define _STD_LOGE(format, ...) do{  \
    if (STD_LOCAL_LOG_LEVEL >= STD_LOG_ERROR)\
		LOG_SPECFIC(STD_LOG_ERROR, format, ##__VA_ARGS__);\
}while (0)

#define _STD_LOGW(format, ...) do{\
    if(STD_LOCAL_LOG_LEVEL >= STD_LOG_WARN)\
		LOG_SPECFIC(STD_LOG_WARN, format, ##__VA_ARGS__);\
}while (0)

#define _STD_LOGI(format, ...) do{\
    if(STD_LOCAL_LOG_LEVEL >= STD_LOG_INFO)\
		LOG_SPECFIC(STD_LOG_INFO, format, ##__VA_ARGS__);\
}while (0)


#define _STD_LOGD(format, ...) do{\
    if(STD_LOCAL_LOG_LEVEL >= STD_LOG_DEBUG)\
		LOG_SPECFIC(STD_LOG_DEBUG, format, ##__VA_ARGS__); \
}while (0)

#define _STD_LOGV(format, ...) do{\
    if(STD_LOCAL_LOG_LEVEL >= STD_LOG_VERBOSE)\
		LOG_SPECFIC(STD_LOG_VERBOSE, format, ##__VA_ARGS__); \
}while(0)


#define STD_LOGM(format, ...) do{\
    if(STD_GLOBAL_LOG_LEVEL >= STD_LOG_MARK)\
        _STD_LOGM(format, ##__VA_ARGS__);\
}while(0) 

#define STD_LOGE(format, ...) do{\
    if(STD_GLOBAL_LOG_LEVEL >= STD_LOG_ERROR)\
        _STD_LOGE(format, ##__VA_ARGS__);\
}while(0) 

#define STD_LOGW(format, ...) do{\
    if(STD_GLOBAL_LOG_LEVEL >= STD_LOG_WARN)\
        _STD_LOGW(format, ##__VA_ARGS__);\
}while(0) 

#define STD_LOGI(format, ...) do{\
    if(STD_GLOBAL_LOG_LEVEL >= STD_LOG_INFO)\
        _STD_LOGI(format, ##__VA_ARGS__);\
}while(0) 

#define STD_LOGD(format, ...) do{\
    if(STD_GLOBAL_LOG_LEVEL >= STD_LOG_DEBUG)\
        _STD_LOGD(format, ##__VA_ARGS__);\
}while(0) 

#define STD_LOGV(format, ...) do{\
    if(STD_GLOBAL_LOG_LEVEL >= STD_LOG_VERBOSE)\
        _STD_LOGV(format, ##__VA_ARGS__);\
}while(0) 




#define STD_FAIL -1
#define STD_OK 0

#define STD_ASSERT(con) do{\
    if(!(con)){\
            STD_LOGE("assert fail");\
            assert(NULL);\
        }\
    }while(0)

#define STD_ERROR_GOTO_FAIL(value) do { \
        int _value = value;\
        if (_value != 0) { \
            STD_LOGD("error jump fail, value: %d", _value); \
            goto FAIL; \
        } \
    }while (0)

#define STD_CHECK_GOTO_FAIL(con) do { \
        if (!(con)) { \
            STD_LOGD("check jump fail"); \
            goto FAIL; \
        } \
    }while (0)

#define STD_ERROR_RETURN(value) do { \
        int _value = value;\
        if (_value != 0) { \
            STD_LOGD("error return fail, value: %d", _value); \
            return STD_FAIL; \
        } \
    }while (0)

#define STD_CHECK_RETURN(con) do { \
        if (!(con)) { \
            STD_LOGD("check return fail"); \
            return STD_FAIL; \
        } \
    }while (0)

#define STD_ERROR_BREAK(value) \do{\
        int _value = value;\
        if (_value != 0) { \
            STD_LOGD("check break"); \
            break; \
        }}while(0)

#define STD_CHECK_BREAK(con) \
        if (!(con)) { \
            STD_LOGD("check break"); \
            break; \
        }

#define STD_ERROR_CONTIUNE(value) do{\
        int _value = value;\
        if (_value != 0) { \
            STD_LOGD("check continue"); \
            continue; \
        }}while(0)

#define STD_CHECK_CONTINUE(con) \
        if (!(con)) { \
            STD_LOGD("check continue"); \
            continue; \
        }

#define STD_END(format, ...) do{\
	STD_LOGE(format, ##__VA_ARGS__);\
	vTaskDelay(portMAX_DELAY);\
}while (0)

#define STD_CONDITION_RES(con) ({\
    int res = 0;\
    if(!(con)){\
        res = -1;\
    }\
    res;\
})

#define STD_RETRY_TRUE_BREAK(con, retry_times) ({\
    int res = -1;\
    for(int __n = 0; __n < retry_times; __n++)\
    {\
        if(con)\
        {\
            res = 0;\
            break;\
        }\
        STD_LOGW("retry[%d]", __n);\
    }\
    res;\
})

#define STD_RETRY_FALSE_BREAK(con, retry_times) ({\
    int res = 0;\
    for(int __n = 0; __n < retry_times; __n++)\
    {\
        if(!(con))\
        {\
            res = -1;\
            break;\
        }\
        STD_LOGW("retry[%d]", __n);\
    }\
    res;\
})


/**
 * @brief count the number of function parameters
 */
#define COUNT_PARMS2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _, ...) _
#define COUNT_PARMS(...)\
    COUNT_PARMS2(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

/**
 * @brief Set breakpoint
 */

#define STD_MALLOC(size) ({void *ptr = malloc(size); \
        if(!ptr){STD_LOGE("malloc size: %d, ptr: %p, heap free: %d", size, ptr, esp_get_free_heap_size());assert(ptr);} \
        STD_LOGV("malloc size: %d, ptr: %p, heap free: %d", size, ptr, esp_get_free_heap_size()); ptr;})

#define STD_CALLOC(n, size) ({void *ptr = calloc(n, size); \
        if(!ptr){STD_LOGE("calloc size: %d, ptr: %p, heap free: %d", (n) * (size), ptr, esp_get_free_heap_size());assert(ptr);} \
        STD_LOGV("calloc size: %d, ptr: %p, heap free: %d", (n) * (size), ptr, esp_get_free_heap_size()); ptr;})

#define STD_FREE(ptr) ({if(ptr){ free(ptr); \
            STD_LOGV("free ptr: %p, heap free: %d", ptr, esp_get_free_heap_size()); ptr = NULL;}})


#define PRINT_HEX(buf, len) do{ \
    int i;\
    printf("[%s, %d]:", __FUNCTION__ , __LINE__);\
    for (i = 0; i < len; i++) {\
        printf("%02x ", *(buf + i));}\
    printf("\r\n");\
}while(0)


#ifndef MAC2STR
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#endif

static inline char *mac2str(const uint8_t *mac, char *mac_str)
{
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(mac));

    return mac_str;
}


static inline uint8_t *str2mac(uint8_t *mac, const char *mac_str)
{
    uint32_t mac_data[6] = {0};
    int res;
    res = sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_data, mac_data + 1, mac_data + 2, mac_data + 3, mac_data + 4, mac_data + 5);
    if(res != 6)
        return NULL;
        
    for (int i = 0; i < 6; i++) {
        mac[i] = mac_data[i];
    }

    return mac;
}

static inline bool check_mac(char *mac_str)
{
    uint32_t mac_data[6] = {0};
    int res;
    res = sscanf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_data, mac_data + 1, mac_data + 2, mac_data + 3, mac_data + 4, mac_data + 5);
    if(res != 6)
        return false;  
    else
        return true;
}

static inline bool has_mac(uint8_t *list, int len, uint8_t *tar_mac)
{
    for(int n = 0; n < len; n++)
    {
        if(memcpy(&list[6 * n], tar_mac, 6) == 0)
            return true;       
    }
    return false;
}

#define PRINT_NUM(_num_list, _len) do{\
    printf("[%s, %d]:", __FUNCTION__ , __LINE__);\
    int __type =    + __builtin_types_compatible_p(typeof(_num_list), bool *) * 1\
                    + __builtin_types_compatible_p(typeof(_num_list), int8_t *) * 1\
                    + __builtin_types_compatible_p(typeof(_num_list), uint8_t *) * 2\
                    + __builtin_types_compatible_p(typeof(_num_list), short *) * 3\
                    + __builtin_types_compatible_p(typeof(_num_list), uint16_t *) * 4\
                    + __builtin_types_compatible_p(typeof(_num_list), int *) * 5\
                    + __builtin_types_compatible_p(typeof(_num_list), uint32_t *) * 6\
                    + __builtin_types_compatible_p(typeof(_num_list), long *) * 7\
                    + __builtin_types_compatible_p(typeof(_num_list), unsigned long *) * 8\
                    + __builtin_types_compatible_p(typeof(_num_list), float *) * 9\
                    + __builtin_types_compatible_p(typeof(_num_list), double *) * 10;\
    switch(__type)\
    {\
        case 1:\
            for(int n = 0; n < _len; n++)\
            {\
                int8_t *p = (int8_t *)(_num_list);\
                printf("%d ", p[n]);\
            }\
        break;\
        case 4:\
            for(int n = 0; n < _len; n++)\
            {\
                uint16_t *p = (uint16_t *)(_num_list);\
                printf("%d", p[n]);\
            }\
        break;\
    }\
    printf("\r\n");\
}while(0)
#ifdef __cplusplus
}
#endif /**< _cplusplus */
#endif /**< __STD_COMMON_H__ */
