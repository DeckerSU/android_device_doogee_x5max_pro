/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "ma_fprint.h"

uint8_t HW_AUTH_TOKEN_VERSION = 0;

extern int ma_set_notify(fingerprint_notify_t notify);  

/* Fingerprint pre-enroll enroll request:
 * Generates a unique token to upper layers to indicate the start of an enrollment transaction.
 * This token will be wrapped by security for verification and passed to enroll() for
 * verification before enrollment will be allowed. This is to ensure adding a new fingerprint
 * template was preceded by some kind of credential confirmation (e.g. device password).
 *
 * Function return: 0 if function failed
 *                  otherwise, a uint64_t of token
 */
static uint64_t fingerprint_pre_enroll(struct fingerprint_device __unused *dev) {	
	return ma_pre_enroll();	  
}

/* Fingerprint enroll request:
 * Switches the HAL state machine to collect and store a new fingerprint
 * template. Switches back as soon as enroll is complete
 * (fingerprint_msg.type == FINGERPRINT_TEMPLATE_ENROLLING &&
 *  fingerprint_msg.data.enroll.samples_remaining == 0)
 * or after timeout_sec seconds.
 * The fingerprint template will be assigned to the group gid. User has a choice
 * to supply the gid or set it to 0 in which case a unique group id will be generated. *
 * Function return: 0 if enrollment process can be successfully started
 *                  or a negative number in case of error, generally from the errno.h set.
 *                  A notify() function may be called indicating the error condition.
 */
static int fingerprint_enroll(struct fingerprint_device __unused *dev,
		const hw_auth_token_t __unused *hat,
        uint32_t __unused gid, uint32_t __unused timeout_sec) {		
	int ret = ma_enroll(hat, gid, timeout_sec);		
    return ret<0? FINGERPRINT_ERROR: 0;
}

/* Finishes the enroll operation and invalidates the pre_enroll() generated challenge.
 * This will be called at the end of a multi-finger enrollment session to indicate
 * that no more fingers will be added. *
 * Function return: 0 if the request is accepted
 *                  or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_post_enroll(struct fingerprint_device *dev) {			
	int ret = ma_post_enroll();	
	return ret<0? FINGERPRINT_ERROR: 0;
}

/* get_authenticator_id:
 * Returns a token associated with the current fingerprint set. This value will
 * change whenever a new fingerprint is enrolled, thus creating a new fingerprint
 * set.
 * Function return: current authenticator id or 0 if function failed.
 */
static uint64_t fingerprint_get_auth_id(struct fingerprint_device __unused *dev) { 	
	return ma_get_auth_id();	  
}

/* Cancel pending enroll or authenticate, sending FINGERPRINT_ERROR_CANCELED
 * to all running clients. Switches the HAL state machine back to the idle state.
 * Unlike enroll_done() doesn't invalidate the pre_enroll() challenge. *
 * Function return: 0 if cancel request is accepted
 *                  or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_cancel(struct fingerprint_device __unused *dev) {
	int ret = ma_cancel();		
    return ret<0? FINGERPRINT_ERROR: 0;
}

/* Enumerate all the fingerprint templates found in the directory set by
 * set_active_group()
 * This is a synchronous call. The function takes:
 * - A pointer to an array of fingerprint_finger_id_t.
 * - The size of the array provided, in fingerprint_finger_id_t elements.
 * Max_size is a bi-directional parameter and returns the actual number
 * of elements copied to the caller supplied array.
 * In the absence of errors the function returns the total number of templates
 * in the user directory.
 * If the caller has no good guess on the size of the array he should call this
 * function witn *max_size == 0 and use the return value for the array allocation.
 * The caller of this function has a complete list of the templates when *max_size
 * is the same as the function return.
 *
 * Function return: Total number of fingerprint templates in the current storage directory.
 *     or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_enumerate(struct fingerprint_device *dev,
		fingerprint_finger_id_t *results, uint32_t *max_size) {
	int ret = ma_enumerate(results, max_size);
	return ret<0? FINGERPRINT_ERROR: 0;
}

/* Fingerprint remove request:
 * Deletes a fingerprint template.
 * Works only within a path set by set_active_group().
 * notify() will be called with details on the template deleted.
 * fingerprint_msg.type == FINGERPRINT_TEMPLATE_REMOVED and
 * fingerprint_msg.data.removed.id indicating the template id removed.
 * Function return: 0 if fingerprint template(s) can be successfully deleted
 *                  or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_remove(struct fingerprint_device __unused *dev,
		uint32_t __unused gid, uint32_t __unused fid) {
	int ret = ma_remove(gid, fid);		
    return ret<0? FINGERPRINT_ERROR: 0;
} 

/* Restricts the HAL operation to a set of fingerprints belonging to a
 * group provided.
 * The caller must provide a path to a storage location within the user's
 * data directory.
 * Function return: 0 on success
 *                  or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_set_active_group(struct fingerprint_device __unused *dev,
		uint32_t __unused gid, const char __unused *store_path) {
	int ret = ma_set_active_group(gid, store_path);
    return ret<0? FINGERPRINT_ERROR: 0;
} 

/* Authenticates an operation identifed by operation_id
 * Function return: 0 on success
 *                  or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_authenticate(struct fingerprint_device __unused *dev,
		uint64_t __unused operation_id, __unused uint32_t gid) {
    int ret = ma_verify(operation_id, gid);    	
    return ret<0? FINGERPRINT_ERROR: 0;
}

static int set_notify_callback(struct fingerprint_device *dev,
		fingerprint_notify_t notify) {	
	int ret = ma_set_notify(notify);		
    return ret<0? FINGERPRINT_ERROR: 0;
} 

static int fingerprint_close(hw_device_t *dev) {   
	int ret;	
		
    if (dev) {
        free(dev);     
        ret = ma_close();     
    } else {
        ret = FINGERPRINT_ERROR;
    }  
          
    return ret; 
}

/*  Set Navigation mode or identifying mode.
 *  Input 1 = navigation mode
 *  Input 0 = identifying mode
 *  Function return: 0 in sucess
 *       			 or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_setNavMode(struct fingerprint_device *dev, uint32_t nav) {
	return ma_setNavMode(nav);
}

/*  Get current mode
 *  Funtion return: 1 = navigation mode, 0 = identifying mode.
 */
static int fingerprint_getMode(struct fingerprint_device *dev) {
	return ma_getMode();
}

/*  Defines the duration in milliseconds we will wait to see if a touch event
 *  is a tap or a scroll. If the user does not move within this interval, it is
 *  considered to be a tap.
 *  Function return: 0 in sucess
 *      			 or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_setTapTimeout(struct fingerprint_device *dev, uint32_t timeout) {
	return ma_setTapTimeout(timeout);
}

/*  Defines the minimum duration in milliseconds between the first tap's up event and
 *  the second tap's down event for an interaction to be considered a
 *  double-tap. *
 *  Function return: 0 in sucess
 *                   or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_setDoubleTapMinTime(struct fingerprint_device *dev, uint32_t min) {
	return ma_setDoubleTapMinTime(min);
}

/*  Defines the duration in milliseconds between the first tap's up event and
 *  the second tap's down event for an interaction to be considered a
 *  double-tap.
 *  Function return: 0 in sucess
 *                   or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_setDoubleTapTimeout(struct fingerprint_device *dev, uint32_t timeout) {
	return ma_setDoubleTapTimeout(timeout);
}

/*  Defines the default duration in milliseconds before a press turns into
 *  a long press
 *  Function return: 0 in sucess
 *                   or a negative number in case of error, generally from the errno.h set.
 */
static int fingerprint_setLongPressTimeout(struct fingerprint_device *dev, uint32_t timeout) {
	return ma_setLongPressTimeout(timeout);
}

static int fingerprint_open(const hw_module_t* module, const char __unused *id,
		hw_device_t** device) {			
    if ( device==NULL ) return -EINVAL;   
    if(ma_open()<0) return FINGERPRINT_ERROR;
    fingerprint_device_t *dev = malloc(sizeof(fingerprint_device_t));
    memset(dev, 0, sizeof(fingerprint_device_t));
 
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = FINGERPRINT_MODULE_API_VERSION_2_0;
    dev->common.module = (struct hw_module_t*) module;
    dev->common.close = fingerprint_close;

    dev->pre_enroll = fingerprint_pre_enroll;
    dev->enroll = fingerprint_enroll;
	dev->post_enroll = fingerprint_post_enroll;
    dev->get_authenticator_id = fingerprint_get_auth_id;
    dev->cancel = fingerprint_cancel;
    dev->remove = fingerprint_remove;
    dev->set_active_group = fingerprint_set_active_group;
    dev->authenticate = fingerprint_authenticate;
    dev->set_notify = set_notify_callback;
    dev->notify = NULL;
    *device = (hw_device_t*) dev;  
 
    return 0;
}

static struct hw_module_methods_t fingerprint_module_methods = {
    .open = fingerprint_open,
};

fingerprint_module_t HAL_MODULE_INFO_SYM = {  
    .common = {
        .tag                = HARDWARE_MODULE_TAG,
        .module_api_version = FINGERPRINT_MODULE_API_VERSION_2_0,
        .hal_api_version    = HARDWARE_HAL_API_VERSION,
        .id                 = FINGERPRINT_HARDWARE_MODULE_ID,
        .name               = "MicroArray Fingerprint",
        .author             = "MicroArray software team",
        .methods            = &fingerprint_module_methods,
    },
};



