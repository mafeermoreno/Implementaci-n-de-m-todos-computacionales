// ===========================================================================================
// File: resaltador.cpp
// Author: María Fernanda Moreno Gómez A01708653
//         Uri Jared Gopar Morales  A01709413
// Description: Este archivo contiene el código para realizar el resaltador de sintaxis de C# 
//              en C++, utilizando expresiones regulares para cada categoría léxica.
//              To compile: g++ -std=c++17 resaltador.cpp -lpthread -o app   y después  .\app
// ===========================================================================================
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <filesystem>
#include <thread>
#include <mutex>
#include "utils.h"

using namespace std;
namespace fs = std::filesystem;

const int THREADS = 8;  
mutex mtx;

struct Block {
    int start, end;
    vector<string>* array;
};

void resaltarLexico(const string& archivo, const string& directorioSalida) {
    ifstream file(archivo);
    if (!file) {
        cerr << "Error al abrir el archivo: " << archivo << endl;
        return;
    }

    stringstream ss;
    ss << file.rdbuf();
    string contenido = ss.str();

    // Define las expresiones regulares
    string comentarios = "//.*\n?";
    string keyword = "\\b(abstract|as|base|bool|break|byte|case|catch|char|checked|class|const|continue|decimal|default|delegate|do|double|else|enum|event|explicit|extern|false|finally|fixed|float|for|foreach|goto|if|implicit|in|int|interface|internal|is|lock|long|namespace|new|null|object|operator|out|override|params|private|protected|public|readonly|ref|return|sbyte|sealed|short|sizeof|stackalloc|static|string|struct|switch|this|throw|true|try|typeof|uint|ulong|unchecked|unsafe|ushort|using|virtual|void|volatile|while)\\b";
    string operadores = "\\+|-|\\*|/|%|\\^|&|\\||~|!|=|<|>|\\?|:|;|,|\\.|\\+\\+|--|&&|\\|\\||==|!=|<=|>=|\\+=|-=|\\*=|/=|%\\=|\\^=|&\\=|\\|=|<<=|>>=|=>|\\?\\?";
    string reales = "-*[0-9]+\\.[0-9]+([E][-*][0-9]+)?|-*[0-9]+(\\.[0-9]+)?";
    string especiales = "[\\(\\)|!]";
    string espacios = "\\s+";
    string variable = "[a-zA-Z][a-zA-Z_0-9]*";
    string lineBreak = "\n";
    string strings = "\".*\"";
    string system = "\\b(System|Console|Program|program)\\b";
    string separators = "[\\(\\)\\{\\}\\[\\];,.]";

    string resaltado;
    resaltado += "<style>"  
                    ".Variable { color: blue; }"
                    ".Real { color: green; }"
                    ".Comentario { color: gray; }"
                    ".Especial { color: red; }"
                    ".Operador { color: purple; }"
                    ".Keyword { color: cyan; }"
                    ".body { backgroud-color: black;   }"
                    ".System { color: orange; }" // color para las palabras clave del sistema
                    ".Separators { color: red; }"
                    ".String { color: pink; }"
                    "</style>";
    resaltado += "<pre>";
// Asegurarte de que keyword y variable se verifiquen primero
regex regex_tokens(keyword + "|" + comentarios + "|" + strings + "|" + variable + "|" + reales + "|" + especiales + "|" + operadores + "|" + separators + "|" + system + "|" + espacios + "|" + lineBreak);

// Asegurarte de que keyword se verifique antes que variable
for (sregex_iterator it(contenido.begin(), contenido.end(), regex_tokens); it != sregex_iterator(); ++it) {
    string token = it->str();
    string tipoToken;

    if (regex_match(token, regex(lineBreak))) {
        resaltado += "</pre><pre>";
    } else if (regex_match(token, regex(comentarios))) {
        tipoToken = "Comentario";
    } else if (regex_match(token, regex(keyword))) {
        tipoToken = "Keyword";
    }else if (regex_match(token, regex(system))) {
        tipoToken = "System";
    }else if (regex_match(token, regex(separators))) {
        tipoToken = "Separators";
    }else if (regex_match(token, regex(strings))) {
        tipoToken = "String";
    } else if (regex_match(token, regex(variable))) {
        tipoToken = "Variable";
    } else if (regex_match(token, regex(operadores))) {
        tipoToken = "Operador";
    } else if (regex_match(token, regex(reales))) {
        tipoToken = "Real";
    } else if (regex_match(token, regex(especiales))) {
        tipoToken = "Especial";
    } else {
        tipoToken = ""; // Espacios no deben ser resaltados
    }

    if (token != "\n") {
         resaltado += "<span class=\"" + tipoToken + "\">" + token + "</span>";
    }
}
    resaltado += "</pre>";

    lock_guard<mutex> lock(mtx);
    string nombreArchivo = fs::path(archivo).filename().string();
    ofstream outFile(directorioSalida + nombreArchivo + ".html");
    outFile << resaltado;
    outFile.close();
} 

void *resaltar(void* arg) {
    Block* data = (Block*) arg;
    string directorioSalida = "./output/";
    for (int i = data->start; i < data->end; ++i) {
        string archivo = (*data->array)[i];
        resaltarLexico(archivo, directorioSalida);
    }
    return nullptr;
}

int main() {
    vector<string> archivos;
    string directorioSalida = "./output/";
    if (!fs::exists(directorioSalida)) {
        fs::create_directory(directorioSalida);
    }

        for (auto &p : fs::recursive_directory_iterator("./csharp_examples")) {
        if (fs::is_regular_file(p) && p.path().extension() == ".cs") {
            archivos.push_back(p.path().string());
        }
    }
    

    // Inicio de la ejecución paralela
    int blockSize = archivos.size() / THREADS;
    pthread_t threads[THREADS];
    Block blocks[THREADS];

    start_timer();      

    for (int i = 0; i < THREADS; ++i) {
        blocks[i].start = i * blockSize;
        blocks[i].end = (i == THREADS - 1) ? archivos.size() : (i + 1) * blockSize;
        blocks[i].array = &archivos;

        pthread_create(&threads[i], NULL, resaltar, (void*)&blocks[i]);
    }

    for (int i = 0; i < THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    double tiempoParalelo = stop_timer();
    cout << "Tiempo de ejecucion paralelo: " << tiempoParalelo << " ms" << endl;

    // Limpieza de archivos generados por ejecución paralela
        for (auto &p : fs::recursive_directory_iterator(directorioSalida)) {
        if (fs::is_regular_file(p) && p.path().extension() == ".html") {
            fs::remove(p);
        }
    }

    // Inicio de la ejecución secuencial
    start_timer();

    for (auto& archivo : archivos) {
        resaltarLexico(archivo, directorioSalida);
    }

    double tiempoSecuencial = stop_timer();
    cout << "Tiempo de ejecucion secuencial: " << tiempoSecuencial << " ms" << endl;

    double speedup = tiempoSecuencial / tiempoParalelo;
    cout << "Speedup: " << speedup << endl;

    return 0;
}