/*-------------------------------------------------------------------------------
IPC using Shared Memory & Signals -- Counter Version 00
Written By: 
     1- Dr. Mohamed Aboutabl
-------------------------------------------------------------------------------*/

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include  <signal.h>
#include <stdbool.h>

bool FOREVER = true ;

//------------------------------------------------------------
/* Wrapper for sigaction */

typedef void Sigfunc( int ) ;

Sigfunc * sigactionWrapper( int signo, Sigfunc *func )
// 'signo' specifies the signal and can be any valid signal 
//  except SIGKILL and SIGSTOP.

{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset( &act.sa_mask );
	act.sa_flags   = 0;

	if( sigaction( signo, &act, &oact ) < 0 )
		return( SIG_ERR );

	return( oact.sa_handler );
}

//------------------------------------------------------------
void sigHandler_A( int sig ) 
{
    fflush( stdout ) ;
    printf("\n\n### I (%d) received Signal #%3d.\n\n"
           , getpid() , sig );  
}

//------------------------------------------------------------
void sigHandler_B( int sig ) 
{
    fflush( stdout ) ;
    printf("\n\n### I (%d) received Signal #%3d.\n\n"
           , getpid() , sig );  
}

//------------------------------------------------------------
void sigHandler_CONT( int sig ) 
{
    fflush( stdout ) ;
    printf("\n\n### I (%d) have been asked to RESUME by Signal #%3d.\n\n"
           , getpid() , sig );  
}

//------------------------------------------------------------

int main()
{

    pid_t  mypid ;

    mypid = getpid() ;
    printf("\nHELLO! I AM THE COUNT PROCESS WITH ID= %d\n" , mypid );
      
    // Set up Signal Catching here
    
    unsigned i=0;
    while(  FOREVER )
        printf("%10X\r" , i++ ) ;

    printf("\nCOUNTER: Stopped Counting. The FOREVER flag must have become FALSE\n\n");
    printf("\nCOUNTER: Goodbye\n\n");
    
	return 0 ;

}
