#
# libbluedroid
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	bluetooth.c

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	system/bluetooth/bluez-clean-headers

LOCAL_SHARED_LIBRARIES := \
	libcutils

LOCAL_MODULE := libbluedroid

include $(BUILD_SHARED_LIBRARY)

###
#LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES:= \
	cmd.c \
	hci.c \
	opt.c \
	pnx.c

LOCAL_CFLAGS := \

LOCAL_MODULE:= pnx
LOCAL_MODULE_TAGS := debug

include $(BUILD_EXECUTABLE)

