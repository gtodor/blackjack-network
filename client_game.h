#ifndef CLIENT_GAME_H
#define CLIENT_GAME_H

#include "card.h"

typedef enum{
  req_pseudo,
  req_other_pseudo,
  req_connected,
  pseudo_enabled,
  start_game,
  players_info,
  player_disconnected,
  first_card,
  second_card,
  req_bet,
  spread_bet,
  play_turn,
  update_stand,
  asked_card,
  hit_action,
  stand_action,
  end_game,
  unknown
} message;

typedef struct game_instance{
  int socket_fd;//socket descriptor to speak with the server
  char my_pseudo[20];
  int number_of_players;//max 10
  char players_pseudos[10][20];
  action players_actions[10];//this vector is updated at each tour
  card_t* players_cards[10][20];//max 10 players at a table and one player can hold a maximum of 20 cards
  int players_connected[10];//state of connexion of these players
  int is_playing[10];
  int players_bets[10];
  int players_money[10];
  int my_tour_number;
  int cards_sum[10];
}game_instance;

//this method initialize the game instance with some default values then
//fetches all the other values from the server
game_instance* init_game(int socket_fd, char* pseudo);

//defines the main loop of the game executed in the client
//all calls to the server are done through this loop
void main_loop(game_instance* gi);

//send a hit action to the server
void hit(game_instance* gi);

//send a stand action to the server
void stand(game_instance* gi);

//update the content of the data structure before printing
void update(game_instance* gi);

message get_message(int socket_fd,char* message,int size);

//sends a chosen pseudo over the network
void send_pseudo(int socket_fd, char* pseudo);

//resends a pseudo if the one sent before is not valid
void send_other_pseudo(int socket_fd,char* pseudo);

//send a message to server to keep the connection with the client
void send_keep_connection(int socket_fd);

//adds the card to the list of cards of player pseudo
void add_card_to_hand(game_instance* gi, card_t* card,char* pseudo);

//manage the graphical output for the game
void print_game(game_instance* gi);

#endif//CLIENT_GAME_H
