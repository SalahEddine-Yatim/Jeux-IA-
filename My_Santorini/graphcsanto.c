#define _GNU_SOURCE

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#include <SDL/SDL.h>


SDL_Surface *ecran = NULL, *background = NULL, *firstEt = NULL,
  *secEt = NULL, *thirdEt = NULL, *dome = NULL, *ouv1 = NULL,*ouv1l = NULL,
  *ouv2 = NULL,*ouv2l = NULL;

SDL_Rect positionBackground;

#define SIZE 5
#define TRUE 1
#define FALSE 0

int height[SIZE][SIZE]={{0}};
int workers[SIZE][SIZE]={{0}};

int to[2],from[2];


void init_sdl(){

  positionBackground.x = 0;
  positionBackground.y = 0;
  
  /* Initialisation de la SDL */
  SDL_Init(SDL_INIT_VIDEO);
  /* Ouverture de la fenêtre */
  ecran = SDL_SetVideoMode(800, 800, 32, SDL_HWSURFACE); 
  SDL_WM_SetCaption("Santorini", NULL);
  
  background = SDL_LoadBMP("BITMAPS/table.bmp");
  SDL_BlitSurface(background, NULL, ecran, &positionBackground);
  
  firstEt = SDL_LoadBMP("BITMAPS/1.bmp");
  SDL_SetColorKey(firstEt, SDL_SRCCOLORKEY, SDL_MapRGB(firstEt->format, 255, 255, 255));
  
  secEt = SDL_LoadBMP("BITMAPS/2.bmp");
  SDL_SetColorKey(secEt,SDL_SRCCOLORKEY, SDL_MapRGB(secEt->format, 255, 255, 255));
    
  thirdEt = SDL_LoadBMP("BITMAPS/3.bmp");
  SDL_SetColorKey(thirdEt, SDL_SRCCOLORKEY, SDL_MapRGB(thirdEt->format, 255, 255, 255));
  
  
  ouv1 = SDL_LoadBMP("BITMAPS/ouv1.bmp");
  SDL_SetColorKey(ouv1, SDL_SRCCOLORKEY, SDL_MapRGB(ouv1->format, 255, 255, 255));
  
  ouv1l = SDL_LoadBMP("BITMAPS/ouv1l.bmp");
  SDL_SetColorKey(ouv1l, SDL_SRCCOLORKEY, SDL_MapRGB(ouv1->format, 255, 255, 255));
  
  ouv2 = SDL_LoadBMP("BITMAPS/ouv2.bmp");
  SDL_SetColorKey(ouv2, SDL_SRCCOLORKEY, SDL_MapRGB(ouv2->format, 255, 255, 255));
  
  ouv2l = SDL_LoadBMP("BITMAPS/ouv2l.bmp");
  SDL_SetColorKey(ouv2l, SDL_SRCCOLORKEY, SDL_MapRGB(ouv2->format, 255, 255, 255));
  
  dome = SDL_LoadBMP("BITMAPS/4.bmp");
  SDL_SetColorKey(dome, SDL_SRCCOLORKEY, SDL_MapRGB(dome->format, 255, 255, 255));

  SDL_Flip(ecran);
  
}

void affiche(){
  int etage,ouv,y,x;
  SDL_Rect positionEt;
  
  SDL_BlitSurface(background, NULL, ecran, &positionBackground);
	
  for (y = 0; y<SIZE; y++){
    for (x = 0; x<SIZE; x++){
      positionEt.x = 40+150*x;
      positionEt.y = 40+150*y;
      etage = height[x][y];
      if (etage == 1){
	positionEt.y -=10;
	SDL_BlitSurface(firstEt, NULL, ecran, &positionEt);
      }
      else if (etage == 2){
	positionEt.y -=40;
	SDL_BlitSurface(secEt, NULL, ecran, &positionEt);
      }
      else if (etage == 3){
	positionEt.y -=50;
	SDL_BlitSurface(thirdEt, NULL, ecran, &positionEt);
      }
      else if (etage == 4){
	positionEt.y -=60;
	SDL_BlitSurface(dome, NULL, ecran, &positionEt);
      }
      
      positionEt.x +=25;
      positionEt.y -=20;
      
      ouv = workers[x][y];
      if (ouv == 1){
	positionEt.y -=15;
	SDL_BlitSurface(ouv1, NULL, ecran, &positionEt);
      }
      else if (ouv == -1){
	SDL_BlitSurface(ouv2, NULL, ecran, &positionEt);
      }
      else if (ouv == 2){
	positionEt.y -=15;
	SDL_BlitSurface(ouv1l, NULL, ecran, &positionEt);
      }
      else if (ouv == -2){
	SDL_BlitSurface(ouv2l, NULL, ecran, &positionEt);
      }
    }
  }
  SDL_Flip(ecran);
}

#define NOT_FINISHED -1
#define WIN 32767
int handle_move(int x,int y,int turn) {
  static int state=0, sx, sy, nx, ny;
#ifdef DEBUG
  printf("x=%d y=%d state=%d\n",x,y,state);
#endif
  switch(state) {
  case 0:
    if (workers[x][y]==turn) {
      workers[x][y]=2*turn;
      sx=x;sy=y;
      state=1;
      affiche();
    }
    return NOT_FINISHED;
  case 1:
    if (abs(x-sx)<=1 && abs(y-sy)<=1 &&(x!=sx||y!=sy) &&
	workers[x][y]==0 && height[x][y]<=(height[sx][sy]+1)) {
      workers[x][y]=2*turn;
      workers[sx][sy]=0;
      if (height[x][y]==3) {
	state=3;
	affiche();
	return WIN;}
      nx=x;ny=y;
      state=2;
    }
    else {
      state=0;
      workers[sx][sy]=turn;
    }
    affiche();
    return NOT_FINISHED;
  case 2:
    if (abs(x-nx)<=1 && abs(y-ny)<=1 &&(x!=nx||y!=ny) &&
	workers[x][y]==0 && height[x][y]!=4) {
      height[x][y]++;
      workers[nx][ny]=turn;
      state=0;
      affiche();
#ifdef DEBUG
      printf("(%d,%d) (%d,%d) (%d,%d)\n",sx,sy,nx,ny,x,y);
#endif
      return (sx*5+sy)+25*(nx*5+ny)+25*25*(x*5+y);
    }
    else {
      workers[nx][ny]=0;
      workers[sx][sy]=turn;
      state=0;
      affiche();
      return NOT_FINISHED;
    }
  default:
    printf("state=%d???\n",state);
    exit(-1);
    break;
  }
}

void send_move(int turn,int move) {
  char buf[256];
  int ret;
  sprintf(buf,"%d\n",move);
  ret=write(to[(1-turn)/2],buf,strlen(buf));
#ifdef DEBUG
  printf("written:%d %s\n",ret,buf);
#endif
}

void MainLoop(int turn,int* programs)
{
    SDL_Event event;
    int x,y,ret,sx,sy,i;
    int pos,npos,build;

    do {
      SDL_WaitEvent(&event);
      switch(event.type) {
      case SDL_MOUSEBUTTONDOWN:
	x=event.button.x/160;
	y=event.button.y/160;
#ifdef DEBUG
	printf("%d %d\n",x,y);
#endif
	ret=handle_move(x,y,turn);
	if (ret==WIN) {
	  printf("Game finished %d wins\n",turn);
	  affiche();
	  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_DISABLE);
	}
	else if (ret>=0) {
	  turn=-turn; /* If program to play, send move */
	  if (programs[(1-turn)/2]) {
	    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_DISABLE);
	    send_move(turn,ret);
	  }
	}
	break;
      case SDL_QUIT:
	return;
      case SDL_VIDEOEXPOSE:
	affiche();
	break;
      case SDL_USEREVENT:
#ifdef DEBUG
	printf("USEREVENT received=%d\n",event.user.code);
#endif
	pos=event.user.code%25;
	npos=(event.user.code/25)%25;
	build=(event.user.code/25)/25;
	x=pos/5;y=pos%5;
#ifdef DEBUG
	printf("pos=%d x=%d y=%d\n",pos,x,y);
#endif
	if (workers[x][y]!=turn) {
	  printf("pos=%d x=%d y=%d workers=%d\n",pos,x,y,workers[x][y]);
	  break;
	}
	workers[x][y]=0;
	sx=x;sy=y;
	x=npos/5;y=npos%5;
#ifdef DEBUG
	printf("npos=%d x=%d y=%d\n",npos,x,y);
#endif
	if (workers[x][y]!=0 || height[x][y]==4) {
	  printf("npos=%d x=%d y=%d workers=%d heights=%d\n",
		 pos,x,y,workers[x][y],height[x][y]);
	  break;
	}
	workers[x][y]=turn;
	affiche();
	for (i=0;i<2;i++) {
	  usleep(250000);
	  workers[x][y]=0;
	  workers[sx][sy]=turn;
	  affiche();
	  usleep(250000);
	  workers[x][y]=turn;
	  workers[sx][sy]=0;
	  affiche();
	}
	if (height[x][y]==3) {
	  printf("Game finished %d wins\n",turn);
	  affiche();
	  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_DISABLE);
	  SDL_EventState(SDL_USEREVENT, SDL_DISABLE);
	  break;
	}
	x=build/5;y=build%5;
#ifdef DEBUG
	printf("build=%d x=%d y=%d\n",build,x,y);
#endif
	if (workers[x][y]!=0 || height[x][y]==4) {
	 printf("npos=%d x=%d y=%d workers=%d heights=%d\n",
		 pos,x,y,workers[x][y],height[x][y]);
	  break;
	}
	height[x][y]++;
	affiche();
	for (i=0;i<2;i++) {
	  height[x][y]--;
	  usleep(250000);
	  affiche();
	  height[x][y]++;
	  usleep(250000);
	  affiche();
	}
	
	turn = -turn;
#ifdef DEBUG
	printf("turn=%d programs=%d\n",turn,programs[(1-turn)/2]);
#endif
	if (programs[(1-turn)/2]) {
	  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_DISABLE);
	  send_move(turn,event.user.code);
	}
	else
	  SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
	break;
      default:
	break;
      }
    }
    while (1);
    /*    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE); */
}

void handler (int signum,siginfo_t *si,void *ptr) {
  SDL_Event event;
  char buf[256];
  int ret,fd,v;
  
#ifdef DEBUG
  printf("Signal %d received\n",signum);
  if (signum==SIGIO)
    printf("SIGIO confirmed once\n");
  if (si->si_signo==SIGIO)
    printf("SIGIO confirmed twice\n");
#endif
  if (signum!=SIGIO||si->si_signo!=SIGIO)
    return;
  fd=si->si_fd;
#ifdef DEBUG
  printf("File descriptor is %d\n",fd);
  printf("si_code=%d POLL_IN=%d\n",si->si_code,POLL_IN);
#endif
  if (si->si_code==POLL_IN) {
#ifdef DEBUG
    printf("Reading from %d\n",fd);
#endif
    ret=read(fd,buf,256);
#ifdef DEBUG
    printf("read %d bytes\n",ret);
#endif
    if (ret>0) {
      buf[ret]=0;
#ifdef DEBUG
      printf("message read:%s\n",buf);
#endif
      v=atoi(buf);
#ifdef DEBUG
      printf("v=%d\n",v);
#endif
      SDL_memset(&event,0,sizeof(event));
      event.type = SDL_USEREVENT;
      event.user.code = v;
      event.user.data1 = NULL;
      event.user.data2 = NULL;
      SDL_PushEvent(&event);
    }
    else
      printf("This value of ret is strange!!!!!\n");
  }
  else
    printf("No data available????\n");
}

void start(char *path,char **args, int *to,int *from,int *pid) {
  int fd1[2],fd2[2],res,ret,i;

  printf("Start called with path=%s and args=",path);
  for (i=0;i<7;i++) printf(" %s",args[i]);
  printf("\n");
  res=pipe(fd1);
  if (res==-1) {perror("pipe1");exit(-1);}
  res=pipe(fd2);
  if (res==-1) {perror("pipe2");exit(-1);}
  *pid=fork();
  if (*pid==0) {/*Child*/
    dup2(fd1[0],0);
    dup2(fd2[1],1);
    usleep(100000);
    res=execv(path,args);
    if (res==-1) {perror("execv");exit(-1);}
  }
  *to=fd1[1];
  *from=fd2[0];

  ret=fcntl(*from,F_SETFL,O_ASYNC|O_NONBLOCK);
  if (ret!=0) {
    perror("fcntl0:");
    exit(-1);
  }
  ret=fcntl(*from,F_SETOWN,getpid());
  if (ret!=0) {
    perror("fcntl1:");
    exit(-1);
  }
  ret=fcntl(*from,F_SETSIG,SIGIO);
  if (ret!=0) {
    perror("fcntl1:");
    exit(-1);
  }
  

}

void check(int a) {if (a<0||a>24) {printf("Bad argument:%d\n",a);exit(-1);}}

int main(int argc,char **argv) {
  int turn=1;
  int programs[2]={FALSE,FALSE};
  int ret,i,x,y;
  char *name[2]={NULL,NULL};
  double time=1;
  int w[2]={12,18};
  int b[2]={13,17};
  char *args[8]={NULL};
  char path[256];
  int pid[2];
  struct sigaction sa;

  setvbuf(stdout,NULL,_IONBF,0);

  for (i=0;i<7;i++) args[i]=(char *)malloc(256);

  i=1;
  while ((i+1)<argc) {
    if (strcmp(argv[i],"-fpn")==0) {
      name[0]=argv[i+1];i+=2;
    }
    else if (strcmp(argv[i],"-spn")==0) {
      name[1]=argv[i+1];i+=2;
    }
    else if (strcmp(argv[i],"-time")==0) {
      time=atof(argv[i+1]);i+=2;
      if (time<1.0 || time>3600.) {
	printf("Bad time\n");
	exit(-1);
      }
    }
    else if (strcmp(argv[i],"-pos")==0 && (i+4)<argc) {
      w[0]=atoi(argv[i+1]);w[1]=atoi(argv[i+2]);
      b[0]=atoi(argv[i+3]);b[1]=atoi(argv[i+4]);
      i+=5;
    }
    else {
      printf("Unknown argument: %s\n",argv[i]);
      i++;
    }
  }

  for (i=0;i<2;i++) {check(w[i]);check(b[i]);}
  if (w[0]==w[1] || w[0]==b[0] || w[0]==b[1] || w[1]==b[0] || w[1]==b[1]
      || b[0]==b[1]) {printf("Bad positions\n");exit(-1);}
  
  bzero(&sa,sizeof(sa));
  sa.sa_flags=SA_SIGINFO;
  sa.sa_sigaction=handler;
  ret=sigaction(SIGIO,&sa,NULL);
  if (ret!=0) {
    perror("sigaction:");
    exit(-1);
  }


  for (i=0;i<2;i++)
    if (name[i]!=NULL) programs[i]=TRUE;

  init_sdl();
  SDL_EventState(SDL_QUIT, SDL_ENABLE);
  SDL_EventState(SDL_VIDEOEXPOSE, SDL_ENABLE);
  SDL_EventState(SDL_USEREVENT, SDL_ENABLE);
  if (!programs[(1-turn)/2]) 
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);

  for (i=0;i<2;i++) {
    x=w[i]/5;y=w[i]%5;workers[x][y]=1;
    x=b[i]/5;y=b[i]%5;workers[x][y]=-1;
  }
  
  affiche();

  

  for (i=0;i<2;i++)
    if (programs[i]) {
      strcpy(path,name[i]);
      strcpy(args[0],name[i]);
      sprintf(args[1],"%d",(i+1));
      sprintf(args[2],"%f",time);
      sprintf(args[3],"%d",w[0]);
      sprintf(args[4],"%d",w[1]);
      sprintf(args[5],"%d",b[0]);
      sprintf(args[6],"%d",b[1]);
      args[7]=NULL;
      start(path,args,&to[i],&from[i],&pid[i]);
    }
  
  MainLoop(turn,programs);

  printf("Closing descriptors and killing children\n");
  for (i=0;i<2;i++)
    if (programs[i]) {
      close(from[i]);
      close(to[i]);
      kill(pid[i],SIGKILL);
    }
  
  return 0;
}
