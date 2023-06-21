#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXSIZE 100000

int **matrix;
int failCheck = 0;

struct values {
  int size;         //
  int iteraction;   //
  int *allNums;     //
  int currentGridX; //
  int currentGridY; //
  int gridX;        //
  int gridY;        //
};
typedef struct values values;

void *checkLines(void *pointer);
void *checkBoxesX(void *pointer);
void *checkBoxesY(void *pointer);
void checkFormattingSudokuSize(FILE *fp);
void checkFormattingGridSize(FILE *fp, int size);
void checkFormattingSudoku(FILE *fp, int size);

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Invalid number of parameters");
    exit(1);
  }

  FILE *fp = fopen(argv[1], "r");

  FILE *output = fopen("sudoku_lmsa.out", "w");

  if (fp == NULL) {
    printf("Error to read file");
    exit(1);
  }

  checkFormattingSudokuSize(fp);

  fseek(fp, 0, SEEK_SET);

  char sudokuDimension[100], gridDimension[100], *token, x[2] = "x", sudokuLine[MAXSIZE];
  char space[2] = " ";
  int size, gridX, gridY, matrixValue, dimensionLength1, dimensionLength2;
  pthread_t threads[5000];
  pthread_t threadsGrid[5000];
  values *save;

  fgets(sudokuDimension, 100, fp);
  dimensionLength1 = strlen(sudokuDimension);

  token = strtok(sudokuDimension, x);

  while (token != NULL) {
    size = atoi(token);

    token = strtok(NULL, x);
  }

  checkFormattingGridSize(fp, size);

  fseek(fp, dimensionLength1, SEEK_SET);

  save = (values *)calloc(size, sizeof(values));

  for (int i = 0; i < size; i++) {
    save[i].size = size; // struct
  }

  fgets(gridDimension, 100, fp);
  dimensionLength2 = strlen(gridDimension);

  token = strtok(gridDimension, x);
  int counter = 0;

  while (token != NULL) {
    if (counter == 0) {
      gridX = atoi(token);
    } else {
      gridY = atoi(token);
    }
    counter++;

    token = strtok(NULL, x);
  }

  checkFormattingSudoku(fp, size);

  fseek(fp, (dimensionLength1 + dimensionLength2), SEEK_SET);

  for (int i = 0; i < size; i++) {
    save[i].gridX = gridX; // struct
    save[i].gridY = gridY; // struct
  }

  int allNums[size];

  for (int i = 0; i < size; i++) {
    save[i].allNums = allNums;
  }

  matrix = (int **)malloc(size * sizeof(int *));
  for (int i = 0; i < size; i++) {
    matrix[i] = (int *)malloc(size * sizeof(int));
  }

  for (int i = 0; i < size; i++) {
    fgets(sudokuLine, 5000, fp);

    token = strtok(sudokuLine, space);
    int matrixPos = 0;

    while (token != NULL && strcmp(token, "\n") != 0) {

      matrixValue = atoi(token);
      matrix[i][matrixPos] = matrixValue;

      matrixPos++;

      token = strtok(NULL, space);
    }
  }

  for (int j = 0; j < size; j++) {

    save[j].iteraction = j;

    for (int i = 1; i <= size; i++) {
      save[j].allNums[i - 1] = i;
    }

    if (pthread_create(&threads[j], NULL, (void *)checkLines,
                       (void *)&save[j]) != 0) {
      perror("Deu merda na criação\n");
      return 1;
    }
  }

  for (int j = 0; j < size; j++) {
    if (pthread_join(threads[j], NULL) != 0) {
      return 2;
    }
  }

  if (failCheck > 0) {
    fputs("FAIL", output);
    exit(1);
  }

  for (int i = 0; i < size; i++) {
    save[i].currentGridX = 0;
    save[i].currentGridY = 0;
  }

  int yCounter = 0;
  int xCounter = 0;

  if (gridY >= gridX) {

    for (int a = 0; a < gridX; a++) {

      save[a].currentGridY += yCounter;
      yCounter += gridY;
    }

    for (int i = 0; i < gridX; i++) {

      if (pthread_create(&threadsGrid[i], NULL, (void *)checkBoxesX,
                         (void *)&save[i]) != 0) {
        return 1;
      }
    }

    for (int j = 0; j < gridX; j++) {
      if (pthread_join(threadsGrid[j], NULL) != 0) {
        return 2;
      }
    }

  } else {

    for (int a = 0; a < gridY; a++) {

      save[a].currentGridX += xCounter;
      xCounter += gridX;
    }

    for (int i = 0; i < gridY; i++) {

      if (pthread_create(&threadsGrid[i], NULL, (void *)checkBoxesY,
                         (void *)&save[i]) != 0) {
        return 1;
      }
    }

    for (int j = 0; j < gridY; j++) {
      if (pthread_join(threadsGrid[j], NULL) != 0) {
        return 2;
      }
    }
  }

  if (failCheck > 0) {
    fputs("FAIL", output);
    exit(1);
  }

  fclose(fp);

  fputs("SUCCESS", output);

  fclose(output);

  return 0;
}

void *checkLines(void *pointer) {

  values *save;
  save = (values *)pointer;

  int size = save->size;
  int iteraction = save->iteraction;
  int allNums[size];

  for (int i = 0; i < size; i++) {
    allNums[i] = save->allNums[i];
  }

  for (int j = 0; j < size; j++) {
    for (int k = 0; k < size; k++) {
      if (matrix[iteraction][k] == allNums[j]) {
        allNums[j] = -1;
      }
    }
  }

  for (int i = 0; i < size; i++) {
    if (allNums[i] != -1) {
      failCheck++;
    }
  }

  for (int i = 1; i <= size; i++) {
    allNums[i - 1] = i;
  }

  for (int k = 0; k < size; k++) {
    for (int j = 0; j < size; j++) {
      if (matrix[j][iteraction] == allNums[k]) {
        allNums[k] = -1;
      }
    }
  }

  for (int i = 0; i < size; i++) {
    if (allNums[i] != -1) {
      failCheck++;
    }
  }

  for (int i = 1; i <= size; i++) {
    allNums[i - 1] = i;
  }

  return NULL;
}

void *checkBoxesX(void *pointer) {

  values *save;
  save = (values *)pointer;

  int size = save->size;
  int currentGridX = save->currentGridX;
  int currentGridY = save->currentGridY;
  int gridX = save->gridX;
  int gridY = save->gridY;
  int allNums[size];

  for (int i = 0; i < size; i++) {
    allNums[i] = save->allNums[i];
  }

  for (int a = 0; a < gridY; a++) {

    for (int cont = 0; cont < size; cont++) {
      for (int i = currentGridX; i < (currentGridX + gridX); i++) {
        for (int j = currentGridY; j < (currentGridY + gridY); j++) {
          if (matrix[i][j] == allNums[cont]) {
            allNums[cont] = -1;
          }
        }
      }
    }

    for (int i = 0; i < size; i++) {
      if (allNums[i] != -1) {
        failCheck++;
      }
    }

    for (int i = 0; i < size; i++) {
      allNums[i] = save->allNums[i];
    }

    currentGridX += gridX;
  }

  return NULL;
}

void *checkBoxesY(void *pointer) {

  values *save;
  save = (values *)pointer;

  int size = save->size;
  int currentGridX = save->currentGridX;
  int currentGridY = save->currentGridY;
  int gridX = save->gridX;
  int gridY = save->gridY;
  int allNums[size];

  for (int i = 0; i < size; i++) {
    allNums[i] = save->allNums[i];
  }

  for (int a = 0; a < gridX; a++) {

    for (int cont = 0; cont < size; cont++) {
      for (int i = currentGridX; i < (currentGridX + gridX); i++) {
        for (int j = currentGridY; j < (currentGridY + gridY); j++) {
          if (matrix[i][j] == allNums[cont]) {
            allNums[cont] = -1;
          }
        }
      }
    }

    for (int i = 0; i < size; i++) {
      if (allNums[i] != -1) {
        failCheck++;
      }
    }

    for (int i = 0; i < size; i++) {
      allNums[i] = save->allNums[i];
    }

    currentGridY += gridY;
  }

  return NULL;
}

void checkFormattingSudokuSize(FILE *fp) {

  int a, b, cont = 0, length, findX = 0;
  char sudokuDim[500], x[2] = "x", *token;

  fgets(sudokuDim, 500, fp);

  length = strlen(sudokuDim);

  for (int i = 0; i < length; i++) {
    if (sudokuDim[i] == 'x') {
      findX++;
    }else if(sudokuDim[i] == ' '){
      printf("File out of format\n");
      exit(1);
    }
  }

  if (findX == 0) {
    printf("File out of format\n");
    exit(1);
  }

  token = strtok(sudokuDim, x);

  if (token == NULL) {
    printf("File out of format\n");
    exit(1);
  }

  while (token != NULL) {

    length = strlen(token);
    for (int j = 0; j < length; j++) {
      if ((token[j] < 48 || token[j] > 57) && token[j] != 10) {
        printf("File out of format\n");
        exit(1);
      }
    }

    a = atoi(token);

    if (cont == 0) {
      b = a;
    }

    if (cont > 1) {
      printf("File out of format\n");
      exit(1);
    }
    cont++;

    token = strtok(NULL, x);
  }

  if (a != b) {
    printf("File out of format\n");
    exit(1);
  }
}

void checkFormattingGridSize(FILE *fp, int size) {

  int a, b, cont = 0, length, findX = 0;
  char gridDim[500], x[2] = "x", *token;

  fgets(gridDim, 500, fp);

  length = strlen(gridDim);

  for (int i = 0; i < length; i++) {
    if (gridDim[i] == 'x') {
      findX++;
    }else if(gridDim[i] == ' '){
      printf("File out of format\n");
      exit(1);
    }
  }

  if (findX == 0) {
    printf("File out of format\n");
    exit(1);
  }

  token = strtok(gridDim, x);

  if (token == NULL) {
    printf("File out of format\n");
    exit(1);
  }

  while (token != NULL) {

    length = strlen(token);
    for (int j = 0; j < length; j++) {
      if ((token[j] < 48 || token[j] > 57) && token[j] != 10) {
        printf("File out of format\n");
        exit(1);
      }
    }

    a = atoi(token);

    if (cont == 0) {
      b = a;
    }

    if (cont > 1) {
      printf("File out of format\n");
      exit(1);
    }
    cont++;

    token = strtok(NULL, x);
  }

  if ((a * b) != size) {
    printf("File out of format\n");
    exit(1);
  }
}

void checkFormattingSudoku(FILE *fp, int size) {

  char line[MAXSIZE], *token, space[2] = " ";
  int lineNums, spaceCounter;

  for(int i=0; i<size; i++){
    lineNums = 0;
    fgets(line, MAXSIZE, fp);
    spaceCounter = 0;

    for(int a=0; a<strlen(line); a++){
      if(line[a] == ' '){
        spaceCounter++;
      }
    }

    if(spaceCounter != size -1){
      printf("File out of format\n");
      exit(1);
    }
    
    if(atoi(line) <1 || atoi(line) > size){
      printf("File out of format\n");
      exit(1);
    }

    token = strtok(line, space);

    while (token != NULL && strcmp(token, "\n") != 0) {

      for(int j=0; j<strlen(token); j++){
        if((token[j] < 48 || token[j] > 57) && token[j] != 10){
          printf("File out of format\n");
          exit(1);
        }
      }

      if(atoi(token) <1 || atoi(token) > size){
        printf("File out of format\n");
        exit(1);
      }

      lineNums++;

      token = strtok(NULL, space);
    }

    if(lineNums != size){
      printf("File out of format\n");
      exit(1);
    }
  }
  
  if(fgets(line, MAXSIZE, fp) != NULL){
    printf("File out of format\n");
    exit(1);
  }
  
}