#include <mpi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main (int argc, char *argv[]) {

 struct stat info;
 stat(argv[1], &info);

 //INITIALIZE.
 MPI_Init(&argc, &argv);

 int world_size, rank;
 MPI_Comm_size(MPI_COMM_WORLD, &world_size);
 MPI_Comm_rank(MPI_COMM_WORLD, &rank);

 MPI_Barrier(MPI_COMM_WORLD);
 double start = MPI_Wtime();

 MPI_File file;

 if (MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &file) < 0) {
  perror("");
  return -1;
 }

 int numWords = 0, bytesToRead;
 int remainingBytes = info.st_size % world_size;
 //LOCATE POINTER.
 if (rank < remainingBytes) {

  bytesToRead = (int) ((info.st_size/world_size) +1);
  int displacement = bytesToRead * rank;
  if (MPI_File_seek(file, displacement, MPI_SEEK_SET) < 0) {
   perror("");
   return -1;
  }
 }
 else {

  bytesToRead = (int) (info.st_size/world_size);
  int displacement = bytesToRead * (rank - world_size);
  if (MPI_File_seek(file, displacement, MPI_SEEK_END) < 0) {
   perror("");
   return -1;
  }
 }

 //READ.
 char *character = malloc(sizeof(char));
 //char *prev = malloc(sizeof(char));
 int i, limit = (rank+1) * bytesToRead;
 for (i = (bytesToRead*rank); i < limit; i++) {

  if (MPI_File_read(file, character, 1, MPI_CHAR, MPI_STATUS_IGNORE) < 0) {
   perror("");
   return -1;
  }
  //printf("%c\n", *character);
  if ((((int)*character) == 32) || (((int)*character) == 10)) //&& (((int)*prev) != 10))
   numWords++;
  memset(character, 0, sizeof(char));
  //memset(prev, 0, sizeof(char));
  //prev = character;
 }

 //GATHER.
 int *pieces = malloc(sizeof(int) * world_size), total = 0;
 MPI_Gather(&numWords, 1, MPI_INT, pieces, 1, MPI_INT, 0, MPI_COMM_WORLD);

 //ACHIEVE TOTAL NUMBER OF WORDS.
 if (rank == 0) {
  for (i = 0; i < world_size; i++) total += pieces[i];
  printf("Total number of words: %d\n", (total+1));
 }

 MPI_File_close(&file);

 //CONCLUDE.
 MPI_Barrier(MPI_COMM_WORLD);
 double end = MPI_Wtime();
 printf("Execution time: %f\n", (end - start));
 MPI_Finalize();

 return 0;
}
