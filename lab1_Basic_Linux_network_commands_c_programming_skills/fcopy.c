#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define BUF_SIZE 100

void copyWithFiles(const char *source, const char *dest) {
  FILE *sourceFile;
  FILE *destFile;
  sourceFile = fopen(source, "r");
  if (!sourceFile) {
    printf("Error reading source file: %s\n", source);
    exit(1);
  }
  destFile = fopen(dest, "w");
  if (!destFile) {
    printf("Error reading or creating destination file: %s\n", dest);
    exit(1);
  }

  clock_t start, end;
  double cpu_time_used;
  start = clock();

  size_t bytesRead;
  char buf[BUF_SIZE];
  do {
    bytesRead = fread(&buf, 1, sizeof(buf), sourceFile);
    fwrite(&buf, 1, bytesRead, destFile);
  } while (bytesRead);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("File copied in CPU time: %f\n", cpu_time_used);

  fclose(sourceFile);
  fclose(destFile);
}

void copyWithSystem(const char *source, const char *dest) {
  int sourceFD = open(source, O_RDONLY);
  if (sourceFD < 0) {
    printf("Error reading source file: %s\n", source);
    exit(1);
  }
  int destFD = open(dest, O_WRONLY | O_CREAT, S_IRWXU);
  if (destFD < 0) {
    printf("Error reading or creating destination file: %s\n", dest);
    exit(1);
  }

  clock_t start, end;
  double cpu_time_used;
  start = clock();

  size_t bytesRead;
  char buf[BUF_SIZE];
  do {
    bytesRead = read(sourceFD, &buf, sizeof(buf));
    write(destFD, &buf, bytesRead);
  } while (bytesRead);

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("File copied in CPU time: %f\n", cpu_time_used);

  close(sourceFD);
  close(destFD);
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Correct usage: %s source dest mode\n", argv[0]);
    exit(1);
  }

  if (strcmp(argv[3], "file") == 0) {
    copyWithFiles(argv[1], argv[2]);
  } else if (strcmp(argv[3], "system") == 0) {
    copyWithSystem(argv[1], argv[2]);
  } else {
    printf("Mode should be \"file\" or \"system\"; received: %s\n", argv[3]);
  }
}
