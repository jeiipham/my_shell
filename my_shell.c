// my_shell.c

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

void readLine(char *line)
{
	fseek(stdin,0,SEEK_END);
	printf("%% ");  
	int c; int i = 0;
	for(; (c = getchar()) != '\n' && c != EOF; i++) line[i] = c;
	line[i] = '\0';
}

void getInFile(char *line, char *inFile)
{
	int i; char *ptr = strchr(line, '<');
	if(!ptr) return; 
	while(isspace(*ptr) || *ptr == '<') *ptr++;
	for(i = 0; isalnum(*ptr); i++, *ptr++) inFile[i] = *ptr;
}

void getOutFile(char *line, char *outFile)
{
	int i; char *ptr = strchr(line, '>');
	if(!ptr) return; 
	while(isspace(*ptr) || *ptr == '>') *ptr++;
	for(i = 0; isalnum(*ptr); i++, *ptr++) outFile[i] = *ptr;
}

void getInOutAndTruncate(char *line, char *inFile, char *outFile)
{
	getInFile(line, inFile); getOutFile(line, outFile);
	char *ptr = line;
	for(; *ptr != '<' && *ptr != '>' && *ptr != '\0'; *ptr++);
	*ptr = '\0';	
}

int getArgCount(char *line) 
{
	int num = 0; char temp[256]; char *token;
	strcpy(temp, line);
	for(token = strtok(temp, " "); token != NULL; num++)
		token = strtok(NULL, " ");
	return num;
}

void initArgValues(char *argv[], char *line)
{
	char *token = strtok(line, " "); int i = 0; 
	for(; token != NULL; i++)
	{
		argv[i] = token; 
		token = strtok(NULL, " ");
	}
	argv[i] = NULL;
}

int checkPath(char *token, char *path, char *line)
{
	struct stat mystat; char buf[256]; sprintf(buf, "%s/%s", token, path);
	if(stat(buf, &mystat) == 0)
	{ 
		sprintf(buf, "%s/%s", token, line);
		strcpy(line, buf); return 1;
	}
	return 0;
}

void catLineToEnvPath(char *line)
{
	struct stat mystat; char buf[256], tempLine[256], tempEnv[1024];
	strcpy(tempLine, line); strcpy(tempEnv, getenv("PATH"));
	char *path = strtok(tempLine, " "), *token = strtok(tempEnv, ":");
	for(; token != NULL; token = strtok(NULL, ":")) 
		if(checkPath(token, path, line)) return;		
}

void closeInOutFD(int inFD, int outFD)
{
	if(inFD != 0) 
		close(inFD);
	if(outFD != 0) 
		close(outFD);
}

void forkSwitch(char *inFile, char *outFile, int inFD, int outFD, char*args[], char *envp[])
{	
	int pid = fork();
	switch(pid)
	{
		case -1: perror("fork"); break; 
		case 0: 
			if(*inFile != 0)  dup2(inFD, STDIN_FILENO);
			if(*outFile != 0) dup2(outFD, STDOUT_FILENO);
			closeInOutFD(inFD, outFD);
			if(execve(args[0], args, envp) == -1) perror("exec error");
		default: ; // parent case 
			waitpid(pid, 0, 0);
			closeInOutFD(inFD, outFD);	
	}
}

void readPathAndIO(char *line, char *inFile, char *outFile) 
{
	readLine(line);
	catLineToEnvPath(line);
	getInOutAndTruncate(line, inFile, outFile);
}

void openIOFiles(char *inFile, char *outFile, int *inFD, int *outFD) 
{
	if(*inFile != '\0') 
		*inFD = open(inFile, O_RDONLY);
	if(*outFile != '\0') 
		*outFD = open(outFile, O_WRONLY|O_CREAT, 0666);
}

int main(int argc, char* argv[], char *envp[])
{
	while(1) 
	{
		char line[256], inFile[256] = "", outFile[256] = ""; 
		int inFD = 0, outFD = 0;
		readPathAndIO(line, inFile, outFile);
		char *args[getArgCount(line)]; initArgValues(args, line);
		openIOFiles(inFile, outFile, &inFD, &outFD);
		forkSwitch(inFile,outFile,inFD,outFD,args,envp);
	}
}


