#include <stdio.h>
#include <stdlib.h>

struct NODE {
  int number;
  struct NODE *next;
};

int  find_item( struct NODE *llist, int num );
void add_item( struct NODE *llist, int num );
void print_items( struct NODE *llist );
void del_item( struct NODE *llist, int num );
struct NODE *init_items( void );

unsigned int item_count = 0;

struct NODE *init_items( void )
{
  struct NODE *llist;
  item_count = 0;
  llist = ( struct NODE * )malloc( sizeof( struct NODE ) );
  llist->number = 0;
  llist->next = NULL;
  return llist;
}

int main( void )
{
  int num = 0;
  int input = 1;
  int retval = 0;
  struct NODE *llist;

  llist = init_items();

  while( input != 0 ) {
    printf( "\n-- Menu Selection --\n" );
    printf( "0) Quit\n" );
    printf( "1) Insert\n" );
    printf( "2) Delete\n" );
    printf( "3) Search\n" );
    printf( "4) Display\n" );
    scanf( "%d", &input );

    switch( input ) {
    case 0:
    default:
      printf( "Goodbye ...\n" );
      input = 0;
      break;
    case 1:
      printf( "Your choice: `Insertion'\n" );
      printf( "Enter the value which should be inserted: " );
      scanf( "%d", &num );
      add_item( llist, num );
      break;
    case 2:
      printf( "Your choice: `Deletion'\n" );
      printf( "Enter the value which should be deleted: " );
      scanf( "%d", &num );
      del_item( llist, num );
      break;
    case 3:
      printf( "Your choice: `Search'\n" );
      printf( "Enter the value you want to find: " );
      scanf( "%d", &num );
      if( ( retval = find_item( llist, num ) ) == -1 ) {
        printf( "Value `%d' not found\n", num );
      } else {
        printf( "Value `%d' located at position `%d'\n", num, retval );
      }
      break;
    case 4:
      printf( "You choice: `Display'\n" );
      print_items( llist );
      break;
    } /* switch */
  } /* while */

  free( llist );
  return( 0 );
}

void print_items( struct NODE *llist )
{
  while( llist->next != NULL ) {
    printf( "%d ", llist->number );
    llist = llist->next;
  }

  printf( "%d", llist->number );
}

void add_item( struct NODE *llist, int num )
{
  while( llist->next != NULL ) {
    llist = llist->next;
  }

  llist->next = ( struct NODE * )malloc( sizeof( struct NODE ) );
  llist->next->number = num;
  llist->next->next = NULL;
}

void del_item( struct NODE *llist, int num )
{
  struct NODE *temp;
  temp = ( struct NODE * )malloc( sizeof( struct NODE ) );

  if( llist->number == num ) {
    /* remove the node */
    temp = llist->next;
    free( llist );
    llist = temp;
  } else {
    while( llist->next->number != num ) {
      llist = llist->next;
    }

    temp = llist->next->next;
    free( llist->next );
    llist->next = temp;
  }
}

int find_item( struct NODE *llist, int num )
{
  int retval = -1;
  int i = 1;

  while( llist->next != NULL ) {
    if( llist->next->number == num ) {
      return i;
    } else {
      i++;
    }

    llist = llist->next;
  }

  return retval;
}

