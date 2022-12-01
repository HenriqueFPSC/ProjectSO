#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

const char* command(const char* command, int argc, const char** argv);
const char* fileName2Epub(const char* filename);

int main(int argc, char* argv[]){
    if(argc <= 1){
        puts("Usage:\ntxt2epub [FILES]");
        return EXIT_FAILURE;
    }
    const char* zipArgs[argc];
    zipArgs[0] = "ebooks.zip";
    for(int i = 1; i < argc; i++)
        zipArgs[i] = fileName2Epub(argv[i]);
    pid_t pid = 0;
    int currFileIndex = 0;
    bool shouldCompress = true;
    while(pid == 0){ // child process
        currFileIndex++;
        if (currFileIndex >= argc) exit(0);
        pid = fork();
        if(pid == 0) shouldCompress = false;
        if(pid < 0){
            fprintf(strerror, "%s: fork error: %s\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    const char* filename = argv[currFileIndex];
    const char* epubFilename = zipArgs[currFileIndex];
    printf("[pid%d]converting %s ...\n", pid, filename);
    const char* args[] = {filename, "-o", epubFilename};
    system(command("pandoc", 3, args));
    while(wait(NULL) > 0);
    if(shouldCompress){
        puts("Compressing files...");
        system(command("zip", argc, zipArgs));
    }
    return 0;
}

const char* appendStrings(const char* s1, const char* s2){
    int size1 = strlen(s1);
    int size2 = strlen(s2);
    char* aux = (char*) malloc(size1 + size2 + 1);
    aux[size1 + size2] = 0;
    strcpy(aux, s1);
    strcpy(aux + size1, s2);
    return aux;
}

const char* fileName2Epub(const char* filename){
    int lastDot = 0;
    for(int i = 0; filename[i] != 0; i++)
        if(filename[i] == '.')
            lastDot = i;
    char nameSlice[lastDot + 1];
    nameSlice[lastDot] = 0;
    for(int i = 0; i < lastDot; i++)
        nameSlice[i] = filename[i];
    return appendStrings(nameSlice, ".epub");
}

const char* command(const char* command, int argc, const char** argv){
    char* aux = (char*) command;
    for(int i = 0; i < argc; i++)
        aux = (char*) appendStrings((const char*) aux, appendStrings(" ", argv[i]));
    return (const char*) aux;
}
