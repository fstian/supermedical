LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libjitterbuffer

LOCAL_SRC_FILES := com_witted_ptttalk_util_JitterBuffer.c \
					jb_fsm.c \
					jb_mgmt.c \
					jb_timer.c \
					jb_utl.c \
					rtp_interface.c \
					voip_oa_log.c


include $(BUILD_SHARED_LIBRARY)