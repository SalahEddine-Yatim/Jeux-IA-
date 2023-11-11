#define _XOPEN_SOURCE

// 29 bits take 6GB of memory, it might be too much for you!
// 28 bits : 3GB
// 27 bits : 1.5GB
#ifndef NB_BITS
#define NB_BITS 28
#endif

#ifndef SIZEX
#define SIZEX 6
#endif
#ifndef SIZEY
#define SIZEY 6
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>

typedef uint64_t signature;
#define RANDV(x) rand64_v(x)

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

int eval_xy(int x,int y,int color,int tab[SIZEX][SIZEY]) {
  int i,j,nb,d;
  /* For y search only below */
  nb=1;
  for (j=y-1;j>=max(0,y-3);j--)
    if (tab[x][j]==color) nb++; else break;
  if (nb>=4) return color;
  /* For x search right and left */
  nb=1;
  for (i=x+1;i<=min(SIZEX-1,x+3);i++)
    if (tab[i][y]==color) nb++; else break;
  for (i=x-1;i>=max(0,x-3);i--)
    if (tab[i][y]==color) nb++; else break;
  if (nb>=4) return color;
  /* First diagonal above and below */
  nb=1;
  for (d=1;d<=3;d++) {
    i=x+d;j=y+d;
    if ((i>=SIZEX)||(j>=SIZEY)) break;
    if (tab[i][j]==color) nb++; else break;
  }
  for (d=1;d<=3;d++) {
    i=x-d;j=y-d;
    if ((i<0)||(j<0)) break;
    if (tab[i][j]==color) nb++; else break;
  }
  if (nb>=4) return color;
  /* Second diagonal below and above*/
  nb=1;
  for (d=1;d<=3;d++) {
    i=x+d;j=y-d;
    if ((i>=SIZEX)||(j<0)) break;
    if (tab[i][j]==color) nb++; else break;
  }
  for (d=1;d<=3;d++) {
    i=x-d;j=y+d;
    if ((i<0)||(j>=SIZEY)) break;
    if (tab[i][j]==color) nb++; else break;
  }
  if (nb>=4) return color;
  return 0;
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
  signature sig;
  int8_t v_inf,v_sup;
  uint8_t d;
};
typedef struct _hash_type hash_type;
#define HASH_SIZE ((uint64_t)1<<(uint64_t)NB_BITS)
#define HASH_MASK ((uint64_t)HASH_SIZE-(uint64_t)1)
signature hashesw[SIZEX][SIZEY];
signature hashesb[SIZEX][SIZEY];
hash_type *hash_v;

uint64_t rand64_v()
{
  return
    (((uint64_t)lrand48())<<62) ^
    (((uint64_t)lrand48())<<31) ^
    ((uint64_t)lrand48());
}

signature init_hash()
{
  for (int i=0;i<SIZEX;i++)
    for (int j=0;j<SIZEY;j++) {
      hashesw[i][j]=RANDV();
      hashesb[i][j]=RANDV();
    }
  hash_v=(hash_type *)calloc(HASH_SIZE,sizeof(hash_type));
  if (hash_v==NULL) {printf("Error in calloc\n");exit(-1);}
  return (RANDV());
}

bool retrieve_v_hash(int *v_inf,int *v_sup,signature hv)
{
  int ind=hv&HASH_MASK;
  if (hash_v[ind].sig==hv) {
    *v_inf=hash_v[ind].v_inf;
    *v_sup=hash_v[ind].v_sup;
    return true;
  }
  else return false;
}

void store_v_hash_both(int v,int depth,signature hv)
{
  int d = (SIZEX*SIZEY+1)-depth;
  int ind=hv&HASH_MASK;
  if (hash_v[ind].d<=d) {
    hash_v[ind].d=(uint8_t)d;
    hash_v[ind].v_inf=(int8_t)v;
    hash_v[ind].v_sup=(int8_t)v;
    hash_v[ind].sig=hv;
    };
}

void store_v_hash(int alpha,int beta,int g,int depth,signature hv)
{
  int d = (SIZEX*SIZEY+1)-depth;
  int ind = hv&HASH_MASK;
  if (hash_v[ind].d<=d) {
    if (hash_v[ind].sig!=hv) {
      /* Not an update. Have to initialize/reset everything */
      hash_v[ind].d=(uint8_t)d;
      hash_v[ind].v_inf=INT8_MIN;
      hash_v[ind].v_sup=INT8_MAX;
      hash_v[ind].sig=hv;
    }
    if ((g>alpha)&&(g<beta))
      {hash_v[ind].v_inf=(int8_t)g;hash_v[ind].v_sup=(int8_t)g;}
    else if (g<=alpha) hash_v[ind].v_sup=(int8_t)g;
    else if (g>=beta) hash_v[ind].v_inf=(int8_t)g;
    };
}

int eval(int color,int tab[SIZEX][SIZEY],int first[SIZEX]) {
  for (int ix=0;ix<SIZEX;ix++) {
    int x=indx[ix];
    int y=first[x];
    if (y!=SIZEY) {
      int v=eval_xy(x,y,color,tab);
      if (v!=0) return v;
    }
  }
  return 0;
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


int alpha_beta(int alpha,int beta,int color,int depth,
	       int tab[SIZEX][SIZEY],int first[SIZEX],
	       signature hv , signature hvsym)
{
  // evalpos
  int v = eval(color,tab,first);
  if (v!=0) return v;
  /* If no move is a win and there is only one move left, it's a draw */
  if (depth==(SIZEX*SIZEY-1)) return 0;

  int a = alpha ;
  int b = beta ;
  int g ;
  int v_inf ;
  int v_sup ;

  if ( retrieve_v_hash(&v_inf , &v_sup , min(hv,hvsym)))
  {
    if (v_inf == v_sup) return v_inf ;
    if (v_inf >= beta ) return v_inf ;
    if (v_sup <= alpha) return v_sup ;
    a = max(a , v_inf);
    b = min(b , v_sup);
  }

  if (color ==  WHITE)
  {
    g = -20000 ;
    init_index() ;
    int i =0 ;
    while( (g < beta) & (i < SIZEX))
    {
      int x = indx[i];
      int y = first[x];
      if (y != SIZEY )
      {
        play(x , y , color , tab , first);
        g = max(g , alpha_beta(a , beta , BLACK , depth+1 , tab , first , hv^hashesw[x][y] , hvsym^hashesw[SIZEX-1-x][y])) ;
        unplay(x , y , tab , first);
        a = max(g,a);
      }
      i++ ;
    }
  }

  if (color == BLACK)
  {
    g = 20000 ;
    init_index() ;
    int i =0 ;
    while( (g > alpha) & (i < SIZEX))
    {
      int x = indx[i];
      int y = first[x];
      if (y != SIZEY )
      {
        play(x , y , color , tab , first);
        g = min(g , alpha_beta(alpha , b , WHITE , depth+1 , tab , first , hv^hashesb[x][y] , hvsym^hashesb[SIZEX-1-x][y] )) ;
        unplay(x , y , tab , first);
        b = min(b,g) ;
      }
      i++ ;
    }
  }

  
  store_v_hash(alpha , beta , g , depth , min(hv,hvsym)) ;
  return g ;

}


int main()
{
  int tab[SIZEX][SIZEY];
  int first[SIZEX];
  bzero(tab,sizeof(tab));
  bzero(first,sizeof(first));
  init_index();
  signature hv=init_hash();
  signature hvsym = hv ;
  clock_t time=clock();
  int ret=alpha_beta(-1,1,WHITE,0,tab,first,hv , hvsym);  
  time=clock()-time;
  printf("NB_BITS=%d SIZEX=%d SIZEY=%d ret=%d %f\n",
	 NB_BITS,SIZEX,SIZEY,ret,((double)time/(double)CLOCKS_PER_SEC));
  return(0);
}

