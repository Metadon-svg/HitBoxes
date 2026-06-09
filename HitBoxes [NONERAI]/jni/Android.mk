LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := HitBoxes

# Флаги компиляции (оставляем как у тебя)
LOCAL_CFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CFLAGS += -fno-rtti -fexceptions -fpermissive -Oz

LOCAL_CPPFLAGS += -Wno-format-security -fvisibility=hidden -ffunction-sections -fdata-sections -w
LOCAL_CPPFLAGS += -fno-rtti -fexceptions -fpermissive -Oz -std=c++17
LOCAL_CPPFLAGS += -Wno-c++17-narrowing -fms-extensions -DNDEBUG

# ВАЖНО: Добавляем библиотеки OpenGL ES 3 и EGL для работы ImGui графики
LOCAL_LDFLAGS += -Wl,--gc-sections -Wl,--strip-all -llog -lGLESv3 -lEGL

# Собираем все .cpp и .c файлы из корня, KittyMemory и папки imgui
FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/*.c)
FILE_LIST += $(wildcard $(LOCAL_PATH)/KittyMemory/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/imgui/*.cpp)

# ИСПРАВЛЕНО: Указываем чистые пути для поиска заголовочных файлов (.h)
# Теперь компилятор точно зайдет в папку imgui и увидит imgui.h
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/KittyMemory
LOCAL_C_INCLUDES += $(LOCAL_PATH)/imgui

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)
