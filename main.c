#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

clock_t start, end;

int matStart, matEnd;

int matA[100][100];
int matB[100][100];
int matRes[100][100];
int res[100][100];
int thRes[100][100];
int detRes[100][100];


int numOfProcesses = 4;
int numOfThreads = 4;

//To fill matrix A with ID number ==> 1211705
void fillMatrixA(int matrix[100][100], long long ID) {
    int num;
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 100; j++) {
            num = ID % 10;
            matrix[i][j] = num;
            ID /= 10;
            if (ID == 0) {
                ID = 5071121;
            }

        }
    }
}


//To fill matrix B with ID number * Birth year ===> 1211705 * 2003 = 2427045115
void fillMatrixB(int matrix[100][100], long long BirthY) {
    int num;
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 100; j++) {
            num = BirthY % 10;
            matrix[i][j] = num;
            BirthY /= 10;
            if (BirthY == 0) {
                BirthY = 5115407242;
            }

        }
    }
}

void printMatrix(int matrix[100][100]) {
    printf("\n[");
    for(int i = 0; i < 100; i++) {
        for(int j = 0; j < 100; j++) {
            printf("%d ", matrix[i][j]);
        }
    }
    printf("]\n");
}


void multMatrix(int res[100][100], int A[100][100], int B[100][100], int startRow, int endRow) {
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < 100; j++) {
            int s = 0;
            for (int k = 0; k < 100; k++) {
                s += A[i][k] * B[k][j];
            }
            res[i][j] = s;
        }
    }
}

void matrixMult (int res[100][100], int A[100][100], int B[100][100]) {
    int s;
    for(int i = 0; i < 100; i++){
        for(int j = 0; j < 100; j++){
            s = 0;
            for (int k = 0; k < 100; k++){
                 s += A[i][k] * B[k][j];
                 res[i][j] = s;
            }
        }
    }
}


// Thread function
void *threadMult(void *arg) {
    int threadId = *((int *)arg);

    // Calculate the range for matrix multiplication
    int matStart = threadId * 100 / numOfThreads;
    int matEnd = (threadId + 1) * 100 / numOfThreads;

    if (threadId == numOfThreads - 1) {
        matEnd = 100; // Last thread should handle the remaining elements
    }

    // Perform matrix multiplication
    for (int i = matStart; i < matEnd; i++) {
        for (int j = 0; j < 100; j++) {
            int s = 0;
            for (int k = 0; k < 100; k++) {
                s += matA[i][k] * matB[k][j];
            }
            thRes[i][j] = s;
        }
    }

    pthread_exit(NULL);
}


void *detachMult(void *arg) {
    int threadId = *((int *)arg);

    // Calculate the range for matrix multiplication
    int matStart = threadId * 100 / numOfThreads;
    int matEnd = (threadId + 1) * 100 / numOfThreads;

    if (threadId == numOfThreads - 1) {
        matEnd = 100; // Last thread should handle the remaining elements
    }

    // Perform matrix multiplication
    for (int i = matStart; i < matEnd; i++) {
        for (int j = 0; j < 100; j++) {
            int s = 0;
            for (int k = 0; k < 100; k++) {
                s += matA[i][k] * matB[k][j];
            }
            detRes[i][j] = s;
        }
    }
}

int main(int argc, char *argv[])
{

    start = clock();

    fillMatrixA(matA, 5071121);
    fillMatrixB(matB, 5115407242);

    matrixMult(matRes, matA, matB);

    end = clock();
    double time_N = end - start;
    double timeCode_N = time_N / (CLOCKS_PER_SEC);
    printf("\nThe execution time of Naive is: %f sec\n", timeCode_N);


    /******************* Process ********************************/

    start = clock();

    //File descriptors for the pipe
    int fd [numOfProcesses][2];

    for(int i = 0; i < numOfProcesses; i++){
       if (pipe(fd[i]) == -1){
           printf("An error ocurred opening pipe\n");
           return 1;
        }
    }

    //For loop for Processes
    for(int i = 0; i < numOfProcesses; i++){

    pid_t pid = fork();

    if (pid == -1){
        printf("An error ocurred with fork\n");
        return 2;
    }

    if (pid == 0) {
            // Child process
            close(fd[i][0]); // Close the read end of the pipe for the child

            // Calculate the range for matrix multiplication
            matStart = i * 100 / numOfProcesses;
            matEnd = matStart + 100 / numOfProcesses;

            multMatrix(res, matA, matB, matStart, matEnd);

            // Write the result to the pipe
            if (write(fd[i][1], res, sizeof(res)) == -1) {
                printf("An error occurred with writing to the pipe\n");
                return 3;
            }

            close(fd[i][1]); // Close the write end of the pipe for the child

            // Exit the child process
            exit(0);
        }

     // Wait for the child process to finish (only in the parent process)
        if (pid > 0 && i == numOfProcesses - 1) {
            for (int j = 0; j < numOfProcesses; j++) {
                wait(NULL);
            }
        }
    }

    int y[100][100] = {0};

    for (int i = 0; i < numOfProcesses; i++) {

        close(fd[i][1]);
        int res[100][100];

        if (read(fd[i][0], res, sizeof(res)) == -1){
            printf("An erroe ocurred with reading to the pipe\n");
            return 4;
        }
        close(fd[i][0]);

        for (int row = 0; row < 100; row++){
             for (int col = 0; col < 100; col++){
                  y[row][col] += res[row][col];
             }
        }
    }

    printf("\n---------------Got from child process:----------------\n");
    printMatrix(y);

    end = clock();
    double time1 = end - start;
    double timeCode1 = time1 / (CLOCKS_PER_SEC);
    printf("\nThe execution time of process is: %f sec\n", timeCode1);


    /*********** Threads joinable***********************/

    start = clock();

    /* the thread identifier */
    pthread_t tid[numOfThreads];
    int threadIds[numOfThreads];

    // Create joinable threads
    for (int i = 0; i < numOfThreads; i++) {
        threadIds[i] = i;
        if (pthread_create(&tid[i], NULL, threadMult, &threadIds[i]) != 0) {
            perror("Failed to create thread\n");
            return 5;
        }
    }

    for (int i = 0; i < numOfThreads; i++) {
         pthread_join(tid[i], NULL);
    }



    printf("\n---------------Got from Threads:----------------\n");
    printMatrix(thRes);

    end = clock();
    double time2 = end - start;
    double timeCode2 = time2 / (CLOCKS_PER_SEC);
    printf("\nThe execution time of joinable is: %f sec\n", timeCode2);

    /*********** Detached Threads ***********************/

    start = clock();

    pthread_attr_t detThread;
    pthread_attr_init(&detThread);
    pthread_attr_setdetachstate(&detThread, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < numOfThreads; i++) {
        threadIds[i] = i;
        if (pthread_create(&tid[i], &detThread, detachMult, &threadIds[i]) != 0) {
            perror("Failed to create thread\n");
            return 5;
        }
    }

    printf("\n---------------Got from Detached Threads:----------------\n");
    printMatrix(detRes);

    pthread_attr_destroy(&detThread);

    sleep(3);

    end = clock();
    double time = end - start;
    double timeCode = time / (CLOCKS_PER_SEC);
    printf("\nThe execution time of detached is: %f sec\n", timeCode);

    pthread_exit(0);
}



