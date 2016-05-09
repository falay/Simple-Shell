#include <vector>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include <regex.h>
#include <sstream>
#include <iostream>
using namespace std ;



bool RegexMatcher(string dirName, string expRule)
{
	regex_t Regex ;
	int retResult ;
	
	if( (retResult = regcomp(&Regex, expRule.c_str(), 0)) != 0 )
	{
		cerr << "Cannot compile regex\n" ;
		exit(0) ;
	}	
	
	if( (retResult = regexec(&Regex, dirName.c_str(), 0, NULL, 0)) == 0 )
		return true ;
	
	else if( retResult == REG_NOMATCH )
		return false ;
	
	else
	{
		cerr << "Cannot execute regex\n" ;
		exit(0) ;
	}
	
	regfree(&Regex) ;
}


vector<string> RegexHandler(string singleCmd)
{
	vector<string> matchedCmds ;
	DIR* curDir ;
	struct dirent* dir ;	
	
	/** Parsed the *? expression and translate**/
	stringstream SS(singleCmd) ;
	string cmd ;
	string expRule ;
	size_t Pos ;
	while( getline(SS, expRule, ' ') )
	{
		if( expRule.find_first_of("*?") != string::npos )
		{
			if( (Pos = expRule.find_first_of("*")) != string::npos )
			{	
				if( expRule[0] != '*' )
				{	
					expRule = "^" + expRule ;
					Pos ++ ;
				}
				expRule.replace(Pos, 1, "[:alnum:]*") ;					
			}
			
			if( (Pos = expRule.find_first_of("?")) != string::npos )
			{	
				if( expRule[0] != '.' )
				{	
					expRule = "^" + expRule ;
					Pos ++ ;
				}	
				expRule.replace(Pos, 1, ".") ;
			}
						
			break ;
		}
		else
			cmd = expRule ;
	}	
	
	/** List the directory and compare **/
	curDir = opendir("./") ;
	if( curDir )
	{
		while( (dir = readdir(curDir)) != NULL )
		{
			if( dir->d_name[0] == '.' )
				continue ;
						
			if( RegexMatcher( dir->d_name, expRule ) )
			{
				string Match = cmd + " " + string( dir->d_name ) ;
				matchedCmds.push_back( Match ) ;
			}
		}	
		
		closedir(curDir) ;
	}	
	
	
	return matchedCmds ;
}


/** print out each dir name when ls* is called **/
void lsSupport(string cmd)
{
	if( cmd.substr(0, 2) == "ls" )
		cout << cmd.substr(3, string::npos) << ": \n" ;	
}


