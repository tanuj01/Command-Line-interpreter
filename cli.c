#include <stdio.h> 
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#define KYEL    	"\x1B[33m"
#define RESET   	"\x1B[0m"
#define RED     	"\x1B[31m"
#define BOLDRED 	"\033[1m\033[31m"
#define BOLDYELLOW  "\033[1m\033[33m"

char hist[1000000][80]; 
char list[2] = {'\0'};
int histcount=0, top;
int logging=1,intlogging=1;
int succfail;


char* strcpyR(char *in, int u, int l)			//UPPER LIMIT'S INDEX ISNT INCLUDED AND IT APPENDS A '\n' AT THE END
{
	char *op;
	op = malloc( sizeof(char) * (u-l+1) );
	for (int i = 0; i < (u-l); ++i)
		op[i] = in[l+i];
	op[strlen(op)]='\n';
	return op;
}

void printhistory()
{
	for (int i = 0; i < histcount; ++i)
		printf("%d.  %s", i+1, hist[i]);
}

void execute(char* in,int len)					//INPUT  EG: "cat hello.c","ps -l","ls"
{
	int a=0, i=0, flag=0,count=0,fail=0;
	char *command, *arg;
	command=malloc(sizeof(char)*10);
	arg=malloc(sizeof(char)*50);

	while(in[i]!=' ' && i!=(len-1))				//SEPERATING THE MAIN COMMAND EG: cat
	{
		command[i] = in[i];
		i++;
		if (in[i] == ' ')
		{
			flag=1;
			count++;							//COUNTING THE NUMBER OF SPACES
		}
	}
	command[i]='\0';
	i++;
	
	if (flag == 1)								//CHECKS IF THERE IS ANY ARGUMENT
	{
		while (i!=len-1)
		{
			arg[a] = in[i];
			i++; 
			a++;
		}
		arg[a]='\0';

		if (strcmp(command,"cd") == 0)
		{
			if(chdir(arg) == -1)
			{
				printf("bash: cd: %s: No such file or directory",arg);
				succfail=0;
			}
			return;
		}
		
		
		int id=fork();
		if (id == 0)
		{
			if( execlp(command,command,arg,0) == -1)
			{
				printf("bash: %s: command not found\n",command);
				succfail=0;

			}
		}
		else
			wait(NULL);
			
	}
	else
	{
		char *in1;
		in1=malloc(sizeof(char)*len-1);
		strncpy(in1,in,len-1);

		if ( (strcmp(in1,"cd\0") == 0) || (strcmp(in1,"cd\n") == 0) )
		{
			chdir("/root");
			return;
		}
		
		
		int id=fork();
		if(id == 0)
		{
			if(strcmp(in1,"history\0") == 0)
				printhistory();
			else if(execlp(in1,in1,0) == -1)
			{
				printf("bash: %s: command not found\n",in1);
				succfail=0;
			}
		}
		else
			wait(NULL);
			
	}
}

void makearray(char *a)
{
	top=0;
	list[2] = list[1] = list[0] = '\0';
	for (int i = 0; i < strlen(a); ++i)
		if (a[i] == '|' || a[i] == '>' || a[i] == '<')
			list[top++] = a[i];
}

void printshell()
{
	int fd,n;
	char buff[128];
	fd=open("printredirec.txt",O_WRONLY | O_CREAT,0777);
	printf(BOLDYELLOW  "\ncli@TanSoum:" BOLDRED "~");
	fflush(stdout);
	int id = fork();
	if(id==0)
	{
		dup2(fd,1);
		execlp("pwd","pwd",0);
	}
	else
	{
		wait(NULL);
		close(fd);
		fd=open("printredirec.txt",O_RDONLY);
		while( ( n = read(fd,buff,128) ) > 0 )
			write(1,buff,n-1);					//TO DELETE THE "\n" AT THE END 
		printf(RESET "# ");
	}
}

void append2hist(char* s)
{
	char succ[]={"Success\n"},fail[]={"Failure\n"};
	if(logging!=1)
		return;
		
	int i=0;
	while(i != strlen(s))
	{
		hist[histcount][i] = s[i];
		i++;
	}
	int a=0;
	if (succfail==1)
		while(i != strlen(s) + 9)
		{	
			hist[histcount][i] = succ[a];
			i++;
			a++;
		}
	else
		while(i != strlen(s) + 9)
		{	
			hist[histcount][i] = fail[a];
			i++;
			a++;
		}

	histcount++;
	int id=fork();
	char buff[512];
	if(id==0)
	{
		int fd;
		fd=open("Logcomm.txt",O_CREAT | O_WRONLY,0777);
		dup2(fd,1);
		printhistory();
	}
	else
		wait(NULL);
}

void execpr(char *a)
{
	char *c[3];
	int u=0, l=0, no=0;

	for (int i = 0; i < strlen(a); ++i) 			//THIS WILL SEPERATE THE COMMANDS AND ARGUMENTS INTO A CHAR * ARRAY
		if (a[i] == '|' || a[i] == '>')
		{
			if (l != 0)
				l = u + 3;
			u = i-1;
			c[no] = malloc( sizeof(char) * (u-l) );
			c[no] = strcpyR(a,u,l);
			no++;
			l++;
		}
	l = u + 3;
	u = strlen(a);
	c[no] = strcpyR(a,u-1,l);

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < strlen(c[i]); ++j)
			if(c[i][j]=='\n')
				c[i][j] = '\0';
/*****************************************************************/
	int id1 = fork();
	if (id1 == 0)
	{
		int fd=open("interlog.txt",O_CREAT | O_RDWR,0777);
		if(!fork())
		{
			dup2(fd,1);
			execlp(c[0],c[0],0);
		}
		else
		{
			wait(NULL);
			fd=open("interlog.txt",O_CREAT | O_RDWR,0777);
			int fd1=open(c[2],O_CREAT | O_RDWR,0777);
			if(!fork())
			{
				dup2(fd,0);
				dup2(fd1,1);
				printf("%d",intlogging);
				execlp(c[1],c[1],0);
				
			}
			if(intlogging==0)
				if(!fork())
					execlp("rm","rm","interlog.txt",0);
				else
					wait(NULL);
		}
	}
	else
		wait(NULL);
	return;
}

void execlr(char *a)
{
	char *c[3];
	int u=0, l=0, no=0;

	for (int i = 0; i < strlen(a); ++i) 			//THIS WILL SEPERATE THE COMMANDS AND ARGUMENTS INTO A CHAR * ARRAY
		if (a[i] == '<' || a[i] == '>')
		{
			if (l != 0)
				l = u + 3;
			u = i - 1;
			c[no] = malloc( sizeof(char) * (u-l) );
			c[no] = strcpyR(a, u, l);
			no++;
			l++;
		}
	l = u + 3;
	u = strlen(a) - 1;
	c[no] = malloc( sizeof(char) * (u-l) );
	c[no] = strcpyR(a,u,l);

	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < strlen(c[i]); ++j)
			if(c[i][j] == '\n')
				c[i][j] = '\0';

	int id2 = fork();
	if (id2 == 0)
	{
		int id1 = fork();
		if (id1 == 0)
		{
			int fd1,fd2;
			if ( (fd1 = open(c[1], O_RDONLY)) == -1)
			{
				printf("Error in opening %s\n",c[1]);
				exit(0);
			}
			if ( (fd2 = open(c[2], O_CREAT | O_WRONLY, 0777) ) == -1 )
				printf("Error in opening %s\n",c[2]);
			dup2(fd1,0);
			dup2(fd2,1);
			execlp(c[0],c[0],0);
			//if(
			exit(0);
		}
		else
			wait(NULL);
	}
	else
		wait(NULL);
}

void execpp(char* a)
{
	char *c[3];
	int u=0, l=0, no=0;

	for (int i = 0; i < strlen(a); ++i) 			//THIS WILL SEPERATE THE COMMANDS AND ARGUMENTS INTO A CHAR * ARRAY
		if (a[i] == '|')
		{
			if (l != 0)
				l = u + 3;
			u = i-1;
			c[no] = malloc( sizeof(char) * (u-l) );
			c[no] = strcpyR(a,u,l);
			no++;
			l++;
		}

	l = u + 3;
	u = strlen(a);
	c[no] = strcpyR(a,u-1,l);

	int id1=fork();
	if (id1 == 0)
	{
		int fd=open("interlog.txt",O_CREAT | O_RDWR,0777);
		if(!fork())
		{
			printf("%d\n", strlen(c[0]));
			dup2(fd,1);
			if( execlp(c[0],c[0],0) == -1)
				printf("Cannot execute %s\n",c[0]);
		}
		else
		{
			wait(NULL);
			fd=open("interlog.txt",O_CREAT | O_RDWR, 0777);
			int fd1=open("interlog1.txt",O_CREAT | O_RDWR, 0777);
			if(!fork())
			{
				dup2(fd,0);
				dup2(fd1,1);
				if(execlp(c[1],c[1],0) == -1)
					printf("Cannot execute %s\n",c[1]);
			}
			else
			{
				wait(NULL);
				fd1=open("interlog1.txt",O_CREAT | O_RDWR,0777);
				if(!fork())
				{
					dup2(fd1,0);
					dup2(fd1,1);				
					if( execlp(c[2],c[2],0) == -1)
						printf("Cannot execute %s\n",c[2]);

				}
				else
					wait(NULL);
			}
			/*if(intlogging==0)
				if(!fork())
					execlp("rm","rm","interlog.txt",0);
				else
					wait(NULL);*/
		}
	}
	else
		wait(NULL);
}

void execP(char* a)
{
	int i=0;
	while(a[i]!='|')
		i++;
	char *arg1,*arg2;
	arg1=malloc(sizeof(char) * (i-1));
	arg2=malloc(sizeof(char) * (strlen(a)-i+2));
	arg1=strcpyR(a,i-1,0);
	arg2=strcpyR(a,strlen(a)-1, i+2);

	int ik=fork();
	if (ik == 0)
	{
		int fd=open("interlog.txt",O_CREAT | O_RDWR,0777);
		if(!fork())
		{
			dup2(fd,1);
			execlp(arg1,arg1,0);
		}
		else
		{
			wait(NULL);
			fd=open("interlog.txt",O_RDWR,0777);
			if(!fork())
			{
				dup2(fd,0);
				execlp(arg2,arg2,0);
			}
			else
				wait(NULL);
			if(intlogging==0)
				if(!fork())
					execlp("rm","rm","interlog.txt",0);
				else
					wait(NULL);
		}
	}
	else
	{
		wait(NULL);
		wait(NULL);
	}
	return ;
}

void execL(char* a)
{
	int i=0;
	while(a[i]!='<')
		i++;
	char *arg1,*arg2;
	arg1=malloc(sizeof(char) * (i-1));
	arg2=malloc(sizeof(char) * (strlen(a)-i+2));
	arg1=strcpyR(a,i-1,0);
	arg2=strcpyR(a,strlen(a)-1, i+2);

	for (int i = 0; i < strlen(arg2); ++i)
		if(arg2[i] == '\n')
			arg2[i] = '\0';

	int id=fork();
	if (id == 0)
	{
		int fd1;
		if( (fd1 = open(arg2,O_RDONLY) ) == -1)
			printf("%s could not be opened/found\n",arg2);
		close(0);
		dup(fd1);
		execute(arg1,strlen(arg1));
	}
	else
		wait(NULL);
}


void execr(char* a)
{
	int i=0;
	while(a[i]!='>')
		i++;
	char *arg1,*arg2;
	arg1=malloc(sizeof(char) * (i-1));
	arg2=malloc(sizeof(char) * (strlen(a)-i+2));
	arg1=strcpyR(a,i-1,0);
	arg2=strcpyR(a,strlen(a)-1, i+2);

	for (int i = 0; i < strlen(arg2); ++i)
		if(arg2[i] == '\n')
			arg2[i] = '\0';

	int id=fork();
	if (id == 0)
	{
		int fd1;
		if( (fd1 = open(arg2,O_CREAT | O_WRONLY,0777) ) == -1)
		{
			printf("%s could not be opened/found\n",arg2);
			exit(0);
		}
		close(1);
		dup(fd1);
		execute(arg1,strlen(arg1));
	}
	else
		wait(NULL);
}


void analyseMulInput(char *a)
{
	int count=1,i,n;
	char* in;
	for (i = 0; i < strlen(a); ++i)
		if( (a[i] == '&') && (a[i+1] == '&') )		//COUNTING THE NUMBER OF OCCURENCE OF &&
		{
			count++;
			i++;
		}
	n = i = 0;
	while((count--)!=1)
	{
		while(a[i+1]!='&')
			i++;
		in=strcpyR(a,i,n);
		execute(in,strlen(in));
		puts("\n");
		free(in);
		i=i+4;
		n=i;
	}
	in=strcpyR(a,strlen(a)-1,n);
	execute(in,strlen(in));	
	free(in);
	return;
}

void analyseinput(char *a)
{
	int len=strlen(a);
	for (int i = 0; i < len; ++i)
	{
		if (a[i] == '|' || a[i] == '>' || a[i] == '<')
		{
			makearray(a);
			if 	(list[0] == '|' && list[1] == '>')   /////////////////////
				execpr(a);
			else if	(list[0] == '<' && list[1] == '>')        ///////////////
				execlr(a);
			else if	(list[0] == '|' && list[1] == '|')        
				execpp(a);
			else if	(list[0] == '|' && list[1] == '\0')         
				execP(a);
			else if	(list[0] == '<' && list[1] == '\0')
				execL(a);
			else if	(list[0] == '>' && list[1] == '\0')
				execr(a);
			return;
		}
		else if (a[i] == '&')
		{
			analyseMulInput(a);
			return;
		}
		
	}
	execute(a,len);								//FOR A SINGLE WORD INPUT
}


char* input()
{
	char *buff;
	buff = malloc( sizeof(char) * 50);
	fgets(buff,50,stdin);
	if(strlen(buff) == 1)
		return NULL;
	return buff;
}

//int main(int argc, char const *argv[])
int main()
{
	if(!fork())
	{
		execlp("clear","clear",0);
		exit(0);
	}
	else
		wait(NULL);

	char *inputs;
	printf("Enter 'START' to start the command line interprator\n");
	inputs=input();
		
	if(strcmp(inputs,"START\n"))
		return 0;
	
	int i = getpid();
	while(i == getpid())
	{
		succfail=1;
		printshell();
		inputs=input();
		if (inputs == NULL)
			continue;
		else if(!strcmp(inputs,"END\n"))
		{
			printf("Exiting the command line interpreter. Thankyou!! \n\n");
			if(!fork()){									//To delete the old LogComm file
				execlp("rm","rm","Logcomm.txt");
				execlp("rm","rm","interlog.txt");				//To delete old interlogfile
			}
			else
			wait(NULL);
			return 0;
		}
		else if(!strcmp(inputs,"LogComm\n"))
		{
			logging=1;
			printf("All the commands will now be logged in Logcomm.txt\n");
			continue;
		}
		else if(!strcmp(inputs,"LogInt\n"))
		{
			intlogging=1;
			printf("All the intermediate values will be stored in interlog.txt and interlog2.txt(If neccesary)\n");
			continue;	
		}
		else if(!strcmp(inputs,"UnLogComm\n"))
		{
			logging=0;
			printf("The commands will not be logged in Logcomm.txt from now\n");
			continue;
		}
		else if(!strcmp(inputs,"UnLogInt\n"))
		{
			intlogging=0;
			printf("The intermediate values will not be stored\n");
			continue;
		}
		analyseinput(inputs);
		append2hist(inputs);
	}
}			
