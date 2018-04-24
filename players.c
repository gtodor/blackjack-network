#include "players.h"
#include "pseudos.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define MAXDATASIZE 100

player* init_player(int socket_fd, pseudo_db* pb){
  //printf("\ninside init_player()\n");
  player* p = (player*)malloc(sizeof(player));
  p->socket_fd = socket_fd; //set the socket descriptor
  p->connected = 1; //set connected to true
  // check if pseudo already exists before we bind it to the player
  //steps : check_existance - bind_pseudo - set pseudo to player
  char pseudo[20];
  memset(pseudo,0,20);
  //printf("before first ask for pseudo\n");
  ask_for_pseudo(socket_fd,pseudo);
  //printf("pseudo is %s\n",pseudo);
  if(check_existance(pb,pseudo) == 0){
    //printf("checking existance for %s\n",pseudo);
    bind_pseudo(&pb,pseudo);
    send_pseudo_confirmation(socket_fd);
  }else{
    //printf("check_existance for %s returned 1\n",pseudo);
    while(check_existance(pb,pseudo) != 0){
      //printf("checking existance for %s in loop\n",pseudo);
      memset(pseudo,0,20);
      ask_for_pseudo(socket_fd,pseudo);
    }
    bind_pseudo(&pb,pseudo);
    send_pseudo_confirmation(socket_fd);
  }
  //printf("now pseudo is %s\n",pseudo);
  strncpy(p->pseudo,pseudo,20);
  //p->card1 = NULL;
  //p->card2 = NULL;
  p->money = 500;
  p->bet = 0;
  p->act = NO_ACTION;
  p->card_sum = 0;
  p->card_ind = -1;
  for(int i = 0; i< 20; i++){
    p->cards[i] = NULL;
  }
  
  return p;
}


char* ask_for_pseudo(int socket_fd, char* pseudo){
  //printf("\ninside ask_pseudo()\n ");

   char* init_msg = "10:req_pseudo";

   if(send(socket_fd,init_msg,strlen(init_msg),0) < 0){
     fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
     exit(1); 
   }
  
   char readbuf[MAXDATASIZE];
   memset(readbuf,0,MAXDATASIZE); 
   int numbytes;
   int bytes;
   int num_buf[4];
   memset(num_buf,0,4);
   if((bytes = recv(socket_fd,num_buf,3,0)) == -1){
     fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
     exit(1);
   }
   num_buf[bytes-1] = '\0';
   int sz = atoi(num_buf);
   //  printf("we read %d for pseudo\n",sz);
   
   //printf("receving pseudo from client\n");
   if((numbytes = recv(socket_fd,readbuf,sz,0)) < 0){
     fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
     exit(1);
    
   }
   //printf("numbytes =  %d\n", numbytes);
   readbuf[numbytes] = '\0';
   
   //printf("connected client has pseudo : %s \n",readbuf);
   strncpy(pseudo,readbuf,sz);
   return readbuf;
   
}

void send_pseudo_confirmation(int socket_fd){
  char* init_msg = "14:pseudo_enabled";
  if(send(socket_fd,init_msg,strlen(init_msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return;
  }
}


void send_players_info(blackjack_table* table, int socket_fd){
  //printf("inside send_player_info() \n");

  if(table->number_of_players == 0){
    //printf("no player at the table\n");
    return;
  }
  
  int n = table->size * 20 + table->size * 3;
  char message[n];
  //message ex:
  //1:pseudo1;;2:pseudo2;;
  memset(message,0,n);
  char str[40];
  memset(str,0,40);
  
  sprintf(str,"players_info=%d:%s;;",0,table->players[0]->pseudo);
  //printf("aaa\n");
  strncpy(message,str,40);
  //printf("temp = %s\n",message);
  for(int i=1; i< table->number_of_players; i++){
    memset(str,0,40);
    sprintf(str,"%d:%s;;",i,table->players[i]->pseudo);
    //printf("ccc\n");
    strncat(message,str,40);
    //printf("ddd\n");
  }
  //printf("message = %s\n",message);
  int m = n+3;
  char final_message[m];
  memset(final_message,0,m);
  char length[2];
  memset(length,0,2);
  if(length < 10){
    sprintf(length,"0%d:",strlen(message));
  } else{
    sprintf(length,"%d:",strlen(message));
  }
  strncpy(final_message,length, strlen(length));
  strncat(final_message, message, strlen(message));
  //printf("final message = %s\n",final_message);

  if(send(socket_fd,final_message,strlen(final_message),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return;
  }
  
}

void send_start_game(int socket_fd){
  printf("inside send_start_game()\n");
  char* start_game = "10:start_game";
  if(send(socket_fd,start_game,strlen(start_game),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    return;
  }
  
}

blackjack_table* init_blackjack_table(int size){
  //printf("\ninside init_blackjack_table()\n");
  blackjack_table* pt = (blackjack_table*)malloc(sizeof(blackjack_table));
  pt->size = size;
  pt->number_of_players = 0; 
  pt->players = (player**)malloc(size * sizeof(player*));
  for(int i=0;i<size;i++){
    pt->players[i] = NULL;
  }
  pt->full = 0;
  pt->count_views = 0;
  pt->info_changed = NO_INFO;
  pt->tour = 0;
  pt->card_package = init_card_package();
  shuffle_cards(pt->card_package);
  for(int i = 0; i<20; i++){
    pt->dealer_cards[i] = NULL;
  }
  return pt;
}

int add_player_to_table(blackjack_table* pt, player* p){
  //printf("\ninside add_player_to_table()\n");
  if(pt == NULL){
    printf("blackjack_table doesn't exist\n");
    return -1;
  }
  if(pt->full == 1){
    printf("blackjack table is full\n");
    return -1;
  }
  if(p == NULL){
    printf("player is null\n");
    return -1;
  }
  printf("number of players already at the table = %d\n",pt->number_of_players);
  pt->players[pt->number_of_players] = p;
  pt->number_of_players++;
  if(pt->number_of_players >= pt->size){
    printf("blakjack table became full\n");
    pt->full = 1;
  }
  return 1;
}

int remove_player_from_table(blackjack_table* pt, player* p, pseudo_db* pb){
  int found = 0;
  for(int i=0; i<pt->size; i++){
    if(p == pt->players[i]){
      printf("REMOVING_PLAYER %s from table\n",p->pseudo);
      //close the socket descriptor, unbind pseudo and free the player structure
      close(pt->players[i]->socket_fd);
      unbind_pseudo(&pb,pt->players[i]->pseudo);
      free(pt->players[i]);
      
      pt->players[i] = NULL;
      pt->number_of_players--;
      found = 1;
      for(int j= i; j < pt->size - 1; j++){
	pt->players[j] = pt->players[j+1];
      }
      pt->players[pt->size - 1] = NULL;
      if(pt->number_of_players < pt->size){
	pt->full = 0;
      }
      break;
    }
  }
  if(found == 1){
    //send a message with the new tour to each player
    /* for(int i = 0; i< pt->number_of_players; i++){ */
    /*   send_players_info(pt,pt->players[i]->socket_fd); */
    /* } */
    return 1;
  }
  else return -1;
}

void send_disconnected_to_all(blackjack_table* table, player* p){
  char msg[40];
  memset(msg,0,40);
  int l = strlen(p->pseudo) + strlen("player_disconnected=") + 3;
  sprintf(msg,"%d:player_disconnected=%s",l,p->pseudo);
  for(int i = 0; i<table->number_of_players; i++){
    if(strcmp(table->players[i]->pseudo,p->pseudo) != 0){
      if(send(table->players[i]->socket_fd,msg,strlen(msg),MSG_NOSIGNAL) == -1){
	fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	if(errno == EPIPE){
	  printf("BROKEN PIPE?\n");
	}
	return;
      }
    }
  }
}
      

void send_first_card(blackjack_table* table,card_package_t* pack){
  printf("inside send_first_card()\n");
  while(table->count_views != table->number_of_players){
    //printf("waiting that all threads are ready to send the card\n");
  }
  printf("out of inner while LOOP\n");
  print_card_package(pack);
  for(int i = 0; i< table->number_of_players; i++){
    card_t* c = get_card(pack);
    printf("INFO: after get_card(pack) call\n");
    printf("got card c.symbol=%s\n",c->symbol);
    printf("after accessing card info\n");
    /* if(c == NULL){ */
    /*   printf("ERROR card is null\n"); */
    /* }else{ */
    /*   printf("GOOD card not null\n"); */
    /* } */
    char* str = card_to_string(c);
    printf("INFO: after card_to_string(c) found %s\n",str);
    char msg[30];
    memset(msg,0,30);
    int length = 13+strlen(str)+strlen(table->players[i]->pseudo);
    sprintf(msg,"%d:first_card=%s(%s)",length,str,table->players[i]->pseudo);
    for(int j = 0; j< table->number_of_players; j++){
      printf("sending msg:%s to client %d\n",msg,j);
      if(send(table->players[j]->socket_fd,msg,strlen(msg),0) == -1){
	fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	return;
      }
    }
    table->players[i]->card_ind++;
    table->players[i]->cards[table->players[i]->card_ind] = c;
    
    
      
      
  }
  //send to clients the informations about the dealer

  card_t* c = get_card(pack);
  char* str = card_to_string(c);
  
  char msg[30];
  memset(msg,0,30);
  int length = 13+strlen(str)+strlen("dealer");
  sprintf(msg,"%d:first_card=%s(dealer)",length,str);
  for(int j = 0; j< table->number_of_players; j++){
    //printf("sending msg:%s to client %d\n",msg,j);
    if(send(table->players[j]->socket_fd,msg,strlen(msg),0) == -1){
      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
      return;
    }
  }
  table->dealer_cards[0] = c;
  
  
  //reset the counter to zero
  table->count_views == 0;
  //printf("END sending first card\n");
}

void send_second_card(blackjack_table* table,card_package_t* pack){
  //printf("inside send_second_card()\n");
  //printf("views= %d; players = %d\n",table->count_views,table->number_of_players);
  while(table->count_views != table->number_of_players){
    //printf("waiting that all threads are ready to send the card\n");
  }
  for(int i = 0; i< table->number_of_players; i++){
    card_t* c = get_card(pack);
    char* str = card_to_string(c);
    char msg[30];
    memset(msg,0,30);
    int length = 14+strlen(str)+strlen(table->players[i]->pseudo);
    sprintf(msg,"%d:second_card=%s(%s)",length,str,table->players[i]->pseudo);
    for(int j = 0; j< table->number_of_players; j++){
      //printf("sending message %s to client %d\n",msg,j);
      if(send(table->players[j]->socket_fd,msg,strlen(msg),0) == -1){
	fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	return;
      }
    }
    table->players[i]->card_ind++;
    table->players[i]->cards[table->players[i]->card_ind] = c;
    
  }
  //send infos about the dealer to clients, this card is hidden
  card_t* c = get_card(pack);
  hide_card(c);
  //printf("SECOND DEALER CARD hidden = %d\n",c->hidden);
  char* str = card_to_string(c);
  
  char msg[30];
  memset(msg,0,30);
  int length = 14+strlen(str)+strlen("dealer");
  sprintf(msg,"%d:second_card=%s(dealer)",length,str);
  for(int j = 0; j< table->number_of_players; j++){
    printf("sending msg:%s to client %d\n",msg,j);
    if(send(table->players[j]->socket_fd,msg,strlen(msg),0) == -1){
      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
      return;
    }
  }
  table->dealer_cards[1] = c;
  //reset the counter to zero
  table->count_views == 0;
  //printf("END sending second card\n");
}

/*
void ask_for_bets(blackjack_table* table){
  printf("inside ask_for_bets method\n");
  for(int i = 0; i < table->number_of_players; i++){
    char msg[30];
    memset(msg,0, 30);
    int length = 9 + strlen(table->players[i]->pseudo);
    sprintf(msg,"%d:req_bet(%s)",length,table->players[i]->pseudo);
    printf("asking player %s for his bet\n",table->players[i]->pseudo);
    if(send(table->players[i]->socket_fd,msg,strlen(msg),0) == -1){
      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
      return;
    }

    //get the answer from client i only
    int numbytes; 
    char readbuf[20];
    memset(readbuf,0,20);
    if((numbytes = recv(table->players[i]->socket_fd,readbuf,sizeof(readbuf),0)) < 0){
      fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
      exit(1);
    }
    
    printf("numbytes =  %d\n", numbytes);
    readbuf[numbytes] = '\0';
    printf("received msg: %s\n",readbuf);

    if(strncmp(msg,"send_bet:",9) != 0){
      memset(readbuf,0,20);
      if((numbytes = recv(table->players[i]->socket_fd,readbuf,sizeof(readbuf),0)) < 0){
  	fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
  	exit(1);
      }
      
      printf("numbytes =  %d\n", numbytes);
      readbuf[numbytes] = '\0';
      printf("received msg: %s\n",readbuf);
    }
  }
}
*/
