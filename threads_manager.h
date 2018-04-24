#ifndef threads_manager_h
#define threads_manager_h


#include "players.h"

/**structure that contains infos about the clients present at each table (socket,pseudo, etc )
 *it is an array indexed by the number/id of the "blackjack table"
 */
typedef struct threads_manager{
  int size; //total number of "blackjack tables"
  blackjack_table** tables; //an array containing all "blakjack tables" (games)
  int index; //used to keep track of the actual number of elements in tables array
  int no_players; // max number of players at a particular "blackjack table"
}threads_manager;


/**
 *initialize a thread_manager structure
 *param[in] size = initial size of the array
 *param[in] no_player = max number of players at a "blackjack table"
 *return a pointer to the new created thread_manager
 *table_no initialized at 1
 *index initialized at 0
 */
threads_manager* init_th_manager(int size, int no_players);
  
/**
 *increases the number of "blackjack tables" when necessary
 */
void increase_size(threads_manager* tm);

/**
 *adds a new "blackjack table" in tables array
 *if the current array is full it is increased by increase_size method before adding the new "blackjack table" 
 */
void add_blackjack_table(threads_manager* tm);

/*
 *removes the content of the selected player_table along with all the players
 *param table_no is the number of the "blackjack table" to delete
 *return 1 on success, else -1
 */
int remove_blackjack_table(threads_manager* tm,int table_no);

/**
 *add a new player(thread) to the thread manager players_table array
 *player already created, and his pseudo is already binded
 *if all players_table are full create a new players_table and add the client there  
 *return the blackjack table number for this player 
 */
int add_player(threads_manager* tm, player* p);

/**
 * removes a player from the players_table array
 * calls the remove_player_from_table method
 * is called by check_clients_connectivity when a client is disconnected
 * return -1 if there is no such player, else it returns 1 
 */
int remove_player(threads_manager* tm, int table_no, player* p, pseudo_db* pb);

/*
 *check if a client is connected
 *return 1 if this player is connected, returns 0 if disconected and -1 if check_connectivity fails
 */
int check_connectivity(player* p, int timeout);


/**
 *prints the content of threads_manager's tables array
 */
void print_blackjack_tables(threads_manager* tm);


void initialize_game(blackjack_table* table);

#endif // threads_manager
