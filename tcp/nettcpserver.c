#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <sys/socket.h>


int main() 
{ 
	pid_t pid; //����ID
	int sockfd,client_fd;	   		//sock_fd-�����׽�����������client_fd-�����׽���������
	struct sockaddr_in my_addr;		//������ַ
	struct sockaddr_in remote_addr;	//�ͻ��˵�ַ
	char databuff[1024];
	char buf[] = "Hello esp32, I'am mr.hu, you are connected my tcp server.!";
	//�����׽���
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{ 
		perror("socket"); 
		exit(1); 
	}
	//���õ�ַ�˿ڿ�����
	int val=1;
	setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(val) );

	//���ñ��ص�ַ��Ϣ
	my_addr.sin_family=AF_INET; 				//Э����
	my_addr.sin_port=htons(8070); 		        //�˿�
	my_addr.sin_addr.s_addr=inet_addr("192.168.43.131");//IP��ַ
	bzero(&(my_addr.sin_zero),8); 				//���0

	//�󶨵�ַ���׽�����������
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1) 
	{ 
		perror("bind"); 
		exit(1); 
	}
	//�ڵ�ַ�˿��ϼ���
	if (listen(sockfd, 10) == -1) 
	{ 
		perror("listen"); 
		exit(1); 
	}
	while(1)
	{
		//�ȴ��ͻ������ӣ�����пͻ������ӣ�������µ������׽���
		int sin_size = sizeof(struct sockaddr_in);
		if ((client_fd = accept(sockfd, (struct sockaddr *)&remote_addr,&sin_size)) != -1)
		{
			if((pid = fork()) < 0){
				perror("fork failed");
				exit(0);
			}else if(pid == 0){//�ӽ���
				//����ͻ���IP��ַ
				printf("received a connection from %s\n", inet_ntoa(remote_addr.sin_addr)); 
				//��ͻ��˷��ͻ�ӭ��Ϣ
				if (send(client_fd, buf, sizeof(buf), 0) == -1) 
				{
					perror("send"); 
					close(client_fd); 
					exit(2); 
				}
				//���տͻ�������
				if (recv(client_fd, databuff, sizeof(databuff), 0) == -1)
				{
					perror("recv");
					close(client_fd);
					exit(3);
				}
				//����
				printf("recv databuff :%s\n", databuff);
				//��ͻ��˷��ʹ�����
				if (send(client_fd, "this is return...", 19, 0) == -1) 
				{
					perror("send"); 
					close(client_fd); 
					exit(2); 
				}
				//�ر������׽��� 
				close(client_fd); 
				exit(0);
			}
		}else
		{
			perror("accept"); 
			exit(1);
		}
	}
	//�رռ����׽���
	close(sockfd); 
	return 0;
}
