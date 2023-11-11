// Define timer to exit on fix time instead of fix prof 
#define TIMER
   //#define MAXPROF 10

/*
Cours Initiation à l'IA, Sup Aero, JM Alliot
*/
/* Compile with:
gcc -W -Wall -O3 p4_iter_hash_bmove.c
The program uses __attribute__((packed)) to reduce the global size
of hash tables. This might result in slower programs or even
in core dumps. Be Warned and Check!
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>

#define INFINITE 999999999

#ifndef SIZEX
#define SIZEX 7
#endif
#ifndef SIZEY
#define SIZEY 6
#endif

#define EXIT_ERROR 126
#define MAXV INT16_MAX
#define WIN (MAXV-1)
#define BWIN (WIN-SIZEX*SIZEY)
#define GET_OUT 32768

#ifndef NB_BITS
#if (SIZEX<6)
#define NB_BITS 26
#elif (SIZEX==6)
#define NB_BITS 27
#else
#define NB_BITS 28
#endif
#endif

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


/*  handler function for signal 
 */

int get_out = false;
#ifdef TIMER
void handler(int __attribute__ ((unused)) signum) {get_out=true;};
#endif

long int uuid=0;

#define EMPTY 0
#define WHITE 1
#define BLACK -1

int align_c[] ={0,2,11,101,101,101,101};
int align_nc[]={0,1,10,100,100,100,100};

/*
  The evaluation function is far from being perfect.
  But it looks like it is working OK.
  Currently we don't return ranking informations */
int compute(int color,int col1,int col2,
	    int n11,int n12,int n13,int n21,int n22,int n23) {
  int v;
  
  if (col1==color) {
    if (n11>=3) return WIN;
    if (col2==col1) {
      if ((n11+n21)>=3) return WIN;
      if ((n11+n12+n13+n21+n22+n23)>=3) return align_c[n11+n21];
      else return 0;
    }
    else {/* col2!=col1 */
      if (col2==0) {
	if ((n11+n12+n13+n21+n22+n23)>=3) return align_c[n11];
	else return 0;
      }
      else {/* col2= -color */
	if ((n11+n12+n13)>=3) v=align_c[n11]; else v=0;
	if ((n21+n22+n23)>=3) v-=align_nc[n21];
	return v;
      }
    }
  }
  /* Here we know that (col1 != color) */
  if (col2==color) {
    if (n21>=3) return WIN;
    if (col1==0) {
	if ((n11+n12+n13+n21+n22+n23)>=3) return align_c[n21];
	else return 0;
      }
    else { /* col1 = -color */
      if ((n21+n22+n23)>=3) v=align_c[n21]; else v=0;
      if ((n11+n12+n13)>=3) v-=align_nc[n11];
      return v;
    }
  }
  /* Here we know that (col1 != color) && (col2 != color) */
  if ((n11+n12+n13+n21+n22+n23)>=3) return -align_nc[n11+n21];
  else return 0;
}

int eval_new(int x,int y,int color,int tab[SIZEX][SIZEY]) {
  int d,i,j,n,col,n11,n12,n13,n21,n22,n23,col1,col2;
  int v,ev=0;
  /* For y search only below 
     col is always +1 or -1, alignmnent never stops on empty square
   */
  n=0;
  if (y!=0) {
    col=tab[x][y-1];
    for (j=y-1;j>=max(0,y-3);j--)
      if (tab[x][j]==col) n++; else break;
    if ((col==color)&&(n>=3)) return WIN;
    if ((n+(SIZEY-y))>=4) {
      if (col==color) ev+=align_c[n];else ev-=align_nc[n];
    }
  }
  
  /* For x search right and left */
  n11=0;n12=0;n13=0;col1=0;
  if (x!=(SIZEX-1)) {
    i=x+1;
    col1=tab[i][y];
    if (col1!=0) {
      for (;i<=min(SIZEX-1,x+3);i++)
	if (tab[i][y]==col1) n11++;
	else break;}
    for (;i<=min(SIZEX-1,x+3);i++)
      if (tab[i][y]==0) n12++;
      else if (tab[i][y]==col1) n13++;
      else break;
  }
  n21=0;n22=0;n23=0;col2=0;
  if (x!=0) {
    i=x-1;
    col2=tab[i][y];
    if (col2!=0) {
      for (;i>=max(0,x-3);i--)
	if (tab[i][y]==col2) n21++;
	else break;}
    for (;i>=max(0,x-3);i--)
      if (tab[i][y]==0) n22++;
      else if (tab[i][y]==col2) n23++;
      else break;
  }
  v=compute(color,col1,col2,n11,n12,n13,n21,n22,n23);
  if (v==WIN) return v;
  ev+=v;

  
  /* First diagonal above and below */
  n11=0;n12=0;n13=0;col1=0;
  for (d=1;d<=3;d++) {
    i=x+d;j=y+d;
    if ((i>=SIZEX)||(j>=SIZEY)) break;
    if (d==1) {col1=tab[i][j];if (col1==0) break;}
    if (tab[i][j]==col1) n11++; else break;
  }
  for (;d<=3;d++) {
    i=x+d;j=y+d;
    if ((i>=SIZEX)||(j>=SIZEY)) break;
    if (tab[i][j]==0) n12++;
    else if (tab[i][j]==col1) n13++; else break;
  }
  n21=0;n22=0;n23=0;col2=0;
  for (d=1;d<=3;d++) {
    i=x-d;j=y-d;
    if ((i<0)||(j<0)) break;
    if (d==1) {col2=tab[i][j];if (col2==0) break;}
    if (tab[i][j]==col2) n21++; else break;
  }
  for (;d<=3;d++) {
    i=x-d;j=y-d;
    if ((i<0)||(j<0)) break;
    if (tab[i][j]==0) n22++;
    else if (tab[i][j]==col2) n23++; else break;
  }
  v=compute(color,col1,col2,n11,n12,n13,n21,n22,n23);
  if (v==WIN) return v;
  ev+=v;

  
  /* Second diagonal below and above*/
  n11=0;n12=0;n13=0;col1=0;
  for (d=1;d<=3;d++) {
    i=x+d;j=y-d;
    if ((i>=SIZEX)||(j<0)) break;
    if (d==1) {col1=tab[i][j];if (col1==0) break;}
    if (tab[i][j]==col1) n11++; else break;
  }
  for (;d<=3;d++) {
    i=x+d;j=y-d;
    if ((i>=SIZEX)||(j<0)) break;
    if (tab[i][j]==0) n12++;
    else if (tab[i][j]==col1) n13++; else break;
  }
  n21=0;n22=0;n23=0;col2=0;
  for (d=1;d<=3;d++) {
    i=x-d;j=y+d;
    if ((i<0)||(j>=SIZEY)) break;
    if (d==1) {col2=tab[i][j];if (col2==0) break;}
    if (tab[i][j]==col2) n21++; else break;
  }
  for (;d<=3;d++) {
    i=x-d;j=y+d;
    if ((i<0)||(j>=SIZEY)) break;
    if (tab[i][j]==0) n22++;
    else if (tab[i][j]==col2) n23++; else break;
  }
  v=compute(color,col1,col2,n11,n12,n13,n21,n22,n23);
  if (v==WIN) return v;
  ev+=v;
  /* All done */
  return ev;
}


void printboard(int tab[SIZEX][SIZEY]) {
  int i,j;
  for (j=SIZEY-1;j>=0;j--) {
    for(i=0;i<SIZEX;i++) 
      switch (tab[i][j]) {
      case EMPTY: printf(" .");break;
      case WHITE: printf(" O");break;
      case BLACK: printf(" X");break;
      }
    printf("\n");
  }
  for (i=0;i<SIZEX;i++) printf(" %d",i);
  printf("\n");
}

int indx[SIZEX];
void init_index()
{
  int i;
  for (i=0;i<SIZEX;i++) {
    indx[i]=(SIZEX-1)/2+((i+1)/2*(2*(i%2)-1));
  }
}

struct
__attribute__((packed))
_hash_type 
{
  uint64_t sig;
  int16_t v_inf,v_sup;
  uint8_t bmove;
  uint8_t base;
  uint8_t dist;
};
typedef struct _hash_type hash_type;
#define HASH_SIZE (1<<NB_BITS)
#define HASH_MASK (HASH_SIZE-1)
uint64_t hashesw[SIZEX][SIZEY];
uint64_t hashesb[SIZEX][SIZEY];
uint64_t init_hashv,turn_hashv;
hash_type *hash_v;

uint64_t rand_v()
{
  return
    (((uint64_t)lrand48())<<62) ^
    (((uint64_t)lrand48())<<31) ^
    ((uint64_t)lrand48());
}

void init_hash()
{
  int i,j;
  for (i=0;i<SIZEX;i++)
    for (j=0;j<SIZEY;j++) {
      hashesw[i][j]=rand_v();
      hashesb[i][j]=rand_v();
    }
  hash_v=(hash_type *)calloc(HASH_SIZE,sizeof(hash_type));
  if (hash_v==NULL) {fprintf(stderr,"Not enough memory!!!\n");exit(-1);}
  init_hashv=rand_v();
  turn_hashv=rand_v();
}

void check_hash(uint64_t hv,int tab[SIZEX][SIZEY])
{
  uint64_t hv2=init_hashv;
  int x,y;
  for (x=0;x<SIZEX;x++)
    for (y=0;y<SIZEY;y++)
      if (tab[x][y]==WHITE) hv2^=hashesw[x][y];
      else if (tab[x][y]==BLACK) hv2^=hashesb[x][y];
  if (hv2!=hv) {
    printf("%ld %lx %lx\n",uuid,hv,hv2);
    printboard(tab);
    exit(-1);
  }
}
int retrieve_v_hash(uint64_t hv,int dist,int *v_inf,int *v_sup,int *bmove)
{
  int ind=hv&HASH_MASK;

  if (hash_v[ind].sig==hv) {
    if ((hash_v[ind].dist==dist)

	||
	((hash_v[ind].v_inf==hash_v[ind].v_sup)&&(abs(hash_v[ind].v_inf)>=BWIN))

) {
      *v_inf=hash_v[ind].v_inf;
      *v_sup=hash_v[ind].v_sup;
      *bmove=hash_v[ind].bmove;
      return true;
    }
    *bmove=hash_v[ind].bmove;
    return false;
  }
  return false;
}

void store_v_hash_both(uint64_t hv,int v,int dist,int base,int move)
{
  int ind = hv&HASH_MASK;

  if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
    hash_v[ind].v_inf=(int16_t)v;
    hash_v[ind].v_sup=(int16_t)v;
    hash_v[ind].sig=hv;
    hash_v[ind].base=(uint8_t)base;
    hash_v[ind].bmove=(uint8_t)move;
    hash_v[ind].dist=(uint8_t)dist;
    };
}

void store_v_hash(uint64_t hv,int alpha,int beta,int g,
		  int dist,int base,int move)
{
  int ind = hv&HASH_MASK;

  if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
    if ((hash_v[ind].sig!=hv) || (hash_v[ind].dist!=dist)) {
      /* Not an update. Have to initialize/reset everything */
      hash_v[ind].v_inf=-MAXV;
      hash_v[ind].v_sup=MAXV;
      hash_v[ind].dist=(uint8_t)dist;
      hash_v[ind].sig=hv;
    }
    hash_v[ind].base=(uint8_t)base;
    hash_v[ind].bmove=(uint8_t)move;
    if ((g>alpha)&&(g<beta))
      {hash_v[ind].v_inf=(int16_t)g;hash_v[ind].v_sup=(int16_t)g;}
    else if (g<=alpha) hash_v[ind].v_sup=(int16_t)g;
    else if (g>=beta) hash_v[ind].v_inf=(int16_t)g;
    };
}

int eval(int color,
	 int tab[SIZEX][SIZEY],int first[SIZEX],
	 int* best) {
  int ev=0;
  for (int ix=0;ix<SIZEX;ix++) {
    int x=indx[ix];
    int y=first[x];
    if (y<SIZEY) {
      int v=eval_new(x,y,color,tab);
      if (color==BLACK) v= -v;
      ev+=v;
      if (abs(v)==WIN) {
	      *best=x;
	      return v;
      } 
    }
  }
  return ev;
}

void play(int x,int y,int color,
	  int tab[SIZEX][SIZEY],int first[SIZEX]) {
  tab[x][y]=color;
  first[x]++;
}
void unplay(int x,int y,
	  int tab[SIZEX][SIZEY],int first[SIZEX]) {
  first[x]--;
  tab[x][y]=EMPTY;
}


int best_move;
int alpha_beta(int alpha,int beta,int color,int depth,
	       int tab[SIZEX][SIZEY],int first[SIZEX],
	       int maxdepth,int base,uint64_t hv)
{
  int a = alpha ;
  int b = beta ;
  int g ;
  // int v_inf ;
  // int v_sup ;
  // int bmove ;
  int dist = maxdepth - depth ;
  //int old_best = -1 ;
  int curr_best = -1 ;


  int v = eval(color , tab , first , &best_move);
  if (abs(v) == WIN) return v;

  if (depth == (SIZEX*SIZEY-1)) return 0;

  if ( dist == 0 ) return v;

  // if (retrieve_v_hash(hv , dist , &v_inf , &v_sup , &bmove))
  // {
  //   //old_best = bmove ;

  //   if (v_inf == v_sup) return v_inf ;
  //   if (v_inf >= beta ) return v_inf ;
  //   if (v_sup <= alpha) return v_sup ;
  //   a = max(a , v_inf) ;
  //   b = min(b , v_sup) ;
  //   if (base == depth) best_move = bmove ;
  // }

  
  
  if (color ==  WHITE)
  {
    g = -INFINITE ;
    init_index() ;
    int i =0 ;

    while((g < b) & (i < SIZEX))
    {
      int x = indx[i];
      int y = first[x];

      if (y != SIZEY )
      {
        play(x , y , color , tab , first);
        int t = alpha_beta(a , beta , BLACK , depth+1 , tab , first , maxdepth , base , hv^hashesw[x][y]) ;
        unplay(x , y , tab , first);

        if (t > g) 
        {
        g = t ;
        curr_best = x;
        }
        if (depth == base) best_move = curr_best;
        a = max(a , g);
      }
      i++ ;
    }
  }

  if (color ==  BLACK)
  {
    g = +INFINITE ;
    init_index() ;
    int i =0 ;

    while( (g > a) & (i < SIZEX))
    {
      int x = indx[i];
      int y = first[x];

      if (y != SIZEY )
      {
        play(x , y , color , tab , first);
        int t = alpha_beta(alpha , b , WHITE , depth+1 , tab , first , maxdepth , base , hv^hashesb[x][y]) ;
        unplay(x , y , tab , first);

        if (t < g) 
        {
        g = t ;
        curr_best = x ;
        }
        if (depth == base) best_move = curr_best;
        b = min(b,g);
      }
      i++ ;
    }
  }

  //if (curr_best != -1) bmove = curr_best ;

  // store_v_hash(hv , alpha , beta , g , dist , base , curr_best) ; 

  return g ; 
}

int main(int argc,char **argv)
{
  int tab[SIZEX][SIZEY]={EMPTY};
  int first[SIZEX]={0};;


  int ret,maxdepth,move,base,old_best_move;
  clock_t time,otime;
  double ftime;
  uint64_t hv;
  int player=-1;

#ifdef TIMER
  if (argc!=3) {
    printf("usage: a.out <color> <time_by_move>\n");
    exit(-1);
  }
#else
  if (argc!=2) {
    printf("usage: a.out <color>\n");
    exit(-1);
  }
#endif
  player = atoi(argv[1]);
  if ((player!=WHITE)&&(player!=BLACK)) {
    printf("Color must be -1 or 1\n");
    exit(-1);
  }
#ifdef TIMER
  double limit_time,timef,timei;
  limit_time=atof(argv[2]);
  struct sigaction sa;
  struct itimerval timer;
  if ((limit_time<1.)||(limit_time>60.0)) {
    printf("Time must be >=1 and <=60 seconds\n");
    exit(-1);
  }

  timei=(int)limit_time;
  timef=limit_time-timei;
  printf("timef=%f timei=%f\n",timef,timei);

  bzero(&timer,sizeof(timer));
  bzero(&sa,sizeof(sa));
  sa.sa_handler=handler;
  ret=sigaction(SIGALRM,&sa,NULL);
  if (ret!=0) {
    perror("sigaction:");
    exit(-1);
  }
#endif
  init_index();
  init_hash();

  hv=init_hashv;
  base=0;
  while (true) {
    if (player==1) {
      maxdepth=base;
      otime=clock();
      old_best_move=-1;
      /* Arm timer */
#ifdef TIMER
      timer.it_value.tv_sec=(time_t)timei;
      timer.it_value.tv_usec=(suseconds_t)(1000000.0*timef);
      // Activate timer at your own risks
      // In debug mode it is sometimes a nuisance
      // Sometimes it is easier to set a limit to depth for exiting
      ret=setitimer(ITIMER_REAL,&timer,NULL);
      if (ret!=0) {
	perror("setitimer:");
	exit(-1);
      }
#endif
      do {
	maxdepth++;
	best_move=-1;
	get_out=false;
	ret=alpha_beta(-MAXV,MAXV,WHITE,base,tab,first,maxdepth,base,hv);
	time=clock()-otime;
	ftime=(double)time/(double)CLOCKS_PER_SEC;
	printf("best_move=%d ret=%d base=%d maxdepth=%d time=%f\n",
	       best_move,ret,base,maxdepth,ftime);
	/* If the previous best_move is searched first in the alphabeta
	   and best_move has been updated we use the current search  */
	if (best_move==-1) best_move=old_best_move; else old_best_move=best_move;
	/* If the previous best_move is not searched first in the alphabeta 
	   we discard completely the current search : */
	//      if (get_out) best_move=old_best_move; else old_best_move=best_move;
	if (best_move==-1) {printf("error best_move\n");exit(EXIT_ERROR);}
	if (first[best_move]>=SIZEY) {printf("error SIZEY\n");exit(EXIT_ERROR);}
      }
#ifndef TIMER
      while (((maxdepth-base)<MAXPROF)&&
	     (maxdepth<(SIZEX*SIZEY-1))&&
	     (ret<BWIN));
#else
      while ((!get_out)&&
	     (maxdepth<(SIZEX*SIZEY-1))&&
	     (ret<BWIN));
      /* Disarm timer */
      timer.it_value.tv_sec=0;
      timer.it_value.tv_usec=0;
      setitimer(ITIMER_REAL,&timer,NULL);
#endif
      ret=eval_new(best_move,first[best_move],WHITE,tab);
      tab[best_move][first[best_move]]=WHITE;
      hv^=hashesw[best_move][first[best_move]];
      first[best_move]++;
      printf("My move:%d\n",best_move);
      printboard(tab);
      base++;
      if (ret==WIN) break;
    }
    player=1;
    while (true) {
      printf("Your move:");
      int ret=scanf("%d",&move);
      if ((ret!=EOF)&&(move>=0)&&(move<SIZEX)&&(first[move]<SIZEY)) {
	ret=eval_new(move,first[move],BLACK,tab);
	tab[move][first[move]]=BLACK;
	hv^=hashesb[move][first[move]];
	first[move]++;
	base++;
	break;
      }
    }
    printboard(tab);
    if (ret==WIN) break;
  }
  exit(0);
}

