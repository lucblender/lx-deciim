# Project Name
TARGET = lx-deciim-libdaisy

# Sources
CPP_SOURCES = lx-deciim-libdaisy.cpp ./inc/DownSampler.cpp

LDFLAGS += -u _printf_float
# Library Locations
LIBDAISY_DIR = ./libDaisy
DAISYSP_DIR = ./DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

