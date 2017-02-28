#include <errno.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <poll.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <endian.h>
#include <sys/sysinfo.h>
#include "ma_fprint.h"
#include "header/type.h"
#include "header/xprint.h"
#include "header/fprint.h"

#define STD_INFO "std_version: 3.0 beta6, build time: 2016-08-16/15:07"
#define MTAG 	"MTAG"
#define DUPLICATE_AREA		//重复区域
#define DUPLICATE_FINGER		//重复手指

struct mafprint {
	char path[128];		//路径
	int fdev; 		//设备
	int fdb;		//数据库
	int gid; 		//用户ID
	int fid;		//指纹ID
	int grade;		//注册分数
	volatile int stop;	//停止标志
	volatile int wait;  	//等待标志
	volatile int fpok; 	//成功标志
	volatile int first; 	//首次标志
	volatile int busy;  	//忙标志
	uint64 oper_id;
	uint64 auth_id;
	hw_auth_token_t hat;
};
static struct mafprint smaf = {
	.path = "\0",
	.fdev = -1,
	.fdb = -1,
	.fpok = FP_OK,
	.grade = 0,
	.busy = FALSE,
	.first = 1,
};

struct fp_gesture {
	uchar nav;
	int tap_tm;
	int dtap_min_tm;
	int dtap_max_tm;
	int ltap_tm;
};
static struct fp_gesture sfpg = {
	.nav = 1,
	.tap_tm = 100,
	.dtap_min_tm = 40,
	.dtap_max_tm = 300,
	.ltap_tm = 500,
};

struct fp_config {
	int times; 		//注册次数
	int level;		//匹配等级
	int detTH;		//检测阀值
	int tm_dtap;		//双击时间
	int tm_lpress; 		//长按时间
};
static struct fp_config sfpc = {
	.times = 15,
	.level = 4,
	.detTH = 128,
	.tm_dtap = 360,
	.tm_lpress = 500,
};

static pthread_t sthread;
static fingerprint_notify_t snotify;
static void* thread_run(void* arg);

static int read_property(void) {
	int ret, val, tmp, grade;

	dprint(MTAG, "[Decker] read_property", __func__);

	/*
	sfpg.nav = fp_getProp("persist.sys.fp.navigation", 0); //导航开关
	dprint(MTAG, "%s: navigation=%d\n", __func__, sfpg.nav);

	val = fp_getProp("persist.sys.fp.enroll.times", 15); //注册次数
	tmp = (100%val>0)? 1: 0;
	grade = 100/val + tmp;
	fp_setEnrollAdd(grade);
	dprint(MTAG, "%s: enroll times=%d grade=%d\n", __func__, val, grade);

	sfpc.level = fp_getProp("persist.sys.fp.match.level", 5); //匹配等级
	dprint(MTAG, "%s: level=%d\n", __func__, sfpc.level);

	sfpc.detTH = fp_getProp("persist.sys.fp.check.detTH", 10); //检测阀值
	fp_setCheckTH(sfpc.detTH);
	dprint(MTAG, "%s: detTH=%d\n", __func__, sfpc.detTH);

	sfpc.tm_dtap = fp_getProp("persist.sys.fp.dtap.time", 360); //双击时间
	dprint(MTAG, "%s: dtap_time=%d\n", __func__, sfpc.tm_dtap);

	sfpc.tm_lpress = fp_getProp("persist.sys.fp.lpress.time", 500); //长按时间
	dprint(MTAG, "%s: lpress_time=%d\n", __func__, sfpc.tm_lpress);

	val = fp_getProp("persist.sys.fp.debug.drv", 0); //驱动信息开关
	fp_setValue(DRV_DEBUG, val);
	dprint(MTAG, "%s: debug_drv=%d\n", __func__, val);

	val = fp_getProp("persist.sys.fp.debug.lib", 0);  //LIB库信息开关
	fp_setDebug(val);
	dprint(MTAG, "%s: debug_lib=%d\n", __func__, val);
	*/

	return FP_OK;
}

static int save_token(int fid, const void *token, int len) {
	char name[128];
	int ret;

	dprint(MTAG, "%s: start. len=%d\n", __func__, len);
	sprintf(name, "token%d", fid);
	ret = db_write(TNAME, name, token, len);
	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret;
}

static int load_token(int fid, void *token, int len) {
	char name[128];
	int ret;

	dprint(MTAG, "%s: start. len=%d\n", __func__, len);
	sprintf(name, "token%d", fid);
	ret = db_read(TNAME, name, token, len);
	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret;
}

/* 打开设备
 * @return
 * 		成功: >=0
 * 		设备打开失败: FP_FILE_FAIL
 * 		内存分配失败: FP_NOMEM
 * 		通讯失败: FP_COMM_FAIL
 */
int ma_open() {
	int ret, val;
	char buf[128];
	char *dev = "/dev/madev0";

	wprint(MTAG, "%s: %s\n", __func__, STD_INFO);
	dprint(MTAG, "%s: start. fdev=%d\n", __func__, smaf.fdev);

	if(smaf.fdev!=-1) { //设备已打开
		wprint(WTAG, "%s: already do func.\n", __func__);
		return smaf.fdev;
	}
	smaf.wait = 0;
	ret = fp_open(dev);
	if(ret>=0) smaf.fdev = ret;
	// read_property();
	pthread_create(&sthread, NULL, thread_run, NULL);

	dprint(MTAG, "%s: end. fptr=%d fpok=%d\n", __func__, ret, smaf.fpok);

	return ret;
}

/* 打开数据库
 * @path 路径
 * @return
 * 		成功: >=0
 *  		SQL执行失败: FP_SQL_FAIL
 *  		通讯失败: FP_COMM_FAIL
 * 		检测参数失败: FP_IMG_ZERO
 */
static int dbOpen(const char *path) {
	int i, ret;
	char *dbn = "fprint.db";

	dprint(MTAG, "%s: start. fdb=%d\n", __func__, smaf.fdb);

	if(smaf.fdb!=-1) {
		wprint(WTAG, "%s: already do func.\n", __func__);
		return smaf.fdb;
	} else if(smaf.fdev>=0) {
		// fp_setProcINT(FALSE);
		ret = fp_dbOpen((char*)path, dbn);
		if(ret==FP_OK) {
			smaf.fdb = ret;
			smaf.fpok = ret = fp_initBoot();
			if(smaf.fpok!=FP_OK) { //首次执行校准
				fp_setWork(DO_FACTORY);
				for(i=0; i<16; i++) {
					msleep(1000);
					dprint(MTAG, "%s: wait i=%d, fpok=%d\n", __func__, i, smaf.fpok);
					if(smaf.fpok==FP_OK) {
						dprint(MTAG, "%s: calibrate successfully.\n", __func__);
						break;
					}
				}
				ret = FP_OK;
				if(smaf.fpok!=FP_OK) dprint(MTAG, "%s: calibrate failed.\n", __func__);
			} else { //首次设置DETECT参数
				int fid, val[MAXF], what, suc=0;
				for(fid=1; fid<=MAXF; fid++) { //检测是否有注册指纹
					val[fid-1] = fp_getEnrollment(fid);
					if(val[fid-1]>0) suc = 1;
				}
				dprint(MTAG, "%s: temp %d %d %d %d %d\n",
					__func__, val[0], val[1], val[2], val[3], val[4]);
				fp_detectParam();
				what = suc==1? DO_MATCH: DO_IDLE;
				fp_setWork(what);
				fp_detectMode();
			}
		}
	}

	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret;
}

/* 关闭数据库
 * @return:
 *  	成功: FP_OK
 */
static int dbClose() {
	int i=-1, ret = 0;

	dprint(MTAG, "%s: start. \n", __func__);

	smaf.stop = TRUE;
	for(i=0; i<10; i++) {
		msleep(50);
		if(smaf.busy==FALSE) break;
	}
	ret = fp_dbClose();
	smaf.fdb = -1;

	dprint(MTAG, "%s: end. fdb=%d\n", __func__, smaf.fdb);

	return ret;
}

static void notify_msg(int msg, int type) {
	fingerprint_acquired_t faq = {
		.acquired_info = type,
	};
	fingerprint_msg_t fm = {
		.type = msg,
		.data.acquired = faq,
	};
	snotify(&fm);
}

static void notify_enroll(int gid, int fid, int grade) {
	int remain = 100 - grade;
	fingerprint_enroll_t fe = {
		.finger = {
			.gid = gid,
			.fid = fid,
		},
		.samples_remaining = (remain>=0? remain: 0),
		.msg = 1000,
	};
	fingerprint_msg_t fm = {
		.type = FINGERPRINT_TEMPLATE_ENROLLING,
		.data.enroll = fe
	};
	snotify(&fm);
	dprint(MTAG, "%s: remain=%d\n", __func__, remain);
}

uint64 timestamp(void) {
	struct timespec ts;
	uint64 gap;

	//dprint(MTAG, "%s: start.\n", __func__);
	clock_gettime(CLOCK_BOOTTIME, &ts);
	gap = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
	//dprint(MTAG, "%s: end. gap=%ldms\n", __func__, gap);

	return gap;
}

/* 匹配通知
 * @gid
 * @fid 1~5成功, 0失败
 */
static void notify_match(int gid, int fid) {
	fingerprint_acquired_t faq = {
		.acquired_info = FINGERPRINT_ACQUIRED_GOOD,
	};
	fingerprint_msg_t fm = {
		.type = FINGERPRINT_ACQUIRED,
		.data.acquired = faq,
	};
	fingerprint_authenticated_t fa;
	hw_auth_token_t hat;

	dprint(MTAG, "%s: start. fid=%d\n", __func__, fid);

	snotify(&fm);
	fm.type = FINGERPRINT_AUTHENTICATED;
	fa.finger.fid = fid;
	fa.finger.gid = gid;

	if(fid>0) {
		load_token(fid, &hat, sizeof(hat));
		fa.hat.version = 0;
		fa.hat.challenge = smaf.oper_id;
		fa.hat.user_id = smaf.oper_id==0? hat.user_id: smaf.auth_id;
		fa.hat.authenticator_id = htobe64(1);
		fa.hat.authenticator_type = htonl(2);
		fa.hat.timestamp = htobe64(timestamp());
		memset(fa.hat.hmac, 0, sizeof(fa.hat.hmac));
	}
	fm.data.authenticated = fa;
	snotify(&fm);

	dprint(MTAG, "%s: end.\n", __func__);
}

/* 关闭设备
 * @return
 * 		成功: FP_OK
 * 		关闭失败: FP_FILE_FAIL
 */
int ma_close() {
	int ret=FP_OK;

	dprint(MTAG, "%s: start. \n", __func__);

	pthread_join(sthread, NULL);
	dbClose();
	ret = fp_close();
	smaf.fdev = -1;

	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret;
}

/* pre_enroll
 * @return
 *		0 if function failed
 *  	otherwise, a uint64_t of token
 */
uint64 ma_pre_enroll() {
	static uint32 num = 123456;
	uint64 ret, val;

	dprint(MTAG, "%s: start.\n", __func__);

	val = rand_r(&num);
	ret = (((uint64)num<<16)<<16) | val;

	dprint(MTAG, "%s: end. num=%u ret=0x%llx\n", __func__, num, ret);

	return ret;
}

/* 创建指纹ID
 * @return 成功: FID
 * 		失败: FP_NEWF_FAIL
 */
static int new_fid(void) {
	int fid, ret;

	dprint(MTAG, "%s: start.\n", __func__);

	for(fid=1; fid<=MAXF; fid++) {
		ret = fp_getEnrollment(fid);
		dprint(MTAG, "%s: finger%d tnum=%d\n", __func__, fid, ret);
		if(ret==0) break;
	}

	dprint(MTAG, "%s: end ret=%d fid=%d.\n", __func__, ret, fid);

	return (ret==0)? fid: FP_NEWF_FAIL;
}

/* enroll
 * @return
 * 	0 if enrollment process can be successfully started
 *     	or a negative number in case of error, generally from the errno.h set.
 *      A notify() function may be called indicating the error condition.
 */
int ma_enroll(const hw_auth_token_t *hat, int gid, int timeout) {
	int i=-1, ret=FP_OK;

	dprint(MTAG, "%s: start. gid=%d timeout=%d\n", __func__, gid, timeout);

	smaf.hat = *hat;
	if(smaf.fdev!=-1 && smaf.fdb!=-1) {
		smaf.stop = TRUE;
		for(i=0; i<10; i++) {
			msleep(50);
			if(smaf.busy==FALSE) break;
		}
		smaf.stop = FALSE;
		smaf.gid = gid;
		smaf.fid = new_fid();
		fp_setWork(DO_ENROLL);
		fp_initEnroll();
		smaf.grade = 0;
	}

	dprint(MTAG, "%s: end. i=%d ret=%d\n", __func__, i, ret);

	return ret;
}

/* post enroll
 * @return
 * 		0 if the request is accepted
 *      or a negative number in case of error, generally from the errno.h set.
 */
int ma_post_enroll() {
	int ret = FP_OK;

	dprint(MTAG, "%s: start.\n", __func__);

	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret;
}

/* get authenticator id
 * @return current authenticator id or 0 if function failed.
 */
uint64 ma_get_auth_id() {
	uint32 num = 234567;
	uint64 ret, val;

	dprint(MTAG, "%s: start.\n", __func__);

	val = rand_r(&num);
	ret = (((uint64)num<<16)<<16) | val;
	smaf.auth_id = ret;

	dprint(MTAG, "%s: end. num=%u ret=0x%llx\n", __func__, num, ret);

	return ret;
}

/* cancel
 * @return
 * 		0 if cancel request is accepted
 *      or a negative number in case of error, generally from the errno.h set.
 */
int ma_cancel() {
	int i, ret=FP_OK;

	dprint(MTAG, "%s: start.\n", __func__);

	if(smaf.fdev!=-1 && smaf.fdb!=-1) {
		smaf.stop = TRUE;
		for(i=0; i<10; i++) {
			msleep(50);
			if(smaf.busy==FALSE) break;
		}
		fp_setWork(DO_IDLE);
	}

	dprint(MTAG, "%s: end. i=%d ret=%d\n", __func__, i, ret);

	return ret;
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
 * Function return: Total number of fingerprint templates in the current storage directory.
 *     or a negative number in case of error, generally from the errno.h set.
 */
int ma_enumerate(fingerprint_finger_id_t *results, uint32 *max_size) {
	int id, ret;

	dprint(MTAG, "%s: start.\n", __func__);

	*max_size = 0;
	for(id=1; id<=MAXF; id++) {
		ret = fp_getEnrollment(id);
		if(ret>0) {
			*max_size += 1;
			results[id-1].gid = smaf.gid;
			results[id-1].fid = id;
		}
	}

	dprint(MTAG, "%s: end. size=%d\n", __func__, *max_size);
	return ret;
}

/* authenticator
 *  @return
 *  	0 on success
 *      or a negative number in case of error, generally from the errno.h set.
 */
int ma_verify(uint64 operation_id, int gid) {
	int i=-1, ret = FP_OK;

	dprint(MTAG, "%s: start. oper_id=0x%llx gid=%d\n", __func__, operation_id, gid);

	smaf.oper_id = operation_id;
	if(smaf.fdev!=-1 && smaf.fdb!=-1) {
		int what = fp_getWork();
		if (what != DO_DEBUG) {
			smaf.stop = TRUE;
			for(i=0; i<10; i++) {
				msleep(50);
				if(smaf.busy==FALSE) break;
			}
			smaf.stop = FALSE;
			smaf.gid = gid;
			smaf.first = 0;
			fp_setWork(DO_MATCH);
		}
	}

    dprint(MTAG, "%s: end. i=%d ret=%d\n", __func__, i, ret);

    return ret;
}

/* remove finger
 * @gid userID
 * @fid fingerID
 * @return:
 * 		0 if fingerprint template(s) can be successfully deleted
 *      or a negative number in case of error, generally from the errno.h set.
 */
int ma_remove(int gid, int fid) {
	int id, ret=-1;
	fingerprint_finger_id_t fit = {
        .gid = gid,
		.fid = fid,
    };
    fingerprint_removed_t frt = {
    	.finger = fit,
    };
    struct fingerprint_msg fm = {
		.type = FINGERPRINT_TEMPLATE_REMOVED,
		.data.removed = frt,
    };

    dprint(MTAG, "%s: start. gid=%d fid=%d\n", __func__, gid, fid);

    if(smaf.fdev!=-1) {
		if(fid>0) {
			ret = fp_clearEnrollment(fid);
			if(ret==FP_OK) {
				ret = fp_dbUpdate(fid);
				if(ret==FP_OK) snotify(&fm);
			}
		} else if(fid==0) {
			for(id=1; id<=MAXF; id++) {
				fm.data.removed.finger.fid = id;
				ret = fp_clearEnrollment(id);
				if(ret==FP_OK) {
					ret = fp_dbUpdate(id);
					if(ret==FP_OK) snotify(&fm);
				}
			}
		}
    }

    dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

    return ret;
}

/* set group
 * @gid userID
 * @path store path
 * @return
 * 		0 on success
 *      or a negative number in case of error, generally from the errno.h set.
 */
int ma_set_active_group(int gid, const char *path) {
	int ret=FP_OK;

	dprint(MTAG, "%s: start. gid=%d path=%s\n", __func__, gid, path);

	smaf.gid = gid;
	if(smaf.fdb==0) {	
		dbClose();
	}
	strcpy(smaf.path, path);
	ret = dbOpen(path);

	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret>=0? 0: ret;
}

/* set notify
 *  @return
 * 		0 if callback function is successfuly registered
 *      or a negative number in case of error, generally from the errno.h set.
 */
int ma_set_notify(fingerprint_notify_t notify) {
	int ret = FP_OK;

	dprint(MTAG, "%s: start.\n", __func__);

	snotify = notify;

	dprint(MTAG, "%s: end. ret=%d\n", __func__, ret);

	return ret;
}

/* 手指检测
 * @return
 * 		取消：FP_CANCEL
 * 		无手指:	FP_OK
 * 		手指按压: FP_PRESSED
 * 		部分接触：FP_CHK_PART
 * 		全部接触：FP_CHK_FULL
 * 		手指离开：FP_CHK_UP
 */
static int check(int timeout) {
	struct timeval tv1, tv2;
	int ret=FP_OK, gap=0;
	uchar fdown=0; //按下标志
	
	dprint(MTAG, "%s ########: start. stop=%d\n", __func__, smaf.stop);
	smaf.busy = TRUE;
	gettimeofday(&tv1, NULL);	
	fp_initCheck();
	while(gap<timeout) {
		int what = fp_getWork();
		// int repo = fp_getProp("persist.sys.fp.key.repo", 1);
		int repo = 1;
		if(smaf.stop==TRUE || what==DO_DEBUG) {
			ret = FP_CANCEL;
			break;
		}
		ret = fp_getImage();
		if(ret!=FP_OK) break;
		ret = fp_check();
		dprint(MTAG, "%s ######## fp_check: ret=%d\n", __func__, ret);
		if(ret==FP_CHK_UP) { //抬起
			dprint(MTAG, "%s: finger up.\n", __func__);
			//if(what==DO_IDLE || repo==1) fp_setValue(DRV_KEY_UP, 0);
			//if(what==DO_IDLE) fp_setCmd(DRV_KEY_UP, 0);
			dprint(MTAG, "%s ######## ret==FP_CHK_FULL || ret==FP_CHK_PART\n", __func__);
			//myioctl(DRV_KEY_UP, 0);
			break;
		} else if(ret==FP_CHK_FULL || ret==FP_CHK_PART) { //全部、部分接触
			dprint(MTAG, "%s ######## ret==FP_CHK_FULL || ret==FP_CHK_PART\n", __func__);
			break;
		} else if(ret==FP_CHK_DOWN && !fdown) { //按压
			dprint(MTAG, "%s: finger down.\n", __func__);
			//if(what==DO_IDLE || repo==1) fp_setValue(DRV_KEY_DOWN, 0);
			//if(what==DO_IDLE) fp_setCmd(DRV_KEY_DOWN, 0);
			//myioctl(DRV_KEY_DOWN, 0);
			fdown = 1;
			timeout += 300;	//延时
		}
		gettimeofday(&tv2, NULL);
		gap = ((tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec)) / 1000;
		if(gap>timeout) break;
	}
	if(fdown==1 && (ret==FP_CHK_UP||ret==FP_OK)) ret = FP_CHK_PART; //快速点击
	smaf.busy = FALSE;

	dprint(MTAG, "%s ########: end. ret=%d\n", __func__, ret);
	
	return ret;
}

/* 手势检测
 * @timeout 超时
 * @return
 * 		手势完成: FP_OK
 * 		手势取消: FP_CANCEL
 */
static int gesture(int timeout) {
	struct timeval tv1, tv2;
	int x, y, tx=0, ty=0;
	int dtc1, dtc2;
	int gap, val, ret=FP_OK;
	uchar done=0;	//0:未完成，完成(1单击,2双击,3长按,4移动)
	uchar fdown=0;	//按下标志
	uchar dcnt=0;	//按下次数
	uchar ucnt=0;	//抬起次数

	dprint(MTAG, "%s: start.\n", __func__, timeout);

	smaf.busy = TRUE;
	gettimeofday(&tv1, NULL);
	while(1) {
		int what = fp_getWork();
		if(smaf.stop==TRUE || what!=DO_GESTURE) {
			dprint(MTAG, "%s: stop==1\n", __func__);
			ret = FP_CANCEL;
			break;
		}
		val = fp_moveOffset(&x, &y);
		dtc1 = val%10;
		dtc2 = val/10;
		if(dtc1==1 && dtc2==1) { //累加偏移
			tx += x;
			ty += y;
		}
		dprint(MTAG, "%s: dtc1=%d dtc2=%d x=%d y=%d\n", __func__, dtc1, dtc2, x, y);

		gettimeofday(&tv2, NULL);
		gap = ((tv2.tv_sec - tv1.tv_sec) * 1000000 + (tv2.tv_usec - tv1.tv_usec)) / 1000;
		if(dtc1+dtc2>0) {
			if(fdown==0 && done==0) { //按下
				dprint(MTAG, "%s: press down gap=%d\n", __func__, gap);
				fdown = 1;
				dcnt++;
			}
		} else if(dtc2==0) {
			if(fdown==1) { //抬起
				dprint(MTAG, "%s: press up\n", __func__);
				fdown = 0;
				ucnt++;
			}
		}
		if(done>0) { //等待手指离开
			if(fdown==0) break;
			else continue;
		} else if(gap>timeout) {
			wprint(MTAG, "%s: timeout.\n", __func__);
			break;
		}
		if(done==0) {
			if(gap>=sfpg.ltap_tm) {
				if(dcnt==1) {  //长按、移动
					done = (tx==0 && ty==0)? 3: 4;
					//done==3? fp_setTap(done): fp_setXY(tx, ty);
				}
			} else if(gap>sfpg.dtap_max_tm) { //单击、双击和移动
				if(dcnt==1 && ucnt==1) {  //单击、移动
					done = (tx==0 && ty==0)? 1: 4;
					//done==1? fp_setTap(done): fp_setXY(tx, ty);
				} else if(dcnt>=2) {  //双击、移动
					const int TH = 6;
					done = (abs(tx)<TH && abs(ty)<TH)? 2: 4;
					//done==2? fp_setTap(done): fp_setXY(tx, ty);
				}
			}
			if(done>0) {
				dprint(MTAG, "%s: done=%d tx=%d ty=%d dcnt=%d ucnt=%d gap=%dms\n",
					__func__, done, tx, ty, dcnt, ucnt, gap);
			}
		}
	}
	smaf.busy = FALSE;

	dprint(MTAG, "%s: end. ret=%d done=%d\n", __func__, ret, done);

	return ret;
}

static int enroll(int val) {
	int id, ret, grade;

	dprint(MTAG, "%s: start. val=%d\n", __func__, val);

	if(val==FP_PRESSED || smaf.grade==100) return smaf.grade;
	if(val==FP_CHK_PART) { //部分接触
		dprint(MTAG, "%s: FINGERPRINT_ACQUIRED_LOW_COVER\n", __func__);
		notify_msg(FINGERPRINT_ACQUIRED, FINGERPRINT_ACQUIRED_LOW_COVER);
	} else if(val==FP_CHK_FULL) { //完全接触
#ifdef DUPLICATE_FINGER
		for(id=1; id<=MAXF; id++) {
			int ret = fp_match(id, 4);
			if(ret==FP_OK) { //重复手指
				dprint(MTAG, "%s: FINGERPRINT_ACQUIRED_DUPLICATE_FINGER\n", __func__);
				notify_msg(FINGERPRINT_ACQUIRED, FINGERPRINT_ACQUIRED_DUPLICATE_FINGER);
				msleep(3000);
				return smaf.grade;
			}
		}
#endif
		grade = fp_enroll(smaf.fid);
#ifdef DUPLICATE_AREA
		if(grade==smaf.grade+2) { //重复区域
			notify_msg(FINGERPRINT_ACQUIRED, FINGERPRINT_ACQUIRED_DUPLICATE_AREA);
		}
#endif
		smaf.grade = grade;
		notify_enroll(smaf.gid, smaf.fid, grade);
		if(smaf.grade==100) {
			fp_dbUpdate(smaf.fid);
			save_token(smaf.fid, &smaf.hat, sizeof(smaf.hat));
		}
	}

	dprint(MTAG, "%s: end. sgrade=%d\n", __func__, smaf.grade);

	return smaf.grade;
}

static int match(int val) {
	int suc=0;
	int id, ret, cnt=0;

	dprint(MTAG, "%s: start. val=%d\n", __func__, val);

	for(id=1; id<=MAXF; id++) {
		int ret = fp_match(id, sfpc.level);
		if(ret==FP_OK) {
			suc = id;
			break;
		} else if(ret==FP_EMPTY) { //计算空指纹次数
			cnt++;
		}
	}
	if(suc>0) { //匹配成功
		dprint(MTAG, "%s: fp_match successfully.\n", __func__);
		notify_match(smaf.gid, suc);
		fp_dbUpdate(suc);
	} else if(cnt<MAXF) { //匹配失败
		dprint(MTAG, "%s: fp_match failed. \n", __func__);
		notify_match(smaf.gid, 0);
	}

	dprint(MTAG, "%s: end.\n", __func__);

	return suc;
}

typedef void (*handler_t)(int);
static void handler(int sig) {
	handler_t pfunc = handler;
	int what;

	dprint(MTAG, "%s: start. swait=%d\n", __func__, smaf.wait);

	what = fp_getWork();
	if(what==DO_MATCH && smaf.wait==TRUE) fp_setWork(DO_DETECT);
	signal(SIGALRM, pfunc);
	alarm(180);

    dprint(MTAG, "%s: end. what=%d\n", __func__, what);
}

/* 中断事件
 * @what
 * @return
 */
static void interrupt_event(int what) {
	int i, j, proc;
	int ret, repo, nav;

	dprint(MTAG, "%s: start. what=%d\n", __func__, what);

	// if(what==DO_DEBUG || fp_getProcINT()==FALSE) return;
	fp_setValue(DRV_WAKE_LOCK, 0);
	// nav = fp_getProp("persist.sys.fp.navigation", 0);
	// repo = fp_getProp("persist.sys.fp.key.repo", 1);
	nav = 0;
	repo = 1;

	dprint(MTAG, "%s: repo=%d\n", __func__, repo);
	fp_imageMode();
	if(what==DO_IDLE && repo==1) { //应用上报事件
		smaf.stop = FALSE;
		for(i=0, j=0; i<10; i++) {
			if(what!=DO_IDLE) break;
			ret = check(300);
			dprint(MTAG, "%s: app i=%d check ret=%d", __func__, i,  ret);
			if(ret==FP_CANCEL || ret==FP_CHK_UP) { //取消
				if(smaf.stop==TRUE) smaf.stop = FALSE;
				fp_resetCheck();
				break;
			} else if(ret==FP_OK) { //防止误触发中断，3次无手指结束循环(或手已抬起）
				j++;
				if(j==3) break;
			}
		}
	} else if(what==DO_ENROLL || what==DO_MATCH ) { //注册、匹配
		// ########
		dprint(MTAG, "%s ########: sys i=%d check ret=%d", __func__, i,  ret);
		for(i=0, j=0; i<20; i++) {
			ret = check(300);
			dprint(MTAG, "%s: sys i=%d check ret=%d", __func__, i,  ret);
			if(ret==FP_COMM_FAIL) { //通讯失败
				notify_msg(FINGERPRINT_ERROR, FINGERPRINT_ERROR_HW_UNAVAILABLE);
			} else if(ret==FP_CANCEL || ret==FP_CHK_UP) { //取消
				if(smaf.stop==TRUE) smaf.stop = FALSE;
				fp_resetCheck();
				break;
			} else if(ret==FP_CHK_FULL || ret==FP_CHK_PART) {
				if(what==DO_ENROLL) {
					dprint(MTAG, "%s ########: ret = %d ", __func__,ret);
					enroll(ret);
				}
				else if(what==DO_MATCH) match(ret);
			} else if(ret==FP_PRESSED && smaf.first==0) {
				if(what==DO_ENROLL) enroll(ret);
				else if(what==DO_MATCH) match(ret);
				smaf.first = 1;
			} else if(ret==FP_OK) { //防止误触发中断，3次无手指结束循环
				j++;
				if(j==3) break;
			}
		}
	} else if(what==DO_GESTURE && sfpg.nav==1) { //手势
		ret = gesture(600);
		dprint(MTAG, "%s: check ret=%d", __func__, ret);
		if(ret==FP_COMM_FAIL) { //通讯失败
			notify_msg(FINGERPRINT_ERROR, FINGERPRINT_ERROR_HW_UNAVAILABLE);
		} else if(ret==FP_CANCEL || ret==FP_OK) { //取消
			if(smaf.stop==TRUE) smaf.stop = FALSE;
			fp_resetCheck();
		}
	}
	fp_detectParam();
	fp_detectMode();
	fp_setValue(DRV_WAKE_UNLOCK, 0);

	dprint(MTAG, "%s: end.\n", __func__);
}

/* 唤醒事件
 * @what
 * @return
 */
static void wakeup_event(int what) {
	int i, ret;

	dprint(MTAG, "%s: start. what=%d\n", __func__, what);

	fp_setValue(DRV_WAKE_LOCK, 0);
	if(what==DO_DETECT) { //检测
		fp_setWork(DO_MATCH);
	} else if(what==DO_FACTORY) { //校准
		dprint(MTAG, "%s: do_calibrate.\n", __func__);
		smaf.fpok = fp_calibrate();
		fp_setWork(DO_IDLE);
	} else if(what==DO_DEBUG ) { //调试
		smaf.stop = TRUE;
		for(i=0; i<30; i++) {
			dprint(MTAG, "%s: wait i=%d\n", __func__, i);
			msleep(100);
			if(smaf.busy==FALSE) break;
		}
	} else if(what==DO_RESTORE) { //恢复
		fp_initBoot();
	}
	dprint(MTAG, "%s: what=%d\n", __func__, what);
	if(what!=DO_DEBUG) { //配置中断参数
		fp_imageMode();
		ret = fp_pressed();
		if(ret==FP_CHK_DOWN) {
			// fp_setProcINT(TRUE);
			interrupt_event(what);
		} else {
			fp_detectParam();
			fp_detectMode(); //手指按压配置不触发中断
		}
	}
	fp_setValue(DRV_WAKE_UNLOCK, 0);

	dprint(MTAG, "%s: end.\n", __func__);
}

static void* thread_run(void* arg) {
	struct pollfd event;
	int i, j, ret=0;
	int repo, what, prev;

	dprint(MTAG, "%s: start.\n", __func__);

	handler(60);
	event.fd = smaf.fdev;
	event.events = POLLIN | POLLPRI; //等待要发生的事件
    while(1) {
		smaf.wait = TRUE;
		poll(&event, 1, -1); //等待事件
		smaf.wait = FALSE;
		what = fp_getWork();
		if(event.revents & POLLIN) { //中断事件
			interrupt_event(what);
		} else if(event.revents & POLLPRI) { //唤醒事件
			wakeup_event(what);
		}
	}

	dprint(MTAG, "%s: end.\n", __func__);
}


