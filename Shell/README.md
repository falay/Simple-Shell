Implementing a Simple Shell

	Designer: Ching Tzu Chen
	Date: 2016/5/9


/***Description***/
This is a simple shell implementation which supports the basic features of a real shell


/***Usage***/
	make clean all
	./Shell
And you can enter the command in the shell


/***Features***/
This shell supports the following features:

1. Execute a single command. (Any command in /bin are supported)
2. Properly block or unblock signals.
3. Command redirection through < and >.
4. Create pipelines (No limit number of pipes restriction).
5. Setup foreground process group and background process groups.
6. Manipulate environment variables (export and unset are supported).
7. Support regular expression (Expand * and ? characters)
8. Support directory changing using cd command 

Ways to leave the shell: press Ctrl-z or enter "exit"



