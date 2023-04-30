// =================================================================
// File: cruzandoUnPuente.cpp
// Author: María Fernanda Moreno Gómez A01708653
//         Uri Jared Gopar Morales  A01709413
// Description: Este archivo contiene el código para obtener los autos que cruzan un puente
//              los cuales deben de pasar 3 a la vez XD
//              To compile: g++ -std=c++11 cruzandoUnPuente.cpp -o app   y después  ./app
// =================================================================
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

// Variables globales
int north = 0;
int south = 0;
int numcoches=20;
int cochesfinales=0;

//variable global
const int vehiculos = 3;
pthread_mutex_t puente = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t norte = PTHREAD_COND_INITIALIZER;
pthread_cond_t sur = PTHREAD_COND_INITIALIZER;

// Funciones
void enpuente(int direction) {
    pthread_mutex_lock(&puente);
    if (direction == 0) { // Norte a Sur
        while (south > 0 ||  north== vehiculos) {
            pthread_cond_wait(&norte, &puente);
        }
        north++;
    } else { // Sur a Norte
        while (north > 0 || south == vehiculos) {
            pthread_cond_wait(&sur, &puente);
        }
        south++;
        
    }
    pthread_mutex_unlock(&puente);
}

void salio(int direction) {
    pthread_mutex_lock(&puente);
    if (direction == 0) { // Norte a Sur
        north--;
        if (north == 0) {
            pthread_cond_broadcast(&sur);
        }
    } else { // Sur a Norte
        south--;
        if (south == 0) {
            pthread_cond_broadcast(&norte);
        }
    }
    cochesfinales ++;
    pthread_mutex_unlock(&puente);
}

void cruce(int direction) {
    printf("Grupo de 3 coches que pasan por el puente: %d\n", numcoches - cochesfinales);
    printf("Los carros van de: %s \n", direction == 0 ? "N a S" : "S a N");
    sleep(rand() % 3 + 1);
}

void *OneVehicle(void *arg) {
    int direction = *((int *)arg);
    enpuente(direction);
    cruce(direction);
    salio(direction);
    return NULL;
}

int main() {
    int numcoches = 20;
    pthread_t threads[numcoches];
    int directions[numcoches];

    for (int i = 0; i < numcoches; i++) {
        directions[i] = rand() % 2;
        pthread_create(&threads[i], NULL, OneVehicle, &directions[i]);
    }

    for (int i = 0; i < numcoches; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}