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
struct State {
    unordered_map<char, char> transitions;  // Transiciones del estado
    bool accepting;  // Indica si el estado es de aceptación
};

class Automata {
public:
    vector<State> states;  // Vector de estados del AFD

    Automata(char c) {
        states.push_back(State{{}, false});  // Estado inicial
        states.push_back(State{{}, true});   // Estado de aceptación

        if (c != 'E') {
            states[0].transitions[c] = 'B';
        }
    }

    Automata(size_t numStates) : states(numStates) {}

    void addTransition(char from, char to, char c) {
        states[from - 'A'].transitions[c] = to;
    }

    void printAutomata() {
        for (size_t i = 0; i < states.size(); i++) {
            cout << "State " << static_cast<char>(i + 'A') << (states[i].accepting ? " (Accepting)" : "") << ":\n";
            for (auto& [c, to] : states[i].transitions) {
                cout << "  --(" << c << ")--> " << to << '\n';
            }
        }
    }
};

Automata concatenateAutomata(const Automata& a, const Automata& b) {
    Automata result(a.states.size() + b.states.size());

    for (char i = 'A'; i < 'A' + a.states.size(); i++) {
        for (auto& [c, to] : a.states[i - 'A'].transitions) {
            result.addTransition(i, to, c);
        }
    }

    for (char i = 'A'; i < 'A' + b.states.size(); i++) {
        for (auto& [c, to] : b.states[i - 'A'].transitions) {
            result.addTransition(i + a.states.size(), to + a.states.size(), c);
        }
    }

    result.states[a.states.size() - 1].accepting = false;
    result.addTransition(a.states.size() - 1 + 'A', a.states.size() + 'A', 'E');

    return result;
}

Automata unionAutomata(const Automata& a, const Automata& b) {
    Automata result(a.states.size() + b.states.size() + 2);

    for (char i = 'A'; i < 'A' + a.states.size(); i++) {
        for (auto& [c, to] : a.states[i - 'A'].transitions) {
            result.addTransition(i + 1, to + 1, c);
        }
    }

    for (char i = 'A'; i < 'A' + b.states.size(); i++) {
        for (auto& [c, to] : b.states[i - 'A'].transitions) {
            result.addTransition(i + 1 + a.states.size(), to + 1 + a.states.size(), c);
        }
    }

    result.addTransition('A', 'B', 'E');
    result.addTransition('A', a.states.size() + 'B', 'E');

    result.states[a.states.size()].accepting = false;
    result.states[a.states.size() + b.states.size()].accepting = false;

    result.addTransition(a.states.size() + 'A', result.states.size() - 1 + 'A', 'E');
    result.addTransition(a.states.size() + b.states.size() + 'A', result.states.size() - 1 + 'A', 'E');

    return result;
}

Automata starAutomata(const Automata& a) {
    Automata result(a.states.size() + 2);

    for (char i = 'A'; i < 'A' + a.states.size(); i++) {
        for (auto& [c, to] : a.states[i - 'A'].transitions) {
            result.addTransition(i + 1, to + 1, c);
        }
    }

    result.addTransition('A', 'B', 'E');
    result.addTransition('A', result.states.size() - 1 + 'A', 'E');

    result.states[a.states.size()].accepting = false;

    result.addTransition(a.states.size() + 'A', 'B', 'E');
    result.addTransition(a.states.size() + 'A', result.states.size() - 1 + 'A', 'E');

    return result;
}

string infixToPostfixRegex(string infixRegex) {
    stack<char> operatorStack;
    string postfixRegex;

    unordered_map<char, int> precedence;
    precedence['*'] = 3;
    precedence['.'] = 2;
    precedence['|'] = 1;
    precedence['+'] = 1;

    for (char c : infixRegex) {
        switch (c) {
            case '*':
            case '.':
            case '|':
            case '+':
                while (!operatorStack.empty() && precedence[operatorStack.top()] >= precedence[c]) {
                    postfixRegex += operatorStack.top();
                    operatorStack.pop();
                }
                operatorStack.push(c);
                break;
            case '(':
                operatorStack.push(c);
                break;
            case ')':
                while (operatorStack.top() != '(') {
                    postfixRegex += operatorStack.top();
                    operatorStack.pop();
                }
                operatorStack.pop();  // Pop '('
                break;
            default:  // Operand
                postfixRegex += c;
                break;
        }
    }

    while (!operatorStack.empty()) {
        postfixRegex += operatorStack.top();
        operatorStack.pop();
    }

    return postfixRegex;
}

Automata constructAutomataFromRegex(string postfixRegex) {
    stack<Automata> s;

    for (char c : postfixRegex) {
        switch (c) {
            case '*':
                s.push(starAutomata(s.top()));
                s.pop();
                break;
            case '|':
            case '+':
                {
                Automata b = s.top(); s.pop();
                Automata a = s.top(); s.pop();
                s.push(unionAutomata(a, b));
                }
                break;
            case '.':
                {
                Automata b = s.top(); s.pop();
                Automata a = s.top(); s.pop();
                s.push(concatenateAutomata(a, b));
                }
                break;
            default:
                s.push(Automata(c));
                break;
        }
    }

    return s.top();
}

int main() {
    ifstream file("input.txt");

    string infixRegex, alphabet;
    file >> infixRegex >> alphabet;

    string postfixRegex = infixToPostfixRegex(infixRegex);

    Automata a = constructAutomataFromRegex(postfixRegex);

    a.printAutomata();

    return 0;
}