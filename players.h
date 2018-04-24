#ifndef players_h
#define players_h

#include "pseudos.h"
#include "card.h"

typedef enum{
  CONNEXION,
  CARD1,
  CARD2,
  ACTION,
  BET,
  MONEY,
  DEALER_TOUR,
  RESULTS,
  NO_INFO,
}info_t;

/*
 *structure defining a player
 * includes a socket descriptor and the pseudo plus future attributes
 */
typedef struct player{
  int socket_fd; //socket descriptor associated with this client
  char pseudo[20]; // unique identifiant for the player along socket descriptor
  int connected; // boolean: 1 - player is connected ; 0 - not connected
  //card_t* card1;
  //card_t* card2;
  card_t* cards[20];
  int card_ind;
  int money;
  int bet;
  action act;
  int card_sum;
  int my_place;
  //int countdown;//init with the number of threads that will read this variable
}player;

/*
 *structure containing all infos about a particular instance of the game
 * it includes all players at a "blackjack table" plus additional infos
 */
typedef struct blackjack_table{
  int number_of_players; //current number of players at this table
  int size; // size of the table (necessary for the loops)
  player** players; // array of players
  int full; //boolean: 1 - table is full ; 0 - table is not yet full
  int count_views;// how many threads have got the information; it is reset by the game thread once count_views==size
  info_t info_changed;
  int tour;
  card_package_t* card_package;
  card_t* dealer_cards[20]; 
}blackjack_table;

/*
 *initialize a player structure
 *if pseudo is not good(already existent in the database) ask the client for another one
 *return a pointer to the newly created player
 */
player* init_player(int socket_fd, pseudo_db* pb);

/*
 *this method will be called inside thread_manager add_player_table
 * argument size is passed by thread manager's max no of players at a table
 *return a pointer to a new created player_table
 */
blackjack_table* init_blackjack_table(int size);

/*
 *add a completly created player to a player_table
 *this method is called by thread_manager add_player method when adding a player as a mean to encapsulate structure logic
 * return -1 if pt is already full and a player cannont be added or 1 if it succeeds
 */
int add_player_to_table(blackjack_table* pt, player* p);

/*
 *removes the selected player from the player_table becouse either client disconnected or game ended and the clent had chosen to close the app
 *the pseudo must be unbinded for future use
 *returns -1 if an error occured like there is no such player at that table
 *returns 1 on success
 */
int remove_player_from_table(blackjack_table* pt, player* p, pseudo_db* pb);

/*
 *method to retrieve from the client another pseudo
 *uses socket_fd of the client to pass the messages
 *only used in the init_player method
 */
char* ask_for_pseudo(int socket_fd, char* pseudo); 

/*
 *tells the client that the server succeded to bind a pseudo
 */
void send_pseudo_confirmation(int socket_fd);

/*
 *sends the client the initial values for each player at the table
 */
void send_players_info(blackjack_table* table,int socket_fd);

/*
 *tells the client that the game is ready to begin
 */
void send_start_game(int socket_fd);

/*
 *tells the other clients from the same table that this client has disconnected
 */
void send_disconnected_to_all(blackjack_table* table, player* p);

/*
 *sends first card to every client and informs the others about it
 */
void send_first_card(blackjack_table* table,card_package_t* pack);

/*
 *sends second card to every client and informs others about it
 */
void send_second_card(blackjack_table* table, card_package_t* pack);

/*
 *asks the clients in order what are their bets
 */
void ask_for_bets(blackjack_table* table);

#endif //players_h
