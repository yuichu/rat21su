#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <stack>
#include <iomanip>
using namespace std;

char separators[] = { '(', ')', '[', ']', '{', '}', ',', ':', ';' };
char operators[] = { '*', '+', '-', '=', '/', '>', '<', '%', '|' };
string keywords[13] = { "integer", "float", "boolean", "true", "false", "if", "else", "while", "put", "get", "begin", "end", "endif" };
string INSTR_TABLE[400][3];  // assembly instruction
string SYMBOL_TABLE[100][2]; // symbol table
stack<int> jumpstack;

vector<char> buffer;
ifstream inFile;
ofstream outFile;
ofstream outFile2;
string token;
string lexeme;
bool printRule = true;


int s_size = 0; //size of the symbol table
int memory_address = 10000; // increment by one when a new identifier is declared and placed into the symbol table
string currId_type = "";
string currId = "";
int instr_address = 1; // increment after each new item in the instruction list

int tokenizer(char currChar, int currState);
// print(int state) -> prints token and lexeme
void print(int state);
bool isKeyword();
void lexer();

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
bool s_lookup();
int s_getAddress(string token);
void s_insert();
void s_print();
void i_print();
void gen_instruction(string op, int oprnd);
void back_patch(int jump_addr);


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
    outFile.open("Tokens.txt");

    //checks if file opens correctly
    if (!inFile)
    {
        cout << "Error: Cannot open file\n";
        exit(1);
    }

    cout << "Tokens            Lexemes\n" << endl;
    outFile << "Tokens            Lexemes\n" << endl;

    while (inFile.get(character))
    {
        currentState = tokenizer(character, currentState);
        if (currentState == 5) {
            print(currentState);
            cout << "Exiting Program\n";
            exit(EXIT_FAILURE);
        }
    }

    inFile.close();
    outFile.close();

    inFile.open("Tokens.txt");
    outFile.open("Parse.txt");
    outFile2.open(argv[2]);

    cout << endl;

    // skip over the "Token" and "Lexeme" labels,
    inFile >> token >> lexeme;
    // then get and print the first token from the list
    lexer();

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

// Get the next token from the file
void lexer() {
    inFile >> token >> lexeme;
    // print token and lexeme
    cout << "Token: " << token << "          Lexeme: " << lexeme << "\n";
    outFile << "Token: " << token << "          Lexeme: " << lexeme << "\n";
}


//--------------------------------------------------------------------------
// Following are functions for each production rule.
// Each function will begin with r_ to indicate that it's a production rule.
// Please see documentation for the production rule list.
//--------------------------------------------------------------------------

// Rule 1: <Rat21SU> ::= %% <Opt Declaration List> <Statement List> %%
void r_rat21Su() {
    if (lexeme == "%%") {
        if (printRule) {
            // print production rule
            cout << "\t <Rat21SU> ::= %% <Opt Declaration List> <Statement List> %%" << "\n";
            outFile << "\t <Rat21SU> ::= %% <Opt Declaration List> <Statement List> %%" << "\n";
        }
        lexer();

        r_optDeclarationList();
        r_statementList();
    }
    else {
        //error: Expected %%
        cout << "Rule 1 Error: expected %%\n";
        exit(EXIT_FAILURE);
    }

    if (lexeme != "%%") {
        //error: Expected %%
        cout << "Rule 1 Error: expected final %%\n";
        exit(EXIT_FAILURE);
    }
    else {
        cout << "\n------- End of Rat21SU -------\n";
    }
    cout << "\n\n";
    i_print();
    cout << "\n\n";
    outFile2 << "\n\n";
    s_print();
    return;
}

// Rule 2: <Opt Declaration List> ::= <Declaration List>  |  <Empty>
void r_optDeclarationList() {
    if (printRule) {
        // print production rule
        cout << "\t <Opt Declaration List> ::= <Declaration List>  |  <Empty>" << "\n";
        outFile << "\t <Opt Declaration List> ::= <Declaration List>  |  <Empty>" << "\n";
    }
    if (lexeme != "%%" && !inFile.eof() && (lexeme == "integer" || lexeme == "boolean")) {
        r_declarationList();
    }
    else {
        cout << "\t <Opt Declaration List> ::= <Empty>" << "\n";
        outFile << "\t <Opt Declaration List> ::= <Empty>" << "\n";
    }
}

// Rule 3: <Declaration List> ::= <Declaration> ; <Declaration List P>
void r_declarationList() {

    if (printRule) {
        // print production rule
        cout << "\t <Declaration List> ::= <Declaration> ; <Declaration List P>" << "\n";
        outFile << "\t <Declaration List> ::= <Declaration> ; <Declaration List P>" << "\n";
    }
    r_declaration();
    if (lexeme == ";") {
        // LOOKUP & INSERT TO SYMBOL TABLE
        s_insert();
        lexer();
        r_declarationListP();
    }
    else {
        //error: Expected ';'
        cout << "Rule 3 Error: expected ;\n";
        outFile << "Rule 3 Error: expected ;\n";
        exit(EXIT_FAILURE);
    }
}

// Rule 21: <Declaration List P> ::= <Declaration List>  |  <Empty>
void r_declarationListP() {
    if (printRule) {
        // print production rule
        cout << "\t <Declaration List P> ::= <Declaration List>  |  <Empty>" << "\n";
        outFile << "\t <Declaration List P> ::= <Declaration List>  |  <Empty>" << "\n";
    }
    if (lexeme != "%%" && !inFile.eof() && (lexeme == "integer" || lexeme == "boolean")) {
        r_declarationList();
    }
    else {
        if (printRule) {
            // print production rule
            cout << "\t <Declaration List P> ::= <Empty>" << "\n";
            outFile << "\t <Declaration List P> ::= <Empty>" << "\n";
        }
    }
}

// Rule 4: <Declaration> ::= <Qualifier> <identifier> 
void r_declaration() {
    if (printRule) {
        // print production rule
        cout << "\t <Declaration> ::= <Qualifier> <identifier> " << "\n";
        outFile << "\t <Declaration> ::= <Qualifier> <identifier> " << "\n";
    }
    r_qualifier(); // <Qualifier>

    if (token == "Identifier") {    // <identifier>
        currId = lexeme; // save token for symbol table
        lexer();
    }
    else {
        // error: Expected id
        cout << "Rule 4 Error: expected id\n";
        outFile << "Rule 4 Error: expected id\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 5: <Qualifier> ::= integer  |  boolean
void r_qualifier() {
    if (lexeme == "integer" || lexeme == "boolean") {
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
        currId_type = lexeme; // save token for symbol table
        lexer();
    }
    else {
        // error
        cout << "Rule 5 Error: expected integer or boolean\n";
        outFile << "Rule 5 Error: expected integer or boolean\n";
        exit(EXIT_FAILURE);
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
    if (lexeme != "%%" && lexeme != "end") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement List P> ::= <Statement List>" << "\n";
            outFile << "\t <Statement List P> ::= <Statement List>" << "\n";
        }
        r_statementList();
    }
    else {
        if (printRule) {
            // print production rule
            cout << "\t <Statement List P> ::= <Empty>" << "\n";
            outFile << "\t <Statement List P> ::= <Empty>" << "\n";
        }
    }
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
    else if (lexeme == "put") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <Put>" << "\n";
            outFile << "\t <Statement> ::= <Put>" << "\n";
        }
        r_put();
    }
    else if (lexeme == "get") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <Get>" << "\n";
            outFile << "\t <Statement> ::= <Get>" << "\n";
        }
        r_get();
    }
    else if (lexeme == "while") {
        if (printRule) {
            // print production rule
            cout << "\t <Statement> ::= <While>" << "\n";
            outFile << "\t <Statement> ::= <While>" << "\n";
        }
        r_while();
    }
    else {
        cout << "Rule 7 error: No valid statement begins for " << token << " " << lexeme << "\n";
        outFile << "Rule 7 error: No valid statement begins for " << token << " " << lexeme << "\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 8: <Compound> ::= begin <Statement List> end
void r_compound() {
    if (lexeme == "begin") {
        if (printRule) {
            // print production rule
            cout << "\t <Compound> ::= begin <Statement List> end" << "\n";
            outFile << "\t <Compound> ::= begin <Statement List> end" << "\n";
        }
        lexer();

        r_statementList();  // <Statement List>
        if (lexeme == "end") {
            lexer();
        }

    }
    else {
        // error
        cout << "Rule 8 Error: expected begin\n";
        outFile << "Rule 8 Error: expected begin\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 9: <Assign> ::= <Identifier> = <Expression> ;
void r_assign() {
    if (token == "Identifier") {
        if (printRule) {
            // print production rule
            cout << "\t <Assign> ::= <Identifier> = <Expression> ;" << "\n";
            outFile << "\t <Assign> ::= <Identifier> = <Expression> ;" << "\n";
        }
        string save = lexeme;

        lexer();

        if (lexeme == "=") {
            lexer();

            r_expression(); // <Expression>

            gen_instruction("POPM", s_getAddress(save));

            if (lexeme == ";") {
                lexer();
            }
            else {
                // error
                cout << "Rule 9 Error: expected ;\n";
                outFile << "Rule 9 Error: expected ;\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            // error
            cout << "Rule 9 Error: expected =\n";
            outFile << "Rule 9 Error: expected =\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        //error
        cout << "Rule 9 Error: expected id\n";
        outFile << "Rule 9 Error: expected id\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 10: <If> ::= if ( <Condition> ) <Statement> <If P>
void r_if() {
    if (lexeme == "if") {

        int addr = instr_address;

        if (printRule) {
            // print production rule
            cout << "\t <If> ::= if ( <Condition> ) <Statement> <If P>" << "\n";
            outFile << "\t <If> ::= if ( <Condition> ) <Statement> <If P>" << "\n";
        }
        lexer();

        if (lexeme == "(") {
            lexer();

            r_condition();  // <Condition>

            if (lexeme == ")") {
                lexer();

                r_statement();  // <Statement>

                back_patch(instr_address);

                r_ifP();    // <If P>
            }
            else {
                // error on ')'
                cout << "Rule 10 Error: expected )\n";
                outFile << "Rule 10 Error: expected )\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            // error on '('
            cout << "Rule 10 Error: expected (\n";
            outFile << "Rule 10 Error: expected (\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        // error on "if"
        cout << "Rule 10 Error: expected if\n";
        outFile << "Rule 10 Error: expected if\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 23: <If P> ::= endif  |  else <Statement> endif
void r_ifP() {
    if (lexeme == "endif" || lexeme == "else") {
        if (printRule && lexeme == "endif") {
            // print production rule
            cout << "\t <If P> ::= endif" << "\n";
            outFile << "\t <If P> ::= endif" << "\n";
            gen_instruction("LABEL", 0);
        }
        if (printRule && lexeme == "else")
        {
            // print production rule
            cout << "\t <If P> ::= else <Statement> endif" << "\n";
            outFile << "\t <If P> ::= else <Statement> endif" << "\n";
            gen_instruction("LABEL", 0);
            lexer();
            r_statement();  // <Statement>
            if (lexeme != "endif") {
                // error
                cout << "Rule 23 Error: expected endif\n";
                outFile << "Rule 23 Error: expected endif\n";
                exit(EXIT_FAILURE);
            }
        }
        lexer();
    }
    else {
        // error
        cout << "Rule 23 Error: expected endif or else\n";
        outFile << "Rule 23 Error: expected endif or else\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 11: <Put> ::= put ( <identifier> );
void r_put() {
    if (lexeme == "put") {
        if (printRule) {
            // print production rule
            cout << "\t <Put> ::= put ( <identifier> );" << "\n";
            outFile << "\t <Put> ::= put ( <identifier> );" << "\n";
        }
        lexer();

        if (lexeme == "(") {
            lexer();

            if (token == "Identifier") {

                string save = lexeme;

                lexer();

                if (lexeme == ")") {
                    lexer();

                    if (lexeme == ";") {
                        lexer();

                        gen_instruction("PUSHM", s_getAddress(save));
                        gen_instruction("STDOUT", 0);

                    }
                    else {
                        // error for ';'
                        cout << "Rule 11 Error: expected ;\n";
                        outFile << "Rule 11 Error: expected ;\n";
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    // error for ')'
                    cout << "Rule 11 Error: expected )\n";
                    outFile << "Rule 11 Error: expected )\n";
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // error for identifier
                cout << "Rule 11 Error: expected id\n";
                outFile << "Rule 11 Error: expected id\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            // error for '('
            cout << "Rule 11 Error: expected (\n";
            outFile << "Rule 11 Error: expected (\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        // error for "put"
        cout << "Rule 11 Error: expected put\n";
        outFile << "Rule 11 Error: expected put\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 12: <Get> ::= get ( <Identifier> );
void r_get() {
    if (lexeme == "get") {
        if (printRule) {
            // print production rule
            cout << "\t <Get> ::= get ( <Identifier> );" << "\n";
            outFile << "\t <Get> ::= get ( <Identifier> );" << "\n";
        }
        lexer();

        if (lexeme == "(") {
            lexer();

            if (token == "Identifier") {

                string save = lexeme;

                lexer();

                if (lexeme == ")") {
                    lexer();

                    if (lexeme == ";") {
                        lexer();

                        gen_instruction("STDIN", 0);
                        gen_instruction("POPM", s_getAddress(save));

                    }
                    else {
                        // error for ';'
                        cout << "Rule 12 Error: expected ;\n";
                        outFile << "Rule 12 Error: expected ;\n";
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    // error for ')'
                    cout << "Rule 12 Error: expected )\n";
                    outFile << "Rule 12 Error: expected )\n";
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // error for identifier
                cout << "Rule 12 Error: expected id\n";
                outFile << "Rule 12 Error: expected id\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            // error for '('
            cout << "Rule 12 Error: expected (\n";
            outFile << "Rule 12 Error: expected (\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        // error for "get"
        cout << "Rule 12 Error: expected get\n";
        outFile << "Rule 12 Error: expected get\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 13: <While> ::= while ( <Condition> ) <Statement>  
void r_while() {
    if (lexeme == "while") {

        int addr = instr_address;

        gen_instruction("LABEL", 0);

        if (printRule) {
            // print production rule
            cout << "\t <While> ::= while ( <Condition> ) <Statement>" << "\n";
            outFile << "\t <While> ::= while ( <Condition> ) <Statement>" << "\n";
        }
        lexer();

        if (lexeme == "(") {
            lexer();

            r_condition();  // <Condition>

            if (lexeme == ")") {
                lexer();

                r_statement(); // <Statement>

                // TODO: gen_instr(JUMP, addr);
                gen_instruction("JUMP", addr);
                back_patch(instr_address);
            }
            else {
                // error for ')'
                cout << "Rule 13 Error: expected )\n";
                outFile << "Rule 13 Error: expected )\n";
                exit(EXIT_FAILURE);
            }
        }
        else {
            // error for "("
            cout << "Rule 13 Error: expected (\n";
            outFile << "Rule 13 Error: expected (\n";
            exit(EXIT_FAILURE);
        }
    }
    else {
        // error for "while"
        cout << "Rule 13 Error: expected while\n";
        outFile << "Rule 13 Error: expected while\n";
        exit(EXIT_FAILURE);
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
    // If token is relop then
    // TODO: op = token;
    string op = lexeme;
    r_relop();
    r_expression();

    if (op == "<") {
        gen_instruction("LES", 0);
        jumpstack.push(instr_address);  //another stack need
        gen_instruction("JUMPZ", 0);
    }
    else if (op == ">") {
        gen_instruction("GRT", 0);
        jumpstack.push(instr_address);
        gen_instruction("JUMPZ", 0);
    }
    else if (op == "==" || op == "/=") {
        gen_instruction("EQU", 0);
        jumpstack.push(instr_address);
        gen_instruction("JUMPZ", 0);
    }
    return;
}

// Rule 15: <Relop> ::= ==  |  >  |  <  |  /=    
void r_relop() {
    if (lexeme == "==" || lexeme == ">" || lexeme == "<" || lexeme == "/=") {
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
        lexer();
    }
    else {
        // error
        cout << "Rule 15 Error: expected ==, >, <, or /=\n";
        outFile << "Rule 15 Error: expected ==, >, <, or /=\n";
        exit(EXIT_FAILURE);
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
        string save = lexeme;
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
        lexer();

        r_term();
        if (save == "+") {
            // gen_instr (ADD, nil);
            gen_instruction("ADD", 0);
        }
        if (save == "-") {
            gen_instruction("SUB", 0);
        }

        r_expressionP();
    }
    else {
        if (printRule) {
            // print production rule
            cout << "\t <Expression P> ::= <Empty>" << "\n";
            outFile << "\t <Expression P> ::= <Empty>" << "\n";
        }
    }
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
        string save = lexeme;
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
        lexer();

        r_factor();

        if (save == "*") {
            // TODO: gen_instr(MUL, nil);
            gen_instruction("MUL", 0);
        }
        if (save == "/") {
            gen_instruction("DIV", 0);
        }

        r_termP();
    }
    else {
        if (printRule) {
            // print production rule
            cout << "\t <Term P> ::= <Empty>" << "\n";
            outFile << "\t <Term P> ::= <Empty>" << "\n";
        }
    }
}

// Rule 18: <Factor> ::= -  <Primary>  |  <Primary>
void r_factor() {
    if (lexeme == "-") {
        if (printRule) {
            // print production rule
            cout << "\t <Factor> ::=  - <Primary>" << "\n";
            outFile << "\t <Factor> ::=  - <Primary>" << "\n";
        }
        lexer();

        r_primary();
    }
    else if (token == "Identifier" || token == "Integer" || token == "Keyword" || token == "Separator") {
        if (printRule) {
            // print production rule
            cout << "\t <Factor> ::= <Primary>" << "\n";
            outFile << "\t <Factor> ::= <Primary>" << "\n";
        }

        //if (token == "Identifier") {

        //}

        r_primary();
    }
    else {
        // error
        cout << "Rule 18 Error: expected -, id, int, keyword, or sep\n";
        outFile << "Rule 18 Error: expected -, id, int, keyword, or sep\n";
        exit(EXIT_FAILURE);
    }

    return;
}

// Rule 19: <Primary> ::= <Identifier>  |  <Integer>  |  ( <Expression> )   |  true   |  false   
void r_primary() {
    if (token == "Identifier" || token == "Integer") {
        if (printRule && token == "Identifier") {
            // print production rule
            cout << "\t <Primary> ::= <Identifier>" << "\n";
            outFile << "\t <Primary> ::= <Identifier>" << "\n";
            // gen_instr(PUSHM,  get_address (token));
            gen_instruction("PUSHM", s_getAddress(lexeme));
        }
        if (printRule && token == "Integer") {
            // print production rule
            cout << "\t <Primary> ::= <Integer>" << "\n";
            outFile << "\t <Primary> ::= <Integer>" << "\n";
            int n;
            n = stoi(lexeme);
            gen_instruction("PUSHI", n);
        }

        lexer();
    }
    else if (lexeme == "true" || lexeme == "false") {
        if (printRule && lexeme == "true") {
            // print production rule
            cout << "\t <Primary> ::= true" << "\n";
            outFile << "\t <Primary> ::= true" << "\n";
            gen_instruction("PUSHI", 1);
        }
        if (printRule && lexeme == "false") {
            // print production rule
            cout << "\t <Primary> ::= false" << "\n";
            outFile << "\t <Primary> ::= false" << "\n";
            gen_instruction("PUSHI", 0);
        }
        lexer();
    }
    else if (lexeme == "(") {
        if (printRule) {
            // print production rule
            cout << "\t <Primary> ::= ( <Expression> ) " << "\n";
            outFile << "\t <Primary> ::= ( <Expression> ) " << "\n";
        }
        lexer();

        r_expression(); // <Expression>

        if (lexeme == ")") {
            lexer();
        }
    }
    else {
        //error
        cout << "Rule 19 Error: expected id, int, bool, (, )\n";
        outFile << "Rule 19 Error: expected id, int, bool, (, )\n";
        exit(EXIT_FAILURE);
    }
    return;
}

// Rule 20: <Empty> ::= ε
void r_empty() {
    // check eof
    return;
}


//--------------------------------------------------------------------------
// Following are procedures for symbol table.
// Each function will begin with s_ to indicate that it's a symbol table procedure.
//--------------------------------------------------------------------------

// return true if identifier already exists
bool s_lookup() {
    for (int i = 0; i < s_size; i++) {
        if (currId == SYMBOL_TABLE[i][0])
            return 1;
    }
    return 0;
}

// return address of identifier from SYMBOL_TABLE
int s_getAddress(string token) {
    for (int i = 0; i < s_size; i++) {
        if (token == SYMBOL_TABLE[i][0])
            return i + 10000;
    }
    cout << "s_getAddress() error: Token " << token << " does not exist in the symbol table.\n";
    exit(EXIT_FAILURE);
}

// insert identifier, address, and type into SYMBOL_TABLE
void s_insert() {
    if (s_lookup() == 0) {
        SYMBOL_TABLE[s_size][0] = currId;
        SYMBOL_TABLE[s_size][1] = currId_type;
        s_size++;
    }
    else {
        cout << "Indentifier already exists in the table. The same identifier cannot be declared more than once.\n";
        exit(EXIT_FAILURE);
    }
}

// output symbol table
void s_print() {
    cout << "             SYMBOL TABLE         \n\n";
    cout << left;
    cout << setw(15) << "Identifier" << setw(18) << "Memory Location" << setw(8) << "Type" << endl;
    outFile2 << "             SYMBOL TABLE         \n\n";
    outFile2 << left;
    outFile2 << setw(15) << "Identifier" << setw(18) << "Memory Location" << setw(8) << "Type" << endl;
    for (int i = 0; i < s_size; i++) {
        cout << setw(15) << SYMBOL_TABLE[i][0] << setw(18) << i + 10000 << SYMBOL_TABLE[i][1] << endl;
        outFile2 << setw(15) << SYMBOL_TABLE[i][0] << setw(18) << i + 10000 << SYMBOL_TABLE[i][1] << endl;
    }
    return;
}

// ouput assembly instructions
void i_print() {
    cout << "Assembly Instructions\n\n";
    outFile2 << "Assembly Instructions\n\n";
    for (int i = 1; i < instr_address; i++) {
        cout << left << setw(6) << INSTR_TABLE[i][0] << setw(10) << INSTR_TABLE[i][1] << INSTR_TABLE[i][2] << endl;
        outFile2 << left << setw(6) << INSTR_TABLE[i][0] << setw(10) << INSTR_TABLE[i][1] << INSTR_TABLE[i][2] << endl;
    }
    return;
}

//--------------------------------------------------------------------------
// Following is the procedure for generating assembly instructions
//--------------------------------------------------------------------------

void gen_instruction(string op, int oprnd) {

    string s_instr_address = to_string(instr_address); // This conversion prolly works but idk ill check later
    string s_oprnd = to_string(oprnd);
    INSTR_TABLE[instr_address][0] = s_instr_address;
    INSTR_TABLE[instr_address][1] = op;
    if (op == "STDIN" || op == "STDOUT" || op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" || op == "GRT" || op == "LES" || op == "EQU" || op == "JUMPZ" || op == "LABEL")
        INSTR_TABLE[instr_address][2] = "";
    else
        INSTR_TABLE[instr_address][2] = s_oprnd;
    instr_address++;

    // note, instr_address should begin at 1 NOT 0!!
    return;
}
// set JUMPZ operand
void back_patch(int jump_addr) {
    int addr = 0;
    addr = jumpstack.top();
    jumpstack.pop();
    INSTR_TABLE[addr][2] = to_string(jump_addr);
}
