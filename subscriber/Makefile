include $(LITE_HOME)/make/make.cfg

SOURCE_PATH = ../generated
BINARY = subscriber
SOURCE = weather.c subscriber.c
CC_INCS += -I$(SOURCE_PATH)
LD_LIBS += dds

include $(LITE_HOME)/make/rules.cfg
