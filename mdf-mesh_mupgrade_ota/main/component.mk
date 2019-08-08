#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

COMPONENT_PRIV_INCLUDEDIRS += ../components/ccom/include
COMPONENT_SRCDIRS += ../components/ccom

COMPONENT_PRIV_INCLUDEDIRS += ../components/modbus/include
COMPONENT_SRCDIRS += ../components/modbus

COMPONENT_PRIV_INCLUDEDIRS += ../components/epcom/include
COMPONENT_SRCDIRS += ../components/epcom

COMPONENT_PRIV_INCLUDEDIRS += ../components/mcb/include
COMPONENT_SRCDIRS += ../components/mcb