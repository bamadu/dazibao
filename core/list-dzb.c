#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/file.h>
#include <unistd.h>
#include <math.h>
#include "helpers.h"

int dzb_entete (int fd) ;
void dbz_file(char *name, char *buf, int length) ;
int tlv_parcours_v1(int fd, int length, int esp) ;

int main(int argc, char *argv[]) {
	
  int fd ;
  struct stat st;

  // vérifier les arguments...
  if(argc != 2) 
    err_quit("usage: %s <binary file>", argv[0]) ;
  // Ouverture du Dazibao...
   if((fd = open(argv[1],O_RDONLY)) < 0)
     err_sys("open") ;
  // verrou partagé ...
   if (flock(fd, LOCK_SH))
     err_sys ("flock") ;
  // récupérer la taille du dazibao
  if ((fstat(fd, &st)) == -1)
    err_sys ("stat") ;
  // cas de fichier vide
  if (!st.st_size)
    err_quit("Fichier vide !!!") ;
  // Parcours du dazibao ...
  if ((dzb_entete(fd)) < 0)
    err_quit("dazibao non réconnu !!!") ;
  if((lseek(fd, 4, SEEK_SET)) == -1)
    err_sys("lseek") ;
  // Parcours du dazibao de l'octect 4 à size
  printf("dazibao\n{\n") ;
  tlv_parcours_v1 (fd, st.st_size-4, 1) ;
  printf(" }\nfin dazibao\n") ;
  // Fermeture du fichier dazibao et libérer flock
  if (flock(fd, LOCK_UN))
    err_sys ("flock") ;
  if (close(fd) == -1 ) 
    err_sys("close") ;
  // sortie ...
  exit (0) ; 
}

/* lecture de l'entête du dazibao
 * retourne 0 en cas de succes
 * ret<0 Sinon ... 
 * ne lis pas MBZ ?! Pourquoi pas ?! 
*/
int dzb_entete (int fd) {
  
  int8_t m ;

  if ( (read(fd, &m, 1)) == -1 ) // Magic 
    err_sys ("read") ;
  if ( m != 53 ) 
    return -1 ;
  if ( (read(fd, &m, 1)) == -1 ) // Version 
    err_sys ("read") ;
  if (m) return -2 ;  
  return 0 ;
}

// Fonction recursive de parcours du dazibao...
int tlv_parcours_v1(int fd, int length, int esp) {
  
  // variables locales
  int i, n = -1, tlv_length, nb_char_lu = 0 ;
  unsigned char clength[4], type ; 

  while (nb_char_lu < length) {
  
    if ( (n = read(fd, &type, 1)) == -1 ) // Version 
      err_sys ("read") ;

    if(!n) 
      return 0 ;

    if (type) { 
      for (i=0; i<3; i++)
	if ( (n = read(fd, &clength[i], 1)) == -1 ) 
	  err_sys ("read") ;
      tlv_length = clength[0]*256*256 + clength[1]*256 + clength[2] ;
    }
    
    //espacement...
    for(i=0; i<esp; i++)
      printf("  ") ;

    switch (type) {
      //unsigned int date ;
      
    case 0:  printf("[type: Pad1]\n") ;
             break ;
      
    case 1: //printf("[type: PadN] [taille: %i]",tlv_length) ;
            printf("[type: PadN]\n") ;
            if((lseek(fd, tlv_length, SEEK_CUR)) == -1)
	      err_sys("lseek") ;
	    nb_char_lu += tlv_length + 4;
	    break ;
    
    case 2: //printf("[type: Text] [taille: %i]",tlv_length) ;        
            printf("[type: text]\n") ;
            if((lseek(fd, tlv_length, SEEK_CUR)) == -1)
	      err_sys("lseek") ;
	    nb_char_lu += tlv_length + 4 ;
	    break ;

    case 3: //printf("[type: Image PNG] [taille: %i]",tlv_length) ;
            printf("[type: PNG]\n") ;
            if((lseek(fd, tlv_length, SEEK_CUR)) == -1)
	      err_sys("lseek") ;
	    nb_char_lu += tlv_length + 4 ;
	    break ;

    case 4: //printf("[type: Image JPEG] [taille: %i]",tlv_length) ;
            printf("[type: JPEG]\n") ;
            if((lseek(fd, tlv_length, SEEK_CUR)) == -1)
	      err_sys("lseek") ;
	    nb_char_lu += tlv_length + 4 ;
	    break ;

    case 5: //printf("[type: Compisite] [taille: %i]",tlv_length) ;
            printf("[type: Conpound]\n") ;
            tlv_parcours_v1(fd, tlv_length, esp+1) ;
	    nb_char_lu += tlv_length + 4 ;
	    break ;
  
    case 6: //printf("[type: Daté] [taille: %i] ",tlv_length) ;    
            printf("[type: Dated]\n") ;
	    for (i=0; i<4; i++) 
	      if ( (n = read(fd, &clength[i], 1)) == -1 ) 
		err_sys ("read") ;
	    /*date = clength[0]*256*256*256 + clength[1]*256*256 + 
	      clength[2]*256 + clength[3] ; */
	    //printf("[date: %i]",date) ;
	    n = tlv_parcours_v1(fd, tlv_length-4, esp+1) ;
	    nb_char_lu += tlv_length + 4 ;
	    break ;

    default: err_msg ("ERROR [type: Inconnu]") ;
             /*printf("[type: Inconnu] [taille: %i]",tlv_length) ; 
             if((lseek(fd, tlv_length, SEEK_CUR)) == -1)
	       err_sys("lseek") ;
	     nb_char_lu += tlv_length + 4 ;
	     break ;*/
    } //fin switch...
  }
  
  //printf(" }\n") ;
  // --- à definir ---
  return n ;
 }
