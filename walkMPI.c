#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <mpi.h>


int listDirectories (char *basePath, const int root, int *count)
{
   int i, j, stringSize;
   char *path;

   struct dirent **fileList;
   int numOfFiles;

   numOfFiles = scandir(basePath, &fileList, NULL, NULL);

   for (i = 0; i < numOfFiles; i++)
   {
      if (strcmp (fileList[i] -> d_name, ".") != 0 && strcmp (fileList[i] -> d_name, "..") != 0)
      {
         for(j = 0; j <= root; j++)
            printf("|");

         printf("|-%s\n", fileList[i] -> d_name);
         fflush(stdout);

         stringSize = strlen (basePath) + strlen (fileList[i] -> d_name) + 2;
         path = (char*)malloc (stringSize * sizeof (char));

         strcpy(path, basePath);

         strcat(path, "/");

         strcat(path, fileList[i] -> d_name);

         (*count)++;
         listDirectories(path, root + 1, count);
         free(path);
      }
   }

   for (i = 0; i < numOfFiles; i++)
      free(fileList[i]);

   return 0;
}

int main (int argc, char *argv[])
{
   
   char *path;
 
   double elapsed_time;

   int rank;
   int number_processes;

   int count = 0;
   int globalCount = 0;
     
   struct dirent **fileList;
   int numOfFiles;
   int i, j, stringSize;
  
   numOfFiles = scandir(".", &fileList, NULL, NULL);
   
   
   MPI_Init (&argc, &argv);
   MPI_Comm_rank (MPI_COMM_WORLD, &rank);
   MPI_Comm_size (MPI_COMM_WORLD, &number_processes);
   MPI_Barrier (MPI_COMM_WORLD);
   elapsed_time = - MPI_Wtime();

   printf("rank: %d, processes: %d\n", rank, number_processes);
   fflush(stdout);     

      for (j = rank; j < numOfFiles; j += number_processes)
      {
         if (strcmp((fileList[j] -> d_name), ".") != 0 && strcmp((fileList[j] -> d_name), "..") != 0)
         {   
            stringSize = strlen (fileList[j] -> d_name) + 3;
            path = (char*)malloc (stringSize * sizeof (char));
            strcpy(path, "./");
            strcat(path, (fileList[j] -> d_name));
    
            printf("|-%s\n", (fileList[j] -> d_name));
            fflush(stdout);
            count++;
 
            listDirectories(path, 0, &count);
            free(path);
         }
      }

   for (i = 0; i < numOfFiles; i ++)
      free(fileList[i]);

   free(fileList);

   MPI_Reduce (&count, &globalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
   
   elapsed_time += MPI_Wtime();

   if(!rank)
   {
      printf("Execution time: %8.6f\n\n", elapsed_time);
      fflush(stdout);
   }

   MPI_Finalize();

   printf("Rank : %d, count: %d\n", rank, count); 

   if (!rank)
      printf("Number of total directories: %d\n", globalCount);

   return 0;
}
