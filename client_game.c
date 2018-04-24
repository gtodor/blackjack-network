#include "client_game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAXDATASIZE 100

int global = 0; 

//this method should wait for all players to join the game before
//ending and entering the main loop 
game_instance* init_game(int socket_fd, char* pseudo){
  game_instance* game = (game_instance*)malloc(sizeof(game_instance));
  game->socket_fd = socket_fd;
  memset(game->my_pseudo,0,20);
  strncpy(game->my_pseudo,pseudo,strlen(pseudo));

  //setting up the client's pseudo
  char msg[MAXDATASIZE];
  memset(msg,0,MAXDATASIZE);
  message m = get_message(socket_fd,msg,MAXDATASIZE);
  while(m != pseudo_enabled){
    //printf("inside pseudo init while loop\n");
    //printf("message received: %s\n",msg);
    if(m == req_pseudo){
      //printf("sending pseudo to server\n");
      send_pseudo(socket_fd, pseudo);
    }else if(m == req_other_pseudo){
      //printf("retrying to send pseudo to server\n");
      send_other_pseudo(socket_fd,pseudo);
      memset(game->my_pseudo,0,20);
      strncpy(game->my_pseudo,pseudo,strlen(pseudo));
    }else{
      //printf("message not recognized\n");
    }
    memset(msg,0,MAXDATASIZE);
    m = get_message(socket_fd,msg,MAXDATASIZE);
  }
  
  //init players_pseudo table and other variables
  for(int i = 0; i < 10; i++){
    memset(game->players_pseudos[i],0,20);
    game->players_actions[i] = NO_ACTION;
    for(int j = 0; j<20; j++){
      game->players_cards[i][j] = NULL;
    }
    game->players_bets[i] = 0;
    game->players_money[i] = 500;
    game->cards_sum[i] = 0;
    game->players_connected[i] = 0;//initially all players are disconnected
    game->is_playing[i] = 0;//initially no one is playing
    
  }

  //printf("pseudo resolved\n");
  int mm = 1;//players at the table-- there is already the dealer
  
  while( m != start_game){
    //wait for players to join the game
    //printf("inside waiting loop...\n");
    memset(msg,0,MAXDATASIZE);
    m = get_message(socket_fd,msg,MAXDATASIZE);
    //printf("received from server '%s'\n",msg);
    if(m == players_info){
      //read the players info message
      //printf("players info message:\n %s\n",msg);

      int i = 13;//starting where the begining of the message ends
      
      while(msg[i] != '\0'){
	char digits[3];
	char pseudo_p[20];//here could raise a problem
	memset(digits,0,3);
	memset(pseudo,0,20);
	int k=0;
	int l = 0;
	int text = 0;//separate between reading digits and pseudos
	while(msg[i] != ';'){
	  if(text == 0){
	    while(msg[i] != ':'){
	      digits[k] = msg[i];
	      k++;
	      i++;
	    }
	    i++;//skip ':'
	    text = 1;
	  }
	  digits[k]='\0';
	  pseudo_p[l] = msg[i];
	  l++;
	  i++;
	}
	int player_turn = atoi(digits);
	pseudo_p[l]='\0';
	strncpy(game->players_pseudos[mm],pseudo_p,strlen(pseudo_p));
	game->players_connected[mm] = 1;
	if(strcmp(pseudo_p,game->my_pseudo) == 0){
	  game->my_tour_number = mm;
	}
	mm++;
	i += 2;//skip the ";;" separator
      }
      game->number_of_players = mm;
      
    }else if(m == req_connected){
      //printf("requested connection validation\n");
      send_keep_connection(socket_fd);
    }
  }
  strncpy(game->players_pseudos[0],"dealer",strlen("dealer"));
  game->players_connected[0] = 1;
  
  return game;
}

message get_message(int socket_fd,char* message, int size){
  char rbuf[MAXDATASIZE];
  int numbytes;
  int bytes;
  int size_d;
  char size_data[4];//maximum 99 caracters can be read need for : and \0 
  
  memset(rbuf,0,MAXDATASIZE);
  memset(size_data,0,4);
  //printf("before reading from the server\n");
  //read the header on 2 chars
  //read something like 4:toto or 11:req: pseudo
  if((bytes = recv(socket_fd,size_data,3,0)) == -1){
    fprintf(stderr,"recv : error while reading from the server : %s\n",strerror(errno));
    exit(1);
  }
  size_data[bytes-1] = '\0';
  size_d = atoi(size_data);
  //printf("We should read : %d bytes of data\n",size_d);
  
  if((numbytes = recv(socket_fd,rbuf,size_d,0)) == -1){
    fprintf(stderr,"recv : error while reading from the server : %s\n",strerror(errno));
    exit(1);
  }
  rbuf[numbytes] = '\0';
  printf("GET MESSAGE(): received '%s'\n",rbuf);
  strncpy(message,rbuf,size_d);

  if(strncmp(rbuf,"req_pseudo",10) == 0 && global == 0){
    global++;
    // printf("req_pseudo\n");
    return req_pseudo;
  }else if (strncmp(rbuf,"req_pseudo",10) == 0 && global != 0){
    //printf("req: pseudo -- other\n");
    global++;
    return req_other_pseudo;
  }else if(strncmp(rbuf,"req_connected",13) == 0){
    //printf("req_connected\n");
    return req_connected;
  }else if(strncmp(rbuf,"pseudo_enabled",14) == 0){
    //printf("pseudo_enabled\n");
    return pseudo_enabled;
  }else if(strncmp(rbuf,"start_game",10) == 0){
    //printf("start_game\n");
    return start_game;
  }else if(strncmp(rbuf,"players_info=",13)==0){
    //printf("game info message:\n");
    return players_info;
  }else if(strncmp(rbuf,"player_disconnected=",20)==0){
    //printf("player disconnected\n"); 
    return player_disconnected;
  }else if(strncmp(rbuf,"first_card=",11)==0){
    //printf("\n\n\n\nfirst card received\n\n\n\n");
    return first_card;
  }else if(strncmp(rbuf,"second_card=",12)==0){
    //printf("\n\n\n\nsecond card received\n\n\n\n");
    return second_card;
  }else if(strncmp(rbuf,"req_bet",7) == 0){
    //printf("request to bet\n");
    return req_bet;
  }else if(strncmp(rbuf,"spread_bet",10) == 0){
    //printf("bet from another player\n");
    return spread_bet;
  }else if(strncmp(rbuf,"play_turn",9) == 0){
    //printf("choose actions to do this round\n");
    return play_turn;
  }else if(strncmp(rbuf,"update_stand",12) == 0){
    //printf("\n\ntell other players that a player stands\n\n");
    return update_stand;
  }else if(strncmp(rbuf,"asked_card",10) == 0){
    //printf("\n\nhit action that gets a new card\n\n");
    return asked_card;
  }else if(strncmp(rbuf,"stand",5) == 0){
    //printf("\n\nSTAND\n\n");
    return stand_action;
  }else if(strncmp(rbuf,"hit",3) == 0){
    //printf("\n\nHIT\n\n");
    return hit_action;
  }else if(strncmp(rbuf,"end_game",8) == 0){
    //printf("\n\nSTAND\n\n");
    return end_game;
  }else return unknown;
  
}

void send_pseudo(int socket_fd,char* pseudo){
  int plen = strlen(pseudo);
  int n = plen+3;
  char msg[n];
  if(plen < 10){
    sprintf(msg,"0%d:%s",plen,pseudo);
  }else{
    sprintf(msg,"%d:%s",plen,pseudo);
  }
  if(send(socket_fd,msg,strlen(msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
  printf("finish sending\n");
  
}

void send_other_pseudo(int socket_fd, char* pseudo){
  printf("The pseudo %s is already taken. Please enter a new one : ",pseudo);
  fgets(pseudo,19,stdin);//one byte for '\0' terminator
  if ((strlen(pseudo)>0) && (pseudo[strlen (pseudo) - 1] == '\n'))
    pseudo[strlen (pseudo) - 1] = '\0';
  printf("you entered : %s\n",pseudo);
  while(strcmp(pseudo,"") == 0){
    printf("empty pseudo not accepted. Please enter again:");
    fgets(pseudo,19,stdin);
    if ((strlen(pseudo)>0) && (pseudo[strlen (pseudo) - 1] == '\n'))
      pseudo[strlen (pseudo) - 1] = '\0';
    printf("you entered : %s\n",pseudo);
  }
  int plen = strlen(pseudo);
  int n = plen+3;
  char msg[n];
  if(plen < 10){
    sprintf(msg,"0%d:%s",plen,pseudo);
  }else{
    sprintf(msg,"%d:%s",plen,pseudo);
  }
  
  if(send(socket_fd,msg,strlen(msg),0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
  printf("finish sending\n");
}

void send_keep_connection(int socket_fd){
  if(send(socket_fd,"3:yes",5,0) == -1){
    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
    exit(1);
  }
}



void print_game(game_instance* gi){
  printf("you are %s \n",gi->my_pseudo);
  for(int i = 0; i<gi->number_of_players; i++){
    char string[300];
    memset(string,0,300);
    char action[10];
    memset(action,0,10);
    if(gi->players_actions[i] == NO_ACTION){
      strncpy(action,"no_action",9);
      action[9]='\0';
    }else if(gi->players_actions[i] == HIT){
      strncpy(action,"hit",3);
      action[3] = '\0';
    }else if(gi->players_actions[i] == STAND){
      strncpy(action,"stand",5);
      action[5]='\0';
    }

    char cards[100];
    memset(cards,0,100);
    if(gi->players_cards[i][0] == NULL){
      sprintf(cards,"empty_hand");
      cards[10] ='\0';
    }else{
      char* c = show_card(gi->players_cards[i][0]);
      char cd[10];
      memset(cd,0,10);
      sprintf(cd,"%s, ",c);
      strncpy(cards,cd,10);
      for(int j = 1; j < 10; j++){
	if(gi->players_cards[i][j] != NULL){
	  c = show_card(gi->players_cards[i][j]);
	  memset(cd,0,10);
	  sprintf(cd,"%s, ",c);
	  strncat(cards,cd,10);
	}
      }
    }
    char* connexion = NULL;
    if(gi->players_connected[i] == 1){
      connexion = "connected";
    }else{
      connexion = "disconnected";
    }
    char* playing = NULL;
    if(gi->is_playing[i] == 1){
      playing = "playing";
    }else{
      playing = "waiting";
    }
    
    sprintf(string,"%d pseudo: %s ; action: %s ; cards: { %s } ; money: %d ; bet: %d ; sum: %d ; state: %s -- %s\n",i,gi->players_pseudos[i],action,cards,gi->players_money[i],gi->players_bets[i], gi->cards_sum[i], playing, connexion);

    printf("%s\n",string);
  }
}

void add_card_to_hand(game_instance* gi, card_t* card, char* pseudo){
  //printf("ADDING CARD TO PLAYER: %s\n",pseudo);
  int index = -1;
  for(int i = 0; i < gi->number_of_players; i++){
    if(strcmp(gi->players_pseudos[i], pseudo) == 0){
      index = i;
      break;
    }
  }
  //printf("\n\nINDEX = %d\n\n", index);
  if(index != -1){
    for(int j = 0; j < 20; j++){
      if(gi->players_cards[index][j] == NULL){
	gi->players_cards[index][j] = card;
	//gi->cards_sum[index] += card->value;
	break;
      }
    }
  }else{
    printf("this player does not exists in the game\n");
  }
}
