/*
Cours Initiation à l'IA, Sup Aero, JM Alliot
*/
/* Compile with:
gcc -W -Wall -O3 p4_iter.c
Remove -DBMOVE if you don't want best movesuggestion by transp tables
Remove -DHASH if you don't want transposition tables.
The program uses __attribute__((packed)) to reduce the global size
of hash tables. This might result in slower programs or even
in core dumps. Be Warned and Check!
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define SIZEX 7
#define SIZEY 6

#define FALSE 0
#define TRUE 1

#define EXIT_ERROR 126
#define MAXV INT16_MAX
#define WIN (MAXV-1)
#define BWIN (WIN-SIZEX*SIZEY)

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

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
	return v;} 
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
	       int maxdepth,int base)
{
  int a = alpha ;
  int b = beta ;
  int g ;
  best_move = -1 ;


  
}

int main()
{
  int ret,maxdepth,move,base;
  clock_t time,otime;
  double ftime;
  int tab[SIZEX][SIZEY]={{EMPTY}};
  int first[SIZEX]={0};
  init_index();

  base=0;
  while (TRUE) {
    maxdepth=base;
    otime=clock();
    do {
      maxdepth++;
      best_move=-1;
      ret=alpha_beta(-MAXV,MAXV,WHITE,base,tab,first,maxdepth,base);
      time=clock()-otime;
      ftime=(double)time/(double)CLOCKS_PER_SEC;
      printf("best_move=%d ret=%d base=%d maxdepth=%d time=%f\n",
	     best_move,ret,base,maxdepth,ftime);
      if (best_move== -1) {printf("error -1\n");exit(EXIT_ERROR);}
      if (first[best_move]>=SIZEY) {printf("error SIZEY\n");exit(EXIT_ERROR);}
    } while ((ftime<=1.0)&&(maxdepth<(SIZEX*SIZEY-1))&&(ret<BWIN));
    ret=eval_new(best_move,first[best_move],WHITE,tab);
    tab[best_move][first[best_move]]=WHITE;
    first[best_move]++;
    printf("My move:%d\n",best_move);
    printboard(tab);
    base++;
    if (ret==WIN) break;
    while (TRUE) {
      printf("Your move:");
      scanf("%d",&move);
      if ((move>=0)&&(move<SIZEX)&&(first[move]<SIZEY)) {
	ret=eval_new(move,first[move],BLACK,tab);
	tab[move][first[move]]=BLACK;
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

