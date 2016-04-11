//
//  ShellHW.c
//  Created by David Ashinsky on 4/8/16.
//
#include    <stdlib.h>
#include    <stdio.h>
#include    <unistd.h>
#include    <string.h>
#include    <stdbool.h>
#include    <sys/types.h>
#include    <sys/wait.h>
#include    <unistd.h>
#include <fcntl.h>


// A line may be at most 100 characters long, which means longest word is 100 chars,
// and max possible tokens is 51 as must be space between each
size_t MAX_WORD_LENGTH = 100;
size_t MAX_NUM_WORDS = 51;
size_t MAX_FILE_LENGTH = 4096;

typedef struct{
    char **entireGroupText;
    bool IORedirect;
    bool pipe; //only pipe or IORedirect can be true, not both
    char *outputFile; //NULL if there is no IORedirect
    char *inputFile; //NULL if there is no IORedirect OR if there's already an outputFile
}TokenNode;


/** Function Declarations
**/
char** readLineOfWords();
int getNumNodes(char **line); //gets the number of total commands on the line (delimted by a pipe). This is only 1 if there is no pipe on the line
TokenNode **getStartNode(char **line); //returns array of nodes for processing (or just 1 node if there's no pipe
bool isBackgroundProcess(char **line); //YES if the process is backgrounded (&)
void processLineOfTokens(TokenNode **tokens); //processes a single line of tokens
void processToken(TokenNode *node); //processes a single token

int main(){
    int status;
    //read line of words
    char** line = readLineOfWords();
    TokenNode **tokens;
    
    while (line != NULL)
    {
        int pid = fork();
        bool isBackground = isBackgroundProcess(line);
        if (pid == 0){
            tokens = getStartNode(line);
        }
        if (isBackground){
            if (pid == 0){
                processLineOfTokens(tokens);
            }
        }else{
            if (pid == 0){
                processLineOfTokens(tokens);
            }else{
                int status;
                waitpid(pid,&status,0);
            }
        }
        line = readLineOfWords();
    }
    return 0;
}

void processLineOfTokens(TokenNode **tokens){
    int i = 0;
    TokenNode *pointer = tokens[i];
    
    if (!pointer->pipe){
        processToken(pointer);
    }
    else
    {
        while (pointer != NULL){
            if (!pointer->pipe){
                processToken(pointer);
                //process will terminate in processToken function
            }
            int desp[2];
            pipe(desp);
            int pid = fork();
            if (pid == 0){
                close(desp[0]);
                dup2(desp[1],1);
                processToken(pointer);
            }else{
                close(desp[1]);
                int status;
                waitpid(pid,&status,0);
                dup2(desp[0],STDIN_FILENO);
            }
            i++;
            pointer = tokens[i];
        }
    }
}

void processToken(TokenNode *pointer){
    if (pointer->IORedirect){
        if (pointer->outputFile){
            int outFileFD = open(pointer->outputFile,O_CREAT|O_WRONLY,0644);
            fflush(stdout);
            dup2(outFileFD,1);
            close(outFileFD);
        }
        if (pointer->inputFile){
            int inFileFD = open(pointer->inputFile,O_RDONLY,0644);
            dup2(inFileFD,STDIN_FILENO);
            close(inFileFD);
        }
    }
    execvp(pointer->entireGroupText[0],pointer->entireGroupText);
}
                       
bool isBackgroundProcess(char **line){
    int counter = 0;
    char *word = line[counter];
    while(word != NULL){
        if (*word == '&'){
            free(word);
            line[counter] = NULL;
            return true;
        }
        counter ++;
        word = line[counter];
    }
    counter = 0;
    word = line[counter];
    return false;
}

TokenNode **getStartNode(char **line){
    int numNodes = getNumNodes(line);
    TokenNode **nodes = malloc(sizeof(TokenNode)*(numNodes+1)); //creates an array of tokens plus the end (NULL terminator)
    nodes[numNodes] = NULL;
    
    int numWordsInGroup[numNodes];
    for (int i=0; i<numNodes; i++){
        numWordsInGroup[i]=0;
    }
    
    int groupForWord = 0;
    int counter = 0;
    char *wordIterator = line[counter];
    
    while (wordIterator != NULL){
        if(*wordIterator == '|'){
            groupForWord ++;
        }else{
            numWordsInGroup[groupForWord] ++;
        }
        counter ++;
        wordIterator = line[counter];
    }
    
    int indexInLine = 0;
    for (int i=0; i<numNodes; i++){
        char **entireGroupWord = malloc(sizeof(char *)*(numWordsInGroup[i]+1));
        TokenNode *node = malloc(sizeof(TokenNode));
        char *outputIfNeeded = NULL;
        char *inputIfNeeded = NULL;
        int indexInArray = 0;
        for (int j=0; j<numWordsInGroup[i]; j++){
            char *wordInGroup = line[indexInLine];
            if (*wordInGroup == '>'){
                indexInLine = indexInLine+1;
                outputIfNeeded = line[indexInLine];
                break;
            }else if (*wordInGroup == '<'){
                indexInLine = indexInLine+1;
                inputIfNeeded = line[indexInLine];
            }else if(*wordInGroup == '|'){
                printf("Shouldn't be in here...\n");
            }else{
                entireGroupWord[indexInArray]=wordInGroup;
                indexInLine ++;
                indexInArray ++;
            }
        }
        
        char *tempPointer = entireGroupWord[0];
        int lastKnownWordIndex = 0;
        while (tempPointer != NULL){
            lastKnownWordIndex ++;
            tempPointer = entireGroupWord[lastKnownWordIndex];
            if (tempPointer == NULL){
                lastKnownWordIndex = lastKnownWordIndex - 1;
                break;
            }
        }
        for (int j=lastKnownWordIndex+1; j<numWordsInGroup[i]+1; j++){
            free(entireGroupWord[j]);
            entireGroupWord[j] = NULL;
        }
        
        node->entireGroupText = entireGroupWord;
        if (outputIfNeeded || inputIfNeeded){
            node->IORedirect = true;
        }else{
            node->IORedirect = false;
        }
        node->pipe = (i == (numNodes-1) ? false : true);
        node->outputFile = outputIfNeeded;
        node->inputFile = inputIfNeeded;
        indexInLine ++;
        nodes[i] = node;
    }
    return nodes;
}

int getNumNodes(char **line){
    int numNodes = 1;
    int counter = 0;
    char *wordIterator = line[counter];
    
    while (wordIterator != NULL){
        if (*wordIterator == '|'){
            numNodes ++;
        }
        counter ++;
        wordIterator = line[counter];
    }
    return numNodes;
}

// This function was created by S. GOINGS

/*
 * reads a single line from terminal and parses it into an array of tokens/words by
 * splitting the line on spaces.  Adds NULL as final token
 */
char** readLineOfWords() {
    // allocate memory for array of array of characters (list of words)
    char** words = (char**) malloc( MAX_NUM_WORDS * sizeof(char*) );
    int i;
    for (i=0; i<MAX_NUM_WORDS; i++) {
        words[i] = (char*) malloc( MAX_WORD_LENGTH );
    }
    
    // read actual line of input from terminal
    int bytes_read;
    char *buf;
    buf = (char*) malloc( MAX_WORD_LENGTH+1 );
    bytes_read = getline(&buf, &MAX_WORD_LENGTH, stdin);
    
    // take each word from line and add it to next spot in list of words
    i=0;
    char* word = (char*) malloc( MAX_WORD_LENGTH );
    word = strtok(buf, " \n");
    while (word != NULL && i<MAX_NUM_WORDS) {
        strcpy(words[i++], word);
        word = strtok(NULL, " \n");
    }
    
    // check if we quit because of going over allowed word limit
    if (i == MAX_NUM_WORDS) {
        printf( "WARNING: line contains more than %d words!\n", (int)MAX_NUM_WORDS );
    }
    else
        words[i] = NULL;
    
    // return the list of words
    return words;
}