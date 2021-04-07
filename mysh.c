#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

//ADS for linkedlist
typedef struct node node;
struct node
{
    char *pastcmmd;
    node *next;
    node *prev;
};

// global vars
node *head;
int exitStatus = 0;
node *tail;
int activePID[100];
int pidCount;
char currentDir[100];
char *validCommands[] = {"movetodir", "whereami", "history", "byebye", "replay",
                         "start", "background", "dalek", "repeat", "dalekall",
                         "help", "maik", "dwelt", "coppy", "coppyabode"};

//function signatures
void insert(char *cmmd);
void printList();
void tokenize(char *cmmd);
int findCommand(char *cmmd);
void delegateCommand(char *cmmd, char **args);
void moveToDir(char **args);
void getCurrDir();
void showHistory(char **args);
void deleteList();
void replayCommand(int num);
void startProcess(char **args);
void dalek(char **args);
void backgroundProcess(char **args);
void getHistory();
void saveHistory();
void repeatNumber(char **args);
void dalekAll();
void maikFile(char **args);
void dweltCheck(char **args);
void copyFilesOver(char **args);

int main()
{
    // initialize current directory
    getCurrDir();
    char cmmd[100];
    // get history or create new history
    getHistory();
    // loop until user exits program
    while (!exitStatus)
    {
        printf("# ");
        // input from stdin
        fgets(cmmd, sizeof(cmmd), stdin);
        // insert command into history
        insert(cmmd);
        //break apart command
        tokenize(cmmd);
    }
    saveHistory();
    return 0;
}

void getCurrDir()
{
    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    strcpy(currentDir, cwd);
}

void tokenize(char *cmmd)
{
    // strtok destroys string input
    // copy to a static string instead
    char **args;
    args = calloc(10, sizeof(char **));
    for (int i = 0; i < 10; i++)
        args[i] = calloc(30, sizeof(char *));

    char tmp[100];
    strcpy(tmp, cmmd);

    for (int i = 0; i < strlen(tmp); i++)
    {
        if (tmp[i] == '\n') // trying to remove newline char
            tmp[i] = '\0'; 
    }

    strcpy(args[0], "");
    char commandToken[100];

    char *token = strtok(tmp, " ");
    int i = 0;
    while (token)
    {
        if (i > 8)
        {
            printf("Too many commands.\n");
            return;
        }
        if (strcmp(args[0], "") == 0)
        {
            strcpy(args[0], "nothing");
            strcpy(commandToken, token);
            token = strtok(NULL, " ");
            continue;
        }
        strcpy(args[i++], token);
        token = strtok(NULL, " ");
    }
    if (i != 0)
    {
        args[i] = NULL;
    }

    delegateCommand(commandToken, args);

    for (int i = 0; i < 10; i++)
        free(args[i]);
    free(args);
    return;
}

void delegateCommand(char *cmmd, char **args)
{
    int index = findCommand(cmmd);
    if (index == -1)
    {
        // Not valid command
        printf("Sorry, :%s: is not recognized.\n", cmmd);
        return;
    }

    switch (index)
    {
    case 0: // move to dir
        moveToDir(args);
        break;
    case 1: // whereami
        printf("%s\n", currentDir);
        break;
    case 2: // history
        showHistory(args);
        break;
    case 3: // byebye
        exitStatus = 1;
        break;
    case 4: // replay
        replayCommand(atoi(args[0]));
        break;
    case 5: // start
        startProcess(args);
        break;
    case 6: // background
        backgroundProcess(args);
        break;
    case 7: // dalek
        dalek(args);
        break;
    case 8: // repeat number of times
        repeatNumber(args);
        break;
    case 9: // dalekall
        dalekAll();
        break;
    case 10:
        printf("Note from dev -> [] defines an optional choice\n\
        1.movetodir dir\n\
        2.whereami\n\
        3.history [-c]\n\
        4.byebye\n\
        5.replay [number]\n\
        6.start program [parameters]\n\
        7.background program [parameters]\n\
        8.dalek pid\n");
        break;
    case 11:
        maikFile(args);
        break;
    case 12:
        dweltCheck(args);
        break;
    case 13:
        copyFilesOver(args);
        break;
    }
}

void copyFilesOver(char **args){
    FILE *fp1, *fp2;

    // error checking
    if(access(args[0], F_OK) != 0){
        // source file does not exist
        printf("********ERROR********\n");
        printf("Source file does not exist\n");
        return;
    }
    if(access(args[1], F_OK) == 0){
        // destination file exists
        printf("********ERROR********\n");
        printf("Destination file already exists\n");
        return;
    }

    fp1 = fopen(args[0], "r");
    fp2 = fopen(args[1], "w");

    if(fp1 == NULL){
        printf("Cannot open file1\n");
        exit(0);
    }
    if(fp2 == NULL){
        printf("Cannot open file or directory\n");
        exit(0);
    }

    char tmp = fgetc(fp1);
    while(tmp != EOF){
        fputc(tmp, fp2);
        tmp = fgetc(fp1);
    }

    fclose(fp1);
    fclose(fp2);
}

void dweltCheck(char **args){

    // checking directory first
    // because access treats dir like file
    DIR *dir = opendir(args[0]);
    if(dir){
        // check if it's a directory
        printf("Abode is\n");
        return;
    }
    
    if(access(args[0], F_OK) == 0){
        // check if it's a directory
        printf("Dwelt indeed\n");
        return;
    }
    
    // neither
    printf("Dwelt not\n");
}

void maikFile(char **args){
    if(access(args[0], F_OK) == 0){
        // file already exists
        printf("********ERROR********\n");
        printf("File already exists\n");
        return;
    }
    // file does not exist
    FILE *fp;

    //check if it's an absolute path
    if(args[0][0] == '/'){
        fp = fopen(args[0], "w");
        fputs("Draft", fp);
        fclose(fp);
        return;
    }

    // create file in current directory
    // so need to append file onto current directory
    char tmp[200];
    strcpy(tmp, currentDir);
    strcat(tmp, "/");
    strcat(tmp, args[0]);

    fp = fopen(tmp, "w"); // open for writing
    fputs("Draft", fp);
    fclose(fp);
}

void startProcess(char **args)
{
    pid_t pid;

    // fork child process
    pid = fork();

    if (pid == 0)
    {
        pid = getpid();
        execvp(args[0], args);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}

void backgroundProcess(char **args)
{
    pid_t pid;

    // fork child process
    pid = fork();
    if (pid == 0)
    {
        pid = getpid();
        execvp(args[0], args);
    }
    else
    {
        //int status;
        waitpid(-1, NULL, WNOHANG);
    }
    activePID[pidCount++] = (int)pid;
    printf("PID: %d\n", (int)pid);
}

void dalek(char **args)
{
    int status = kill(atoi(args[0]), SIGKILL);
    if (status == -1)
    {
        printf("Failed to kill PID\n");
        return;
    }
    else if (errno)
    {
        printf("%d\n", errno);
    }
    else
    {
        pidCount--;
        for (int i = 0; i < 100; i++)
        {
            if (activePID[i] == atoi(args[0]))
            {
                activePID[i] = -1;
                return;
            }
        }
        printf("Killed process: %d\n", atoi(args[0]));
    }
}

void dalekAll()
{
    for (int i = 0; i < 100; i++)
    {
        if (activePID[i] < 30)
        {
            continue;
        }
        int status = kill(activePID[i], SIGKILL);
        if (status == -1)
        {
            printf("Failed to kill PID: %d\n", activePID[i]);
            return;
        }
        else
        {
            printf("Exterminating Process: %d\n", activePID[i]);
            activePID[i] = -1;
            pidCount--;
        }
    }
}

void repeatNumber(char **args)
{
    int num = atoi(args[0]);
    char tmp[100] = "";
    int j = 0;

    while (args[j + 1] != NULL)
    {
        strcpy(args[j], args[j + 1]);
        j++; // copying over array, removing number
    }
    args[j] = NULL;

    printf("PIDS: ");
    for (int i = 0; i < num; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            pid = getpid();
            execvp(args[0], args);
        }
        else
        {
            int status;
            waitpid(-1, &status, WNOHANG);
        }
        activePID[pidCount++] = (int)pid;
        printf("%d ", (int)pid);
    }
    printf("\n");
}

void replayCommand(int num)
{
    node *tmp = tail;
    for (int i = 0; i <= num; i++)
    {
        tmp = tmp->prev;
    }
    if (tmp == NULL)
    {
        printf("Sorry, replay couldn't be performed.\n");
        return;
    }
    char *cmmd = tmp->pastcmmd;
    tokenize(cmmd);
}

void showHistory(char **args)
{
    if (strcmp(args[0], "-c") == 0)
    {
        deleteList();
    }
    else if (strcmp(args[0], "nothing") != 0)
    {
        printf("Sorry, that parameter is not recognized\n");
    }
    else
    {
        printList();
    }
}

void getHistory()
{
    FILE *fp;
    char cmmd[100];

    fp = fopen("history.txt", "a+");
    while (fgets(cmmd, sizeof(cmmd), fp) != NULL)
    {
        insert(cmmd);
    }
    fclose(fp);
}

void saveHistory()
{
    FILE *fp;
    fp = fopen("history.txt", "w");
    node *tmp = head;
    while (tmp != NULL)
    { // prints in reverse due to assignment specs

        fprintf(fp, "%s", tmp->pastcmmd);
        if (tmp->next == NULL)
        {
            break;
        }
        tmp = tmp->next;
    }
    fclose(fp);
}

void moveToDir(char **args)
{
    if (strcmp(args[0], "") == 0)
    {
        printf("Please enter a dir to move to.\n");
        return;
    }

    char newPath[100];

    if (args[0][0] == '.' && args[0][1] == '.' && args[0][2] == '/')
    {
        for (int i = strlen(currentDir); i > 0; i--)
        {
            if (currentDir[i] == '/')
            {
                char tmp2[100] = "";
                for (int j = 0; j < i; j++)
                {
                    tmp2[j] = currentDir[j]; // remove dir from currdir
                }
                strcpy(currentDir, tmp2);
                break;
            }
        }
        for (int i = 3; i < strlen(args[0]); i++)
        {
            newPath[i - 3] = args[0][i];
        }
    }
    else if (args[0][0] == '.' && args[0][1] == '/')
    {
        for (int i = 2; i < strlen(args[0]); i++)
        {
            newPath[i - 2] = args[0][i];
        }
    }
    else
    {
        strcpy(newPath, args[0]);
    }

    char copydir[100];
    strcpy(copydir, currentDir);

    if (newPath[0] != '/')
    {
        strcat(copydir, "/");
    }

    strcat(copydir, newPath);
    DIR *dir = opendir(copydir); // testing if dir exists

    if (dir)
    {
        // logic for cd ..
        // it's just string manipulation
        if (strcmp(newPath, "..") == 0)
        {
            for (int i = strlen(currentDir); i > 0; i--)
            {
                if (currentDir[i] == '/')
                {
                    char tmp2[100] = "";
                    for (int j = 0; j < i; j++)
                    {
                        tmp2[j] = currentDir[j]; // remove dir from currdir
                    }
                    strcpy(currentDir, tmp2);
                    break;
                }
            }
        }
        else
            strcpy(currentDir, copydir);

        closedir(dir);
    }
    else // dir does not exist
        printf("Sorry, %s does not exist.\n", newPath);
}

int findCommand(char *cmmd)
{
    // length in bytes divided by one partition of array
    size_t length = sizeof(validCommands) / sizeof(validCommands[0]);

    for (int i = 0; i < length; i++)
    {
        if (strcmp(validCommands[i], cmmd) == 0)
            return i;
    }
    return -1;
}

//linked list functions

void insert(char *cmmd)
{
    // allocate mem for node
    node *temp = (node *)calloc(1, sizeof(node));
    // allocate mem for string
    size_t length = 100;
    temp->pastcmmd = (char *)calloc(length, sizeof(char));
    strcpy(temp->pastcmmd, cmmd);

    if (head == NULL)
    {
        head = temp;
        tail = head;
        return;
    }

    //appending to end
    temp->next = NULL;
    temp->prev = tail;
    temp->prev->next = temp;
    tail = temp;
}

void deleteList()
{ // loops and frees list
    node *tmp;
    node *curr = head;
    while (curr != NULL)
    {
        tmp = curr->next;
        free(curr);
        curr = tmp;
    }
    tail = NULL;
    head = NULL;
}

void printList()
{
    node *tmp = tail;
    int index = 0;
    while (tmp != NULL)
    { // prints in reverse due to assignment specs
        printf("%d: %s", index++, tmp->pastcmmd);
        if (tmp->prev == NULL)
        {
            break;
        }
        tmp = tmp->prev;
    }
}
