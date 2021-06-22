#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

// Record Class ** WORK IN PROGRESS **
class Record
{
private:
    // Contains token value and lexeme
    string token, lexeme;

public:
    Record()
    {
        cout << "New Record created." << endl;
    }

    ~Record()
    {
        return;
    }

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

char separators[] = {'(', ')', '[', ']', '{', '}', ',', ':', ';'};
char operators[] = {'*', '+', '-', '=', '/', '>', '<', '%', '|'};
string keywords[13] = {"integer", "float", "boolean", "true", "false", "if", "else", "while", "put", "get", "begin", "end", "endif"};

vector<char> buffer;
vector<Record> rec_vec;
ifstream inFile;
ofstream outFile;

int tokenizer(char currChar, int currState);
void print(int state, string o = "");
void production_print();
bool isKeyword();

// FSM for identifier and integer tokens
int DFSM[6][3] = {
    /*
    l = letters
    d = digits
    f = not letter or digit
    */

    //                        l, d, f
    /* Starting   State 0: */ {1, 4, 5}, // not accepted, need at least one l or d input.
    /* Identifier State 1: */ {2, 3, 5}, // accepted
    /* Id is l,   State 2: */ {2, 3, 5}, // accepted
    /* Id is d,   State 3: */ {2, 3, 5}, // accepted
    /* Integer    State 4: */ {5, 4, 5}, // accepted
    /* End,       State 5: */ {5, 5, 5}  // not accepted
};

int main(int argc, char *argv[])
{
    char character;
    int currentState = 0;

    inFile.open("test1.txt");
    outFile.open("output1.txt");

    //checks if file opens correctly
    if (!inFile)
    {
        cout << "error\n";
        exit(1);
    }

    cout << "Tokens            Lexemes\n"
         << endl;
    outFile << "Tokens            Lexemes\n"
            << endl;

    while (inFile.get(character))
    {
        currentState = tokenizer(character, currentState);
    }

    cout << endl
         << endl
         << endl;
    cout << "REC VEC: " << endl;
    for (int i = 0; i < rec_vec.size(); i++)
    {
        cout << "RECORD TOKEN: " << rec_vec[i].getToken() << endl;
        cout << " RECORD LEXEME: " << rec_vec[i].getLexeme() << endl
             << endl;
    }

    inFile.close();
    outFile.close();

    return 0;
}

// Tokenizer returns the next state
int tokenizer(char currChar, int currState)
{
    int currentState = currState;

    //Check if it's a /**/ comment
    if (currChar == '/' && inFile.peek() == '*')
    {
        char c;
        inFile.ignore();
        do
        {
            inFile.ignore(256, '*');
            inFile.get(c);
        } while (c != '/');
        return 0;
    }
    // If character is a letter, check first column of states
    if (isalpha(currChar))
    {
        currentState = DFSM[currentState][0];
        buffer.push_back(currChar);
        return currentState;
    }
    // If character is a digit, check second column of states
    else if (isdigit(currChar))
    {
        currentState = DFSM[currentState][1];
        buffer.push_back(currChar);
        return currentState;
    }
    else if (isspace(currChar))
    {
        currentState = DFSM[currentState][2];
        if (buffer.size() != 0)
            print(currState);
        currentState = 0;

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
                currentState = 0;
                print(currentState, "seperator");

                return currentState;
            }
        }
        for (int i = 0; i <= 8; i++)
        {
            // check for operators
            if (currChar == operators[i])
            {
                currentState = DFSM[currentState][2];

                if (buffer.size() != 0) // Terminate and print token if it exists
                    print(currState);

                buffer.push_back(currChar);
                // check for relational operators /= and ==
                if ((currChar == '/' || currChar == '=') && inFile.peek() == '=')
                {
                    char c;
                    inFile.get(c);
                    buffer.push_back(c);
                }
                // check for program starter %%
                if (currChar == '%' && inFile.peek() == '%')
                {
                    char c;
                    inFile.get(c);
                    buffer.push_back(c);
                    // Print separator
                    cout << "Separator         ";
                    outFile << "Separator         ";
                    currentState = 0;
                    print(currentState, "seperator");
                    return currentState;
                }
                // Print operator
                cout << "Operator          ";
                outFile << "Operator          ";
                currentState = 0;
                print(currentState, "operator");

                return currentState;
            }
        }
        buffer.push_back(currChar);
        currentState = DFSM[currentState][2];
        print(currentState);
    }

    return 0;
}

// outputs token and lexeme
void print(int state, string o)
{

    Record *rec = new Record();
    if (o == "operator")
    {
        rec->setToken("Operator");
    }
    else if (o == "seperator")
    {
        rec->setToken("Seperator");
    }

    if (state == 1 || state == 2 || state == 3)
    {
        // Print Keyword label if true
        if (isKeyword())
        {
            rec->setToken("Keyword");
            cout << "Keyword           ";
            outFile << "Keyword           ";
        }
        // Print Indentier label
        else
        {
            rec->setToken("Identifier");
            cout << "Identifier        ";
            outFile << "Identifier        ";
        }
    }
    // Print Integer label if true
    else if (state == 4)
    {
        rec->setToken("Integer");
        cout << "Integer           ";
        outFile << "Integer           ";
    }
    else if (state == 5)
    {
        cout << "NOT ACCEPTED   X  ";
        outFile << "NOT ACCEPTED   X  ";
    }

    string lexeme;

    // Print the entire token buffer
    for (vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it)
    {
        cout << *it;
        lexeme += *it;
        outFile << *it;
    }

    rec->setLexeme(lexeme);
    rec_vec.push_back(*rec);

    cout << endl;
    outFile << endl;
    buffer.clear();
}

// check to see if identifier is actually a keyword
bool isKeyword()
{
    // Convert char buffer into type string
    string s;
    for (vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it)
    {
        s.push_back(*it);
    }
    // Compare new string against keyword list
    for (int i = 0; i <= 12; i++)
    {
        if (s == keywords[i])
            return true;
    }
    return false;
}

//--------------------------------------------------------------------------
// Following are functions for each production rule.
// Each function will begin with r_ to indicate that it's a production rule.
// Please see documentation for the production rule list.
//--------------------------------------------------------------------------

// Rule 1: <Rat21SU> ::= %% <Opt Declaration List> <Statement List> %%
void r_rat21Su()
{
    return;
}

// Rule 2: <Opt Declaration List> ::= <Declaration List>  |  <Empty>
void r_optDeclarationList()
{
    return;
}

// Rule 3: <Declaration List> ::= <Declaration> ; <Declaration List P>
void r_declarationList()
{
    return;
}

// Rule 21: <Declaration List P> ::= <Declaration List>  |  <Empty>
void r_declarationListP()
{
    return;
}

// Rule 4: <Declaration> ::= <Qualifier> <identifier>
void r_declaration()
{
    return;
}

// Rule 5: <Qualifier> ::= integer  |  boolean
void r_qualifier()
{
    return;
}

// Rule 6: <Statement List> ::= <Statement> <Statement List P>
void r_statementList()
{
    return;
}

// Rule 22: <Statement List P> ::= <Statement List>  |  <Empty>
void r_statementListP()
{
    return;
}

// Rule 7: <Statement> ::= <Compound>  |  <Assign>  |  <If>  |  <Get>  |  <Put>  |  <While>
void r_statement()
{
    return;
}

// Rule 8: <Compound> ::= begin <Statement List> end
void r_compound()
{
    return;
}

// Rule 9: <Assign> ::= <Identifier> = <Expression> ;
void r_assign()
{
    return;
}

// Rule 10: <If> ::= if ( <Condition> ) <Statement> <If P>
void r_if()
{
    return;
}

// Rule 23: <If P> ::= endif  |  else <Statement> endif
void r_ifP()
{
    return;
}

// Rule 11: <Put> ::= put ( <identifier> );
void r_put()
{
    return;
}

// Rule 12: <Get> ::= get ( <Identifier> );
void r_get()
{
    return;
}

// Rule 13: <While> ::= while ( <Condition> ) <Statement>
void r_while()
{
    return;
}

// Rule 14: <Condition> ::= <Expression> <Relop> <Expression>
void r_condition()
{
    return;
}

// Rule 15: <Relop> ::= ==  |  >  |  <  |  /=
void r_relop()
{
    return;
}

// Rule 16: <Expression> ::= <Term> <Expression P>
void r_expression()
{
    return;
}

// Rule 24: <Expression P> ::= + <Term> <Expression P>  |  - <Term> <Expression P>  |  <Empty>
void r_expressionP()
{
    return;
}

// Rule 17: <Term> ::= <Factor> <Term P>
void r_term()
{
    return;
}

// Rule 25: <Term P> ::= * <Factor> <Term P>   |   / <Factor> <Term P>   |   <Empty>
void r_termP()
{
    return;
}

// Rule 18: <Factor> ::= -  <Primary>  |  <Primary>
void r_factor()
{
    return;
}

// Rule 19: <Primary> ::= <Identifier>  |  <Integer>  |  ( <Expression> )   |  true   |  false
void r_primary()
{
    return;
}

// Rule 20: <Empty> ::= ε
void r_empty()
{
    return;
}
