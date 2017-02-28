/*
 * type.h
 *
 *  Created on: 2014-10-22
 *      Author: czl
 */
#ifndef CTYPE_H_
#define CTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

// 返回值类型
#define FP_CANCEL			  8		//取消
#define FP_FREE				  7		//闲
#define FP_BUSY				  6		//忙
#define FP_CHK_UP			  5 	//手指离开
#define FP_PRESSED			  4		//手指已按
#define FP_CHK_FULL			  3		//全部接触
#define FP_CHK_PART			  2		//部分接触
#define FP_CHK_DOWN			  1 	//手指按下
#define FP_OK				  0 	//成功/完成
#define FP_ENROLL_FAIL		 -1		//注册失败
#define FP_MATCH_FAIL		 -2		//匹配失败
#define FP_EMPTY			 -3 	//空指纹
#define FP_NEWF_FAIL	 	 -4		//创建FID失败
#define FP_FILE_FAIL		-10		//文件打开/关闭失败
#define FP_NOMEM			-11		//内存分配失败
#define FP_PARA_ERROR		-12 	//参数错误
#define FP_COMM_FAIL		-13		//通讯失败
#define FP_DB_FAIL			-20		//数据库打开/关闭失败
#define FP_SQL_FAIL			-21		//SQL执行失败
#define FP_SQL_EMPTY		-22		//SQL数据为空
#define FP_NOCAP			-30		//未采集数据
#define FP_BKG_FAIL			-31		//选背景失败
#define FP_IMG_ZERO			-32		//采集数据为0
#define FP_BAD_PIXEL		-33		//坏点

#define TNAME	"image"

#define MAXF	   	5			//指纹个数
#define PNUM		50			//每个指纹模板数
#define PTMP  		3924		//较大每个模板大小

#define	MAXW  		120			//最大宽
#define MAXH	    192			//最大高
#define MAXB  		(16*1024)	//最大缓冲区
#define MAXI  		15360		//最大图像

//SENSOR型号
#define FS120		120
#define FS121		121
#define FS80		80
#define FS81		81

//COVER型号
#define COVER_T		1
#define	COVER_N		2
#define COVER_M		3

//DRV命令
#define DRV_DEBUG		0x100	//驱动信息
#define DRV_IRQ_ENABLE	0x101	//驱动使能
#define DRV_SPI_SPEED	0x102	//SPI速度
#define DRV_READ_LEN	0x103	//读长度(废弃)
#define DRV_LINK_DEV	0x104	//连接设备(废弃)
#define DRV_COVER_NUM 	0x105	//表面型号
#define DVR_GET_VDATE	0x106	//版本日期

#define DRV_CLR_INTF	0x110	//复位中断标志
#define DRV_GET_INTF	0x111	//获取中断标志
#define DRV_REPO_FLAG	0x112 	//上报标志
#define DRV_REPO_KEY	0x113 	//上报键值
#define DRV_SET_WORK	0x114	//设置工作
#define DRV_GET_WORK	0x115	//获取工作
#define DRV_SET_VALUE	0x116	//设值
#define DRV_GET_VALUE	0x117	//取值
#define DRV_TRIGGER		0x118	//自触发
#define DRV_WAKE_LOCK	0x119	//电源上锁
#define DRV_WAKE_UNLOCK	0x120	//电源解锁
#define DRV_KEY_DOWN	0x121	//按下
#define DRV_KEY_UP		0x122	//抬起
#define DRV_SET_X		0x123	//偏移X
#define DRV_SET_Y		0x124	//偏移Y
#define DRV_KEY_TAP		0x125	//单击
#define DRV_KEY_DTAP	0x126	//双击
#define DRV_KEY_LTAP	0x127	//长按

// APP命令
#define JNI_DEBUG		0x200	//调试信息
#define JNI_DB_CLR 		0x201	//数据表清除
#define JNI_DB_SYN		0x202	//数据同步开关

#define JNI_MODE 		0x210 	//位图模式
#define JNI_BMP_SRCH	(JNI_MODE+1)
#define JNI_BMP_SRCB	(JNI_MODE+2)
#define JNI_BMP_OUTB	(JNI_MODE+3)
#define JNI_BMP_SUPB	(JNI_MODE+4)

#define JNI_SET_SENSOR	0x220	//sensor类型
#define JNI_SET_COVER	0x221	//cover类型

//工作模式
#define DO_IDLE			0	//空闲
#define DO_ENROLL		1	//执行注册
#define DO_MATCH		2	//执行匹配
#define DO_FACTORY	 	3	//执行校准
#define DO_DETECT		4	//执行DETECT
#define DO_DEBUG		5	//执行调试
#define DO_RESTORE		6	//恢复
#define DO_GESTURE		7	//手势

#define TRUE	1
#define FALSE 	0

typedef unsigned char uchar;
struct sensor_work {
	int what;		//工作模式
	int cmd;		//命令
	int grade;		//累加分
	int cur_grade; 	//当前分数
	uchar temp[(MAXF+1)*PNUM*PTMP];	//0临时模板，1～MAXF指纹模板
};
extern struct sensor_work *gwork;

struct report_value {
	char x;
	char y;
};
union char_int {
	struct report_value repo;
	int val;
};

#ifdef __cplusplus
}
#endif

#endif /* CTYPE_H_ */
