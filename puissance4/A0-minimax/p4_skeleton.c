/*
Cours Initiation à l'IA, Sup Aero, JM Alliot
*/

// Compile with: gcc -W -Wall -O3 -std=c99 p4_minimax.c

#include <stdio.h>
#include <time.h>

#define SIZEX 4
#define SIZEY 4

// max and min macros properly defined in C99 style
#define max(a,b)	       \
       ({ __typeof__ (a) _a = (a); \
           __typeof__ (b) _b = (b); \
         _a > _b ? _a : _b; })
#define min(a,b)	       \
       ({ __typeof__ (a) _a = (a); \
           __typeof__ (b) _b = (b); \
         _a < _b ? _a : _b; })


#define EMPTY 0
#define WHITE 1
#define BLACK -1

/* 
   -eval function
   -It is nice to notice that a player wins when he plays a disc which
   completes a line, a column or a diagonal. He can never lose when playing,
   nor can he win with an alignment which does not contain the new disc.
   -Thus we only have to check alignments that contain the disc to play next 
   to know if we win...
   - x and y are the coordinates of the disc to play next
   - color is the color of this disc
*/
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

// Une position est gagnante s'il existe un coup gagnant
// On teste tous les coups possibles et on regarde s'il y en a un qui gagne
int eval(int color,int tab[SIZEX][SIZEY],int first[SIZEX]) {
  for (int x=0;x<SIZEX;x++) {
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

/* 
   -minimax function
   -color is the color of the player to move (WHITE or BLACK)
   -depth is the depth of the search tree, which is the same thing 
   as the number of moves played in this branch. Useful for checking
   that the maximal number of moves has been played.
*/
int minimax(int color,int depth,int tab[SIZEX][SIZEY],int first[SIZEX])
{
  if ( eval(color , tab , first) != 0 ) return color ;

  if (depth == SIZEX * SIZEY) return 0 ;

  if (color == WHITE) 
  {
    int g = -2 ;
    for (int x=0 ; x < SIZEX ; x++) {
      int y = first[x];
      if (y != SIZEY) {
        play(x , y , color , tab , first);
        g = max(g , minimax(BLACK , depth+1 , tab , first));
        unplay(x , y , tab , first);
      }
    }
    return g ;
  }
  else 
  {
    int g = 2 ;
    for (int x=0 ; x < SIZEX ; x++) {
      int y = first[x];
      if (y != SIZEY)
      {
        play(x , y , color , tab , first);
        g = min(g , minimax(WHITE , depth+1 , tab , first));
        unplay(x , y , tab , first);
      }    
    }
    return g;
  } 
}

int main()
{
  int tab[SIZEX][SIZEY]={0};
  int first[SIZEX]={0};

  int ret;
  clock_t time;
  time=clock();
  ret=minimax(WHITE,0,tab,first);
  time=clock()-time;
  printf("ret=%d %f\n",ret,((double)time/(double)CLOCKS_PER_SEC));
  return 0;
}
