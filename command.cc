
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <wait.h>
#include <sys/stat.h>
#include "command.h"
void handler (int sig){
ssize_t r;
int log = open("monitor.log" , O_WRONLY |O_APPEND | O_CREAT, 0666);
int n = strlen("Child is terminated\n");
r=write(log,"Child is terminated\n",n);
}
SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	append = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();

	// Add execution here
	// For every simple command fork a new process
	// Setup i/o redirection
	// and call exec
	int defaultin = dup( 0 );
	int defaultout = dup( 1 );
	int defaulterr = dup( 2 );
	int outfd;
	int infd;
	int errfd;
	pid_t pid;
	if(_outFile && append){
		outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
		append = 0;
		if( outfd<0 ){
			perror("e");
		}
		dup2(outfd,1);
		close(outfd);
	}
	if(_outFile && !append){
		outfd = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if( outfd<0 ){
			perror("error");
		}
		dup2(outfd,1);
		close(outfd);
	}
	if(_inputFile){
		infd = open(_inputFile,O_RDONLY,0666);
		if( infd<0 ){
			perror("error");
		}
		dup2(infd,0);
		close(infd);
	}
	if(_errFile){
		errfd = open(_errFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
		if( errfd<0 ){
			perror("error");
		}
		dup2(errfd,2);
		close(errfd);
	}
	signal(SIGCHLD,handler);
	int fdpipe[2];
	pipe(fdpipe);
		for(int i=0;i<_numberOfSimpleCommands;i++){
			if(strcmp(_simpleCommands[i]->_arguments[0],"cd")==0){
				if(!(_simpleCommands[i]->_arguments[1])){chdir("/home");}
				else{chdir(_simpleCommands[i]->_arguments[1]);}
				break;
			}
		if(i==0){
			if(_inputFile){
				infd = open(_inputFile,O_RDONLY,0666);
				if( infd<0 ){
					perror("error");
				}
				dup2(infd,0);
				close(infd);
			}
			else{
				dup2(defaultin,0);
				close(defaultin);
			}
		}
		else if (i!=0 ){
			dup2(fdpipe[0],0);
			close(fdpipe[0]);
			
		}
		if(i==(_numberOfSimpleCommands-1)){
			if(_outFile && append){
				outfd = open(_outFile, O_WRONLY | O_CREAT | O_APPEND, 0666);
				if( outfd<0 ){
					perror("error");
				}
				dup2(outfd,1);
				close(outfd);
			}
			else if(_outFile && !append){
				outfd = open(_outFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if( outfd<0 ){
					perror("error");
				}
				dup2(outfd,1);
				close(outfd);
			}
			else{
				dup2(defaultout,1);
				close(defaultout);
			}
			}
		else if (i!=(_numberOfSimpleCommands-1)){
			dup2(fdpipe[1],1);
			close(fdpipe[1]);
		}
		

		pid = fork();
		if (pid == -1 ) {
			perror( "error");
		}
	
		if (pid == 0) {
			
		// You can use execvp() instead if the arguments are stored in an array
			execvp(_simpleCommands[i]->_arguments[0],_simpleCommands[i]->_arguments);

		// exec() is not suppose to return, something went wrong
		}
		else if (pid > 0){
			dup2(defaultin,0);
			close(defaultin);
			dup2(defaultout,1);
			close(defaultout);

		}
	
	
	
	}
	
	
	dup2(defaultin,0);
	dup2(defaultout,1);
	dup2(defaulterr,2);
	close(defaultin);
	close(defaultout);
	close(defaulterr);
	if(!_background){
		waitpid( pid, 0, 0 );
	}
	//if(!_background)
	//	waitpid( pid, 0, 0 );
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void sigintHandler(int sig_num){
	signal(SIGINT, sigintHandler);
	printf("\n you can't use ctrl c\n");
}
void
Command::prompt()
{
	signal(SIGINT, sigintHandler);
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

