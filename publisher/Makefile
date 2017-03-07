include $(LITE_HOME)/make/make.cfg

SOURCE_PATH = ../generated
BINARY = publisher
SOURCE = weather.c publisher.c
CC_INCS += -I$(SOURCE_PATH)
LD_LIBS += dds

include $(LITE_HOME)/make/rules.cfg
