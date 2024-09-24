#define SET_BIT(p,n)      ((p) |= (1 << (n)))
#define CLR_BIT(p,n)      ((p) &= (~(1) << (n)))
#define IS_BIT_SET(val, bit_no) (((val) >> (bit_no)) & 1)

#define IMPLIES(x, y) (!(x) || (y))

#define COMPARE(x, y) (((x) > (y)) - ((x) < (y)))
#define SIGN(x) COMPARE(x, 0)

#define SWAP(x, y, T) do { T tmp = (x); (x) = (y); (y) = tmp; } while(0)
#define SORT2(a, b, T) do { if ((a) > (b)) SWAP((a), (b), T); } while (0)

#define BZERO(x, y)		memset(x, 0, y)


#if defined NDEBUG
#define TRACE( format, ... )
#else
#define TRACE( format, ... )   printf( "%s::%s(%d)" format, __FILE__, __FUNCTION__,  __LINE__, __VA_ARGS__ )
#endif

#define LENGTH(array) (sizeof(array) / sizeof (array[0]))

#define QUOTE(name) #name
#define STR(name) QUOTE(name)

#define Loop(i,x) for(i=0; i<x; i++)

/// Determine whether the given signed or unsigned integer is odd.
#define IS_ODD( num )   ((num) & 1)

/// Determine whether the given signed or unsigned integer is even.
#define IS_EVEN( num )  (!IS_ODD( (num) ))

/**
Determine whether the given number is between the other two numbers
(both inclusive).
*/
#define IS_BETWEEN( numToTest, numLow, numHigh )
( ( unsigned char )( ( numToTest ) >= ( numLow ) && ( numToTest ) <= ( numHigh ) ) )

/* some macros to make C more human readable, and confuse c programmers */
/* from: https://aartaka.me/c-not-c */
#include <iso646.h>
#define eq ==
#define bitnot ~
#define bitxor ^
 
#define is ==
#define isnt !=

#if defined(4) || defined(__GNUG__)
#define var __auto_type
#define let __auto_type
#define local __auto_type
#elif __STDC_VERSION__ > 201710L || defined(__cplusplus)
#define var auto
#define let auto
#define local auto
#endif

