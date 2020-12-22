#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "timer.h"
#include <math.h>

long long int N; //dimensão do vetor de entrada
int nthreads; //número de threads
double *serie; //vetor com N elementos da série Madhava–Leibniz

/* função das threads
 * Nessa função optei por realizar as somas de forma invertida com o intuito de tentar minimizar o erro, pois, assim,
 * cada thread começa somando números menores. O mesmo raciocínio foi utilizado para o cálculo sequencial.*/
void * somaSerie(void * arg) {
    long int id = (long int) arg; //identificador da thread
    double *somaLocal; //variável local da soma de elementos
    long int tamBloco = N / nthreads; //tamanho do bloco de cada thread
    long int fim = N - (id * tamBloco); //elemento final do bloco da thread
    long int ini; //elemento inicial do bloco da thread

    somaLocal = (double*) malloc(sizeof(double));
    if(somaLocal == NULL) {
        fprintf(stderr, "Erro ao executar malloc();\n"); exit(1);
    }
    * somaLocal = 0;
    if(id == nthreads - 1)
        ini = 0;
    else
        ini = fim - tamBloco;

    //soma os elementos do bloco das threads
    for(long int i = fim - 1; i >= ini; i--) {
        *somaLocal += serie[i];
    }
    pthread_exit((void *) somaLocal);
}

int main(int argc, char *argv[]) {

    //valida os parâmetros de entrada (dimensão do vetor da série, número de threads)
    if(argc < 3) {
        fprintf(stderr, "Digite: %s <dimensão da série> <número de threads>", argv[0]);
        return 1;
    }

    double somaSeq = 0; //soma sequencial
    double somaConc = 0; //soma concorrente
    double ini, fim; //tomada de tempo
    pthread_t *tid; //identificadores das threads no sistema
    double *retorno; //valor de retorno das threads

    N = atoll(argv[1]);
    nthreads = atoi(argv[2]);
    //aloca o vetor da série
    serie = (double*) malloc(sizeof(double) * N);
    if(serie == NULL) {
        fprintf(stderr, "Erro ao executar malloc();\n");
        return 2;
    }
    //calcula os elementos da série Madhava–Leibniz
    for(long int i = 0; i < N; i++)
        if(i%2 == 0)
            serie[i] = (double) 1 / (double) (2 * i + 1);
        else
            serie[i] = (double) - 1 / (double) (2 * i + 1);

    //cálculo sequencial
    GET_TIME(ini);
    for(long int i = N - 1; i >= 0; i--) //for invertido para minimizar erros das operações
        somaSeq += serie[i];
    GET_TIME(fim);
    printf("Tempo sequencial: %lf\n", fim - ini);

    //cálculo concorrente
    GET_TIME(ini);
    tid = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    if(tid == NULL) {
        fprintf(stderr, "Erro ao executar malloc();\n");
        return 2;
    }
    //cria as threads
    for(long int i = 0; i < nthreads; i++) {
        if (pthread_create(tid + i, NULL, somaSerie, (void *) i)) {
            fprintf(stderr, "Erro ao executar pthread_create();\n");
            return 3;
        }
    }
    //aguarda o término das threads
    for(long int i = 0; i < nthreads; i++) {
        if(pthread_join(*(tid+i), (void **) &retorno)) {
            fprintf(stderr, "Erro ao executar pthread_join();\n");
            return 3;
        }
        //soma global
        somaConc += *retorno;
        free(retorno);
    }
    GET_TIME(fim);
    printf("Tempo concorrente: %lf\n", fim - ini);

    //exibir os resultados
    printf("%.20f  -> Erro sequencial.\n", fabs(M_PI - 4*somaSeq));
    printf("%.20f  -> Erro concorrente.\n", fabs(M_PI - 4*somaConc));

    //libera as areas de memória alocadas
    free(serie);
    free(tid);

    return 0;
}
