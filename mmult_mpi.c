/** 
 * Incomplete program to multiply a matrix times a matrix using both
 * MPI to distribute the computation among nodes and OMP
 * to distribute the computation among threads.
 */

#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#define min(x, y) ((x) < (y) ? (x) : (y))

#include "mat.h"

int main(int argc, char *argv[])
{
    int nrows, ncols;
    double *aa; /* the A matrix */
    double *bb; /* the B matrix */
    double *b;
    double *cc1; /* A x B computed using the omp-mpi code you write */
    double *cc2; /* A x B computed using the conventional algorithm */
    int numbersent = 0;
    int send;
    double recive;
    int rowNumber; //for recive
    double *buffer;
    int myid, numprocs;
    double starttime, endtime;
    MPI_Status status;

    /* insert other global variables here */

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if (argc > 1)
    {
        nrows = atoi(argv[1]);
        ncols = nrows;

        if (myid == 0)
        {
            // Master Code goes here
            aa = gen_matrix(nrows, ncols);
            bb = gen_matrix(ncols, nrows);
            cc1 = malloc(sizeof(double) * nrows * nrows);
            starttime = MPI_Wtime();

            //one row of a
            buffer = malloc(sizeof(double) * nrows);
            //whole b matix
            MPI_Bcast(bb, nrows * ncols, MPI_DOUBLE, 0, MPI_COMM_WORLD);

            //send a row by row
            for (int i = 0; i < min(numprocs-1, nrows); i++)
            {
                for (int j = 0; j < ncols; j++)
                {
                    buffer[j] = aa[i * ncols + j];
                }
                MPI_Send(buffer, ncols, MPI_DOUBLE, i + 1, numbersent + 1, MPI_COMM_WORLD);
                numbersent++;
            }
            //recive
            free(buffer);
            buffer = malloc(sizeof(double) * ncols);
            for (int i = 0; i < nrows; i++)
            {
                MPI_Recv(buffer, ncols, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &status);
                send = status.MPI_SOURCE;
                rowNumber = status.MPI_TAG;

                //mpi calculate reslut put in cc1
                for (int j = 0; j < ncols; j++)
                {
                    cc1[(rowNumber - 1) * ncols + j] = buffer[j];
                }

                //from omp_mpi.c
                if (numbersent < nrows)
                {
                    for (int j = 0; j < ncols; j++)
                    {
                        buffer[j] = aa[numbersent * ncols + j];
                    }
                    MPI_Send(buffer, ncols, MPI_DOUBLE, send, numbersent + 1,
                             MPI_COMM_WORLD);
                    numbersent++;
                }
                else
                {
                    MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, send, 0, MPI_COMM_WORLD);
                }
            }

            /* Insert your master code here to store the product into cc1 */
            endtime = MPI_Wtime();
            printf("%f\n", (endtime - starttime));
            cc2 = malloc(sizeof(double) * nrows * nrows);
            mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
            compare_matrices(cc2, cc1, nrows, nrows);
        }
        else
        {
            // Slave Code goes here
            b = malloc(sizeof(double) * ncols * nrows);
            //recive bcast.
            MPI_Bcast(b, nrows * ncols, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            //form omp_mpi.c
            if (myid <= nrows)
            {
                while (1)
                {
                    double a[ncols];
                    double answer[ncols];
                    // row of a matrix
                    MPI_Recv(&a, ncols, MPI_DOUBLE, 0,
                             MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    if (status.MPI_TAG == 0)
                    {
                        break;
                    }
                    rowNumber = status.MPI_TAG;

                    for (int i = 0; i < ncols; i++)
                    {
                        double sum = 0.0;
                        for (int j = 0; j < nrows; j++)
                        {
                            sum += a[j] * b[j * nrows + i];
                        }
                        answer[i] = sum;
                    }
                    MPI_Send(answer, ncols, MPI_DOUBLE, 0, rowNumber,MPI_COMM_WORLD);
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Usage matrix_times_vector <size>\n");
    }
    MPI_Finalize();
    return 0;
}
