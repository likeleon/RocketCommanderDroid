LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := rocketcommanderdroid
LOCAL_SRC_FILES := Main.cpp EventLoop.cpp Log.cpp RocketCommanderDroid.cpp
LOCAL_LDLIBS	:= -landroid -llog
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)