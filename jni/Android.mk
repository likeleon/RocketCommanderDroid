LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := rocketcommanderdroid

LOCAL_C_INCLUDES := $(OGRE_ROOT)/build/include \
					$(OGRE_ROOT)/OgreMain/include \
					$(OGRE_ROOT)/RenderSystems/GLES2/include \
					$(OGRE_ROOT)/RenderSystems/GLES2/include/EGL \
					$(OGRE_ROOT)/Components/RTShaderSystem/include \
					$(OGRE_ROOT)/Components/Overlay/include \
					$(OGRE_ROOT)/PlugIns/ParticleFX/include \
					$(OGRE_ROOT)/PlugIns/OctreeSceneManager/include \
					$(OGRE_ROOT)/AndroidDependencies/include \
					$(OGRE_ROOT)/AndroidDependencies/include/OIS \
					$(OGRE_ROOT)/build/Samples/include				

LOCAL_CFLAGS := -DGL_GLEXT_PROTOTYPES=1
LOCAL_CFLAGS += -fexceptions -frtti -x c++ -D___ANDROID___ -DANDROID -DZZIP_OMIT_CONFIG_H -DUSE_RTSHADER_SYSTEM=1

LS_CPP=$(subst $(1)/,,$(wildcard $(1)/*.cpp))
LOCAL_SRC_FILES := $(call LS_CPP,$(LOCAL_PATH))
	
LOCAL_LDLIBS	:= -landroid -lc -lm -ldl -llog -lEGL -lGLESv2 -lOpenSLES
LOCAL_LDLIBS	+= -L$(LOCAL_PATH)/prebuild/ogre
LOCAL_LDLIBS	+= -lPlugin_ParticleFXStatic -lPlugin_OctreeSceneManagerStatic -lRenderSystem_GLES2Static -lOgreRTShaderSystemStatic -lOgreOverlayStatic -lOgreMainStatic
LOCAL_LDLIBS	+= -lzzip -lz -lFreeImage -lfreetype -lOIS
LOCAL_LDLIBS	+= $(LOCAL_PATH)/prebuild/ogre/libglsl_optimizer.a $(LOCAL_PATH)/prebuild/ogre/libmesa.a $(LOCAL_PATH)/prebuild/ogre/libglcpp-library.a $(LOCAL_PATH)/prebuild/ogre/libsupc++.a $(LOCAL_PATH)/prebuild/ogre/libstdc++.a $(LOCAL_PATH)/../obj/local/armeabi/libcpufeatures.a

LOCAL_STATIC_LIBRARIES := android_native_app_glue cpufeatures

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)
$(call import-module,android/native_app_glue)