#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#include "../headers/infectivity_monitor.h"

char pid_file[30] =".pid_monitor";

int shutdown_thread(){
	pid_t pid_th=0;
	int ret;
	printf("Atempting to shutdown...\n");
	FILE* file;
	file = fopen(pid_file,"r");
	if(file){
		fscanf(file,"%d",&pid_th);
		fclose(file);
		printf("PID extracted\n");
	}
	else{
		perror("Cannot open pid file\n");
		return -1;
	}
	if(pid_th <= 0) {
		perror("Invalid pid\n");
		return -1;
	}
	ret = kill(pid_th,SIGTERM);
	if(ret) perror("Failed to stop thread!\n" );
	else printf("Thread shutdowned\n");
	return ret;
}


int deploy_thread(char* filename){
	printf("Atempting to deploy...\n");
	pid_t pid_th,sid_th;
	char file[MAX_FILE_PATH_SIZE];
	if(filename)
		strncpy(file,filename,MAX_FILE_PATH_SIZE);
	else file[0]='\0';
	printf("file2 %s\n",file);
	pid_th = fork();
	if(pid_th <0)
		return -1;
	if(pid_th){
		//parent
		printf("Child started with pid %d\n",pid_th);
		FILE* file;
		file = fopen(pid_file,"w");
		if(file){
			fprintf(file,"%d",pid_th);
			fclose(file);
			printf("PID saved\n");
		}
		sleep(2);
	}
	else{
		printf("Child created\n");
		//child
		//Create a new session
		sid_th = setsid();
		if (sid_th < 0) {
			printf("Cannot sid\n");
			exit(-1);
		}
		printf("Child detached\n");
		// Change the working directory to a safe location in root
		chdir("../");
		printf("Child migrated\n");

		// Redirect standard input, output, and error to /dev/null
		// int fd = open("/dev/null", O_RDWR);
		// if (fd >= 0){
		// 	printf("Child muted\n");
		// 	dup2(fd, STDIN_FILENO);
		// 	dup2(fd, STDOUT_FILENO);
		// 	dup2(fd, STDERR_FILENO);
		// 	close(fd);	
		// }

		//Start working
		start_monitoring(file);
		return 0;
	}
	return 0;
}

int main(int argc, char** argv){
	srand(time(NULL));
	if(argc>1){
		if(strncmp("-start",argv[1],6) == 0){
			char *filename;
			if(argc>2){
				filename = (char *)malloc(sizeof(char)*strlen(argv[2])+2);
				strncpy(filename,argv[2],strlen(argv[2]));
				printf("file1 %s\n",filename);
			}
			// else{
			// 	perror("No file location\n");
			// 	return -1;
			// }
			deploy_thread(filename);
			if(argc>2)free(filename);
		}
		else if(strncmp("-stop",argv[1],5) == 0){
			shutdown_thread();
		}
		else{
			perror("Invalid command! Try -start or -stop\n");
			return -1;
		}
	}
	else{
		perror("No args found\n");
		return -1;
	}
	return 0;
}