#define _XOPEN_SOURCE

/*
Version avec l'alpha-beta corrigée
Il faudra le faire sur tous les programmes.
*/
/*
Cours Initiation à l'IA, Sup Aero, JM Alliot
*/
/*
Fastest solver with symmetries

Intel(R) Core(TM) i9-9900K

ALPHA_BETA_SORT simple
      clang -O3 (gcc -O3 is usually slower by 5%)
5x6       21s
6x5       39s
5x7:     795s
7x5:    2176s

STRAT_DONT_USE (slightly slower because it has to handle hash tables structures)
      clang -O3
5x6:      23s
6x5:      45s
5x7:     910s
7x5:    2563s

STRAT_MAX
     NB_BITS clang -O3 
6x5:  20        870ms
6x5:  21        820ms
6x5:  22        810ms
6x5:  23        830ms
6x5:  31       1260ms
5x7:  19         27s
5x7:  20         17s
5x7:  21         12s
5x7:  22         9s
5x7:  23         7s
5x7:  24         6s
5x7:  25         6s
5x7:  26         6s
5x7:  31         8s
7x5:  19         68s
7x5:  20         48s
7x5:  21         33s
7x5:  22         22s
7x5:  23         15s
7x5:  24         11s
7x5:  25         10s
7x5:  26         10s
7x5:  27         10s
7x5:  31         13s
6x6:  19        419s 
6x6:  20        286s 
6x6:  21        188s 
6x6:  22        122s 
6x6:  23         77s 
6x6:  24         48s 
6x6:  25         36s 
6x6:  26         29s 
6x6:  27         27s
6x6:  28         27s
6x6:  29         29s
6x6:  30         36s
6x6:  31         37s
6x7:  28        323s 
6x7:  29        249s 
6x7:  30        270s 
6x7:  31        294s 
7x6:  29        775s 
7x6:  30        608s 
7x6:  31        594s 
7x7:  31        353min 

STRAT_MIN
     NB_BITS clang -O3 
7x5:  24        382s 
7x5:  25         20s 
7x5:  26         10s
6x6:  26        212s 
6x6:  27         35s 
6x6:  28         30s 
6x6:  29         29s
*/
/* Compile with:
gcc -W -Wall -Wconversion -std=c99 -O3 p4_hash_sort_sym.c
-DHASH128 for 128 bits hash sigs (safer but 10% slower).
-DSTORE_LEAF : store evaluation at leaf nodes in hash table (5% slower)
-DSIZEX=x, -DSIZEY=y : to change the size of the board
-DNB_BITS=nn : to change the number of bits of the index of the hash
-DSTRAT=s : to change hash store policy (1: MAX, 2: MIN, 3: DONT_USE)
The number of bits to use for the size of hash tables is probably optimal for my own
machine but might very well be not optimal for yours!
The program uses __attribute__((packed)) to reduce the global size
of hash tables. This might result in slower programs or even
in core dumps. Be Warned and Check!
 */
#define STRAT_MAX 1 /* Default strategy */
#define STRAT_MIN 2 /* Disaster ... */
#define STRAT_DONT_USE 3
#ifndef STRAT
#define STRAT STRAT_MAX
#endif

#ifndef NB_BITS
#define NB_BITS 29
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

typedef __uint128_t uint128_t;

#ifdef HASH128
typedef uint128_t signature;
#define RANDV(x) rand128_v(x)
#else
typedef uint64_t signature;
#define RANDV(x) rand64_v(x)
#endif



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

uint128_t rand128_v()
{
  return
    (((uint128_t)lrand48())<<124) ^
    (((uint128_t)lrand48())<<93) ^
    (((uint128_t)lrand48())<<62) ^
    (((uint128_t)lrand48())<<31) ^
    ((uint128_t)lrand48());
}

signature init_hash()
{
  int i,j;
  for (i=0;i<SIZEX;i++)
    for (j=0;j<SIZEY;j++) {
      hashesw[i][j]=RANDV();
      hashesb[i][j]=RANDV();
    }
  hash_v=(hash_type *)calloc(HASH_SIZE,sizeof(hash_type));
  if (hash_v==NULL) {printf("Error in calloc\n");exit(-1);}
  return (RANDV());
}

bool retrieve_v_hash(int *v_inf,int *v_sup,signature hv1,signature hv2)
{
  signature hv;
  uint64_t ind;
  hv=min(hv1,hv2);
  ind=hv&HASH_MASK;
  if (hash_v[ind].sig==hv) {
    *v_inf=hash_v[ind].v_inf;
    *v_sup=hash_v[ind].v_sup;
    return true;
  }
  else return false;
}

void store_v_hash_both(int v,int depth,signature hv1,signature hv2)
{
  signature hv;
  uint64_t ind;
#if (STRAT==STRAT_MAX)
  int d = (SIZEX*SIZEY+1)-depth;
#elif (STRAT==STRAT_MIN)
  int d = depth;
#elif (STRAT==STRAT_DONT_USE)
  int d=-1;
#endif
  hv=min(hv1,hv2);
  ind=hv&HASH_MASK;
  if (hash_v[ind].d<=d) {
    hash_v[ind].d=(uint8_t)d;
    hash_v[ind].v_inf=(int8_t)v;
    hash_v[ind].v_sup=(int8_t)v;
    hash_v[ind].sig=hv;
    };
}

void store_v_hash(int alpha,int beta,int g,int depth,
		  signature hv1,signature hv2)
{
  signature hv;
  uint64_t ind;
#if (STRAT==STRAT_MAX)
  int d = (SIZEX*SIZEY+1)-depth;
#elif (STRAT==STRAT_MIN)
  int d = depth;
#elif (STRAT==STRAT_DONT_USE)
  int d=-1;
#endif
  hv=min(hv1,hv2);
  ind = hv&HASH_MASK;
  if (hash_v[ind].d<=d) {
    if (hash_v[ind].sig!=hv) {
      /* Not an update. Have to initialize/reset everything */
      hash_v[ind].d=(uint8_t)d;
      hash_v[ind].v_inf=-127;
      hash_v[ind].v_sup=127;
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

int alpha_beta(int alpha,int beta,int color,int depth,
	       int tab[SIZEX][SIZEY],int first[SIZEX],
	       signature hv1,signature hv2)
{

  // evalpos
  int v = eval(color,tab,first);
  if (v!=0) return v;
  /* If no move is a win and there is only one move left, it's a draw */
  if (depth==(SIZEX*SIZEY-1)) return 0;

  /* If position has already been evaluated, return evaluation 
     or update bounds */
  int a=alpha,b=beta,v_inf,v_sup;
  if (retrieve_v_hash(&v_inf,&v_sup,hv1,hv2)) {
    if (v_inf==v_sup) return v_inf; /* Exact evaluation */
    if (v_inf>=b) return v_inf; /* Beta cut */
    if (v_sup<=a)  return v_sup; /* Alpha cut */
    a=max(a,v_inf);
    b=min(b,v_sup);
  }


  /* Classical alpha-beta */
  int g;
  if (color==WHITE) g=-127; else g=127;
  for (int ix=0;ix<SIZEX;ix++) {
    int x=indx[ix];int y=first[x];
    if (y!=SIZEY) {
      tab[x][y]=color;
      first[x]++;
      if (color==WHITE) {
	v=alpha_beta(a,b,-color,depth+1,tab,first,
		     hv1^hashesw[x][y],hv2^hashesw[SIZEX-1-x][y]);
      }
      else {
	v=alpha_beta(a,b,-color,depth+1,tab,first,
		     hv1^hashesb[x][y],hv2^hashesb[SIZEX-1-x][y]);
      }
      first[x]--;
      tab[x][y]=EMPTY;
      if (color==WHITE) {
	g=max(g,v);
	a=max(a,g);
	if (g>=b) break; /* Beta cut */
      }
      else {
	g=min(g,v);
	b=min(b,g);
	if (g<=a) break; /* Alpha cut */
      }
    }
  }
  store_v_hash(alpha,beta,g,depth,hv1,hv2);
  return g;
  }


int main()
{
  int tab[SIZEX][SIZEY]={0};
  int first[SIZEX]={0};

  int ret;
  clock_t time;

  // Useless, they are global variables
  //  bzero(tab,sizeof(tab));
  //  bzero(first,sizeof(first));
  init_index();
  signature hv1=init_hash();
  signature hv2=hv1;
  time=clock();
  ret=alpha_beta(-1,1,WHITE,0,tab,first,hv1,hv2);
  time=clock()-time;
  printf("STRAT=%d NB_BITS=%d SIZEX=%d SIZEY=%d ret=%d %f\n",
	 STRAT,NB_BITS,SIZEX,SIZEY,ret,((double)time/(double)CLOCKS_PER_SEC));
  exit(ret+1);
}

