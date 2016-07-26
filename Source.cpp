#define _CRT_SECURE_NO_WARNINGS
#include <cstring>
#include <iostream>
#include <list>
#include <string>
using namespace std;

char *getExpression() {
    string line;
    string expression;
    do {
        getline(cin, line);
        expression += line;
    } while (expression[expression.size() - 1] != ';');
    return strdup(expression.c_str());
}

void getTokens(list<char *> &tokens, char *src) {
    char delims[] = " ,;()";
    tokens.clear();
    char *tok = strtok(src, delims);
    do {
        tokens.push_back(tok);
        tok = strtok(NULL, delims);
    } while (tok);
}

int main(int argc, char const *argv[]) {
    string line;
    char *module_exp, *inputs_exp, *outputs_exp, *wires_exp;
    list<char *> tokens;
    // clear heading comment
    do {
        getline(cin, line);
    } while (line.substr(0, 2) == "//");
    /*========================================================================*/
    module_exp = getExpression();
    inputs_exp = getExpression();
    outputs_exp = getExpression();
    wires_exp = getExpression();

    getTokens(tokens, inputs_exp);
    for (list<char *>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
        std::cout << *it << std::endl;
    }
    return 0;
}
