#include <iostream>
#include <fstream>
#include <stack>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>

using namespace std;

struct State {
    unordered_map<char, set<int>> transitions;
    bool accepting;
};

class Automata {
public:
    vector<State> states;

    Automata(char c, int) {
        states.push_back(State{{}, false});
        states.push_back(State{{}, true});

        if (c != 'E') {
            states[0].transitions[c].insert(1);
        }
    }

    Automata(size_t numStates) : states(numStates) {}

    void addTransition(int from, int to, char c) {
        states[from].transitions[c].insert(to);
    }
};

Automata concatenateAutomata(const Automata& a, const Automata& b) {
    Automata result(size_t(a.states.size() + b.states.size()));

    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i, to, c);
            }
        }
    }

    for (int i = 0; i < b.states.size(); i++) {
        for (auto& [c, next] : b.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + a.states.size(), to + a.states.size(), c);
            }
        }
    }

    result.states[a.states.size() - 1].accepting = false;
    result.addTransition(a.states.size() - 1, a.states.size(), 'E');

    return result;
}

Automata unionAutomata(const Automata& a, const Automata& b) {
    Automata result(size_t(a.states.size() + b.states.size() + 2));

    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + 1, to + 1, c);
            }
        }
    }

    for (int i = 0; i < b.states.size(); i++) {
        for (auto& [c, next] : b.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + 1 + a.states.size(), to + 1 + a.states.size(), c);
            }
        }
    }

    result.addTransition(0, 1, 'E');
    result.addTransition(0, 1 + a.states.size(), 'E');
    result.states[a.states.size()].accepting = false;
    result.states[a.states.size() + b.states.size()].accepting = false;
    result.states[a.states.size() + b.states.size() + 1].accepting = true;

    return result;
}

Automata starAutomata(const Automata& a) {
    Automata result(size_t(a.states.size() + 2));

    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                result.addTransition(i + 1, to + 1, c);
            }
        }
    }

    result.addTransition(0, 1, 'E');
    result.addTransition(0, a.states.size() + 1, 'E');
    result.addTransition(a.states.size(), a.states.size() + 1, 'E');
    result.addTransition(a.states.size(), 1, 'E');
    result.states[a.states.size()].accepting = false;
    result.states[a.states.size() + 1].accepting = true;

    return result;
}

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

    string regex;
    getline(inFile, regex);

    string alphabet;
    getline(inFile, alphabet);

    inFile.close();

    Automata a = constructAutomataFromRegex(regex);

    vector<vector<int>> adjacencyMatrix(a.states.size(), vector<int>(alphabet.length(), -1));

    for (int i = 0; i < a.states.size(); i++) {
        for (auto& [c, next] : a.states[i].transitions) {
            for (int to : next) {
                int position = alphabet.find(c);
                if (position != string::npos) {
                    adjacencyMatrix[i][position] = to;
                }
            }
        }
    }

    cout << "Matriz de adyacencia del AFD:\n";
    cout << "Estado  ";
    for (char c : alphabet) {
        cout << c << "   ";
    }
    cout << "\n";

    for (int i = 0; i < adjacencyMatrix.size(); i++) {
        cout << i << "       ";
        for (int j = 0; j < adjacencyMatrix[i].size(); j++) {
            if (adjacencyMatrix[i][j] == -1) {
                cout << "-1  ";
            } else {
                cout << adjacencyMatrix[i][j] << "   ";
            }
        }
        cout << "\n";
    }

    cout << "Estados de aceptación del AFD:\n";
    for (int i = 0; i < a.states.size(); i++) {
        if (a.states[i].accepting) {
            cout << i << "\n";
        }
    }

    return 0;
}
