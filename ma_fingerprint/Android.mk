# Copyright (C) 2013 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

#include $(CLEAR_VARS)
#LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE = libfprint-x64
#LOCAL_MODULE_CLASS = SHARED_LIBRARIES
#LOCAL_SRC_FILES_64 = libfprint-x64.so
#LOCAL_MODULE_PATH := $(TARGET_OUT)/lib64
#include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wno-error -Wno-implicit-function-declaration 
LOCAL_MULTILIB = 64
LOCAL_MODULE := fingerprint.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := fingerprint.c ma_fprint.c

LOCAL_SHARED_LIBRARIES := liblog libfprint-x64
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)




