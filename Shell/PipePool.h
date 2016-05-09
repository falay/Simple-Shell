#ifndef PIPEPOOL_H
#define PIPEPOOL_H


struct Pipe
{
	int readFd ;
	int writeFd ;
	Pipe():readFd(-1), writeFd(-1) {}
} ;


class PipePool
{
	public:
		Pipe GetPipeToWrite() ;
		bool GetPipeToRead(Pipe& readPipe) ;
		
	private:
		Pipe latestPipe ;
} ;


#endif //* PIPEPOOL_H *//