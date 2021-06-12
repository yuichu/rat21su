#include <iostream>
#include <string>
#include <fstream>
using namespace std;

char separators[] = {'(', ')', '[', ']', '{', '}', ',', ':', ';'};
char operators[] = {'*', '+', '-', '=', '/', '>', '<', '%'};
string keywords[13] = {"int", "float", "bool", "true", "false", "if", "else", "while", "put", "get", "begin", "end", "endif"};

int tokenizer(char currChar, int currState);

int DFSM[4][4] = {
    /*
    STATE 0 is an accepting state
    l = letters
    d = digits
    s = seperator/operator

  */
    // l, d, s
    {1, 2, 3, 0},
    {1, 2, 3, 0}

};

int main(int argc, char *argv[])
{
    ifstream inFile;
    ofstream outFile;
    char character;
    int currentState = 0;

    inFile.open(argv[1]);
    outFile.open(argv[2]);

    //checks if file opens correctly
    if (!inFile)
    {
        cout << "error\n";
        exit(1);
    }

    while (!inFile.eof())
    {
        character = inFile.get();
        currentState = tokenizer(character, currentState);
    }

    inFile.close();
    outFile.close();

    return 0;
}

// Tokenizer returns the next state
int tokenizer(char currChar, int currState)
{

    int currentState = currState;

    if (isalpha(currChar))
    {
        cout << "Alpha" << endl;
        cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][0];
        cout << "New State: " << currentState << endl;
        return currentState;
    }
    // If character is a digit, check second column of states
    else if (isdigit(currChar))
    {
        cout << "Digit" << endl;
        cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][1];
        cout << "New State: " << currentState << endl;
        return currentState;
    }
    else if (isspace(currChar))
    {
        cout << "Space" << endl;
        cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][0];
        cout << "New State: " << currentState << endl;
        return currentState;
    }
    else
    {
        cout << "Others" << endl;
        for (int i = 0; i < 8; i++)
        {
            if (currChar == separators[i])
            {
                cout << "Seperator" << endl;
                cout << "Current State: " << currentState << endl;
                currentState = DFSM[currentState][4];
                cout << "New State: " << currentState << endl;
                return currentState;
            }
        }
        for (int i = 0; i < 7; i++)
        {
            if (currChar == operators[i])
            {
                cout << "Operator" << endl;
                cout << "Current State: " << currentState << endl;
                currentState = DFSM[currentState][4];
                cout << "New State: " << currentState << endl;
                return currentState;
            }
        }
    }

    return 0;
}

bool isalpha(char ch)
{
}

bool isdigit(char ch)
{
}