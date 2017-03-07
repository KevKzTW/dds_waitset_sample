include $(LITE_HOME)/make/make.cfg

MODULE = WaitSet example
PRE_RULES = idl
CLEAN += weather.c weather.h

include $(LITE_HOME)/make/rules.cfg

.PHONY: idl
idl: weather.c

weather.c: weather.idl
	dds_idlc weather.idl
