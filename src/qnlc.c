/**********************************************************
 * nodelist compiler
 * $Id: qnlc.c,v 1.7 2004/05/27 18:50:03 sisoft Exp $
 **********************************************************/
#include "headers.h"

static int compare_idxent(const void *a,const void *b)
{
	ftnaddr_t *aa=&(((idxent_t*)a)->addr);
	ftnaddr_t *bb=&(((idxent_t*)b)->addr);
	int res;
	if((res=aa->z-bb->z))return res;
	if((res=aa->n-bb->n))return res;
	if((res=aa->f-bb->f))return res;
	if((res=aa->p-bb->p))return res;
	if((res=((idxent_t*)a)->index-((idxent_t*)b)->index))return res;
	return 0;
}


static int nl_ext(char *s)
{
	char *p,*q;
	if(!(p=strchr(s,',')))return -1;
	q=strchr(p+1,',');
	if(q)*q=0;
	if(!p)return -1;
	    else return atoi(p+1);
}

int compile_nodelists()
{
	slist_t *n;
	char fn[MAX_PATH],*p,s[MAX_STRING],nlmax[MAX_PATH];
	FILE *idx,*f;
	DIR *d;
	struct dirent *de;
	int num,max,i=0,gp=0,rc,k,total=0,line=0;
	int firsteq,lasteq,deleted,alloc=0;
	idxh_t idxh;
	idxent_t ie,*ies=NULL;
	unsigned long pos;
	struct stat sb;

	xstrcpy(idxh.sign,NL_SIGN,sizeof(idxh.sign));
	ie.addr.z=cfgal(CFG_ADDRESS)->addr.z;
	ie.addr.n=cfgal(CFG_ADDRESS)->addr.n;
	ie.addr.f=0;ie.addr.p=0;

	setbuf(stdout,NULL);
	snprintf(fn,MAX_PATH,"%s%s.lock",cfgs(CFG_NLPATH),NL_IDX);
	lockpid(fn);
	snprintf(fn,MAX_PATH,"%s%s",ccs,NL_IDX);
	if(!(idx=fopen(fn,"wb"))) {
		write_log("can't open nodelist index %s for writing: %s",fn,strerror(errno));
		snprintf(fn,MAX_PATH,"%s%s.lock",ccs,NL_IDX);lunlink(fn);
		return 0;
	}
	fseek(idx,sizeof(idxh),SEEK_SET);
	printf("compiling nodelists...\n");
	for(n=cfgsl(CFG_NODELIST);n;n=n->next) {
		xstrcpy(s,n->str,MAX_STRING);*fn=0;
		if((p=strchr(s,' '))) {
			*p++=0;
			ie.addr.z=atoi(p);
		}
		if(!strcmp(s+strlen(s)-4,".999")) {
			s[strlen(s)-4]=0;
			if(!(d=opendir(ccs))) {
				write_log("can't open nodelist directory %s: %s",ccs,strerror(errno));
				fclose(idx);
				snprintf(fn,MAX_PATH,"%s%s.lock",ccs,NL_IDX);lunlink(fn);
				return 0;
			}
			max=-1;
			while((de=readdir(d)))
				if(!strncasecmp(de->d_name,s,strlen(s))) {
					p=de->d_name+strlen(s);
					if((*p=='.')&&(strlen(p)==4)&&(strspn(p+1,"0123456789")==3)) {
						num=atoi(p+1);
						if(num>max) {
							xstrcpy(nlmax,de->d_name,MAX_PATH);
							max=num;
						}
					}
				}
			closedir(d);
			if(max<0)write_log("no lists matching %s.[0-9]",s);
			    else {
				xstrcpy(s,nlmax,MAX_STRING);
				xstrcpy(idxh.nlname[i],s,sizeof(idxh.nlname[0]));
				snprintf(fn,MAX_PATH,"%s/%s",ccs,s);
			}
		} else {
			xstrcpy(idxh.nlname[i],s,sizeof(idxh.nlname[0]));
			snprintf(fn,MAX_PATH,"%s/%s",ccs,s);
		}
		if(!*fn)continue;
		if(!stat(fn,&sb))idxh.nltime[i]=sb.st_mtime;
		if(!(f=fopen(fn,"rt")))write_log("can't open %s for reading: %s",fn,strerror(errno));
		    else {
			k=0;pos=0;line=0;
			while(1) {
				pos=ftell(f);
				line++;
				if(!fgets(s,MAX_STRING,f))break;
				if(*s==';'||*s=='\r'||*s=='\n'||*s==0x1a||!*s)continue;
				if(!strncmp(s,"Boss,",5)) {
					rc=parseftnaddr(s+5,&ie.addr,NULL,0);
					if(rc)gp=1;
					continue;
				}
				if(gp&&!strncmp(s,"Point,",6))ie.addr.p=nl_ext(s);
				if(!strncmp(s,"Zone,",5)) {
					gp=0;
					ie.addr.z=ie.addr.n=nl_ext(s);
					ie.addr.f=0;ie.addr.p=0;
				} else if(!strncmp(s,"Host,",5)||!strncmp(s,"Region,",7)) {
					gp=0;
					ie.addr.n=nl_ext(s);
					ie.addr.f=0;ie.addr.p=0;
				} else if(!gp&&strncmp(s,"Down,",5)) {
					ie.addr.f=nl_ext(s);ie.addr.p=0;
				} else if(!gp) continue;
				if(ie.addr.z<0||ie.addr.n<0||ie.addr.f<0||ie.addr.p<0) {
					write_log("can't parse address in %s at line %d",basename(fn),line);
					fclose(idx);
					snprintf(fn,MAX_PATH,"%s%s.lock",ccs,NL_IDX);lunlink(fn);
					return 0;
				}
				ie.offset=pos;ie.index=i;
 				if(total>=alloc) {
 					alloc+=8192/sizeof(ie);
 					ies=ies?xrealloc(ies,sizeof(ie)*alloc):xmalloc(sizeof(ie)*alloc);
				}
 				ies[total++]=ie;
 				k++;
			}
 			fclose(f);
			printf("compiled %s: %d nodes\n",basename(fn),k);
			i++;
			if(i>MAX_NODELIST) {
				write_log("too much lists - increase MAX_NODELIST in config.h and rebuild");
				break;
			}
		}
	}
	if(total) {
		qsort(ies,total,sizeof(ie),compare_idxent);
		/* Delete duplicates */
		k=deleted=0;firsteq=lasteq=-1;
		while(k<total-1) {
			if(!memcmp(&ies[k].addr,&ies[k+1].addr,sizeof(ies->addr))) {
				if(firsteq<0)firsteq=k;
				lasteq=k+1;
				k++;
			} else if(firsteq>=0) {
				memcpy(&ies[firsteq+1],&ies[lasteq+1],sizeof(*ies)*(total-lasteq-1));
				firsteq=lasteq=-1;
				deleted++;
				total--;
			} else k++;
		}
		printf("delete %d duplicate records\n",deleted);
		if(fwrite(ies,sizeof(*ies),total,idx)!=total) {
			xfree(ies);
			write_log("can't write to index: %s",strerror(errno));
			fclose(idx);
			snprintf(fn,MAX_PATH,"%s%s.lock",ccs,NL_IDX);lunlink(fn);
			return 0;
		}
		xfree(ies);
	}
	printf("total %d lists, %d nodes\n",i,total);
	fseek(idx,0,SEEK_SET);
	if(fwrite(&idxh,sizeof(idxh),1,idx)!=1)write_log("can't write to index: %s",strerror(errno));
	fclose(idx);
	snprintf(fn,MAX_PATH,"%s%s.lock",ccs,NL_IDX);lunlink(fn);
	return 1;
}
