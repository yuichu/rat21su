#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

char separators[] = { '(', ')', '[', ']', '{', '}', ',', ':', ';' };
char operators[] = { '*', '+', '-', '=', '/', '>', '<', '%' };
string keywords[13] = { "int", "float", "bool", "true", "false", "if", "else", "while", "put", "get", "begin", "end", "endif" };

vector<char> buffer;
ifstream inFile;
ofstream outFile;

int tokenizer(char currChar, int currState);
void print();

// Record Class
class record
{
private:
    // Contains token value and lexeme
    string token, lexeme;

public:
    string getToken()
    {
        return this->token;
    }

    string getLexeme()
    {
        return this->lexeme;
    }

    void setToken(string s)
    {
        this->token = s;
    }

    void setLexeme(string s)
    {
        this->lexeme = s;
    }
};


int DFSM[6][3] = {
    /*
    l = letters
    d = digits
    f = not letter or digit
    */

    //                        l, d, f
    /* Starting   State 0: */{1, 4, 5}, // not accepted, need at least one l or d input.
    /* Identifier State 1: */{2, 3, 5}, // accepted
    /* Id is l,   State 2: */{2, 3, 5}, // accepted
    /* Id is d,   State 3: */{2, 3, 5}, // accepted
    /* Integer    State 4: */{5, 4, 5}, // accepted
    /* End,       State 5: */{5, 5, 5}  // not accepted
};

int main(int argc, char* argv[])
{
    char character;
    int currentState = 0;

    inFile.open(argv[1]);
    outFile.open("Tokens.txt");

    //checks if file opens correctly
    if (!inFile)
    {
        cout << "error\n";
        exit(1);
    }

    while (inFile.get(character))
    {
        currentState = tokenizer(character, currentState);
        if (currentState == 5)
            currentState = 0;
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
        buffer.push_back(currChar);
        return currentState;
    }
    // If character is a digit, check second column of states
    else if (isdigit(currChar))
    {
        cout << "Digit" << endl;
        cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][1];
        cout << "New State: " << currentState << endl;
        buffer.push_back(currChar);
        return currentState;
    }
    else if (isspace(currChar))
    {
        cout << "Space" << endl;
        cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][2];
        cout << "New State: " << currentState << endl;
        if (buffer.size() != 0)
            print();
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
                currentState = DFSM[currentState][2];
                cout << "New State: " << currentState << endl;
                if (buffer.size() != 0)
                    print();
                return currentState;
            }
        }
        for (int i = 0; i < 7; i++)
        {
            if (currChar == operators[i])
            {
                cout << "Operator" << endl;
                cout << "Current State: " << currentState << endl;
                currentState = DFSM[currentState][2];
                cout << "New State: " << currentState << endl;
                if (buffer.size() != 0)
                    print();
                return currentState;
            }
        }
    }

    return 0;
}

void print() {
    //prints the entire buffer and writes it to token.txt
    for (vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        cout << *it;
        outFile << *it;
    }
    outFile << endl;
    buffer.clear();
}
