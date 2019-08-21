#include "tcp.h"

static const char *TAG = "tcp client";
static int g_sockfd    = -1;
/**
 * @brief Create a tcp client
 */
static int socket_tcp_client_create(const char *ip, uint16_t port)
{
    MDF_PARAM_CHECK(ip);

    MDF_LOGI("Create a tcp client, ip: %s, port: %d", ip, port);

    mdf_err_t ret = ESP_OK;
    int sockfd    = -1;
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = inet_addr(ip),
    };

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    MDF_ERROR_GOTO(sockfd < 0, ERR_EXIT, "socket create, sockfd: %d", sockfd);

    ret = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in));
    MDF_ERROR_GOTO(ret < 0, ERR_EXIT, "socket connect, ret: %d, ip: %s, port: %d",
                   ret, ip, port);
    return sockfd;

ERR_EXIT:

    if (sockfd != -1) {
        close(sockfd);
    }

    return -1;
}


mdf_err_t tcp_client_read(char *data, size_t * size)
{
    mdf_err_t ret = MDF_OK;

    MDF_LOGI("Start TCP client read");

	if (g_sockfd == -1) {
		g_sockfd = socket_tcp_client_create(CONFIG_SERVER_IP, CONFIG_SERVER_PORT);

		if (g_sockfd == -1) {
			vTaskDelay(500 / portTICK_RATE_MS);
			ret = MDF_FAIL;
			goto RET;
		}
	}

	memset(data, 0, *size);
	ret = read(g_sockfd, data, *size);
	*size = ret;
	MDF_LOGD("TCP read, %d, size: %d, data: %s", g_sockfd, *size, data);

	if (ret <= 0) {
		MDF_LOGW("<%s> TCP read", strerror(errno));
		close(g_sockfd);
		g_sockfd = -1;
		ret = MDF_FAIL;
		goto RET;
	}
	
    MDF_LOGI("TCP client read task is exit");
	ret = MDF_OK;
    close(g_sockfd);
    g_sockfd = -1;
RET:
    return ret;
}

mdf_err_t tcp_client_write(char *data)
{
    mdf_err_t ret = MDF_OK;
    size_t size;

    MDF_LOGI("Start TCP client write ");

	if (g_sockfd == -1) {
		g_sockfd = socket_tcp_client_create(CONFIG_SERVER_IP, CONFIG_SERVER_PORT);

		if (g_sockfd == -1) {
			vTaskDelay(500 / portTICK_RATE_MS);
			goto RET;
		}
	}
	size = strlen(data);
	MDF_LOGI("TCP write, size: %d, data: %s", size, data);
	ret = write(g_sockfd, data, size);
	MDF_ERROR_GOTO(ret <= 0, RET, "<%s> TCP write", strerror(errno));
RET:
	MDF_LOGI("TCP client write finish");
    close(g_sockfd);
    g_sockfd = -1;
    return ret;
}