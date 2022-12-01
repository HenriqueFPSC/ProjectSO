#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[]){
    if(argc != 4){
        puts("Usage:\ntokenring numberOfProcesses probabilityOfBlock(0 to 1 value) blockTime(seconds)");
        return EXIT_FAILURE;
    }
    int n = atoi(argv[1]);
    double p = atof(argv[2]);
    int t = atoi(argv[3]);

    // Check Arguments
    if (p < 0 || p > 1){
        puts("Probability must be between 0 and 1.");
        return EXIT_FAILURE;
    }
    if (n <= 0){
        puts("Number of Processes must be positive.");
        return EXIT_FAILURE;
    }
    if (t < 0){
        puts("The Block Time must be non-negative.");
        return EXIT_FAILURE;
    }

    // Create FIFO pipes
    char pipeName[50];
    for(int i = 0; i < n; i++){
        int thisProcess = i + 1;
        int nextProcess = (thisProcess) % n + 1;
        sprintf(pipeName, "/tmp/pipe%dto%d", thisProcess, nextProcess);
        unlink(pipeName);
        int status = mkfifo(pipeName, 0666);
        if (status < 0){
            fprintf(stderr, "%s: mkfifo error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    // Create Processes
    pid_t children[n];
    pid_t pid = 0;
    char buffer[20];
    char writePipe[50];
    char readPipe[50];
    for(int i = 0; i < n; i++){
        pid = fork();
        if (pid < 0){
            fprintf(stderr, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (pid == 0){ // Child Process
            srand(time(NULL) - i * 1234); // "Random" Seed
            int prevProcess = (i == 0) ? n : i;
            int thisProcess = i + 1;
            int nextProcess = (thisProcess) % n + 1;
            sprintf(writePipe, "/tmp/pipe%dto%d", thisProcess, nextProcess);
            sprintf(readPipe, "/tmp/pipe%dto%d", prevProcess, thisProcess);
            int fd;
            if(i == 0){ // Write to pipe on the first process
                sprintf(buffer, "%d", 0);
                if((fd = open(writePipe, O_WRONLY)) < 0){
                    fprintf(stderr, "%s: fifo open error: %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                if(write(fd, buffer, strlen(buffer) + 1) < 0){
                    fprintf(stderr, "%s: fifo write error: %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                close(fd);
            }
            while(true){
                if((fd = open(readPipe, O_RDONLY)) < 0){
                    fprintf(stderr, "%s: fifo open error: %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                if((read(fd, buffer, sizeof(buffer))) < 0){
                    fprintf(stderr, "%s: fifo read error: %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                close(fd);
                
                int token = atoi(buffer);
                token++;
                sprintf(buffer, "%d", token);
    
                // Random Block
                double choice = (double) rand() / RAND_MAX; // Random Number between 0 and 1
                if(choice < p){ // Block
                    printf("[p%d] lock on token (val = %d)\n", thisProcess, token);
                    sleep(t);
                    printf("[p%d] unlock token\n", thisProcess);
                }

                if((fd = open(writePipe, O_WRONLY)) < 0){
                    fprintf(stderr, "%s: fifo open error: %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                if(write(fd, buffer, strlen(buffer) + 1) < 0){
                    fprintf(stderr, "%s: fifo write error: %s\n", argv[0], strerror(errno));
                    return EXIT_FAILURE;
                }
                close(fd);
            }
            break;
        } else {
            children[i] = pid;
        }
    }

    // Wait for children
    for(int i = 0; i < n; i++)
        if(waitpid(children[i], NULL, 0) < 0){
            fprintf(stderr, "%s: waitpid error: %s\n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }

    return 0;
}
