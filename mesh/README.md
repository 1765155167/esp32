# Mesh Manual Networking Example

ESP-MESH provides the function of self-organized networking, but if users disable this function on one node, users must designate a parent for this node.


This example demonstrates how to scan a list of parent candidates, choose an appropriate parent and set as the parent of this node.

If no parent is found through this scan, enable the self-organized function to let the ESP-MESH handle it by itself.

Run `make menuconfig` to configure the mesh network channel, router SSID, router password and mesh softAP settings.

When the mesh network is established and if you happen to run this example on ESP-WROVER-KIT boards, the RGB light indicator will show you on which layer devices are. The pink reprents root; the yellow reprents layer 2; the red reprents layer 3; the blue reprents layer 4; the green reprents layer 5; the white reprents layer greater than 5.

`string --> unsigned char *`
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{
	unsigned char *data;
	char * c_data = (char *)malloc(10*sizeof(char));
	sprintf(c_data,"hello\n");
	data = (unsigned char *)c_data;
	for(int i = 0; i < 10; i++)
	{
		printf("%c",data[i]);
	}
	return 0;
}
