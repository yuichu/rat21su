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


int main(int argc, char * argv[])
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

    // If character is a alpha, check first column of states
    if (isalpha(currChar))
    {
        currentState = DFSM[currentState][0];
        return currentState;
    }
    // If character is a digit, check second column of states
    else if (isdigit(currChar)){
        currentState = DFSM[currentState][1];
    }

    
    return 0;
}

bool isalpha(char ch) {

}

bool isdigit(char ch) {

}