#include <iostream>
#include <unistd.h>
#include "PipePool.h"
using std::cerr ;

Pipe PipePool::GetPipeToWrite()
{
	/*** Create a new pipe ***/
	
	int pipeFd[2] ;
	
	if( pipe(pipeFd) < 0 )
	{
		cerr << "pipe error\n" ;
		exit(0) ;
	}	
	
	latestPipe.readFd = pipeFd[0] ;
	latestPipe.writeFd = pipeFd[1] ;
	
	return latestPipe ;
}


bool PipePool::GetPipeToRead(Pipe& readPipe)
{
	if( latestPipe.readFd == -1 )
		return false ;
	else
	{
		readPipe = latestPipe ;
		latestPipe.readFd = -1 ;
		return true ;
	}		
}