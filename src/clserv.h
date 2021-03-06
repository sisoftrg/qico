/* $Id: clserv.h,v 1.9 2004/05/24 03:21:36 sisoft Exp $ */
#ifndef __CLSERV_H__
#define __CLSERV_H__

#define DEF_SERV_PORT 60178

#define CLS_UDP 1
#define CLS_SERVER 2

#define CLS_UI 0
#define CLS_LINE CLS_UDP
#define CLS_SERV_U CLS_SERVER
#define CLS_SERV_L CLS_SERVER|CLS_UDP

/* hack for some older systems */
#ifndef MSG_WAITALL
#define MSG_WAITALL 0
#endif

typedef struct _cls_cl_t {
	struct _cls_cl_t *next;
	unsigned char *auth;
	short id;
	int sock;
	char type;
} cls_cl_t;

typedef struct _cls_ln_t {
	struct _cls_ln_t *next;
	unsigned short id;
	struct sockaddr *addr;
	int emsilen;
	char *emsi;
} cls_ln_t;

extern int (*xsend_cb)(int sock,char *buf,size_t len);

extern int cls_conn(int type,char *port,char *addr);
extern void cls_close(int sock);
extern void cls_shutd(int sock);
extern int xsendto(int sock,char *buf,size_t len,struct sockaddr *to);
extern int xsend(int sock,char *buf,size_t len);
extern int xrecv(int sock,char *buf,size_t len,int wait);

#endif
