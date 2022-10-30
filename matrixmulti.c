#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <time.h>
#include <sys/time.h>

/*
  Autor: Josimar Viana
  Multiplicacao de matriz com paralelismo
*/

// Numero de linhas e colunas da matriz
#define N 4

MPI_Status status;

// Matrizes principais
double matrix_a[N][N],matrix_b[N][N],matrix_c[N][N];

int main(int argc, char **argv)
{
  int processCount, processId, slaveTaskCount, source, dest, rows, offset;

  struct timeval start, stop;

// Inicializacao ambiente MPI
  MPI_Init(&argc, &argv);
// Cada processo tem um unico ID (rank)
  MPI_Comm_rank(MPI_COMM_WORLD, &processId);
// Numero de processos na variavel -> processCount
  MPI_Comm_size(MPI_COMM_WORLD, &processCount);

// Numero de tarefas na variavel -> slaveTaskCount
  slaveTaskCount = processCount - 1;

// Processo principal
 if (processId == 0) {
	
// Matriz A e Matriz B sao preenchidas com numeros aleatorios
    srand ( time(NULL) );
    for (int i = 0; i<N; i++) {
      for (int j = 0; j<N; j++) {
        matrix_a[i][j]= rand()%10;
        matrix_b[i][j]= rand()%10;
      }
    }
	
  printf("\n\t\tMatriz - Multiplicacao de Matriz usando MPI\n");

// Imprime Matriz A
    printf("\nMatriz A\n\n");
    for (int i = 0; i<N; i++) {
      for (int j = 0; j<N; j++) {
        printf("%.0f\t", matrix_a[i][j]);
      }
	    printf("\n");
    }

// Imprime Matriz B
    printf("\nMatriz B\n\n");
    for (int i = 0; i<N; i++) {
      for (int j = 0; j<N; j++) {
        printf("%.0f\t", matrix_b[i][j]);
      }
	    printf("\n");
    }

// Determina o numero de linhas da Matriz A e envia para cada processo filho
    rows = N/slaveTaskCount;
// Variavel que determina o ponto de inicio de cada linha e envia para o processo filho
    offset = 0;

// Os calculos sao enviados aos processos filhos
// O tag de cada mensagem eh 1
    for (dest=1; dest <= slaveTaskCount; dest++)
    {
      // Deslocamento da Matrix A
      MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      // Numero de linhas
      MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
      // Envia linhas da Matriz A para os processos filho
      MPI_Send(&matrix_a[offset][0], rows*N, MPI_DOUBLE,dest,1, MPI_COMM_WORLD);
      // Envia Matriz B
      MPI_Send(&matrix_b, N*N, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
      
      // O deslocamento eh realizado de acordo com o numero de linhas para cada processo
      offset = offset + rows;
    }

// O processo raiz espera até que cada processo filho termine o calculo
    for (int i = 1; i <= slaveTaskCount; i++)
    {
      source = i;
      // Recebe o resultado de cada processo filho
      MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      // Recebe o numero de linhas que foi processado em cada filho
      MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
      // Armazena as linhas calculadas na Matriz C
      MPI_Recv(&matrix_c[offset][0], rows*N, MPI_DOUBLE, source, 2, MPI_COMM_WORLD, &status);
    }

// Imprime a Matriz C - Resultado
    printf("\nResultado Matriz C = Matriz A * Matriz B:\n\n");
    for (int i = 0; i<N; i++) {
      for (int j = 0; j<N; j++)
        printf("%.0f\t", matrix_c[i][j]);
      printf ("\n");
    }
    printf ("\n");
  }

// Processos filhos
  if (processId > 0) {

    // Define o ID de cada processo filho
    source = 0;

    // O processo filho aguarda a mensagem do processo principal

    // O processo filho recebe os valores enviados pelo processo principal
    MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
    // O processo filho recebe o número de linhas enviadas pelo processo principal 
    MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);
    // O processo filho recebe uma parte da Matrix A que foi atribuída pelo principal
    MPI_Recv(&matrix_a, rows*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);
    // O processo filho recebe a Matrix B
    MPI_Recv(&matrix_b, N*N, MPI_DOUBLE, source, 1, MPI_COMM_WORLD, &status);

    // Mutiplicacao

    for (int k = 0; k<N; k++) {
      for (int i = 0; i<rows; i++) {
        // Valor inicial da soma da linha
        matrix_c[i][k] = 0.0;
        // O elemento(i, j) da Matriz A eh multiplicado pelo elemento(j, k) da Matriz B
        for (int j = 0; j<N; j++)
          matrix_c[i][k] = matrix_c[i][k] + matrix_a[i][j] * matrix_b[j][k];
      }
    }

    // O resultado eh calculado e enviado de volta para o process principal
    MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    // Numero de linha que foi processado
    MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
    // A matriz resultante com as linhas calculadas eh enviadada para o processo principal
    MPI_Send(&matrix_c, rows*N, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD);
  }

  //O programa eh finalizado
  MPI_Finalize();
}