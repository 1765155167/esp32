#ifndef SCREEN_INFO_H
#define SCREEN_INFO_H

#include "config.h"
#include "rtu.h"
#include "moter.h"

typedef struct screen_info_t{
	int id;
    moter_stu *device;
	moter_args *agrs;
    bool en;
}screen_info_t;

mdf_err_t screen_init(void);
void set_device_id(moter_stu *device,int id);
void build_screen_info(moter_stu *device, moter_args *args,int id);
int change_screen_info();
void set_screen_info(int id);
void trig_screen_info_refresh();
int get_drive_id();
#endif