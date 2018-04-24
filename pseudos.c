#include "pseudos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pseudo_db* init_pseudo_db(int size){
  pseudo_db* pb = (pseudo_db*) malloc(sizeof(pseudo_db));
  pb->size = size;
  pb->htable = (entry*)malloc(size * sizeof(entry));
  for(int i = 0; i<size; i++){
    (pb->htable[i]).pseudos_list=NULL;
  }
  pb->elements = 0;
  return pb;
}

void bind_pseudo(pseudo_db** pb, char* pseudo){
  //printf("binding pseudo %s\n",pseudo);
  if((*pb)->elements == (*pb)->size){
    resize_htable(pb);
  }
  unsigned long h = hash(pseudo) % (*pb)->size;
  list l = (list)malloc(sizeof(struct node));
  memset(l->pseudo,0,20);
  strncpy(l->pseudo,pseudo,strlen(pseudo));
  l->next = ((*pb)->htable[h]).pseudos_list;
  ((*pb)->htable[h]).pseudos_list = l;
  (*pb)->elements++;
}

void unbind_pseudo(pseudo_db** pb, char* pseudo){
  unsigned long h = hash(pseudo) % (*pb)->size;
  list tmp = ((*pb)->htable[h]).pseudos_list;
  if(strcmp(tmp->pseudo,pseudo) == 0){
    list tmp2 = tmp->next;
    ((*pb)->htable[h]).pseudos_list = tmp2;
    tmp->next = NULL;
    free(tmp);
  }else{
    while(tmp->next != NULL){
      if(strcmp(tmp->next->pseudo,pseudo) == 0){
	list tmp2 = tmp->next;
	tmp->next = tmp->next->next;
	tmp2->next = NULL;
	free(tmp2);
	break;
      }else{
	tmp = tmp->next;
      }
    }
  }
}

//djb2 hash algorithm by dan bernstein(www.cse.yorku.ca/~oz/hash.html)
unsigned long hash(char* string){
  unsigned long hash = 5381;
  int c;
  
  while (c = *string++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  
  return hash;
}

void resize_htable(pseudo_db** pb){
  int old_size = (*pb)->size;
  int new_size = old_size * 2;
  pseudo_db* npb = init_pseudo_db(new_size);
  for(int i = 0; i< old_size; i++){
    list tmp = ((*pb)->htable[i]).pseudos_list;
    while(tmp != NULL){
      bind_pseudo(&npb,tmp->pseudo);
      tmp = tmp->next;
    }
  }
  pseudo_db* tmp = *pb;
  *pb = npb;
  free(tmp);
}

int check_existance(pseudo_db* pb, char* pseudo){
  //printf("checking existance for %s\n",pseudo);
  unsigned long h = hash(pseudo) % pb->size;
  list tmp = (pb->htable[h]).pseudos_list;
  int found = 0;
  while(tmp != NULL){
    if(strcmp(tmp->pseudo,pseudo) == 0){
      found = 1;
      break;
    }else{
      tmp = tmp->next;
    }
  }
  return found;
}

void print_pseudos(pseudo_db* pb){
  for(int i=0; i<pb->size; i++){
    list tmp = (pb->htable[i]).pseudos_list;
    while(tmp != NULL){
      printf("{%u :  %s }",i, tmp->pseudo);
      tmp = tmp->next;
    }
    printf("\n");
  }
}
  
  
