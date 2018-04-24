#ifndef CARD_H
#define CARD_H

typedef struct card_s{
  int hidden;//1 if hidden else 0
  char* symbol;
  char* color;
  int value;
}card_t;

typedef struct card_package_s{
  card_t cards[52];//all the cards in the packet
  int counter;//how many cards are still in the package
}card_package_t;

typedef enum{HIT, STAND,  NO_ACTION} action;

card_t init_card(char* symbol, char* color, int value, int hidden);

void reveal_card(card_t* card);

void hide_card(card_t* card);

card_package_t* init_card_package();

void shuffle_cards(card_package_t* cp);

card_t* get_card(card_package_t* cp);

void print_card_package(card_package_t* cp);

char* card_to_string(card_t* card);

char* show_card(card_t* card);

card_t string_to_card(char* string);
#endif//CARD_H
