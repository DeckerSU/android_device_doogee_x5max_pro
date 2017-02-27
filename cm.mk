## Specify phone tech before including full_phone

# Release name
PRODUCT_RELEASE_NAME := x5max_pro

# Inherit some common CM stuff.
$(call inherit-product, vendor/cm/config/common_full_phone.mk)

# Inherit device configuration
$(call inherit-product, device/doogee/x5max_pro/device_x5max_pro.mk)

## Device identifier. This must come after all inclusions
PRODUCT_DEVICE := x5max_pro
PRODUCT_NAME := lineage_x5max_pro
PRODUCT_BRAND := Doogee
PRODUCT_MODEL := Doogee X5 Max Pro
PRODUCT_MANUFACTURER := Doogee
