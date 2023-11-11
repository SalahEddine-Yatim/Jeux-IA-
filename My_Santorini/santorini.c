#define TIMER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#ifndef NB_BITS
#define NB_BITS 27
#endif

//Dimension du tableau 
#ifndef SIZEX
#define SIZEX 5
#endif
#ifndef SIZEY
#define SIZEY 5
#endif

#define INFINITE 99999999
#define MAXV INT16_MAX

#define WIN 200000000
#define BWIN (WIN-SIZEX*SIZEY)
FILE *flog ;

//Definition des variables globales sur 8bits 
#define BROWN 0x1
#define BLUE 0x2
#define EMPTY 0x4
#define FIRST 0x8
#define SECONDE 0x10
#define THIRD 0x20
#define DOME 0x40


#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


//Structure pour les positions des pions de chaque couleurs
typedef struct {
    int x1 ;
    int y1 ;
    int x2 ;
    int y2 ;
} color_pos ;

//Structure de  mouvement joué
typedef struct {

    int x ;
    int y ;
    int dx ;
    int dy ;
    int dbuildx ;
    int dbuildy ;

} best ;

//Fonction qui cherche les positions des pions d'un couleurs 
color_pos search(int color , int tab[SIZEX][SIZEY])
{ 
    color_pos pos = { 0, 0, 0, 0 } ;
    int check = 0 ;
    for (int x=0 ; x < SIZEX ; x++){
        for (int y=0 ; y < SIZEY ; y++)
        {
            if ((tab[x][y] & color) == color)
            {
                if(check != 0) 
                {
                    pos.x2 = x ;
                    pos.y2 = y ;
                    break;
                }
                else {
                    pos.x1 = x ;
                    pos.y1 = y ;
                    check = 1; 
                }
            } 
        }
    }

    return pos ;
}

void print_board(int tab[SIZEX][SIZEY])
{
    for (int x=0 ; x < SIZEX ; x++){
        for (int y=0 ; y < SIZEY ; y++){
            fprintf(flog," %d " , tab[x][y]);
        }
        fprintf(flog,"\n");
    }
}

//Partie table de transpositions 
long int uuid = 0;

struct
__attribute__((packed))
_hash_type 
{
  uint64_t sig;
  int16_t v_inf,v_sup;
  best    bmove;
  uint8_t base;
  uint8_t dist;
};

typedef struct _hash_type hash_type;

#define HASH_SIZE (1<<NB_BITS)
#define HASH_MASK (HASH_SIZE-1)

uint64_t hashesbrown[SIZEX][SIZEY];
uint64_t hashesblue[SIZEX][SIZEY];
uint64_t hashesfirst[SIZEX][SIZEY];
uint64_t hashessecond[SIZEX][SIZEY];
uint64_t hashesthird[SIZEX][SIZEY];
uint64_t hashesdome[SIZEX][SIZEY];

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
    for (i=0;i<SIZEX;i++){
        for (j=0;j<SIZEY;j++) {
            hashesbrown[i][j]=rand_v();
            hashesblue[i][j]=rand_v();
            hashesfirst[i][j]=rand_v();
            hashessecond[i][j]=rand_v();
            hashesthird[i][j]=rand_v();
            hashesdome[i][j]=rand_v();
        }
    }

    hash_v=(hash_type *)calloc(HASH_SIZE,sizeof(hash_type));

    if (hash_v==NULL) {fprintf(stderr,"Not enough memory!!!\n");exit(-1);}
    init_hashv = rand_v();
    turn_hashv = rand_v();

}

void check_hash(uint64_t hv,int tab[SIZEX][SIZEY])
{
    uint64_t hv2=init_hashv;
    int x,y;
    for (x=0;x<SIZEX;x++){
        for (y=0;y<SIZEY;y++){
            if ((tab[x][y] & BROWN) == BROWN) hv2^=hashesbrown[x][y];
            if ((tab[x][y] & BLUE) ==BLUE) hv2^=hashesblue[x][y];
            if ((tab[x][y] & FIRST) ==FIRST) hv2^=hashesfirst[x][y];
            if ((tab[x][y] & SECONDE) ==SECONDE) hv2^=hashessecond[x][y];
            if ((tab[x][y] & THIRD) ==THIRD) hv2^=hashesthird[x][y];
            if ((tab[x][y] & DOME) ==DOME) hv2^=hashesdome[x][y] ;
        }    
    }
    if (hv2!=hv) {
        printf("%ld %lx %lx\n",uuid,hv,hv2);
        print_board(tab);
        exit(-1);
  }
}
int retrieve_v_hash(uint64_t hv,int dist,int *v_inf,int *v_sup,best *bmove)
{
    int ind=hv&HASH_MASK;

    if (hash_v[ind].sig==hv) {
        if ((hash_v[ind].dist==dist)

        ||
        ((hash_v[ind].v_inf==hash_v[ind].v_sup)&&(abs(hash_v[ind].v_inf)>=BWIN))

)       {
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

void store_v_hash_both(uint64_t hv,int v,int dist,int base,best move)
{
    int ind = hv&HASH_MASK;

    if ((hash_v[ind].base!=base)||(hash_v[ind].dist<=dist)) {
        hash_v[ind].v_inf=(int16_t)v;
        hash_v[ind].v_sup=(int16_t)v;
        hash_v[ind].sig=hv;
        hash_v[ind].base=(uint8_t)base;
        hash_v[ind].bmove=(best)move;
        hash_v[ind].dist=(uint8_t)dist;
    };
}

void store_v_hash(uint64_t hv,int alpha,int beta,int g,
		  int dist,int base,best move)
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
    hash_v[ind].bmove = (best)move;
    if ((g>alpha)&&(g<beta))
        {hash_v[ind].v_inf=(int16_t)g;hash_v[ind].v_sup=(int16_t)g;}
    else if (g<=alpha) hash_v[ind].v_sup=(int16_t)g;
    else if (g>=beta) hash_v[ind].v_inf=(int16_t)g;
    };
}

//Fonction qui mis a jour les tables de hashes de chaque etage apres le build 
uint64_t hashes_build(int xbuild , int ybuild , int tab[SIZEX][SIZEY]){

    uint64_t hashesbuild ;

    if      (tab[xbuild][ybuild] == FIRST)   hashesbuild = hashesfirst[xbuild][ybuild];
    else if (tab[xbuild][ybuild] == SECONDE) hashesbuild = hashessecond[xbuild][ybuild]^hashesfirst[xbuild][ybuild] ;
    else if (tab[xbuild][ybuild] == THIRD)   hashesbuild = hashesthird[xbuild][ybuild]^hashessecond[xbuild][ybuild];
    else if (tab[xbuild][ybuild] == DOME)    hashesbuild = hashesdome[xbuild][ybuild]^hashesthird[xbuild][ybuild];

    return hashesbuild ;
}

//Fonction d'evaluation qui prend en compte la distance entre les pions et les etages à coté 
int eval_xy_xy(color_pos pos_color ,int tab[SIZEX][SIZEY])
{
    int ev = 0 ; 
    
    int tab_pos_x[2] = {pos_color.x1 , pos_color.x2} ;
    int tab_pos_y[2] = {pos_color.y1 , pos_color.y2} ;

    int player = 0 ;

    ev -= (abs(pos_color.x1-pos_color.x2)*100 + abs(pos_color.y1-pos_color.y2)*100 );

    while (player < 2) 
    {
        int x = tab_pos_x [player];
        int y = tab_pos_y [player];
            for (int dx = -1 ; dx<2 ; dx++){
                for (int dy = -1 ; dy<2 ; dy++){
                    if ((x+dx < SIZEX) && (x+dx > -1) && (y+dy < SIZEY) && (y+dy > -1) ){

                        if (( tab[x][y] & EMPTY ) == EMPTY ){
                            if ((tab[x+dx][y+dy] / 2 ) == (tab[x][y] & 0b11111100 )) ev += 1000 ;
                        } 
                        else if (( tab[x][y] & FIRST ) == FIRST ){
                            if ((tab[x+dx][y+dy] / 2 ) == (tab[x][y] & 0b11111100 )) ev += 3000 ;
                        }
                        else if (( tab[x][y] & SECONDE ) == SECONDE ){
                            if ((tab[x+dx][y+dy] / 2 ) == (tab[x][y] & 0b11111100 )) ev += 7000 ;
                        }   
                    }
                }
            }
        player ++ ;
    }
    return ev ;        
}

//Fonction d'eval qui evalue la hauteur d'un pion 
int eval_xy (int x , int y ,int tab[SIZEX][SIZEY])
{
    int ev ;

    if       ((tab[x][y] & THIRD)   == THIRD) ev = WIN ;
    else if  ((tab[x][y] & SECONDE) == SECONDE) ev = 8000 ;
    else if  ((tab[x][y] & FIRST)  == FIRST) ev = 7000 ;
    else if  ((tab[x][y] & EMPTY)   == EMPTY) ev = 6000 ;

    return ev ;
}

//Fonction d'evaluation globale du tableau qui fait la somme des evaluations eet qui retourne WIN si la partie est gagnante 
int eval (int tab[SIZEX][SIZEY])
{
    int ev ; 
    int brownev ; 
    int blueev ;
    color_pos pos_brown = search(BROWN,tab) ;
    color_pos pos_blue = search(BLUE , tab) ;

    brownev = eval_xy( pos_brown.x1 , pos_brown.y1 ,tab) ;
    brownev += eval_xy( pos_brown.x2, pos_brown.y2 ,tab) ;

    brownev += eval_xy_xy(pos_brown , tab);

    blueev = -eval_xy( pos_blue.x1 , pos_blue.y1 ,tab) ;
    blueev -= eval_xy( pos_blue.x2, pos_blue.y2 ,tab) ;

    blueev -= eval_xy_xy(pos_blue , tab);

    if (brownev > 10000000)
    {
        ev = WIN ;
    }

    else if (blueev < -10000000)
    {
        ev = -WIN ;
    }

    else ev = brownev + blueev ;
    
    return ev ;
}


//Fonction qui construit l'etage convenable dans une position 
void build(int x , int y , int dx , int dy ,
        int build_x , int build_y , int tab[SIZEX][SIZEY])
{
    if      (tab[x + dx + build_x][y + dy + build_y] == EMPTY)   tab[x + dx + build_x][y + dy + build_y] = FIRST ;
    else if (tab[x + dx + build_x][y + dy + build_y] == FIRST)   tab[x + dx + build_x][y + dy + build_y] = SECONDE ;
    else if (tab[x + dx + build_x][y + dy + build_y] == SECONDE) tab[x + dx + build_x][y + dy + build_y] = THIRD ;
    else if (tab[x + dx + build_x][y + dy + build_y] == THIRD)   tab[x + dx + build_x][y + dy + build_y] = DOME ;

}

void unbuild(int x , int y , int dx , int dy ,
        int build_x , int build_y , int tab[SIZEX][SIZEY])
{
    if      (tab[x + dx+ build_x][y + dy +build_y] == FIRST)   tab[x + dx+ build_x][y + dy +build_y] = EMPTY ;
    else if (tab[x + dx+ build_x][y + dy +build_y] == SECONDE) tab[x + dx+ build_x][y + dy +build_y] = FIRST ;
    else if (tab[x + dx+ build_x][y + dy +build_y] == THIRD)   tab[x + dx+ build_x][y + dy +build_y] = SECONDE ;
    else if (tab[x + dx+ build_x][y + dy +build_y] == DOME)    tab[x + dx+ build_x][y + dy +build_y] = THIRD ;

}

//Fonction pour deplacer les pion 
void play(int color , int x , int y , int dx , int dy , int tab[SIZEX][SIZEY])
{ 
    //Enlever le pion de l'ancien case et l'ajouté a la nouvelle 
    tab[x][y] = tab[x][y] & 0b11111100 ;
    tab[x+dx][y+dy] += color;
      
}

void unplay(int color , int x , int y , int dx , int dy , int tab[SIZEX][SIZEY])
{   
    tab[x][y] += color ;
    tab[x+dx][y+dy] -= color ;
}

//Alphabeta 
best best_move;
int alpha_beta(int alpha,int beta,int color,int depth,
	       int tab[SIZEX][SIZEY],int maxdepth,int base , uint64_t hv)
{   
    best curr_best ;
    curr_best.x = -1 ;
    curr_best.y = -1 ;
    curr_best.dx = -1 ;
    curr_best.dy = -1 ;
    curr_best.dbuildx = -1 ;
    curr_best.dbuildy = -1 ;

    int dist = maxdepth - depth ;
    int a=alpha ;
    int b=beta;
    int g ; 
    int v_inf ;
    int v_sup ;
    best bmove ;

    int ev = eval(tab);
    if (abs(ev) == WIN) return ev;

    if (dist == 0) return ev ;

    if (retrieve_v_hash(hv , dist , &v_inf , &v_sup , &bmove))
    {

        if (v_inf == v_sup) return v_inf ;
        if (v_inf >= beta ) return v_inf ;
        if (v_sup <= alpha) return v_sup ;
        a = max(a , v_inf) ;
        b = min(b , v_sup) ;
        if (base == depth) best_move = bmove ;
    }


    if (color==BROWN) 
    {
        color_pos pos_brown = search(BROWN,tab);
        int tab_pos_x[2] = {pos_brown.x1 , pos_brown.x2} ;
        int tab_pos_y[2] = {pos_brown.y1 , pos_brown.y2} ;

        g = -INFINITE ;
        int player = 0 ;
        
        //NB Avant chaque mouvement il faut verifier si c'est possible 
        while ( (g < b) && (player < 2) )
        {
            int x = tab_pos_x [player];
            int y = tab_pos_y [player];
            
            for (int dx = -1 ; dx<2 ; dx++){
                if ((x+dx > SIZEX-1) || (x+dx < 0)) continue; 
                for (int dy = -1 ; dy<2 ; dy++){
                    if ((y+dy > SIZEY-1) || (y+dy < 0)) continue;
                    if ((dx == 0) && (dy == 0)) continue;

                    if ((tab[x+dx][y+dy] != DOME)  && ((tab[x+dx][y+dy] & 0x03) == 0) 
                        && (((tab[x][y] & 0b11111100 ) == (tab[x+dx][y+dy] / 2)) || ((tab[x][y] & 0b11111100) >= tab[x+dx][y+dy] )))
                    {
                        play(color , x , y , dx , dy , tab);
                    }
                    else continue;
                        
                    for (int dbuildx = -1 ; dbuildx<2 ; dbuildx++){
                        if ((x+dx+dbuildx > SIZEX-1) || (x+dx+dbuildx < 0)) continue;
                            
                        for (int dbuildy = -1 ; dbuildy<2 ; dbuildy++){
                            if ((y+dy+dbuildy > SIZEY-1) || (y+dy+dbuildy < 0)) continue;
                            if ((dbuildx == 0) && (dbuildy == 0)) continue;
                            
                            uint64_t hashesbuild ;
                            
                            if ((tab[x + dx+ dbuildx][y + dy +dbuildy] != DOME) && ((tab[x+dx+dbuildx][y+dy+dbuildy] & 0x03) == 0 )) {

                                build(x , y ,dx ,dy , dbuildx , dbuildy , tab) ;

                                int xbuild = x + dx+ dbuildx ;
                                int ybuild = y + dy +dbuildy ;
                                hashesbuild = hashes_build(xbuild , ybuild , tab);
                            }

                            else continue;
                            
                            int t = alpha_beta(a , beta , BLUE , depth+1 , tab , maxdepth , base , hv^hashesbrown[x+dx][y+dy]^hashesbrown[x][y]^hashesbuild^turn_hashv);
                                    
                            if (t > g) 
                            {
                            g = t ;

                            curr_best.x = x ;
                            curr_best.y = y ;
                            curr_best.dx = dx ;
                            curr_best.dy = dy;
                            curr_best.dbuildx = dbuildx ;
                            curr_best.dbuildy = dbuildy ;

                            }
                            if (depth == base) best_move = curr_best;
                            a = max(a , g);
                                    
                            unbuild(x , y ,dx ,dy , dbuildx , dbuildy , tab);
                        }
                    }
                unplay(color , x , y , dx , dy , tab);
                }
            }
        player++ ;
        }
    }  

    if (color==BLUE) 
    {
        color_pos pos_blue = search(BLUE,tab);
        int tab_pos_x[2] = {pos_blue.x1 , pos_blue.x2} ;
        int tab_pos_y[2] = {pos_blue.y1 , pos_blue.y2} ;

        g = +INFINITE ;
        int player = 0 ;

        //NB Avant chaque mouvement il faut verifier si c'est possible 
        while ((g > a) & (player <2))
        {
                int x = tab_pos_x [player];
                int y = tab_pos_y [player];
                for (int dx = -1 ; dx<2 ; dx++){
                    if ((x+dx > SIZEX-1) || (x+dx < 0))      continue; 

                    for (int dy = -1 ; dy<2 ; dy++){
                        if ((y+dy > SIZEY-1) || (y+dy < 0))  continue;
                        if ((dx == 0) && (dy == 0))          continue;

                        if ((tab[x+dx][y+dy] != DOME)  && ((tab[x+dx][y+dy] & 0x03) == 0) 
                            && (((tab[x][y] & 0b11111100) == (tab[x+dx][y+dy] / 2)) || ((tab[x][y] & 0b11111100) >= tab[x+dx][y+dy] )))
                        {
                            play(color , x , y , dx , dy , tab);
                        }
                        else continue;
                        
                        for (int dbuildx = -1 ; dbuildx<2 ; dbuildx++){
                            if ((x+dx+dbuildx > SIZEX-1) || (x+dx+dbuildx < 0))      continue;
                            
                            for (int dbuildy = -1 ; dbuildy<2 ; dbuildy++){
                                if ((y+dy+dbuildy > SIZEY-1) || (y+dy+dbuildy < 0))  continue;
                                if ((dbuildx == 0) && (dbuildy == 0))                continue;
                                
                                uint64_t hashesbuild ;
                                if ((tab[x + dx+ dbuildx][y + dy +dbuildy] != DOME) && ((tab[x+dx+dbuildx][y+dy+dbuildy] & 0x03) == 0 )) {

                                    build(x , y ,dx ,dy , dbuildx , dbuildy , tab) ;

                                    int xbuild = x + dx+ dbuildx ;
                                    int ybuild = y + dy +dbuildy ;
                                    hashesbuild = hashes_build(xbuild , ybuild , tab);                
                                }
                                else continue;

                                int t = alpha_beta(alpha , b , BROWN , depth+1 , tab , maxdepth , base , hv^hashesblue[x+dx][y+dy]^hashesblue[x][y]^hashesbuild^turn_hashv);
                                    
                                if (t < g) 
                                {
                                g = t ;

                                curr_best.x = x ;
                                curr_best.y = y ;
                                curr_best.dx = dx ;
                                curr_best.dy = dy;
                                curr_best.dbuildx = dbuildx ;
                                curr_best.dbuildy = dbuildy ;

                                }
                                if (depth == base) best_move = curr_best;
                                b = min(b , g);
                                    
                                unbuild(x , y ,dx ,dy , dbuildx , dbuildy , tab);

                            }
                        }
                    unplay(color , x , y , dx , dy , tab);
                    }
                }
            player++ ;
        }
    }
    
    store_v_hash(hv , alpha , beta , g , dist , base , curr_best) ; 

    return g ;
}

//Fonction qui decode la pos recus par l'interface graphique 
void dec(int *x1,int *y1,int *x2,int *y2,int *x3,int *y3,int pos) {
  int tmp;
  tmp = pos % 25;
  *x1 = tmp/5;
  *y1 = tmp % 5;
  tmp =(pos/25)%25;
  *x2 = tmp/5 - *x1;
  *y2 = tmp%5 - *y1;
  tmp = pos/25/25;
  *x3 = tmp/5 - *x1 - *x2;
  *y3=tmp%5 - *y1 - *y2;
}



int main()
{
    
    int tab [SIZEX][SIZEY] = {{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY}
                            ,{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY}
                            ,{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY}
                            ,{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY}
                            ,{EMPTY,EMPTY,EMPTY,EMPTY,EMPTY}} ;
    
    tab[2][2] += BROWN  ;
    tab[3][3] += BROWN  ;
    tab[3][2] += BLUE  ;
    tab[2][3] += BLUE  ;

    clock_t time,otime;
    double ftime;

    uint64_t hv;
    init_hash();

    hv=init_hashv;

    flog = stderr ;
    int color = BROWN ;

    int ret , base=0 , maxdepth ; 

    while (true)
    {   
        //Initiation du compteur 
        otime=clock();

        //Verifier si le joueur a gagné contr le robot 
        if ( eval(tab) == -WIN ) {

            fprintf(flog,"Blue WIN \n");
            break;
        }

        //Initialisation du best move
        best_move.x = -1 ;
        best_move.y = -1 ;
        best_move.dx = -1 ;
        best_move.dy = -1 ;
        best_move.dbuildx = -1 ;
        best_move.dbuildy = -1 ;

        maxdepth = base + 4 ;
        
        //Jouer le best coup trouver par le robot 
        ret = alpha_beta(-INFINITE,+INFINITE,color,base,tab,maxdepth,base,hv);

        fprintf(flog,"valeur alphabeta = %d \n " , ret ) ;

        play(color , best_move.x , best_move.y , best_move.dx , best_move.dy , tab);
        build(best_move.x , best_move.y , best_move.dx , best_move.dy , best_move.dbuildx , best_move.dbuildy , tab);


        int xbuild = best_move.x + best_move.dx + best_move.dbuildx ;
        int ybuild = best_move.y + best_move.dy + best_move.dbuildy ;

        uint64_t hashesbuildbrown = hashes_build(xbuild , ybuild , tab) ;

        hv = hv^hashesbrown[best_move.x][best_move.y]^hashesbrown[best_move.x + best_move.dx][best_move.y + best_move.dy]^hashesbuildbrown ;

        int pos;
        pos=(best_move.y+best_move.x*5)+25*(best_move.y+best_move.dy+(best_move.x+best_move.dx)*5)+
            25*25*(best_move.y+best_move.dy+best_move.dbuildy+(best_move.x+best_move.dx+best_move.dbuildx)*5);

        printf("%d\n",pos);       
        fflush(stdout);

        if ( ret == WIN ) {
            fprintf(flog,"BROWN WIN \n") ;
        }

        //Fin de calcule de temps du robot
        time=clock()-otime;
        ftime=(double)time/(double)CLOCKS_PER_SEC;
        fprintf(flog , "time=%f\n",ftime);

        base = base +1  ;

        //Recevoir le mouvement du joueur
        int x , y ,dx ,dy ,dbuildx , dbuildy ;

        scanf("%d",&pos);
        fprintf(flog,"Move received:%d\n",pos);
        fflush(flog);
        
        dec(&x , &y , &dx , &dy , &dbuildx , &dbuildy , pos) ;

        play(BLUE ,x , y ,dx ,dy , tab) ;
        build(x , y ,dx ,dy ,dbuildx , dbuildy , tab) ;
        
        int xbuildblue = x + dx+ dbuildx ;
        int ybuildblue = y + dy +dbuildy ;

        uint64_t hashesbuildblue = hashes_build(xbuildblue , ybuildblue , tab) ;

        hv = hv^hashesbrown[x][y]^hashesbrown[x + dx][y + dy]^hashesbuildblue ;
    }
}
