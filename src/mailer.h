/**********************************************************
 * protocol definitions
 * $Id: mailer.h,v 1.20 2004/06/25 09:46:42 sisoft Exp $
 **********************************************************/
#ifndef __MAILER_H__
#define __MAILER_H__
#include "qcconst.h"

#define P_NCP		0x0001
#ifdef WITH_BINKP
#define P_BINKP		0x0001
#endif
#define P_ZMODEM	0x0002
#define P_ZEDZAP	0x0004
#define P_DIRZAP	0x0008
#define P_HYDRA		0x0010
#define P_JANUS		0x0020
#ifdef HYDRA8K16K
#define P_HYDRA4	0x0040
#define P_HYDRA8	0x0080
#define P_HYDRA16	0x0100
#endif
#define P_MASK		0x01FF

#define S_OK      0
#define S_NODIAL  1
#define S_REDIAL  2
#define S_BUSY    3
#define S_FAILURE 4
#define S_MASK    7
#define S_HOLDR   8
#define S_HOLDX   16
#define S_HOLDA   32
#define S_ADDTRY  64
#define S_ANYHOLD (S_HOLDR|S_HOLDX|S_HOLDA)

#define FOP_OK      0
#define FOP_CONT    1
#define FOP_SKIP    2
#define FOP_ERROR   3
#define FOP_SUSPEND 4

#define SESSION_AUTO    0
#define SESSION_EMSI    1
#define SESSION_BINKP   2

#define RX_SKIP		1
#define RX_SUSPEND	2

#define TIM_CHAT	90

/* z*.c */
extern int zmodem_receive(char *m, int canzap);
extern int zmodem_send(slist_t *, slist_t *);
extern int zmodem_sendfile(char *tosend, char *sendas,
			unsigned long *totalleft, unsigned long	*filesleft);
extern int zmodem_sendinit(int canzap);
extern int zmodem_senddone();
/* emsi.c */
extern char *emsi_makedat(ftnaddr_t *remaddr, unsigned long mail,
			unsigned long files, int lopt, char *protos,
						falist_t *adrs, int showpwd);
extern int emsi_parsedat(char *str, ninfo_t *dat);
extern void emsi_log(ninfo_t *e);
extern int emsi_send(int mode, unsigned char *dat);
extern int emsi_recv(int mode, ninfo_t *rememsi);
extern int emsi_init(int mode);
extern int emsi_parsecod(char *lcod, char *ccod);
/* protfm.c */
extern int rxopen(char *name, time_t rtime, size_t rsize, FILE **f);
extern int rxclose(FILE **f, int what);
extern FILE *txopen(char *tosend, char *sendas);
extern int txclose(FILE **f, int what);
extern void log_rinfo(ninfo_t *e);
extern void check_cps();
extern FILE *txfd;
extern FILE *rxfd;
extern long txpos;
extern long rxpos;
extern word txretries;
extern word rxretries;
extern long txwindow;
extern long rxwindow;
extern word txblklen;
extern word rxblklen;
extern long txsyncid;
extern long rxsyncid;
extern byte *txbuf;
extern byte *rxbuf;
extern dword txoptions;
extern dword rxoptions;
extern unsigned effbaud;
extern byte *rxbufptr;
extern byte txstate;
extern byte rxstate;
extern byte *rxbufmax;
extern long txstart;
extern long rxstart;
extern word txmaxblklen;
extern word timeout;
extern byte txlastc;
/* session.c */
extern int (*receive_callback)(char *fn);
extern void makeflist(flist_t **fl, ftnaddr_t *fa,int mode);
extern void flkill(flist_t **l, int rc);
extern void flexecute(flist_t *fl);
extern void addflist(flist_t **fl, char *loc, char *rem, char kill,
					 off_t off, FILE *lo, int fromlo);
extern void simulate_send(ftnaddr_t *fa);
extern int session(int mode, int type, ftnaddr_t *calladdr, int speed);
/* freq.c */
extern int freq_ifextrp(slist_t *reqs);
extern int freq_recv(char *fn);
/* call.c */
extern int do_call(ftnaddr_t *fa,char *site,char *port);
extern int force_call(ftnaddr_t *fa,int line,int flags);
/* tcp.c */
extern int tcp_dial(ftnaddr_t *fa,char *host);
extern void tcp_done(int fd);
/* for chat */
extern void chatinit(int prot);
extern void c_devrecv(unsigned char *str,unsigned len);
extern void getevt();
/* perl.c */
#ifdef WITH_PERL
#define IFPerl(x) x
extern unsigned short perl_flg;
extern int perl_init(char *script,int mode);
extern void perl_done(int rc);
extern void perl_on_reload(int mode);
extern void perl_on_std(int sub);
extern void perl_on_log(char *str);
extern int perl_on_call(ftnaddr_t *fa,char *site,char *port);
extern int perl_on_session(char *sysflags);
extern void perl_end_session(long sest,int result);
extern int perl_on_recv();
extern char *perl_end_recv(int state);
extern char *perl_on_send(char *tosend);
extern void perl_end_send(int state);
#else
#define IFPerl(x)
#endif

#endif
