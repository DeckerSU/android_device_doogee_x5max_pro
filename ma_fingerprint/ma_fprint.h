/*
 * ma_fprint.h
 *
 *  Created on: 2016-7-21
 *      Author: czl
 *      Version: 3.0
 *      Customer: for standard
 */
#include "fingerprint.h"

#ifndef MA_FPRINT_H_
#define MA_FPRINT_H_
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t uint32;
typedef uint64_t uint64;

enum fp_acquired_info {
	FINGERPRINT_ACQUIRED_ALI_BASE = 1100,
	FINGERPRINT_ACQUIRED_WAIT_FINGER_INPUT = FINGERPRINT_ACQUIRED_ALI_BASE + 1,
	FINGERPRINT_ACQUIRED_FINGER_DOWN = FINGERPRINT_ACQUIRED_ALI_BASE + 2,
	FINGERPRINT_ACQUIRED_FINGER_UP = FINGERPRINT_ACQUIRED_ALI_BASE + 3,
	FINGERPRINT_ACQUIRED_INPUT_TOO_LONG = FINGERPRINT_ACQUIRED_ALI_BASE + 4,
	FINGERPRINT_ACQUIRED_DUPLICATE_FINGER = FINGERPRINT_ACQUIRED_ALI_BASE + 5,
	FINGERPRINT_ACQUIRED_DUPLICATE_AREA = FINGERPRINT_ACQUIRED_ALI_BASE + 6,
	FINGERPRINT_ACQUIRED_LOW_COVER = FINGERPRINT_ACQUIRED_ALI_BASE + 7,
	FINGERPRINT_ACQUIRED_BAD_IMAGE = FINGERPRINT_ACQUIRED_ALI_BASE + 8,
};

/* 打开设备
 * @path 路径
 * @return
 * 		成功: >=0
 * 		设备打开失败: FP_FILE_FAIL
 * 		内存分配失败: FP_NOMEM
 * 		通讯失败: FP_COMM_FAIL
 * 		SQL执行失败: FP_SQL_FAIL
 */
extern int ma_open();

/* 关闭设备
 * @return
 * 		成功: FP_OK
 * 		关闭失败: FP_FILE_FAIL
 */
extern int ma_close();

/* pre_enroll
 * @return
 *		0 if function failed
 *  	otherwise, a uint64_t of token
 */
extern uint64 ma_pre_enroll();

/* enroll
 * @return
 * 		0 if enrollment process can be successfully started
 *     	or a negative number in case of error, generally from the errno.h set.
 *      A notify() function may be called indicating the error condition.
 */
extern int ma_enroll(const hw_auth_token_t *hat, int gid, int timeout);

/* post enroll
 * @return
 * 		0 if the request is accepted
 *      or a negative number in case of error, generally from the errno.h set.
 */
extern int ma_post_enroll();

/* get authenticator id
 * @return current authenticator id or 0 if function failed.
 */
extern uint64 ma_get_auth_id();

/* cancel
 * @return
 * 		0 if cancel request is accepted
 *      or a negative number in case of error, generally from the errno.h set.
 */
extern int ma_cancel();

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
extern int ma_enumerate(fingerprint_finger_id_t *results, uint32_t *max_size);

/* remove finger
 * @gid userID
 * @fid fingerID
 * @return:
 * 		0 if fingerprint template(s) can be successfully deleted
 *      or a negative number in case of error, generally from the errno.h set.
 */
extern int ma_remove(int gid, int fid);

/* set group
 * @gid userID
 * @path store path
 * @return
 * 		0 on success
 *      or a negative number in case of error, generally from the errno.h set.
 */
extern int ma_set_active_group(int gid, const char *path);

/* authenticator
 *  @return
 *  	0 on success
 *      or a negative number in case of error, generally from the errno.h set.
 */
extern int ma_verify(uint64 operation_id, int gid);

/* set notify
 *  @return
 * 		0 if callback function is successfuly registered
 *      or a negative number in case of error, generally from the errno.h set.
 */
extern int ma_set_notify(fingerprint_notify_t notify);

#ifdef __cplusplus
}
#endif
#endif /* MA_FPRINT_H_ */


