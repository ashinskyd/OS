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


// A line may be at most 100 characters long, which means longest word is 100 chars,
// and max possible tokens is 51 as must be space between each
size_t MAX_WORD_LENGTH = 100;
size_t MAX_NUM_WORDS = 51;

typedef struct{
    char **entireGroupText;
    bool IORedirect;
    bool pipe; //only pipe or IORedirect can be true, not both
    char *outputFile; //NULL if there is no IORedirect
}TokenNode;


/** Function Declarations
**/
char** readLineOfWords();
int getNumNodes(char **line); //gets the number of total commands on the line (delimted by a pipe). This is only 1 if there is no pipe on the line
TokenNode **getStartNode(char **line);
bool isBackgroundProcess(char **line);
void processLineOfTokens(char **tokens);

int main(){
    int status;
    //read line of words
    char** line = readLineOfWords();
    TokenNode **tokens = getStartNode(line);
    
    while (line != NULL)
    {
        int pid = fork();
        if (isBackgroundProcess(line)){
            if (pid == 0){
                processLineOfTokens(tokens);
            }
        }else{
            if (pid == 0){
                processLineOfTokens(tokens);
            }else{
                int status;
                waitpidpid,&status,0);
            }
        }
        line = readLineOfWords();
    }
    
    //TODO: free memory
    return 0;
}

void processLineOfTokens(TokenNode **tokens){
    int i = 0;
    TokenNode *pointer = tokens[i];
    while (pointer != NULL){
        execvp(pointer->entireGroupText[0],pointer->entireGroupText);
        i++;
        pointer = tokens[i];
    }
}

bool isBackgroundProcess(char **line){
    int counter = 0;
    char *word = line[counter];
    while(word != NULL){
        if (*word == '&'){
            return true;
        }
        counter ++;
        word = line[counter];
    }
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
        char **entireGroupWord = malloc(sizeof(char *)*MAX_WORD_LENGTH*numWordsInGroup[i]);
        char *command;
        TokenNode *node = malloc(sizeof(TokenNode));
        char *outputIfNeeded = NULL;
        for (int j=0; j<numWordsInGroup[i]; j++){
            char *wordInGroup = line[indexInLine];
            if (*wordInGroup == '>'){
                indexInLine = indexInLine+1;
                outputIfNeeded = line[indexInLine];
                break;
            }else if(*wordInGroup == '|'){
                printf("Shouldn't be in here...\n");
            }else{
                entireGroupWord[j]=wordInGroup;
                indexInLine ++;
            }
        }
        node->entireGroupText = entireGroupWord;
        node->IORedirect = (outputIfNeeded == NULL ? false : true);
        node->pipe = (i == (numNodes-1) ? false : true);
        node->outputFile = outputIfNeeded;
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