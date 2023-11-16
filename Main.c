#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1

#define SEED 1

#pragma comment(lib, "pthreadVC2.lib") 

#define HAVE_STRUCT_TIMESPEC 
#define COLUNAS 10000
#define LINHAS  10000

#define MACRO_LINHA 250 // tamanho da linha do macro bloco
#define MACRO_COLUNA 250 // tamanho da coluna do macro bloco
#define NUM_THREADS 8
#define qtdMacro  (LINHAS * COLUNAS) / (MACRO_LINHA * MACRO_COLUNA) // quantidade de macro blocos da matriz


#include <pthread.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <time.h> 
#include <math.h> 


int cont_primo = 0 ;
int valor;
int **matrix;
int indice_global = 0; // irá até a quantidade de macro blocos totais
void *checando_primos(void *param);


pthread_mutex_t mutex;
pthread_mutex_t mutex_primo;


typedef struct {
    int minimoLinha; 
    int minimoColuna; 
    int maximoLinha; 
    int maximoColuna; 
} MacroBlocos;

MacroBlocos* MacroBloco;

// Cria e aloca os MacroBlocos da Matriz
void criaMacroBlocos()
{
    MacroBloco = malloc(sizeof(MacroBlocos) * qtdMacro); // aloca memoria para o macro bloco
    

    int i = 0, j = 0, cont = 0, x = 0, cont_primo = 0;
    
    
        while (cont < qtdMacro) {
            
            MacroBloco[j].minimoLinha = i * MACRO_LINHA ; 
            MacroBloco[j].maximoLinha = MacroBloco[j].minimoLinha + MACRO_LINHA; 

            MacroBloco[j].minimoColuna = x * MACRO_COLUNA ; 
            MacroBloco[j].maximoColuna = MacroBloco[j].minimoColuna + MACRO_COLUNA; 

            x++;
            if ( MacroBloco[j].maximoColuna == COLUNAS ) // caso o macrobloco tenha chegado no final da linha da matriz, 
                                                         //pula pra proxima. De acordo com o tamanho da linha do macro bloco é claro.
            {
                x = 0; // reseta a coluna
                i++;   // incrimenta o tamanho da linha do macro bloco
            }
            cont++;
            j++;
        }
        
       
}


int ehPrimo(int valor)
{
    if (valor <= 1) return 0;

    for (int i= 2; i <=sqrt(valor); i++)
    {
        if ((valor % i) == 0) return 0;
    }

    return 1;
    
}


void *checando_primos(void *param) {
    int ajuda = 0;
    int cont_aux = 0 ;
    int indice_local = 0;
   
    
    while ( indice_global < qtdMacro)
    {
        cont_aux = 0;
        pthread_mutex_lock(&mutex);
        indice_local = indice_global;
        indice_global++;
        pthread_mutex_unlock(&mutex);


        for (int aux = MacroBloco[indice_local].minimoLinha; aux < MacroBloco[indice_local].maximoLinha; aux++) {
            for (int aux1 = MacroBloco[indice_local].minimoColuna; aux1 < MacroBloco[indice_local].maximoColuna; aux1++) {

                if (ehPrimo(matrix[aux][aux1]) == 1) {
                    cont_aux++;
                }
            }
        }

        pthread_mutex_lock(&mutex_primo);
        cont_primo += cont_aux;
        pthread_mutex_unlock(&mutex_primo);
    }
    
}

int main(int argc, char* argv[])
{
    pthread_t thread[NUM_THREADS];
    srand(SEED);
    
    
    
    clock_t start_seq = 0;
    clock_t fim_seq = 0;
    double total_seq = 0;

    clock_t start_multi = 0;
    clock_t fim_multi = 0;
    double total_multi = 0;


    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex_primo, NULL);

    
    matrix = (int**)malloc(sizeof(int*)  * LINHAS);
    for (int i = 0; i < LINHAS; i++)
    {
        matrix[i] = (int*)malloc(sizeof(int) * COLUNAS);
    }

    for (int i = 0; i < LINHAS; i++) {
        for (int j = 0; j < COLUNAS; j++)
        {
            matrix[i][j] = rand() % 320000;
        }
    }
    
    start_seq = clock();


    for (int lin = 0; lin < LINHAS; lin++) {
        for (int col = 0; col < COLUNAS; col++) {

            if (ehPrimo(matrix[lin][col]) == 1) {
                cont_primo++;
            }
        }
    }
    fim_seq = clock();
    total_seq = (double)(fim_seq - start_seq) / CLOCKS_PER_SEC;
    

    printf("Primos no sequencial: %d\n", cont_primo);
    printf("tempo no sequencial: %f sec \n", total_seq);
    printf("tempo em minutos: %f min \n", total_seq/60);
    printf(" --------------------------------------------\n\n");
    
    start_multi = clock();
    cont_primo = 0;


    criaMacroBlocos();

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&thread[i], NULL, checando_primos, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    fim_multi = clock();
    total_multi = (double)(fim_multi - start_multi) / CLOCKS_PER_SEC;

    printf("Primos na paralea: %d\n", cont_primo);
    printf("Tempo na paralela: %f sec\n", total_multi);
    printf("temp em minutos: %f min\n", total_multi/60);
    printf("qtd macro: %d\n", qtdMacro);

    for (int i = 0; i < LINHAS; i++)
    {
        free(matrix[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex_primo);


    return 0;
}

