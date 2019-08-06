# Automatically generated build file. Do not edit.
COMPONENT_INCLUDES += $(PROJECT_PATH)/components/mcb/include $(PROJECT_PATH)/components/mcb/debug $(PROJECT_PATH)/components/mcb/func $(PROJECT_PATH)/components/mcb/func/ota $(PROJECT_PATH)/components/mcb/func/report $(PROJECT_PATH)/components/mcb/func/device_config $(PROJECT_PATH)/components/mcb/base_info $(PROJECT_PATH)/components/mcb/base_protocol $(PROJECT_PATH)/components/mcb/network $(PROJECT_PATH)/components/mcb/cmd_protocol $(PROJECT_PATH)/components/mcb/out_interface $(PROJECT_PATH)/components/mcb/out_port
COMPONENT_LDFLAGS += -L$(BUILD_DIR_BASE)/mcb -lmcb
COMPONENT_LINKER_DEPS += 
COMPONENT_SUBMODULES += 
COMPONENT_LIBRARIES += mcb
component-mcb-build: 
