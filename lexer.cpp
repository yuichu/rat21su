#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

char separators[] = { '(', ')', '[', ']', '{', '}', ',', ':', ';' };
char operators[] = { '*', '+', '-', '=', '/', '>', '<', '%' };
string keywords[13] = { "integer", "float", "boolean", "true", "false", "if", "else", "while", "put", "get", "begin", "end", "endif" };

vector<char> buffer;
ifstream inFile;
ofstream outFile;

int tokenizer(char currChar, int currState);
void print(int state);
bool isKeyword();

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

// FSM for identifier and integer tokens
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
    outFile.open(argv[2]);

    //checks if file opens correctly
    if (!inFile)
    {
        cout << "error\n";
        exit(1);
    }

    cout << "Tokens            Lexemes\n" << endl;
    outFile << "Tokens            Lexemes\n" << endl;

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

    //Check if it's a comment
    if (currChar == '/' && inFile.peek() == '*') {
        char c;
        inFile.ignore();
        do {
            inFile.ignore(256, '*');
            inFile.get(c);
        } while (c != '/');
        return 0;
    }
    // If character is a letter, check first column of states
    if (isalpha(currChar))
    {
        //cout << "Alpha" << endl;
        //cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][0];
        //cout << "New State: " << currentState << endl;
        buffer.push_back(currChar);
        return currentState;
    }
    // If character is a digit, check second column of states
    else if (isdigit(currChar))
    {
        //cout << "Digit" << endl;
        //cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][1];
        //cout << "New State: " << currentState << endl;
        buffer.push_back(currChar);
        return currentState;
    }
    else if (isspace(currChar))
    {
        //cout << "Space" << endl;
        //cout << "Current State: " << currentState << endl;
        currentState = DFSM[currentState][2];
        //cout << "New State: " << currentState << endl;
        if (buffer.size() != 0)
            print(currState);
        return currentState;
    }
    else
    {
        for (int i = 0; i <= 8; i++)
        {
            // check for separators
            if (currChar == separators[i])
            {
                currentState = DFSM[currentState][2];

                if (buffer.size() != 0) // Terminate and print the token if it exists
                    print(currState);
                // Push and print separator
                buffer.push_back(currChar);
                cout << "Separator         ";
                outFile << "Separator         ";
                print(currentState);
                return currentState;
            }
        }
        for (int i = 0; i <= 7; i++)
        {
            // check for operators
            if (currChar == operators[i])
            {
                currentState = DFSM[currentState][2];

                if (buffer.size() != 0) // Terminate and print token if it exists
                    print(currState);

                buffer.push_back(currChar);
                // check for relational operators /= and ==
                if ((currChar == '/' || currChar == '=') && inFile.peek() == '=') {
                    char c;
                    inFile.get(c);
                    buffer.push_back(c);
                }
                // check for program starter %%
                if (currChar == '%' && inFile.peek() == '%') {
                    char c;
                    inFile.get(c);
                    buffer.push_back(c);
                }
                // Print operator
                cout << "Operator          ";
                outFile << "Operator          ";
                print(currentState);
                return currentState;
            }
        }
    }

    return 0;
}

// outputs token and lexeme
void print(int state) {
    if (state == 1 || state == 2 || state == 3) {
        // Print Keyword label if true
        if (isKeyword()) {
            cout << "Keyword           ";
            outFile << "Keyword           ";
        }
        // Print Indentier label
        else {
        cout << "Identifier        ";
        outFile << "Identifier        ";
        }
    }
    // Print Integer label if true
    else if (state == 4) {
        cout << "Integer           ";
        outFile << "Integer           ";
    }
    // Print the entire token buffer
    for (vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        cout << *it;
        outFile << *it;
    }
    cout << endl;
    outFile << endl;
    buffer.clear();
}

// check to see if identifier is actually a keyword
bool isKeyword() {
    string s;
    for (vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        s.push_back(*it);
    }
    for (int i = 0; i <= 12; i++) {
        if (s == keywords[i])
            return true;
    }
    return false;
}
