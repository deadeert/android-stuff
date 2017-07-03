#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ptrace.h>

#define PATHLEN 128
#define EXIT_FAILURE 1

int main(int argc, char **argv) {

	if (argc < 4) {
		fprintf(stderr,"%s [start] [end] [pid]\n",argv[0]);
		exit(EXIT_FAILURE); 
	}
	char * endptr;
	unsigned long int base = strtol(argv[1],NULL,16);
	unsigned long int fin = strtol(argv[2],NULL,16);
	int pid = atoi(argv[3]); 

	char * mempath, * buffer ;
	mempath = calloc(1,PATHLEN);
	buffer = calloc(1,fin-base);

	snprintf(mempath,PATHLEN-1,"/proc/%d/mem",pid); 


	int memfd = open(mempath,O_RDONLY);
	if (memfd <= 0){
		fprintf(stderr,"Error opening %s file\nerrno : %d\n",mempath,errno);
		exit(EXIT_FAILURE); 
	}

	char * filepath = calloc(PATHLEN,1);
	snprintf(filepath,PATHLEN-1,"dump-from-%d-%d.txt",base,fin); 
	FILE * file = fopen(filepath,"wb+"); 
	if (file == NULL) {
		fprintf(stderr,"Error opening output file.\nerrno : %d\n",errno); 
		exit(EXIT_FAILURE);
	} 

	if (ptrace(PTRACE_ATTACH,(pid_t)pid,NULL,NULL) == -EXIT_FAILURE) {
		fprintf(stderr,"error PTRACE_ATTACH.\nerrno : %d\n",errno);
		exit(EXIT_FAILURE); 
	}

	fprintf(stdout,"Extracting %s from : %ld to  :%ld\n",mempath,(long int) base,(long int) fin);

	waitpid(pid,NULL,0);
	lseek(memfd,base,SEEK_SET);
	read(memfd,buffer,fin-base); 
	fwrite(buffer,fin-base,1,file);
	
	fprintf(stdout,"Dump extracted to %s\n",filepath); 

	ptrace(PTRACE_DETACH,pid,NULL,NULL);
	free(mempath);
	free(buffer);
	free(filepath);
	close(memfd);
}
