#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>


#include "threads_manager.h"

threads_manager* init_th_manager(int size, int no_players){
  //printf("\n inside init_th_manager()\n");
  threads_manager* tm = (threads_manager*)malloc(sizeof(threads_manager));
  tm->size = size;
  tm->index = 0; //before first element is inserted in the table
  tm->no_players = no_players;
  tm->tables = (blackjack_table**)malloc(size * sizeof(blackjack_table*));
  for(int i = 0; i<tm->size; i++){
    tm->tables[i] = NULL;
  }
  return tm;
}

void increase_size(threads_manager* tm){
  //printf("\n inside increase_size()\n");
  int old_size = tm->size;
  int new_size = old_size * 2;
  blackjack_table** tables =(blackjack_table**)malloc(new_size*sizeof(blackjack_table*));
  for(int i= 0; i<old_size; i++){
    tables[i] = tm->tables[i];
  }
  for(int j = old_size ;j<new_size;j++){
    tables[j] = NULL;
  }
  blackjack_table** tmp = tm->tables;
  tm->tables = tables;
  tm->size = new_size;
  free(tmp);
}

void add_blackjack_table(threads_manager* tm){
  //printf("\ninside add_blackjack_table()\n");
  if(tm->index >= tm->size - 1){
    increase_size(tm);
  }
  blackjack_table* pt = init_blackjack_table(tm->no_players);
  printf("blackjack_table initialized\n");
  tm->tables[tm->index] = pt;
  tm->index++;
}

int remove_blackjack_table(threads_manager* tm,int table_no){
  if(table_no < 0 || table_no > tm->index){
    fprintf(stderr,"invalid table_no argument\n");
    return -1;
  }
  free(tm->tables[table_no]);
  tm->tables[table_no] = NULL;
  for(int i = table_no;i<tm->index; i++){
    tm->tables[i] = tm->tables[i+1];
  }
  tm->tables[tm->index - 1] = NULL;
  tm->index--;
  return 1;
}

/* //sends infos to all players at the table */
/* void send_table_info(blackjack_table* bt){ */
/*   printf("inside send_table_info\n"); */
/*   char res[400]; */
/*   memset(res,0,400); */
/*   strcpy(res,"game_info--"); */
/*   char* strings[bt->number_of_players]; */
/*   for(int i = 0; i < bt->number_of_players; i++){ */

/*     char* card1 = card_to_string(bt->players[i]->card1); */
/*     printf("card1 = %s\n",card1); */

/*     char* card2 = card_to_string(bt->players[i]->card2); */
/*     printf("card2 = %s\n",card2); */

/*     char paction[10]; */
/*     memset(paction,0,10); */
/*     switch(bt->players[i]->act){ */
/*     case NO_ACTION: strncpy(paction,"no_action",9);break; */
/*     case HIT: strncpy(paction,"hit",3);break; */
/*     case STAND: strncpy(paction,"stand",5);break; */
/*     default:break; */
/*     } */
/*     printf("action = %s\n",paction); */
/*     char str[50]; */
/*     memset(str,0,50); */
/*     sprintf(str,"%d:%s;%s;%s;%s;%d;%d;%d;;",i,bt->players[i]->pseudo,card1,card2,paction,bt->players[i]->money,bt->players[i]->bet,bt->players[i]->card_sum); */
/*     strcat(res,str); */
/*     printf("inside send_table_info() : %s\n",str); */
/*   } */
  
/*   printf("resulting string = \n%s\n",res); */

/*   for(int i=0; i<bt->number_of_players; i++){ */
/*     if(send(bt->players[i]->socket_fd,res,sizeof(res),0) < 0){ */
/*       fprintf(stderr,"send error:%s\n",strerror(errno)); */
/*       exit(1); */
/*     } */

/*     char rbuf[10]; */
/*     memset(rbuf,0,10); */
/*     int numbytes = recv(bt->players[i]->socket_fd,rbuf,sizeof(rbuf),0); */
/*     if(numbytes < 0){ */
/*       fprintf(stderr,"recv error:%s\n",strerror(errno)); */
/*       exit(1); */
/*     } */
/*     rbuf[numbytes] = '\0'; */
/*     printf("server received '%s'\n",rbuf); */
/*   } */
/* } */

int add_player(threads_manager* tm, player* p){
  //printf("inside add_player()\n");
  if(tm->index == 0){//there is no allocated table
    printf("first table initialization\n");
    add_blackjack_table(tm);
  }
  if(add_player_to_table(tm->tables[tm->index - 1],p) == -1){
    add_blackjack_table(tm);
    add_player_to_table(tm->tables[tm->index - 1],p);
    //send_table_info(tm->tables[tm->index-1]);
    return tm->index - 1;
  }
  //sends message to everyone already connected
  //send_table_info(tm->tables[tm->index - 1]);
  
  return tm->index - 1;
}
  
int remove_player(threads_manager* tm,int table_no, player* p, pseudo_db* pb){
  if(remove_player_from_table(tm->tables[table_no],p,pb) == -1) return -1;
  else return 1;
}


int check_connectivity(player* p, int timeout){

  //printf("\ninside check_connectivity()\n");
  char* isconnected_msg = "13:req_connected";
  
  int sent = send(p->socket_fd,isconnected_msg,strlen(isconnected_msg),MSG_NOSIGNAL);
  if( sent == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    if(errno == EPIPE){
      printf("BROKEN PIPE\n");
      return 0;
    }
    return -1;
  }

  //printf("sent check connection message to client\n");
  //printf("number of bytes sent : %d/%d\n",sent,strlen(isconnected_msg));
  
  fd_set readfds;
  struct timeval t;
  t.tv_sec = timeout;
  t.tv_usec = 0;
  FD_ZERO(&readfds);
  FD_SET(p->socket_fd,&readfds);
  int rv = select(p->socket_fd + 1,&readfds,NULL,NULL,&t);
  if(rv == -1){
    fprintf(stderr,"select: error in select: %s\n",strerror(errno));
    return -1;
  }else if(rv == 0){
    printf("time out\n");
    return 0;
  }else{
    char recvbuf[5];
    int sz =  recv(p->socket_fd,recvbuf,5,0);
    if(sz == -1){
      fprintf(stderr,"recv: error in recv: %s\n",strerror(errno));
      return -1;
    }else if(sz == 0){
      printf("disconnected\n");
      return 0;
    }else{
      printf("received %d bytes\n",sz);
      return 1;
    }
    
  }
}

void print_blackjack_tables(threads_manager* tm){
  //printf("\ninside print_blackjack_table\n");
  printf("number of blackjack tables = %d\n",tm->index); 
  for(int i = 0; i < tm->index ; i++){
    printf("Blackjack Table %d :\n{ ",i);
    //printf("size of the blackjack table = %d\n",tm->tables[i]->size);
    //printf("number of players already at the table = %d\n",tm->tables[i]->number_of_players);
    for(int j = 0; j < tm->tables[i]->size; j++){
      if(j < tm->tables[i]->number_of_players){
         printf(" %s:%d ",tm->tables[i]->players[j]->pseudo,tm->tables[i]->players[j]->socket_fd);
      }else{
         printf(" empty "); 
      }
    }
    printf("}\n\n");
  }
}

