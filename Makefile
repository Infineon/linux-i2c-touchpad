# Makefile for building a Linux kernel module

OUT_DIR := $(PWD)/output
$(shell mkdir -p $(OUT_DIR))

SRC_DIR := $(PWD)/src
INCLUDE_DIR := $(PWD)/include

BUILD_OPTIONS += TOUCHDOWN_LIFTOFF_ON_GESTURE

include builder.mk
