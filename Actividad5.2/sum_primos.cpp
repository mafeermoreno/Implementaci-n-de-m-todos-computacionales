// ==========================================================================
// File: sum_primos.cpp
// Author: María Fernanda Moreno Gómez A01708653
//         Uri Jared Gopar Morales  A01709413
// Description: Este archivo contiene el código para obtener toda la suma de los números
//              primos menores a 5,000,000 (cinco millones) de manera secuencial y de
//              manera paralela. Ambos resultados deben dar 838,596,693,108
//              To compile: g++ -std=c++11 sum_primos.cpp -lpthread -o app   y después  ./app
// ===========================================================================================

#include <iostream>     //Entrada y salida de datos
#include <pthread.h>    //Para trabajar con hilos en c++
#include "utils.h"      //Para la función para contabilizar el tiempo

using namespace std;

//Guarda el inicio, fin y el resultado de la suma del rango de números primos
struct ThreadData {
    int start;
    int end;
    long long result;
};

/*
Recibe un entero, empieza el ciclo en 2 (porque el 1 naturalmente es primo). Mientras el cuadrado del número sea menor 
o igual a n, el ciclo aumenta 1 y se checa si n es divisible entre j (residuo 0), si esto es correcto, no es primo (false) 
ya que un número primo nada más es divisible entre sí mismo y entre el 1.
*/
bool es_primo(int n) {
    for (int j = 2; j * j <= n; j++) {
        if (n % j == 0) {
            return false;
        }
    }
    return true;
}

/*
Calcular la suma de los números primos en el rango especificado. Llama al puntero ThreadData para recibir los rangos a analizar, y 
después, suma para guardar la suma de los números primos. Después, se hace un ciclo para recorrer del inicio del rango al final, 
donde cada número del rango se checa si es primo con la función de es_primo que anteriormente declaramos, si es primo, se suma con 
el número anterior que es primo. Al final, a suma se le da el valor de resultado de la estructura, para poder llamarla en otras 
partes del programa.
*/
void* suma_primos(void* args) {
    ThreadData* data = (ThreadData*)args;
    long long suma = 0;

    for (int i = data->start; i < data->end; i++) {
        if (es_primo(i)) {
            suma += i;
        }
    }

    data->result = suma;
    return nullptr;
}

/*
Cálculo de la suma de números primos menor al límite dado, de manera secuencial, es decir, como habitualmente lo hacemos 
(sin hilos). donde se comienza desde el dos hasta que el número sea menor al límite y se aumenta 1 cada ciclo. Mandamos a 
llamar la función es_primo para checar si es primo o no, si es falso, se rompe el ciclo, pero si es verdad, se suma este número 
a la suma de primos.
*/
long long suma_primos_secuencial(int n) {
    long long suma = 0;
    bool es_primo;

    for (int i = 2; i < n; i++) {
        es_primo = true;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                es_primo = false;
                break;
            }
        }

        if (es_primo) {
            suma += i;
        }
    }

    return suma;
}

int main() {
    //Primos menores a 5 millones
    const int limite = 5000000;
    //Número de hilos 
    const int num_threads = 4;  

    //Almacena los hilos de trabajo
    pthread_t threads[num_threads]; 
    //Almacenar los rangos de cada hilo
    ThreadData thread_data[num_threads]; 

    //Es la cantidad de "trabajo" que va a tener cada hilo
    int segmento = limite / num_threads;   

    //Comienza el temporizador (obtenido de utils.h) para cronometrar el tiempo de ejecución de ambas implementaciones
    start_timer(); 

    // Divide el rango en segmentos y crea hilos para calcular la suma de primos en cada segmento
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].start = i * segmento + (i == 0 ? 2 : 1);
        thread_data[i].end = (i + 1) * segmento + 1;
        pthread_create(&threads[i], nullptr, suma_primos, &thread_data[i]);
    }

    // Espera a que todos los hilos terminen y suma los resultados
    long long resultado = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], nullptr);
        resultado += thread_data[i].result;
    }

    //Detiene el tiempo para la ejecución en paralelo
    double tiempo_paralelo = stop_timer();

    // Calcula la suma de primos de forma secuencial
    start_timer();
    long long resultado_secuencial = suma_primos_secuencial(limite);
    double tiempo_secuencial = stop_timer();

    // Imprime los resultados y el tiempo en milisegundos de ejecución de ambas implementaciones
    cout << "Resultado de la ejecucion paralela (con hilos): " << resultado << endl;
    cout << "Tiempo de la ejecucion paralela (con hilos): " << tiempo_paralelo << "ms" << endl;
    cout << "Resultado de la ejecucion secuencia: " << resultado_secuencial << endl;
    cout << "Tiempo de la ejecucion secuencial: " << tiempo_secuencial << "ms" << endl;

    return 0;
}