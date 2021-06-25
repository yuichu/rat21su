#include <iostream>
#include <string>
#include <fstream>
#include <vector>
using namespace std;

char separators[] = { '(', ')', '[', ']', '{', '}', ',', ':', ';' };
char operators[] = { '*', '+', '-', '=', '/', '>', '<', '%', '|' };
string keywords[13] = { "integer", "float", "boolean", "true", "false", "if", "else", "while", "put", "get", "begin", "end", "endif" };

vector<char> buffer;
ifstream inFile;
ofstream outFile;
string token;
string lexeme;
bool printRule = true;

int tokenizer(char currChar, int currState);
// print(int state) -> prints token and lexeme
void print(int state);
void production_print();
bool isKeyword();

void r_rat21Su();
void r_optDeclarationList();
void r_statementList();
void r_declarationList();
void r_declaration();
void r_declarationListP();
void r_qualifier();
void r_statement();
void r_statementListP();
void r_compound();
void r_assign();
void r_if();
void r_get();
void r_put();
void r_while();
void r_expression();
void r_expressionP();
void r_condition();
void r_ifP();
void r_relop();
void r_term();
void r_termP();
void r_factor();
void r_primary();

// Record Class ** WORK IN PROGRESS **
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
            exit(EXIT_FAILURE);
    }

    inFile.close();
    outFile.close();

    inFile.open(argv[2]);
    outFile.open("Parser.txt");
    // skips over the first "Token" and "Lexeme" labels
    inFile >> token >> lexeme >> token >> lexeme;
    r_rat21Su();

    inFile.close();
    outFile.close();

    return 0;
}

// Tokenizer returns the next state
int tokenizer(char currChar, int currState)
{
    int currentState = currState;

    //Check if it's a /**/ comment
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
                print(currentState);

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
                    // Print separator
                    cout << "Separator         ";
                    outFile << "Separator         ";
                    currentState = 0;
                    print(currentState);
                    return currentState;
                }
                // Print operator
                cout << "Operator          ";
                outFile << "Operator          ";
                currentState = 0;
                print(currentState);

                return currentState;
            }
        }
        buffer.push_back(currChar);
        currentState = DFSM[currentState][2];
        print(currentState);
    }

    return currentState;
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
    else if (state == 5) {
        cout << "NOT ACCEPTED   X  ";
        outFile << "NOT ACCEPTED   X  ";
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
    // Convert char buffer into type string
    string s;
    for (vector<char>::iterator it = buffer.begin(); it != buffer.end(); ++it) {
        s.push_back(*it);
    }
    // Compare new string against keyword list
    for (int i = 0; i <= 12; i++) {
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
void r_rat21Su() {
    if (lexeme == "%%") {
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 1\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Rat21SU> ::= %% <Opt Declaration List> <Statement List> %%" << "\n";
            outFile << "\t <Rat21SU> ::= %% <Opt Declaration List> <Statement List> %%" << "\n";
        }
        inFile >> token >> lexeme;
        r_optDeclarationList();
        r_statementList();
    }
    else {
        //error: Expected %%
        cout << "Rule 1 Error: expected %%\n";
    }
    if (lexeme == "%%") {
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 1\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        inFile >> token >> lexeme;
    }
    else {
        //error: Expected %%
        cout << "Rule 1 Error: expected %%\n";
    }
    return;
}

// Rule 2: <Opt Declaration List> ::= <Declaration List>  |  <Empty>
void r_optDeclarationList() {
    if (lexeme != "%%" && !inFile.eof() && (lexeme == "integer" || lexeme == "boolean")) {
        if (printRule) {
            // print production rule
            cout << "\t <Opt Declaration List> ::= <Declaration List>" << "\n";
            outFile << "\t <Opt Declaration List> ::= <Declaration List>" << "\n";
        }
        r_declarationList();
    }
    return;
}

// Rule 3: <Declaration List> ::= <Declaration> ; <Declaration List P>
void r_declarationList() {
    r_declaration();
    if (lexeme == ";") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 3\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Declaration List> ::= <Declaration> ; <Declaration List P>" << "\n";
            outFile << "\t <Declaration List> ::= <Declaration> ; <Declaration List P>" << "\n";
        }
        inFile >> token >> lexeme;
    }
    else {
        //error: Expected ';'
        cout << "Rule 3 Error: expected ;\n";
    }
    r_declarationListP();
}

// Rule 21: <Declaration List P> ::= <Declaration List>  |  <Empty>
void r_declarationListP() {
    if (lexeme != "%%" && !inFile.eof() && (lexeme == "integer" || lexeme == "boolean")) {
        if (printRule) {
            // print production rule
            cout << "\t <Declaration List P> ::= <Declaration List>" << "\n";
            outFile << "\t <Declaration List P> ::= <Declaration List>" << "\n";
        }
        r_declarationList();
    }
    return;
}

// Rule 4: <Declaration> ::= <Qualifier> <identifier> 
void r_declaration() {
    r_qualifier(); // <Qualifier>
    if (token == "Identifier") {    // <identifier>
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 4\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Declaration> ::= <Qualifier> <identifier> " << "\n";
            outFile << "\t <Declaration> ::= <Qualifier> <identifier> " << "\n";
        }
        inFile >> token >> lexeme;
    }
    else {
        // error: Expected id
        cout << "Rule 4 Error: expected id\n";
    }
    return;
}

// Rule 5: <Qualifier> ::= integer  |  boolean
void r_qualifier() {
    if (lexeme == "integer" || lexeme == "boolean") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 5\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && lexeme == "integer") {
            // print production rule
            cout << "\t <Qualifier> ::= integer" << "\n";
            outFile << "\t <Qualifier> ::= integer" << "\n";
        }
        if (printRule && lexeme == "boolean") {
            // print production rule
            cout << "\t <Qualifier> ::= boolean" << "\n";
            outFile << "\t <Qualifier> ::= boolean" << "\n";
        }
        inFile >> token >> lexeme;
    }
    else {
        // error
        cout << "Rule 5 Error: expected integer or boolean\n";
    }
    return;
}

// Rule 6: <Statement List> ::= <Statement> <Statement List P>
void r_statementList() {
    if (lexeme != "%%") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement List> ::= <Statement> <Statement List P>" << "\n";
            outFile << "\t <Statement List> ::= <Statement> <Statement List P>" << "\n";
        }
        r_statement();
        r_statementListP();
    }
    return;
}

// Rule 22: <Statement List P> ::= <Statement List>  |  %%
void r_statementListP() {
    if (lexeme != "%%") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement List P> ::= <Statement List>" << "\n";
            outFile << "\t <Statement List P> ::= <Statement List>" << "\n";
        }
        r_statementList();
    }
    return;
}

// Rule 7: <Statement> ::= <Compound>  |  <Assign>  |  <If>  |  <Get>  |  <Put>  |  <While>
void r_statement() {
    if (lexeme == "begin") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <Compound>" << "\n";
            outFile << "\t <Statement> ::= <Compound>" << "\n";
        }
        r_compound();
    }
    else if (token == "Identifier") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <Assign>" << "\n";
            outFile << "\t <Statement> ::= <Assign>" << "\n";
        }
        r_assign();
    }
    else if (lexeme == "if") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <If>" << "\n";
            outFile << "\t <Statement> ::= <If>" << "\n";
        }
        r_if();
    }
    else if (lexeme == "get") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <Get>" << "\n";
            outFile << "\t <Statement> ::= <Get>" << "\n";
        }
        r_get();
    }
    else if (lexeme == "put") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <Put>" << "\n";
            outFile << "\t <Statement> ::= <Put>" << "\n";
        }
        r_put();
    }
    else if (lexeme == "while") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <While>" << "\n";
            outFile << "\t <Statement> ::= <While>" << "\n";
        }
        r_while();
    }
    /*else if (lexeme == "end") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 8\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        inFile >> token >> lexeme;
    }*/
    else {
        cout << "Rule 7 error: No valid statement found for " << token << " " << lexeme << "\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 8: <Compound> ::= begin <Statement List> end
void r_compound() {
    if (lexeme == "begin") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 8\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Compound> ::= begin <Statement List> end" << "\n";
        }
        inFile >> token >> lexeme;

        r_statementList();  // <Statement List>

        if (lexeme == "end") {
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 8\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;
        }
        else {
            // error for end
            cout << "Rule 8 Error: expected end\n";
        }
    }
    else {
        // error
        cout << "Rule 8 Error: expected begin\n";
    }
    return;
}

// Rule 9: <Assign> ::= <Identifier> = <Expression> ;
void r_assign() {
    if (token == "Identifier") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 9\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Assign> ::= <Identifier> = <Expression> ;" << "\n";
            outFile << "\t <Assign> ::= <Identifier> = <Expression> ;" << "\n";
        }
        inFile >> token >> lexeme;

        if (lexeme == "=") {
            // print token and lexeme
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 9\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;

            r_expression(); // <Expression>

            if (lexeme == ";") {
                //print token and lexeme
                cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 9\n";
                outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                inFile >> token >> lexeme;
            }
            else {
                // error
                cout << "Rule 9 Error: expected ;\n";
            }
        }
        else {
            // error
            cout << "Rule 9 Error: expected =\n";
        }
    }
    else {
        //error
        cout << "Rule 9 Error: expected id\n";
    }
    return;
}

// Rule 10: <If> ::= if ( <Condition> ) <Statement> <If P>
void r_if() {
    if (lexeme == "if") {
        // print token and lexeme for 'if'
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 10\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <If> ::= if ( <Condition> ) <Statement> <If P>" << "\n";
            outFile << "\t <If> ::= if ( <Condition> ) <Statement> <If P>" << "\n";
        }
        inFile >> token >> lexeme;

        if (lexeme == "(") {
            // print token and lexeme for '('
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 10\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;

            r_condition();  // <Condition>

            if (lexeme == ")") {
                // print token and lexeme for ')'
                cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 10\n";
                outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                inFile >> token >> lexeme;

                r_statement();  // <Statement>
                r_ifP();    // <If P>
            }
            else {
                // error on ')'
                cout << "Rule 10 Error: expected )\n";
            }
        }
        else {
            // error on '('
            cout << "Rule 10 Error: expected (\n";
        }
    }
    else {
        // error on "if"
        cout << "Rule 10 Error: expected if\n";
    }
    return;
}

// Rule 23: <If P> ::= endif  |  else <Statement> endif
void r_ifP() {
    if (lexeme == "endif" || lexeme == "else") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 23\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && lexeme == "endif") {
            // print production rule
            cout << "\t <If P> ::= endif" << "\n";
            outFile << "\t <If P> ::= endif" << "\n";
        }
        if (printRule && lexeme == "else")
        {
            // print production rule
            cout << "\t <If P> ::= else <Statement> endif" << "\n";
            outFile << "\t <If P> ::= else <Statement> endif" << "\n";
        }
        inFile >> token >> lexeme;

        if (lexeme == "else") {
            r_statement();  // <Statement>

            if (lexeme == "endif") {
                // print token and lexeme
                cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 23\n";
                outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                inFile >> token >> lexeme;
            }
            else {
                // error
                cout << "Rule 23 Error: expected endif\n";
            }
        }
    }
    else {
        // error
        cout << "Rule 23 Error: expected endif or else\n";

    }
    return;
}

// Rule 11: <Put> ::= put ( <identifier> );
void r_put() {
    if (lexeme == "put") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 11\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Put> ::= put ( <identifier> );" << "\n";
            outFile << "\t <Put> ::= put ( <identifier> );" << "\n";
        }
        inFile >> token >> lexeme;

        if (lexeme == "(") {
            // print token and lexeme
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 11\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;

            if (token == "Identifier") {
                // print token and lexeme
                cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 11\n";
                outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                inFile >> token >> lexeme;

                if (lexeme == ")") {
                    // print token and lexeme
                    cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 11\n";
                    outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                    inFile >> token >> lexeme;

                    if (lexeme == ";") {
                        // print token and lexeme
                        cout << "Token: " << token << "          Lexeme: " << lexeme << "rule 11\n";
                        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                        inFile >> token >> lexeme;
                    }
                    else {
                        // error for ';'
                        cout << "Rule 11 Error: expected ;\n";
                    }
                }
                else {
                    // error for ')'
                    cout << "Rule 11 Error: expected )\n";
                }
            }
            else {
                // error for identifier
                cout << "Rule 11 Error: expected id\n";
            }
        }
        else {
            // error for '('
            cout << "Rule 11 Error: expected (\n";
        }
    }
    else {
        // error for "put"
        cout << "Rule 11 Error: expected put\n";
    }
    return;
}

// Rule 12: <Get> ::= get ( <Identifier> );
void r_get() {
    if (lexeme == "get") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 12\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Get> ::= get ( <Identifier> );" << "\n";
            outFile << "\t <Get> ::= get ( <Identifier> );" << "\n";
        }
        inFile >> token >> lexeme;

        if (lexeme == "(") {
            // print token and lexeme
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 12\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;

            if (token == "Identifier") {
                // print token and lexeme
                cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 12\n";
                outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                inFile >> token >> lexeme;

                if (lexeme == ")") {
                    // print token and lexeme
                    cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 12\n";
                    outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                    inFile >> token >> lexeme;

                    if (lexeme == ";") {
                        // print token and lexeme
                        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 12\n";
                        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                        inFile >> token >> lexeme;
                    }
                    else {
                        // error for ';'
                        cout << "Rule 12 Error: expected ;\n";
                    }
                }
                else {
                    // error for ')'
                    cout << "Rule 12 Error: expected )\n";
                }
            }
            else {
                // error for identifier
                cout << "Rule 12 Error: expected id\n";
            }
        }
        else {
            // error for '('
            cout << "Rule 12 Error: expected (\n";
        }
    }
    else {
        // error for "get"
        cout << "Rule 12 Error: expected get\n";
    }
    return;
}

// Rule 13: <While> ::= while ( <Condition> ) <Statement>  
void r_while() {
    if (lexeme == "while") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 13\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <While> ::= while ( <Condition> ) <Statement>" << "\n";
            outFile << "\t <While> ::= while ( <Condition> ) <Statement>" << "\n";
        }
        inFile >> token >> lexeme;

        if (lexeme == "(") {
            // print token and lexeme
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 13\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;

            r_condition();  // <Condition>

            if (lexeme == ")") {
                // print token and lexeme
                cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 13\n";
                outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
                inFile >> token >> lexeme;

                r_statement(); // <Statement>
            }
            else {
                // error for ')'
                cout << "Rule 13 Error: expected )\n";
            }
        }
        else {
            // error for "("
            cout << "Rule 13 Error: expected (\n";
        }
    }
    else {
        // error for "while"
        cout << "Rule 13 Error: expected while\n";
    }
    return;
}

// Rule 14: <Condition> ::= <Expression> <Relop> <Expression>
void r_condition() {
    if (printRule) {
        // print production rule
        cout << "\t <Condition> ::= <Expression> <Relop> <Expression>" << "\n";
        outFile << "\t <Condition> ::= <Expression> <Relop> <Expression>" << "\n";
    }
    r_expression();
    r_relop();
    r_expression();
    return;
}

// Rule 15: <Relop> ::= ==  |  >  |  <  |  /=    
void r_relop() {
    if (lexeme == "==" || lexeme == ">" || lexeme == "<" || lexeme == "/=") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 15\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && lexeme == "==") {
            // print production rule
            cout << "\t <Relop> ::=  ==" << "\n";
            outFile << "\t <Relop> ::=  ==" << "\n";
        }
        if (printRule && lexeme == ">") {
            // print production rule
            cout << "\t <Relop> ::=  >" << "\n";
            outFile << "\t <Relop> ::=  >" << "\n";
        }
        if (printRule && lexeme == "<") {
            // print production rule
            cout << "\t <Relop> ::=  <" << "\n";
            outFile << "\t <Relop> ::=  <" << "\n";
        }
        if (printRule && lexeme == "/=") {
            // print production rule
            cout << "\t <Relop> ::=  /=" << "\n";
            outFile << "\t <Relop> ::=  /=" << "\n";
        }
        inFile >> token >> lexeme;
    }
    else {
        // error
        cout << "Rule 15 Error: expected ==, >, <, or /=\n";
    }
    return;
}

// Rule 16: <Expression> ::= <Term> <Expression P>
void r_expression() {
    if (printRule) {
        // print production rule
        cout << "\t <Expression> ::= <Term> <Expression P>" << "\n";
        outFile << "\t <Expression> ::= <Term> <Expression P>" << "\n";
    }
    r_term();
    r_expressionP();
    return;
}

// Rule 24: <Expression P> ::= + <Term> <Expression P>  |  - <Term> <Expression P>  |  <Empty>
void r_expressionP() {
    if (lexeme == "+" || lexeme == "-") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 24\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && lexeme == "+") {
            // print production rule
            cout << "\t <Expression P> ::= + <Term> <Expression P>" << "\n";
            outFile << "\t <Expression P> ::= + <Term> <Expression P>" << "\n";
        }
        if (printRule && lexeme == "-") {
            // print production rule
            cout << "\t <Expression P> ::= - <Term> <Expression P>" << "\n";
            outFile << "\t <Expression P> ::= - <Term> <Expression P>" << "\n";
        }
        inFile >> token >> lexeme;

        r_term();
        r_expressionP();
    }
    return;
}

// Rule 17: <Term> ::= <Factor> <Term P>
void r_term() {
    if (printRule) {
        // print production rule
        cout << "\t <Term> ::= <Factor> <Term P>" << "\n";
        outFile << "\t <Term> ::= <Factor> <Term P>" << "\n";
    }
    r_factor();
    r_termP();
    return;
}

// Rule 25: <Term P> ::= * <Factor> <Term P>   |   / <Factor> <Term P>   |   <Empty>
void r_termP() {
    if (lexeme == "*" || lexeme == "/") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 25\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && lexeme == "*") {
            // print production rule
            cout << "\t <Term P> ::= * <Factor> <Term P>" << "\n";
            outFile << "\t <Term P> ::= * <Factor> <Term P>" << "\n";
        }
        if (printRule && lexeme == "/") {
            // print production rule
            cout << "\t <Term P> ::= / <Factor> <Term P>" << "\n";
            outFile << "\t <Term P> ::= / <Factor> <Term P>" << "\n";
        }
        inFile >> token >> lexeme;

        r_factor();
        r_termP();
    }
    return;
}

// Rule 18: <Factor> ::= -  <Primary>  |  <Primary>
void r_factor() {
    if (lexeme == "-") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 18\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Factor> ::=  - <Primary>" << "\n";
            outFile << "\t <Factor> ::=  - <Primary>" << "\n";
        }
        inFile >> token >> lexeme;

        r_primary();
    }
    else if (token == "Identifier" || token == "Integer" || token == "Keyword" || token == "Separator") {
        if (printRule) {
            // print production rule
            cout << "\t <Factor> ::= <Primary>" << "\n";
            outFile << "\t <Factor> ::= <Primary>" << "\n";
        }
        r_primary();
    }
    else {
        // error
        cout << "Rule 18 Error: expected -, id, int, keyword, or sep\n";
    }

    return;
}

// Rule 19: <Primary> ::= <Identifier>  |  <Integer>  |  ( <Expression> )   |  true   |  false   
void r_primary() {
    if (token == "Identifier" || token == "Integer") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 19\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && token == "Identifier") {
            // print production rule
            cout << "\t <Primary> ::= <Identifier>" << "\n";
            outFile << "\t <Primary> ::= <Identifier>" << "\n";
        }
        if (printRule && token == "Integer") {
            // print production rule
            cout << "\t <Primary> ::= <Integer>" << "\n";
            outFile << "\t <Primary> ::= <Integer>" << "\n";
        }
        inFile >> token >> lexeme;
    }
    else if (lexeme == "true" || lexeme == "false") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 19\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule && lexeme == "true") {
            // print production rule
            cout << "\t <Primary> ::= true" << "\n";
            outFile << "\t <Primary> ::= true" << "\n";
        }
        if (printRule && lexeme == "false") {
            // print production rule
            cout << "\t <Primary> ::= false" << "\n";
            outFile << "\t <Primary> ::= false" << "\n";
        }
        inFile >> token >> lexeme;
    }
    else if (lexeme == "(") {
        // print token and lexeme
        cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 19\n";
        outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
        if (printRule) {
            // print production rule
            cout << "\t <Primary> ::= ( <Expression> ) " << "\n";
            outFile << "\t <Primary> ::= ( <Expression> ) " << "\n";
        }
        inFile >> token >> lexeme;

        r_expression(); // <Expression>

        if (lexeme == ")") {
            // print token and lexeme
            cout << "Token: " << token << "          Lexeme: " << lexeme << " rule 19\n";
            outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
            inFile >> token >> lexeme;
        }
    }
    else {
        //error
        cout << "Rule 19 Error: expected id, int, bool, (, )\n";
    }
    return;
}

// Rule 20: <Empty> ::= ε
void r_empty() {
    // check eof
    return;
}
