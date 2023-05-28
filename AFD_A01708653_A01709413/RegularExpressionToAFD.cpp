// ==========================================================================
// File: RegularExpressionToAFD.cpp
// Author: María Fernanda Moreno Gómez A01708653
//         Uri Jared Gopar Morales  A01709413
// Description: Este archivo contiene el código para un AFD de una expresión regular
//              To compile: g++ -std=c++17 RegularExpressionToAFD.cpp -o app   y después  ./app
// ===========================================================================================
#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>

using namespace std;

// Definición de la estructura State que representa un estado del autómata
struct State {
    unordered_map<char, set<int>> transitions;  // Transiciones del estado, mapeo de caracteres a conjuntos de estados de destino
    bool accepting;  // Indica si el estado es de aceptación
};

// Definición de la clase Automata que representa el autómata finito determinista (AFD)
class Automata {
public:
    vector<State> states;  // Vector de estados del AFD

    // Constructor para un nuevo estado del AFD que representa un carácter específico
    Automata(char c, int) {
        states.push_back(State{{}, false});  // Estado inicial
        states.push_back(State{{}, true});   // Estado de aceptación

        // Agregar transición desde el estado inicial al estado de aceptación usando el carácter especificado
        if (c != 'E') {
            states[0].transitions[c].insert(1);
        }
    }

    // Constructor para un AFD con un número específico de estados
    Automata(size_t numStates) : states(numStates) {}

    // Agregar una transición desde un estado dado hacia un estado de destino usando un carácter específico
    void addTransition(int from, int to, char c) {
        states[from].transitions[c].insert(to);
    }
};

// Función para concatenar dos autómatas
Automata concatenateAutomata(const Automata& a, const Automata& b) {
    Automata result(size_t(a.states.size() + b.states.size()));

    // Copiar las transiciones del primer autómata al autómata resultado
    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i, to, c);
            }
        }
    }

    // Copiar las transiciones del segundo autómata al autómata resultado, ajustando los índices de los estados
    for (int i = 0; i < b.states.size(); i++) {
        for (auto& [c, next] : b.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + a.states.size(), to + a.states.size(), c);
            }
        }
    }

    // Actualizar el estado de aceptación del último estado del primer autómata
    result.states[a.states.size() - 1].accepting = false;
    // Agregar una transición epsilon desde el último estado del primer autómata hacia el primer estado del segundo autómata
    result.addTransition(a.states.size() - 1, a.states.size(), 'E');

    return result;
}

// Función para realizar la unión de dos autómatas
Automata unionAutomata(const Automata& a, const Automata& b) {
    Automata result(size_t(a.states.size() + b.states.size() + 2));

    // Copiar las transiciones del primer autómata al autómata resultado, ajustando los índices de los estados
    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + 1, to + 1, c);
            }
        }
    }

    // Copiar las transiciones del segundo autómata al autómata resultado, ajustando los índices de los estados
    for (int i = 0; i < b.states.size(); i++) {
        for (auto& [c, next] : b.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + 1 + a.states.size(), to + 1 + a.states.size(), c);
            }
        }
    }

    // Agregar transiciones epsilon desde el estado inicial hacia los estados iniciales de los autómatas a y b
    result.addTransition(0, 1, 'E');
    result.addTransition(0, 1 + a.states.size(), 'E');

    // Actualizar los estados de aceptación del autómata resultado
    result.states[a.states.size()].accepting = false;
    result.states[a.states.size() + b.states.size()].accepting = false;
    result.states[a.states.size() + b.states.size() + 1].accepting = true;

    return result;
}

// Función para realizar la operación de estrella (cerradura de Kleene) sobre un autómata
Automata starAutomata(const Automata& a) {
    Automata result(size_t(a.states.size() + 2));

    // Copiar las transiciones del autómata original al autómata resultado, ajustando los índices de los estados
    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + 1, to + 1, c);
            }
        }
    }

    // Agregar transiciones epsilon desde el estado inicial hacia el primer estado y desde el último estado hacia el estado de aceptación
    result.addTransition(0, 1, 'E');
    result.addTransition(0, a.states.size() + 1, 'E');
    result.addTransition(a.states.size(), a.states.size() + 1, 'E');
    result.addTransition(a.states.size(), 1, 'E');

    // Actualizar los estados de aceptación del autómata resultado
    result.states[a.states.size()].accepting = false;
    result.states[a.states.size() + 1].accepting = true;

    return result;
}

// Función para construir un autómata a partir de una expresión regular
Automata constructAutomataFromRegex(const string& regex) {
    stack<Automata> automataStack;

    for (char c : regex) {
        if (c == '*') {
            Automata a = automataStack.top();
            automataStack.pop();
            automataStack.push(starAutomata(a));
        } else if (c == '+') {
            Automata b = automataStack.top();
            automataStack.pop();
            Automata a = automataStack.top();
            automataStack.pop();
            automataStack.push(unionAutomata(a, b));
        } else if (c == '.') {
            Automata b = automataStack.top();
            automataStack.pop();
            Automata a = automataStack.top();
            automataStack.pop();
            automataStack.push(concatenateAutomata(a, b));
        } else {
            automataStack.push(Automata(c, 0));
        }
    }

    return automataStack.top();
}

int main() {
    ifstream inFile("input.txt");

    if (!inFile) {
        cerr << "No se pudo abrir input.txt";
        return 1;   // Detener el sistema
    }

    // Leer la expresión regular desde el archivo
    string regex;
    getline(inFile, regex);

    // Leer el alfabeto desde el archivo
    string alphabet;
    getline(inFile, alphabet);

    inFile.close();

    // Construye el autómata a partir de la expresión regular
    Automata a = constructAutomataFromRegex(regex);

    // Inicializa la matriz de adyacencia con -1 en lugar de 0
    vector<vector<int>> adjacencyMatrix(a.states.size(), vector<int>(alphabet.length(), -1));

    // Generar la matriz de adyacencia del AFD
    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                // Encuentra la posición del carácter en el alfabeto
                int position = alphabet.find(c);
                // Si el carácter está en el alfabeto, almacena el estado de destino en la matriz de adyacencia
                if (position != string::npos) {
                    adjacencyMatrix[i][position] = to;
                }
            }
        }
    }

    // Abrir el archivo de salida
    ofstream outFile("output.txt");

    // Escribir la matriz de adyacencia del AFD en el archivo de salida
    outFile << "Matriz de adyacencia del AFD:\n";
    outFile << "Estado  ";
    for (char c : alphabet) {
        outFile << c << "   ";
    }
    outFile << "\n";

    for (int i = 0; i < adjacencyMatrix.size(); i++) {
        outFile << i << "       ";
        for (int j = 0; j < adjacencyMatrix[i].size(); j++) {
            // Imprimir -1 si no hay transición, de lo contrario imprimir el estado de destino
            if (adjacencyMatrix[i][j] == -1) {
                outFile << "-1  ";
            } else {
                outFile << adjacencyMatrix[i][j] << "   ";
            }
        }
        outFile << "\n";
    }

    // Escribir los estados de aceptación del AFD en el archivo de salida
    outFile << "Estados de aceptación del AFD:\n";
    for (int i = 0; i < a.states.size(); i++) {
        if (a.states[i].accepting) {
            outFile << i << "\n";
        }
    }

    // Cerrar el archivo de salida
    outFile.close();

    return 0;
}

