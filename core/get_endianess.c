#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  long i = 1 ;
  const char *p = (const char *) &i ;
  if (p[0] == 1) printf ("LITTLE ENDIAN\n") ;
  else printf ("BIG ENDIAN\n") ;

  exit (EXIT_SUCCESS) ;
}


  
