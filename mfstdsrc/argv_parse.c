
#include <stdio.h>
#include <unistd.h>


/* basic C command line parameter parsing code */
int main( int argc, char *argv[] )
{

  int oc;

  /* options followed by ':'  will need to have arguments*/
  while( ( oc = getopt( argc, argv, "ABCDe:f:g:" ) ) != -1 ) {
    switch( oc ) {
    case 'A':
      /* set option A */
      printf( "set option A\n\r" );
      break;
    case 'B':
      /* set option B */
      printf( "set option B\n\r" );
      break;
    case 'C':
      /* set option C */
      printf( "set option C\n\r" );
      break;
    case 'D':
      /* set option D */
      printf( "set option D\n\r" );
      break;
    case 'e':
      /* option e argument */
      printf( "arg e option = %s\n\r", optarg );
      break;
    case 'f':
      /* option f argument */
      printf( "arg f option = %s\n\r", optarg );
      break;
    case 'g':
      /* option g argument */
      printf( "arg g option = %s\n\r", optarg );
      break;
    default:
      printf( "invalid command line switch\n\r" );
      return -1;
      break;
    }
  }/*  end of while statement  */


  return 0;
}
