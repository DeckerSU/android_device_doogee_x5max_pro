/*
 * fprint.h
 *
 *  Created on: 2015-1-18
 *      Author: czl
 */

#ifndef FPRINT_H_
#define FPRINT_H_
#ifdef __cplusplus
extern "C" {
#endif

/* 打开设备
 * @fdev 设备名
 * @return:
 * 		成功: >=0
 * 		设备打开失败: FP_FILE_FAIL
 * 		内存分配失败: FP_NOMEM
 */
extern int fp_open(char *fdev);

/* 关闭设备
 * @return:
 * 		成功: FP_OK
 * 		关闭失败: FP_FILE_FAIL
 */
extern int fp_close(void);

/* 设置工作
 * @return
 * 		当前工作
 */
extern int fp_setWork(int what);

/* 获取工作
 * @return
 *		当前工作
 */
extern int fp_getWork(void);

/* 设值
 * @cmd 指令
 * @val 数值
 * @return 成功: FP_OK
 */
extern int fp_setValue(int cmd, int val);

/* 取值
 * @return 成功: >=0
 *  	通讯失败: FP_COMM_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_getValue(int cmd);

/* 设置按键
 * @opt 1: 按下， 0: 抬起
 * @return 成功: FP_OK
 */
extern int fp_setKey(int opt);

/* 设置偏移量
 * @x 偏移x
 * @y 偏移y
 * @return 成功: FP_OK
 */
extern int fp_setXY(int x, int y);

/* 设置点击
 * @opt 1:单击, 2:双击, 3:长按
 * @return 成功: FP_OK
 */
extern int fp_setTap(int opt);

extern int fp_ioctl(int cmd, int arg);

/* 连接设备
 * @return:
 *  	成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 */
extern int fp_connect(void);

/* 断开设备
 * @return:
 * 		成功: FP_OK
 */
extern int fp_disconnect(void);

/* 设置SPI速度
 * @return:
 * 		成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_setSpeed(int speed);

/* 设置累加分
 * @return:
 * 		成功: FP_OK
 */
extern int fp_setEnrollAdd(int grade);

/* 采集
 * @return
 * 		成功: FP_OK
 *		通讯失败:FP_COMM_FAIL
 */
extern int fp_getImage(void);

/* 获取图像大小
 * @return 图像大小
 */
extern int fp_getImageSize(void);

/* 指纹评分
 * @bmp 位图数据
 * @len 位图大小
 * @return 分数值
 */
extern int fp_getImageScore(void *buf, int len);

/* 是否按压
 * @return 未按压: FP_OK
 * 		按压: FP_DOWN
 * 		通讯失败: FP_COMM_FAIL
 */
extern int fp_pressed(void);

/* 检测参数
 * @return
 *		成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_detectParam(void);

/* 检测模式
 * @return:
 * 		成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_detectMode(void);

/* 采图模式
 * @return:
 * 		成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_imageMode(void);

/* RESET MODE
 * @return
 * 		成功: FP_OK
 * 		通讯失败: FP_COMM_FAIL
 */
extern int fp_resetMode(void);

/* POWER DOWN
 * @return
 * 		成功: FP_OK
 * 		通讯失败: FP_COMM_FAIL
 */
extern int fp_powerDown(void);

/* 出厂初始化
 * @return:
 *  	成功: FP_OK
 *  	内存分配失败: FP_NOMEM
 *		通讯失败：FP_COMM_FAIL
 *  	选背景失败: FP_BKG_FAIL
 * 		SQL执行失败: FP_SQL_FAIL
 */
extern int fp_initFactory(void);

/* 开机初始化
 * @return:
 *		成功: FP_OK
 *  	SQL执行失败: FP_SQL_FAIL
 */
extern int fp_initBoot(void);

/* 校准
 * @return:
 *  	成功: FP_OK
 *  	内存分配失败: FP_NOMEM
 *		通讯失败：FP_COMM_FAIL
 *  	选背景失败: FP_BKG_FAIL
 * 		SQL执行失败: FP_SQL_FAIL
 */
extern int fp_calibrate(void);

/* 设置检测阀值
 * @val 数值
 * @return 成功: FP_OK
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_setCheckTH(int val);

/* 重置check
 * @return 成功: FP_OK;
 */
extern int fp_resetCheck(void);

/* 初始化图像检测
 * @return 成功: FP_OK
 */
extern int fp_initCheck(void);

/* 图像检测
 * @return:
 *  	没有手指: FP_OK，
 *  	手指按下: FP_CHK_DOWN
 *  	部分接触: FP_CHK_PART
 *  	完全接触: FP_CHK_FULL
 *  	手指移开: FP_CHK_UP
 *  	采集数据为0: FP_IMG_ZERO
 */
extern int fp_check(void);

/* 设置移动灵敏度
 * @rate 灵敏度
 * @return 成功: FP_OK
 */
extern int fp_setMoveRate(int rate);

/* 移动偏移量
 * @x 偏移量x
 * @y 偏移量y
 * @return dtc2*10+dtc1
 */
int fp_moveOffset(int *x, int *y);

/* 移动偏移量2
 * @x 偏移量x
 * @y 偏移量y
 * @return dtc2*10+dtc1
 */
extern int fp_moveOffset2(int *x, int *y);

/* 初始化注册
 * @return 成功: FP_OK
 */
extern int fp_initEnroll();

/* 注册指纹
 * @fid 指纹ID, 范围：1～5
 * @return:
 * 		成功: >0
 * 		注册失败: FP_ENROLL_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_enroll(int fid);

/* 清除注册指纹
 * @fid 指纹ID
 * @return:
 * 		成功: FP_OK
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_clearEnrollment(int fid);

/* 获取注册状态
 * @fid 指纹ID
 * @return:
 * 		已注册: >0
 * 		未注册: =0
 */
extern int fp_getEnrollment(int fid);

/* 获取注册时间
 * @fid 指纹ID
 * @return: 注册时间
 */
extern int fp_getEnrollTime(int fid);

/* 匹配指纹
 * @fid 手指id
 * @level 等级（1～5由低到高）
 * @return:
 * 		成功: FP_OK
 * 		匹配失败: FP_MATCH_FAIL
 * 		参数错误: FP_PARA_ERROR
 */
extern int fp_match(int fid, int level);

/* 获取厂商信息
 * @buf 厂商信息
 * @len 长度
 * @return 成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 */
extern int fp_getVendor(char *buf, int len);

/* 获取库版本
 * @buf 版本信息
 * @len 长度
 * @return 成功：FP_OK
 */
extern int fp_getVersion(char *buf, int len);

/* 打开数据库
 * @path 数据库路径
 * @return:
 *  	成功: FP_OK
 *  	失败: FP_DB_FAIL
 */
extern int fp_dbOpen(char *path, char *name);

/* 关闭数据库
 * @return:
 *  	成功: FP_OK
 */
extern int fp_dbClose(void);

/* 更新数据 (内存模板更新到数据库)
 * @fid 指纹ID
 * @return:
 * 		成功: FP_OK
 *		SQL执行失败: FP_SQL_FAIL
 */
extern int fp_dbUpdate(int fid);

/* 备份数据库(暂未实现)
 * @return:
 * 		成功: FP_OK
 */
extern int fp_dbBackup(void);

/* 恢复数据库(暂未实现)
 * @return:
 * 		成功: FP_OK
 */
extern int fp_dbRestore(void);

/* 调试开关
 * @deb 1:开, 0:关
 * @return:
 * 		成功: FP_OK
 */
extern int fp_setDeb(int deb);

/* 测试SPI
 * @return:
 *  	成功: FP_OK
 *  	通讯失败: FP_COMM_FAIL
 */
extern int fp_testSPI();

/* 测试图像
 * @bmp 位图数据
 * @len 位图大小
 * @return 分数值
 */
extern int fp_testImage(void *buf, int len);

/* 测试中断
 * @return
 *  	成功: 电平(0或1）
 *  	通讯失败: FP_COMM_FAIL
 */
extern int fp_testInterrupt();

/* 测试坏点
 * @return 成功: FP_OK
 * 		坏点: FP_BAD_PIXEL
 * 		通讯失败:FP_COMM_FAIL
 */
extern int fp_testBadPixel();

/* 获取属性
 * @name 属性名
 * @def 默认值
 * @return 属性值
 */
extern int fp_getProp(const char *name, int def);

/* 设置处理中断标志
 * @proc 处理:TRUE,不处理:FALSE
 * @return 成功: FP_OK
 */
extern int fp_setProcINT(int proc);

/* 获取处理中断标志
 * @return 处理中断标志
 */
extern int fp_getProcINT(void);

#ifdef __cplusplus
}
#endif
#endif /* FPRINT_H_ */


