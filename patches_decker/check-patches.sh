#!/bin/bash
cd ../../../..
cd system/core
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0001-Remove-CAP_SYS_NICE-from-surfaceflinger.patch
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0004-libnetutils-add-MTK-bits-ifc_ccmni_md_cfg.patch
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0012-PATCH-xen0n-some-MTK-services-e.g.-ril-daemon-mtk-re.patch
cd ../..
cd bionic
git apply -v --check ../device/doogee/x5max_pro/patches_decker/0002-Apply-LIBC-version-to-__pthread_gettid.patch
cd ..
cd system/sepolicy
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0003-Revert-back-to-policy-version-29.patch
cd ../..
cd packages/apps/Settings
git apply -v --check ../../../device/doogee/x5max_pro/patches_decker/0005-add-author-info-in-device-info.patch
cd ../../..
cd frameworks/av
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0006-fix-access-wvm-to-ReadOptions.patch
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0007-Disable-usage-of-get_capture_position.patch
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0008-Partial-Revert-Camera1-API-Support-SW-encoders-for-n.patch
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0009-add-mtk-color-format-support.patch
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0013-fix-_ZN7android16MediaBufferGroup14acquire_bufferEPP.patch
cd ../..
cd system/netd
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0010-wifi-tethering-fix.patch
cd ../..
cd packages/apps/FMRadio/jni/fmr/ 
git apply -v --check ../../../../../device/doogee/x5max_pro/patches_decker/0014-fix-fm-radio-power-up-mt6737m-mt6627-chip.patch
cd ../../../../..
cd vendor/cm/ 
git apply -v --check ../../device/doogee/x5max_pro/patches_decker/0015-disable-ResurrectionStats-building.patch
cd ../..