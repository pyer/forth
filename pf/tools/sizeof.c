#include <stdio.h>
 
int main(void)
{
  long int x;
  x=sizeof(char);
  printf("Size of char    is %ld bytes, ie %ld bits\n", x, 8*x );
  x=sizeof(short);
  printf("Size of short   is %ld bytes, ie %ld bits\n", x, 8*x );
  x=sizeof(int);
  printf("Size of integer is %ld bytes, ie %ld bits\n", x, 8*x );
  x=sizeof(long);
  printf("Size of long    is %ld bytes, ie %ld bits\n", x, 8*x );
  x=sizeof(float);
  printf("Size of float   is %ld bytes, ie %ld bits\n", x, 8*x );
  x=sizeof(double);
  printf("Size of double  is %ld bytes, ie %ld bits\n", x, 8*x );
  x=sizeof(void*);
  printf("Size of void*   is %ld bytes, ie %ld bits\n", x, 8*x );
 
  return 0;
}
