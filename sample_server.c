#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "threads_manager.h"

#define PORT "5001"
#define BACKLOG 20
#define MAXDATASIZE 100

pthread_mutex_t mutex;
/* threads_manager tm; */
/* pseudo_db pb; */

// structure used to pass as argument in thread function
struct data_s{
  int* socket_fd;
  threads_manager* tm;
  pseudo_db* pb;
};
typedef struct data_s data;


void* run_game(void* arg){
  printf("INSIDE DEALER THREAD\n\n");
  blackjack_table* table = (blackjack_table*)arg;
  //card_package_t* pack = init_card_package();
  //printf("card pack created\n");
  //printf("test: card[2].sym: %s, card[2].color: %s\n",pack->cards[2].symbol, pack->cards[2].color);
  //shuffle_cards(pack);
  //print_card_package(pack);
  //printf("test shuffle: card[2].sym: %s, card[2].color: %s\n",pack->cards[2].symbol, pack->cards[2].color);
  table->info_changed = CARD1;
  printf("\n\nSENDING FIRST CARDS\n\n");
  printf("number of views(should be 0):%d\n",table->count_views);
  send_first_card(table,table->card_package);
  printf("END send first card\n");

  table->info_changed = CARD2;
  send_second_card(table,table->card_package);
  printf("\n\nFINISH SENDING SECOND CARD\n\n");
  table->info_changed = BET;
  while(table->tour != table->number_of_players){
      
  }
  table->info_changed = ACTION;
  table->tour = 0;
  while(table->tour != table->number_of_players){

  }
  table->info_changed = DEALER_TOUR;
  char* msg = "17:play_turn(dealer)";
  for(int i = 0; i<table->number_of_players; i++){
    if(send(table->players[i]->socket_fd, msg, strlen(msg), 0) == -1){
      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
      return;
    }
  }
  //dealer does a series of hit
  printf("\n\n\t DEALER'S TIME\n\n");
  int dealer_sum = 0;
  int k = 0;
  while(table->dealer_cards[k] != NULL){
    dealer_sum += table->dealer_cards[k]->value;
    k++;
  }
  printf("dealer sum = %d\n",dealer_sum);
  while(dealer_sum <= 15){
    card_t* c = get_card(table->card_package);
    for(int i = 0; i<20; i++){
      if(table->dealer_cards[i] == NULL){
	table->dealer_cards[i] = c;
	break;
      }
    }
    
    char* str = card_to_string(c);
    char msg[50];
    memset(msg,0,50);
    int length = 13+strlen(str)+strlen("dealer");
    sprintf(msg,"%d:asked_card=%s(dealer)",length,str);
    
    for(int i = 0; i<table->number_of_players; i++){
      printf("SENDING DEALER CARD CHOSEN\n");
      if(send(table->players[i]->socket_fd, msg, strlen(msg), 0) == -1){
	fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	return;
      }
    }
    k = 0;
    dealer_sum = 0;
    while(table->dealer_cards[k] != NULL){
      dealer_sum += table->dealer_cards[k]->value;
      k++;
    }
    printf("dealer sum = %d\n",dealer_sum);
    
  }
  table->tour = 0;
  printf("\n\n\t PRINTING END GAME\n\n");
  //ending game
  char* end_msg = "08:end_game";
  for(int i = 0; i<table->number_of_players; i++){
    if(send(table->players[i]->socket_fd, end_msg, strlen(end_msg), 0) == -1){
      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
      return;
    }
  }
  
  printf("TEMPORARY AT THE END\n");
  pthread_exit(NULL);
}


void* manage_games(void* arg){
  pthread_t game_thread;
  pthread_attr_t att;
  pthread_attr_init(&att);
  threads_manager* tm = (threads_manager*)arg;
  printf("thread_manager: %d \n",tm->size);
  int i=0;
  int rc;
  while(1){
    if(tm->tables[i] != NULL){
      //create new thread with the instance of the game
      printf("normal print\n");

      //wait for the table to fill up before creating a dealer thread
      while(!tm->tables[i]->full){
	//waiting
      }
      printf("table : %d %d\n",tm->tables[i]->size, tm->tables[i]->number_of_players);
      rc = pthread_create(&game_thread,&att, run_game ,tm->tables[i]);
      if (rc){
	printf("ERROR; return code from pthread_create() is %d\n", rc);
	exit(-1);
      }
      printf("game instance created\n");
      i++;
    }
  }
}


// This function is executed in every thread and should contain the logic of the player
void* run_thread(void* args){

  //extract useful data from the args argument 
  data* d = (data*)args;
  int socket_fd = *(d->socket_fd);
  threads_manager* tm =  d->tm;
  pseudo_db* pb = d->pb;

  if(check_existance(pb,"dealer")==0){
    bind_pseudo(&pb,"dealer");
  }
  
  //create a new player containig the socket descriptor and his pseudo
  player* p = init_player(socket_fd,pb);
  pthread_mutex_lock(&mutex);
  //add player to a blackjack table
  int table_no = add_player(tm,p);
  pthread_mutex_unlock(&mutex);
  print_pseudos(pb);

  //int place;
  //find my place at the table
  for(int i = 0; i < tm->tables[table_no]->size; i++){
    if(strcmp(tm->tables[table_no]->players[i]->pseudo,p->pseudo)==0){
      //place = i;
      tm->tables[table_no]->players[i]->my_place = i;//????
      break;
    }
  }
  
  //print_blackjack_tables(tm);
  int is_running = 0;
  int reg = 0;
  int reg1 = 0;
  //card_package_t* cp = init_card_package();
  //shuffle_cards(cp);

  
  while(p!=NULL){
    //check if full is 1 to tell client game started
    //printf(".....inside while loop\n");
    if(tm->tables[table_no]->full == 1 && is_running == 0 ){
      //printf("sending player infos\n");
      send_players_info(tm->tables[table_no],socket_fd);
      
      //printf("sending start game signal\n");
      send_start_game(socket_fd);
      is_running = 1;
    }

    
    if(is_running == 1){
      switch(tm->tables[table_no]->info_changed){
      case CARD1: printf("CARD1 STATE\n");break;
      case CARD2: printf("CARD2 STATE\n");break;
      case BET: printf("BET STATE\n");break;
      case ACTION: printf("ACTION STATE\n");break;
      case NO_INFO: printf("NO_INFO_STATE\n");break;
      default:break;
      }
      if(reg == 0 && tm->tables[table_no]->info_changed == CARD1){
	tm->tables[table_no]->count_views++;
	reg=1;
      }
      if(reg1==0 && tm->tables[table_no]->info_changed == CARD2){
	tm->tables[table_no]->count_views++;
	reg1 = 1;
      }










      
      if(tm->tables[table_no]->info_changed == BET && tm->tables[table_no]->tour == p->my_place ){

	char msg[40];
	memset(msg,0, 40);
	int length = 9 + strlen(p->pseudo);
	sprintf(msg,"%d:req_bet(%s)",length,p->pseudo);
	printf("asking player %s for his bet\n",p->pseudo);
	for(int i = 0; i < tm->tables[table_no]->number_of_players; i++){
	  if(send(tm->tables[table_no]->players[i]->socket_fd, msg, strlen(msg), 0) == -1){
	    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	    return;
	  }
	}

	int numbytes; 
	char readbuf[20];
	memset(readbuf,0,20);
	if((numbytes = recv(p->socket_fd,readbuf,sizeof(readbuf),0)) < 0){
	  fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
	  exit(1);
	}
	
	printf("numbytes =  %d\n", numbytes);
	readbuf[numbytes] = '\0';
	printf("received msg: %s\n",readbuf);

	//turn string to int
	int i = 9;
	int k = 0;
	char bet[10];
	memset(bet,0,10);
	while(readbuf[i]!='\0'){
	  bet[k] = readbuf[i];
	  k++;
	  i++;
	}
	bet[k] = '\0';

	int bet_val = atoi(bet);
	printf("the bet is %d\n",bet_val);
	p->bet = bet_val;
	p->money -= bet_val;

	
	
	memset(msg,0,30);
	length = 13 + strlen(p->pseudo)+ strlen(bet);
	sprintf(msg,"%d:spread_bet(%s;%d)",length,p->pseudo,bet_val);
	printf("PREPARING TO SEND BET to all clients\n");
	for(int i = 0; i< tm->tables[table_no]->number_of_players; i++){
	  if(p->socket_fd != tm->tables[table_no]->players[i]->socket_fd){
	    printf("giving chosen bet to %s\n",tm->tables[table_no]->players[i]->pseudo);
	    if(send(tm->tables[table_no]->players[i]->socket_fd,msg,strlen(msg),0) == -1){
	      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	      return;
	    }
	  }
	}

	tm->tables[table_no]->tour++;
      }













      
      if(tm->tables[table_no]->info_changed == ACTION && tm->tables[table_no]->tour == p->my_place){

	char play_msg[40];
	memset(play_msg,0,40);
	int len = 11 + strlen(p->pseudo);
	sprintf(play_msg,"%d:play_turn(%s)",len,p->pseudo);
	//send this message to everyone
	for(int i = 0; i < tm->tables[table_no]->number_of_players; i++){ 
	  if(send(tm->tables[table_no]->players[i]->socket_fd, play_msg,strlen(play_msg),0) == -1){
	    fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	    return;
	  }
	}
	//wait only for this player's response
	int numbytes; 
	char readbuf[20];
	memset(readbuf,0,20);
	if((numbytes = recv(p->socket_fd,readbuf,sizeof(readbuf),0)) < 0){
	  fprintf(stderr,"recv : error while reading from the client : %s\n",strerror(errno));
	  exit(1);
	}
	
	printf("numbytes =  %d\n", numbytes);
	readbuf[numbytes] = '\0';
	printf("received msg: %s\n",readbuf);

	char msg[20];
	memset(msg,0,20);
	sprintf(msg,"0%d:%s",strlen(readbuf),readbuf);

	/* for(int i = 0; i<tm->tables[table_no]->number_of_players; i++){ */
	/*   if(send(tm->tables[table_no]->players[i]->socket_fd,msg,strlen(msg),0) == -1){ */
	/*     fprintf(stderr,"send: error while sending : %s\n", strerror(errno)); */
	/*     return; */
	/*   } */
	/* } */

	if(strncmp(readbuf,"stand",5) == 0){
	  char msg[30];
	  memset(msg,0,30);
	  int len = 14 + strlen(p->pseudo);
	  sprintf(msg,"%d:update_stand(%s)",len,p->pseudo);
	  for(int i=0; i<tm->tables[table_no]->number_of_players; i++){
	    if((send(tm->tables[table_no]->players[i]->socket_fd,msg,strlen(msg),0)) == -1){
	      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	      return;
	    }
	    
	  }
	  printf("CHANGING TOUR\n");
	  tm->tables[table_no]->tour++;
	  
	}else if(strncmp(readbuf,"hit",3)==0){
	  printf("HIT CARD SUM = %d\n",p->card_sum);
	  card_t* c = get_card(tm->tables[table_no]->card_package);
	  if(strcmp(c->symbol,"A") == 0 && p->card_sum <= 10){
	    p->card_sum += 11;
	  }else{
	    p->card_sum += c->value;
	  }
	  
	  char* str = card_to_string(c);
	  char msg[50];
	  memset(msg,0,50);
	  int length = 13+strlen(str)+strlen(p->pseudo);
	  sprintf(msg,"%d:asked_card=%s(%s)",length,str,p->pseudo);
	  for(int j = 0; j< tm->tables[table_no]->number_of_players; j++){
	    printf("sending message %s to client %d\n",msg,j);
	    int rv;
	    if((rv=send(tm->tables[table_no]->players[j]->socket_fd,msg,strlen(msg),0)) == -1){
	      fprintf(stderr,"send: error while sending : %s\n", strerror(errno));
	      return;
	    }
	    printf("SENT %d data\n",rv);  
	  }
	  p->card_ind++;
	  p->cards[p->card_ind] = c;
	  if(p->card_sum >= 21) tm->tables[table_no]->tour++;
	}
      }
    }

    //compute cards sum
    if(p!=NULL){
      printf("cards : \n {");
      int i = 0;
      if(p == NULL) printf("NULL PLAYER\n");
      while(p->cards[i] != NULL){
	char* str = card_to_string(p->cards[i]);
	printf(" %s ;",str);
	i++;
      }
      printf(" }\n");
      
      int index = 0;
      p->card_sum = 0;
      while(p->cards[index] != NULL){
	printf("counting cards\n");
	if(strcmp(p->cards[index]->symbol,"A") == 0 && p->card_sum <=10){
	  p->card_sum += 11;
	}else{
	  p->card_sum += p->cards[index]->value;
	  printf("card_value = %d ,index = %d\n",p->cards[index]->value,index);
	}
	index++;
      }

      printf("CARDS SUM = %d FOR PLAYER %s\n",p->card_sum, p->pseudo);
    }
    
    pthread_mutex_lock(&mutex);
    if(check_connectivity(p,60) == 0){
      printf("after check_connectivity()\n");
      send_disconnected_to_all(tm->tables[table_no],p);
      printf("after send disconnected to all\n");
      for(int i = p->my_place; i < tm->tables[table_no]->number_of_players; i++){
	tm->tables[table_no]->players[i]->my_place--;
      }
      printf("before remove_player\n");
      remove_player(tm,table_no,p,pb);
      printf("after remove player\n");
      p=NULL;
    }
    pthread_mutex_unlock(&mutex);
    //print_blackjack_tables(tm);
    sleep(1);//wait 1 second between frames
  }
  printf("ouside of while\n");
  pthread_exit(NULL);
}


int main(int argc, char** argv){
  int sockfd, new_sockfd;
  struct addrinfo hints, *serverinfo, *p;
  struct sockaddr_storage client_addrs;
  socklen_t sin_size;
  int yes = 1;
  char ip[INET6_ADDRSTRLEN];
  int status;

  // card_package_t* card_package = init_card_package();
  //cp = *card_package;

  pthread_t thread[100], thread1;
  pthread_attr_t att;
  pthread_attr_init(&att);
  pthread_attr_setdetachstate(&att, PTHREAD_CREATE_DETACHED);
  pthread_mutex_init(&mutex, NULL);

  if(argc != 2){
    fprintf(stderr, "%s: usage: sample_server max_player_on_table\n", argv[0]);
    return 1;
  }

  threads_manager* tm = init_th_manager(100,atoi(argv[1]));
  pseudo_db* pb = init_pseudo_db(100);

  data thread_data;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if(( status = getaddrinfo(NULL,PORT,&hints,&serverinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n",gai_strerror(status));
    return 1;
  }

  for(p = serverinfo; p!=NULL; p = p->ai_next){
    if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      fprintf(stderr, "socket: unable to create socket : %s\n", strerror(errno));
      continue;
    }
    
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
      fprintf(stderr, "setsockopt error\n");
      exit(1);
    }
    
    if(bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(sockfd);
      fprintf(stderr, "bind: unable to bind to the address : %s\n",strerror(errno));
      continue;
    }
    break;
  }

  freeaddrinfo(serverinfo);

  if(p == NULL){
    fprintf(stderr, "%s: failed to bind \n", argv[0]);
    exit(1);
  }

  if(listen(sockfd,BACKLOG) == -1){
    fprintf(stderr, "listen: unable to listen : %s\n", strerror(errno));
    exit(1);
  }

  printf("listening\n");
  int t = 0;
  int rc;

  //before we accept we create a new thread to instantiate games
  //every time a new blackjack table is created this thread creates
  //a new thread that speaks through blackjack_table structure to
  //the client
  rc = pthread_create(&thread1,&att, manage_games ,tm);
  if (rc){
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }
  
 
  while(1){
    sin_size = sizeof(client_addrs);
    new_sockfd = accept(sockfd,(struct sockaddr*)&client_addrs, &sin_size);
    if(new_sockfd < 0){
      fprintf(stderr, "accept: error while accepting: %s\n",strerror(errno));
      continue;
    }

    

    printf("accepted\n");

    //initialize the data we pass into the function executed in each thread
    thread_data.socket_fd = &new_sockfd;
    thread_data.tm = tm;
    thread_data.pb = pb;

    rc = pthread_create(&thread[t],&att, run_thread , &thread_data);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }else{
      t++;
    }

  }
  printf("inside main server before ending\n");
  /* for(int i = 0; i < t; i++){ */
  /*   pthread_join(thread[i],NULL); */
  /* } */
  //close thread and mutex resources
  pthread_mutex_destroy(&mutex);
  pthread_exit(NULL);
  return 0;
}
  
