#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "SimpleShell.h"
#include "RegularExp.h"

using namespace std;

vector<pid_t> Children ;

void SigHandler(int sig)
{
	for(pid_t child : Children)
		if( kill(child, SIGTERM) == -1 )
			cerr << "Cannot kill child " << child << endl ;	
}



void SimpleShell::ShellInitializer()
{
	setenv("PATH", "/bin:.", 2) ;
	
	if( signal(SIGINT, SigHandler) == SIG_ERR || 
		signal(SIGQUIT, SigHandler) == SIG_ERR   )
	{	
		cerr << "Fail to set signal handler\n" ;
		exit(0) ;
	}	
	
	else if( sigemptyset(&signalSet) == -1 ||
			 sigaddset(&signalSet, SIGINT) == -1 ||
			 sigaddset(&signalSet, SIGQUIT) == -1 ) 		
	{
		cerr << "Fail to set signal set\n" ;
		exit(0) ;
	}
}



void SimpleShell::Shell()
{
	cout << "/************* Welcome to my Simple Shell *************/\n\n" ;
	
	while(true)
	{
		string Command = ReadLine("shell-prompt$ ") ;
		
		if( Command == "\n" )
			continue ;
		
		else
			Executor( Command ) ;	

		Children.clear() ;	
	}	
}

string SimpleShell::ReadLine(string Prompt)
{
	cout << Prompt << " ";
	
	string input ;
	getline(cin, input) ;
	
	return input ;	
}

void SimpleShell::Executor(string Command)
{
	if( BuiltInCmd( Command ) )
		return ;

	vector<string> parsedCmd = Parser( Command ) ;	
	
	if( sigprocmask(SIG_BLOCK, &signalSet, NULL) == -1 )
	{
		cerr << "Fail to change signal mask\n" ;
		exit(0) ;	
	}	
	
	for(int i=0; i<parsedCmd.size(); i++)
		ExecuteSingleCmd( parsedCmd[i] ) ;
		
}

vector<string> SimpleShell::Parser(string Command)
{
	stringstream SS(Command) ;
	vector<string> parsedCmd ;
	
	string nextToken ;	
	while( getline(SS, nextToken, '|') )
		parsedCmd.push_back(nextToken) ;
	
	for(int i=0; i<parsedCmd.size(); i++)
	{	
		size_t Pos ;
		if( (Pos = parsedCmd[i].find_first_not_of(" ")) != string::npos )
			parsedCmd[i] = parsedCmd[i].substr(Pos, string::npos) ;
	
		while( parsedCmd[i][parsedCmd[i].length()-1] == ' ' )
			parsedCmd[i].pop_back() ;
			
		if( i != parsedCmd.size()-1 )
			parsedCmd[i] += "|" ;
	}	
			
	return parsedCmd ;
}


void SimpleShell::ExecuteSingleCmd(string singleCmd)
{
	/*** Create pipe ***/
	int pipeFd[2] ;
	Pipe readPipe ;
	
	
	/***  Read descriptor ***/
	pipeFd[READFD] = myPipe.GetPipeToRead( readPipe )? readPipe.readFd : 0 ;
	
	size_t Pos;
	if( (Pos = singleCmd.find_first_of("<")) != string::npos )
	{
		if( pipeFd[READFD] != 0 )
		{
			cerr << "Ambiguous input redirect.\n" ;
			return ;	
		}			
		string fileName = singleCmd.substr(Pos+1, string::npos) ;
		string remainCmd = "";
		
		size_t filePos ;
		if( (filePos = fileName.find_first_not_of(" ")) != string::npos )
			fileName = fileName.substr(filePos, string::npos) ;
	
		
		if( (filePos = fileName.find_first_of(" |>")) != string::npos )
		{				
			remainCmd = fileName.substr(filePos, string::npos) ;
						
			fileName.erase(filePos, fileName.length()-filePos) ;
		}
		
		pipeFd[READFD] = open( fileName.c_str(), O_RDONLY | O_CREAT, S_IRUSR ) ;
		
		singleCmd = singleCmd.substr(0, Pos) ;
		while( singleCmd[singleCmd.length()-1] == ' ' )
			singleCmd.pop_back() ;
		
		singleCmd += remainCmd ;
	}
	
	
	/***  Write descriptor ***/	
	if( (Pos = singleCmd.find_first_of(">")) != string::npos )
	{
		string fileName = singleCmd.substr(Pos+1, string::npos) ;
		
		/*** This step eliminate the blank before the file name ***/
		size_t filePos ;
		if( (filePos = fileName.find_first_not_of(" ")) != string::npos )
			fileName = fileName.substr(filePos, string::npos) ;
		
		pipeFd[WRITEFD] = open( fileName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR ) ;
		
		singleCmd = singleCmd.substr(0, Pos) ;
		while( singleCmd[singleCmd.length()-1] == ' ' )
			singleCmd.pop_back() ;
	}
	
		
	else if( (Pos = singleCmd.find_first_of("|")) != string::npos )
	{		
		Pipe writePipe = myPipe.GetPipeToWrite() ;	
		pipeFd[WRITEFD] = writePipe.writeFd ;
		
		singleCmd.pop_back() ;
	}
	
	
	else
		pipeFd[WRITEFD] = 1 ;
		
	
	
	ForkChild(singleCmd, pipeFd, readPipe) ;	
}



/***  Fork a child and dup the descriptor ***/
void SimpleShell::ForkChild(string singleCmd, int pipeFd[2], Pipe readPipe)
{
	pid_t childPid ;
	
	if( pipeFd[READFD] != 1 )
		close(readPipe.writeFd) ;
	
	if( (childPid = fork()) < 0 )
	{
		cerr << "Fork error\n" ;
		exit(0) ;
	}	
	
	if( childPid == 0 )
	{		
		if( sigprocmask(SIG_UNBLOCK, &signalSet, NULL) == -1 )
		{
			cerr << "Fail to change signal mask\n" ;
			exit(0) ;	
		}	
			
		// Handle regular expression
		if( singleCmd.find_first_of("*?") != string::npos )
		{
			vector<string> matchCmds = RegexHandler( singleCmd ) ;
			
				
			if( matchCmds.empty() )
			{
				cerr << "No match\n" ;
				return ;
			}	
			for(int i=0; i<matchCmds.size(); i++)
			{
				pid_t grandChild ;
				
				if( (grandChild = fork()) < 0 )
				{
					cerr << "Grand child fork error\n" ;
					exit(0) ;
				}	
				if( grandChild == 0 )
				{
					dup2(pipeFd[READFD], 0) ;
					dup2(pipeFd[WRITEFD], 1) ;
									
					lsSupport(matchCmds[i]) ;
					
					char** execCmd = ExecFormater( matchCmds[i] ) ;
					execvp( execCmd[0], execCmd ) ;
				}
				else
				{	
					if( pipeFd[READFD] != 0 )
						close( pipeFd[READFD] ) ;
					
					if( pipeFd[WRITEFD] != 1 )
						close( pipeFd[WRITEFD] ) ;
					
					wait(0) ;
					cout << endl;
				}			
			}		
		}	
		
		// No regular expression
		else
		{	
			dup2(pipeFd[READFD], 0) ;
			dup2(pipeFd[WRITEFD], 1) ;
	
			if( pipeFd[READFD] != 0 )
			close( pipeFd[READFD] ) ;
		
			if( pipeFd[WRITEFD] != 1 )
			close( pipeFd[WRITEFD] ) ;
		
	
			char** execCmd = ExecFormater( singleCmd ) ;
			if( execvp( execCmd[0], execCmd ) == -1 )
			{
				cerr << string(execCmd[0]) << ": Command not found.\n" ;			
				exit(0) ;	
			}	
		}			
	}	
	
	else
	{		
		int childState ;		
		if( waitpid( childPid, &childState, 0 )< 0 )
		{
			cerr << "Waitpid error \n" ;
			exit(0) ;
		}	
		
		if( pipeFd[READFD] != 0 )
			close(readPipe.readFd) ;	
		
		Children.push_back( childPid ) ;
	}	
	
}


char** SimpleShell::ExecFormater(string singleCmd)
{
	stringstream SS(singleCmd) ;
	vector<string> argvCmd ;
	
	string cmdToken ;
	while( getline(SS, cmdToken, ' ') )
	{
		if( cmdToken != "<" && cmdToken != ">" )
			argvCmd.push_back( cmdToken ) ;
		else
			break ;
	}	
	
	char** Argv = new char* [ argvCmd.size() ] ;
	for(int i=0; i<argvCmd.size(); i++)
	{
		Argv[i] = new char [ argvCmd[i].length() ] ;
		strcpy(Argv[i], argvCmd[i].c_str()) ;
	}	
	
	Argv[argvCmd.size()] = NULL ;
	
	return Argv ;
}



bool SimpleShell::BuiltInCmd(string Cmd)
{	
	if( Cmd == "exit" )
		exit(0) ;
	
	if( Cmd.substr(0, 6) == "export" )
	{
		size_t Pos = Cmd.find_first_not_of(" ", strlen("export")) ;
		string newEnv = Cmd.substr(Pos, string::npos) ;
		
		Pos = newEnv.find_first_of("=") ;
		string envName  = newEnv.substr(0, Pos) ;
		string envValue = newEnv.substr(Pos+1, string::npos) ;
		
		setenv( (char*)envName.c_str(), (char*)envValue.c_str(), 1 ) ;
		
		return true ;
	}
	
	if( Cmd.substr(0, 5) == "unset" )
	{
		size_t Pos = Cmd.find_first_not_of(" ", strlen("unset")) ;
		string removeEnv = Cmd.substr(Pos, string::npos) ;
		
		unsetenv( (char*) removeEnv.c_str() ) ;
		
		return true ;
	}	
		
	
	if( Cmd.substr(0, 2) == "cd" )
	{
		size_t Pos = Cmd.find_first_not_of(" ", strlen("cd")) ;
		string newDir = Cmd.substr(Pos, string::npos) ;
		
		if( chdir( (char*)newDir.c_str() ) != 0 )
			cerr << newDir << ": No such file or directory.\n" ;
		
		return true ;
	}		
	
	
	return false ;	
	
}
