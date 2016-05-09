#ifndef SIMPLESHELL_H

#define SIMPLESHELL_H

#include <string> 
#include <vector>
#include <unordered_map>
#include "PipePool.h"
#define READFD 0
#define WRITEFD 1

using namespace std ; 

class SimpleShell
{
	public:
		
		void ShellInitializer() ;
				
		void Shell() ;

		string ReadLine(string Prompt) ;
		
		void Executor(string Command) ;
		
		vector<string> Parser(string Command) ;
		
		void ExecuteSingleCmd(string singleCmd) ;
		
		void ForkChild(string singleCmd, int pipeFd[2], Pipe readPipe) ;
		
		char** ExecFormater(string singleCmd) ;
		
		bool BuiltInCmd(string Command) ;
		
		
	private:
		PipePool myPipe ;		
		sigset_t signalSet ;
} ;





#endif //* SIMPLESHELL_H *//


