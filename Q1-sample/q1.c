#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool isValid(char c);

int main(int argc, char* argv[]){
    if (argc != 4){
        puts("Usage:\nsamples file numberfrags maxfragsize");
        return EXIT_FAILURE;
    }
    const char *filename = argv[1];
    int n = atoi(argv[2]);
    int m = atoi(argv[3]);

    if (n < 0){
        puts("Number of Fragments must be non-negative");
        return EXIT_FAILURE;
    }
    if (m < 0){
        puts("Max Fragment Size must be non-negative");
        return EXIT_FAILURE;
    }

    srandom(0);
    FILE *fptr, *fstart;
    fstart = fopen(filename, "r");
    fptr = fstart;
    fseek(fptr, 0L, SEEK_END);
    int filesize = ftell(fptr);
    fptr = fstart;
    for(int i = 0; i < n; i++){
        int aux = random() % (filesize - m);
        fseek(fptr, aux, SEEK_SET);
        putchar('>');
        char c;
        for(int j = 0; j < m;){
            if((c = fgetc(fptr)) == EOF) c = ' ';
            if(isValid(c)){
                putchar(c);
                j++;
            }
        }
        puts("<");
    }
    fclose(fptr);
    return 0;
}

bool isValid(char c){
    return c != '\n' && c != '\r';
}
