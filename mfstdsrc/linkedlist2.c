#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef struct node {
  struct sockaddr_in s;
  unsigned char pkt[ BUFFER_SIZE ];
  struct node *next;
} NODE;

struct node *init_list( void );
int add_node( struct node *lista, struct sockaddr_in s, unsigned char *p );
void print_nodes( struct node *llist );
void free_nodes(struct node *llist);

struct node *init_list( void )
{
  struct node *llist;
  llist = ( struct node * )malloc( sizeof( struct node ) );
  llist->next = NULL;
  return llist;
}

int add_node( struct node *lista, struct sockaddr_in s, unsigned char *p )
{
  // this while loop will prevent duplicate entries
  while( lista->next != NULL ) {
    if( memcmp( lista->next->pkt, p, BUFFER_SIZE ) == 0 ) {
      //printf("skipping duplicate\n\r");
      return 0;
    }
    lista = lista->next;
  }
  // if not duplicate add to new link list struct
  lista->next = ( struct node * )malloc( sizeof( struct node ) );
  memcpy( lista->next->pkt, p, BUFFER_SIZE );
  lista->next->s = s;
  lista->next->next = NULL;
  return 1;
}

void print_nodes( struct node *llist )
{
  unsigned int serial_no = 0;
  while( llist->next != NULL ) {
    serial_no = ( llist->pkt[9] << 16 ) | ( llist->pkt[10] << 8 ) | llist->pkt[11];
    printf( "%15s - %6u - %s\n\r", inet_ntoa( llist->s.sin_addr ), serial_no, &llist->pkt[24] );
    //hexdump(llist->next->pkt, 28);
    llist = llist->next;
  }
}

void free_nodes(struct node *llist)
{
  struct node* tmp;
  while (llist != NULL)
  {
    tmp = llist;
    llist = llist->next;
    free(tmp);
  }
}

