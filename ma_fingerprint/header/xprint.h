/*
 * print.h
 *
 *  Created on: 2014-8-20
 *      Author: czl
 */


#include <android/log.h>

#ifndef CLOG_H_
#define CLOG_H_
#ifdef __cplusplus
extern "C" {
#endif

#define CTAG 	"CTAG"
#define WTAG 	"WTAG"

extern void set_debug(int deb);

extern void dprint(const char *tag, const char *fmt, ...);
extern void nprint(const char *tag, char *buf, int len);
extern void wprint(const char *tag, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif /* MYLOG_H_ */
