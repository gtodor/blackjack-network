#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pseudos.h"


char** random_string_generator(int number_of_strings, int max_string_size){
  time_t t = time(0);
  srand(t);
  char** arr = (char**) malloc(number_of_strings * sizeof(char*));
  for(int i = 0; i<number_of_strings; i++){
    int size = rand() % max_string_size + 4;
    arr[i] = (char*)malloc(size * sizeof(char));
    for(int j = 0; j<size - 1; j++){
      arr[i][j] = 32 + rand() % 95;
    }
    arr[i][size-1] = '\0';
  }
  return arr;
}

void print_string_array(char** array, int size){
  for(int i = 0; i<size;i++){
    printf("%s \n",array[i]);
  }
  //printf("\n");
}


int main(){
  int size = 1000;
  pseudo_db* psdb = init_pseudo_db(size);
  char** array = random_string_generator(size,10);
  //print_string_array(array,size);
  printf("\n\n\n");

  //insert strings in the hashtable
  for(int i = 0; i< size; i++){
    bind_pseudo(&psdb,array[i]);
  }
  printf("\nHASHTABLE : \n");
  print_pseudos(psdb);

  //test that every string in array is indeed in the hashtable
  for(int i = 0; i< size; i++){
    printf("%s  is in the hashtable? : %d\n",array[i],check_existance(psdb,array[i]));
  }

  //generate another array of strings
  char** array2 = random_string_generator(size,20);

  //testing the elements of the new array for existance in the hashtable
  printf("\n\n\n");
  for(int i = 0; i< size; i++){
    printf("%s  is in the hashtable? : %d\n",array2[i],check_existance(psdb,array2[i]));
  }

  //testing unbind method for half of 
  for(int i = 0; i<size/2; i++){
    unbind_pseudo(&psdb,array[i]);
  }
  //unbind_pseudo(&psdb,array[0]);
  printf("\nHASHTABLE : \n");
  print_pseudos(psdb);

  //test existance of elements of first array (half of them should be erased) 
  for(int i = 0; i< size; i++){
    printf("%s  is in the hashtable? : %d\n",array[i],check_existance(psdb,array[i]));
  }

  //printf("%d\n",check_existance(psdb,"%dj)oI"));
  return 0;
}
