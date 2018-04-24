#include "card.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


card_t init_card(char* symbol, char* color, int value, int hidden){
  card_t card;
  card.symbol = (char*) malloc(sizeof(char)*3);
  card.color = (char*) malloc(sizeof(char)*2);
  memset(card.symbol,0,3);
  memset(card.color,0,2);
  strncpy(card.symbol,symbol,2);
  card.symbol[3] = '\n';
  strncpy(card.color,color,1);
  card.color[2] = '\n';
  card.value = value;
  card.hidden = hidden;
  return card;
}

card_package_t* init_card_package(){
  card_package_t* package = (card_package_t*)malloc(sizeof(card_package_t));
  char* symbols[52]={"A","A","A","A","2","2","2","2","3","3","3","3","4","4","4","4","5","5","5","5","6","6","6","6","7","7","7","7","8","8","8","8","9","9","9","9","10","10","10","10","J","J","J","J","Q","Q","Q","Q","K","K","K","K"};
  char* colors[52]={"H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D","H","S","C","D"};
  int values[52] ={1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,8,8,8,8,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
  for(int i = 0; i<52; i++){
    package->cards[i] = init_card(symbols[i],colors[i],values[i],0);
    printf("sym: %s, color: %s\n",package->cards[i].symbol,package->cards[i].color);
  }
  package->counter = 52;//init counter with the number of cards
  return package;
}

void shuffle_cards(card_package_t* cp){
  srand(time(NULL));
  for(int i = 0; i < 51; i++){
    int j = rand()%(i+1);
    if(j != i){
      card_t tmp = cp->cards[i];
      cp->cards[i] = cp->cards[j];
      cp->cards[j] = tmp;
    }
  }
}

void reveal_card(card_t* card){
  card->hidden = 0;
}

void hide_card(card_t* card){
  card->hidden = 1;
}

card_t* get_card(card_package_t* cp){
  if(cp->counter > 0){
    card_t* c = &cp->cards[cp->counter - 1];
    cp->counter--;
    return c;
  }else{
    return NULL;
  }
}

void print_card_package(card_package_t* cp){
  for(int i = 0; i<cp->counter; i++){
    printf("%s %s | ",cp->cards[i].symbol,cp->cards[i].color);
  }
  printf("\n");
}

char* card_to_string(card_t* card){
  if(card == NULL) return "empty";
  char* res=NULL;
  res = (char*) malloc(sizeof(char)*30);
  memset(res,0,30);
  //printf("IN CARD TO STRING: before printf()\n");
  //printf("card.symbol = %s\n",card->symbol);
  //printf("card.color = %s\n",card->color);
  //printf("card->symbol=%s; card->color=%s; card->value=%d; card->hidden=%d\n",card->symbol, card->color, card->value, card->hidden);
  sprintf(res,"%s %s %d %d",card->symbol, card->color, card->value, card->hidden);
  //printf("IN CARD_TO_STRING: %s\n",res);
  return res;
}

char* show_card(card_t* card){
  if(card == NULL){
    return "empty";
  }
  if(card->hidden == 1){
    return "???";
  }else{
    char* res;
    res = (char*) malloc(sizeof(char)*6);
    memset(res,0,6);
    sprintf(res,"%s:%s",card->symbol,card->color);
    return res;
  }
}

card_t string_to_card(char* string){
  int length = strlen(string);
  int i=0;
  int k = 0;
  char symbol[3];
  char color[2];
  char digits_value[3];
  char digit_hidden[2];
  int hidden, value;
  memset(symbol,0,3);
  memset(digits_value,0,2);
  memset(color,0,2);
  memset(digit_hidden,0,2);
  while(string[i] != ' '){
    symbol[k] = string[i];
    k++;
    i++;
  }
  symbol[k] = '\0';
  i++;
  color[0] = string[i];
  color[1] = '\0';
  i = i+2;
  k=0;
  while(string[i] != ' '){
    digits_value[k] = string[i];
    i++;
    k++;
  }
  digits_value[k] = '\0';
  i++;
  digit_hidden[0] = string[i];
  digit_hidden[1] = '\0';
  hidden = atoi(digit_hidden);
  value = atoi(digits_value);
  //printf("Constructed card is : %s %s %d %d\n",symbol,color,value,hidden);
  card_t card = init_card(symbol,color,value,hidden);
  return card;
}

