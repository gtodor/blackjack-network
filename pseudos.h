#ifndef pseudos_h
#define pseudos_h

/*
 *list structure that contains a pseudo and the pointer to the next element 
 */
struct node{
  char pseudo[20];
  struct node* next;
};
typedef struct node* list;

/*
 *the hashtable contains an array of this structure entry
 */
struct entry{
  list pseudos_list; //list of pseudos at this particular hash value
  //unsigned long hash; // hash of the array case in the pseudo_db
  //int depth; //length of the list
};
typedef struct entry entry;

struct pseudo_db{
  int size; // the size of the hashtable
  entry* htable; // hashtable containing pseudos
  int elements; // the number of actual elements in the hashtable
};
typedef struct pseudo_db pseudo_db;

/**
 *initialize a pseudo_db structure (the hashtable)
 *param[in] size = the initial size of the hashtable;
 *return a pointer to the new pseudo_db structure
 */
pseudo_db* init_pseudo_db(int size);

/**
 *insert a pseudo in the hashtable
 *this pseudo is not already in the hashtable
 */
void bind_pseudo(pseudo_db** pb, char* pseudo);

/**
 *removes the pseudo from the hashtable
 */
void unbind_pseudo(pseudo_db** pb, char* pseudo);

/**
 *hash function that operates on strings
 *return a unsigned long integer
 */
unsigned long hash(char* string);

/**
 *checks if the pseudo is already in the hashtable
 *return 1 if it is in the database else it returns 0 
 */
int check_existance(pseudo_db* pb, char* pseudo);

/**
 *increases the size of the hashtable and rehash every element in the new database
 */
void resize_htable(pseudo_db** pb);

/**
 *print the content of the hashtable
 */
void print_pseudos(pseudo_db* pb);

#endif //pseudos_h
