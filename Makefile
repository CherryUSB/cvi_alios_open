SRCTREE := $(CURDIR)
YOC_FILE := $(SRCTREE)/.yoc
#NPROC ?= $(shell expr $(shell nproc) - 1)
NPROC := 7
$(info "NPROC = $(NPROC)")
export SRCTREE OBJYOC_FILE NPROC

ifeq ("$(wildcard $(YOC_FILE))","")
$(error ".yoc is not present, please do 'yoc init' first!!")
endif
#
HOST_TOOLS := $(SRCTREE)/host-tools/Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1/bin
CC := $(HOST_TOOLS)/riscv64-unknown-elf-gcc
CXX := $(HOST_TOOLS)/riscv64-unknown-elf-g++
AR := $(HOST_TOOLS)/riscv64-unknown-elf-ar
STRIP := $(HOST_TOOLS)/riscv64-unknown-elf-strip
export CC CXX AR STRIP
export PATH := $(HOST_TOOLS):$(PATH)
#
TOPSUBDIRS := smart_doorbell testsolution autotest video_doorbell SmartPad cv181x_boot peripherals_test usb_cam live_recorder_demo ipc barcode_scan huashanpi fs_demo
#
.PHONY:all clean install $(TOPSUBDIRS)

TMP = $(foreach f,$(TOPSUBDIRS),$(findstring $f,$(MAKECMDGOALS)))
TMP_SOLUTIONS = $(strip $(TMP))
ifneq ($(TMP_SOLUTIONS), )
	SOLUTIONS = $(TMP_SOLUTIONS)
else
	SOLUTIONS = smart_doorbell
endif
YOC_COMPRESS := n

#
all: $(TOPSUBDIRS)
#
$(TOPSUBDIRS):
	$(MAKE) -C solutions/$@ $(patsubst $@,,$(MAKECMDGOALS))
#
clean:
	$(MAKE) -C solutions/$(SOLUTIONS) clean
	@rm install/ -rf
#
install:
	@mkdir -p install/rawiamges
	@mkdir -p install/update
	@cp solutions/${SRCDIR}/generated/images/*  install/rawiamges/
	@for j in `find install/rawiamges/ -type f`;do	\
		${Q}python3 boards/tools/common/raw2cimg.py $$j install/update `find install/rawiamges/ -name "*.xml"`; \
	done
	@python boards/tools/common/generate_partion_xml.py --file_path=solutions/${SRCDIR}/generated/images/config.yaml --dst_path=install/config.xml
	cp install/rawiamges/fip.bin install/update/
	cp `find install/rawiamges/ -name "*.xml"` install/update/