// ============================================================
//  main.cpp  —  EduLang Recursive-Descent Parser
//  Compiler Construction Project — Phase 2: Syntax Analysis
// ============================================================
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

#include "Lexer.h"
#include "Parser.h"
#include "ASTNode.h"

using namespace std;

// ── ANSI Colors ───────────────────────────────────────────────
#define RESET    "\033[0m"
#define BOLD     "\033[1m"
#define RED      "\033[31m"
#define GREEN    "\033[32m"
#define YELLOW   "\033[33m"
#define CYAN     "\033[36m"
#define MAGENTA  "\033[35m"
#define WHITE    "\033[97m"
#define BLUE     "\033[34m"

// ─────────────────────────────────────────────────────────────
//  PRINT HELPERS
// ─────────────────────────────────────────────────────────────

void printLine(char ch = '-', int w = 70) {
    cout << string(w, ch) << "\n";
}

void printDoubleLine(int w = 70) {
    cout << string(w, '=') << "\n";
}

void printHeader() {
    printDoubleLine();
    cout << BOLD << CYAN;
    cout << "       EduLang — Recursive Descent Parser\n";
    cout << "       Compiler Construction Project\n";
    cout << "       Phase 2: Syntax Analysis + AST Generation\n";
    cout << RESET;
    printDoubleLine();
    cout << "\n";
}

// ─────────────────────────────────────────────────────────────
//  PHASE 1: Show Lexer Token Table (like your lexer output)
// ─────────────────────────────────────────────────────────────

void printTokenTable(const vector<Token>& tokens, const vector<LexicalError>& errors) {
    cout << BOLD << CYAN;
    printDoubleLine();
    cout << "  PHASE 1 — LEXICAL ANALYSIS  (Token Stream)\n";
    printDoubleLine();
    cout << RESET;

    // Token table header
    cout << BOLD
         << left << setw(25) << "  Lexeme"
         << left << setw(20) << "Token Type"
         << left << setw(8)  << "Line"
         << "\n" << RESET;
    printLine();

    for (const auto& t : tokens) {
        if (t.type == TokenType::END_OF_FILE) continue;
        if (t.type == TokenType::TOKEN_ERROR) {
            cout << RED
                 << left << setw(25) << ("  " + t.lexeme)
                 << left << setw(20) << t.typeToString()
                 << left << setw(8)  << t.line
                 << "\n" << RESET;
        } else {
            cout << left << setw(25) << ("  " + t.lexeme)
                 << left << setw(20) << t.typeToString()
                 << left << setw(8)  << t.line
                 << "\n";
        }
    }

    printLine();
    cout << "  Total Tokens : " << BOLD << (tokens.size() - 1) << RESET << "\n";

    // Lexical errors
    if (!errors.empty()) {
        cout << "\n" << RED << BOLD;
        cout << "  LEXICAL ERRORS DETECTED:\n" << RESET;
        printLine('-', 70);
        cout << BOLD
             << left << setw(22) << "  Category"
             << left << setw(20) << "Lexeme"
             << left << setw(8)  << "Line"
             << "Description\n" << RESET;
        printLine();
        for (const auto& e : errors) {
            cout << RED
                 << left << setw(22) << ("  " + e.categoryStr)
                 << left << setw(20) << e.lexeme
                 << left << setw(8)  << e.line
                 << e.message << "\n" << RESET;
        }
        printLine();
        cout << RED << "  Total Lexical Errors : " << errors.size() << RESET << "\n";
    } else {
        cout << GREEN << "\n  [SUCCESS] Lexical Analysis completed with 0 errors!\n" << RESET;
    }

    cout << "\n";
}

// ─────────────────────────────────────────────────────────────
//  PHASE 2: Show Grammar Rules Applied
// ─────────────────────────────────────────────────────────────

void printGrammarRules() {
    cout << BOLD << CYAN;
    printDoubleLine();
    cout << "  PHASE 2 — SYNTAX ANALYSIS  (Grammar Rules Applied)\n";
    printDoubleLine();
    cout << RESET;

    cout << BOLD << "  EduLang Grammar (Recursive-Descent):\n\n" << RESET;

    cout << YELLOW;
    cout << "  program     →  enroll  block  graduate\n";
    cout << "  block       →  {  stmt_list  }\n";
    cout << "  stmt_list   →  stmt*\n";
    cout << "  stmt        →  var_decl | const_decl | assign_stmt\n";
    cout << "               | print_stmt | input_stmt\n";
    cout << "               | check_stmt | repeat_stmt\n\n";

    cout << "  var_decl    →  DT  ID  ( = expr )?  ;\n";
    cout << "  const_decl  →  fixed  DT  ID  =  expr  ;\n";
    cout << "  assign_stmt →  ID  =  expr  ;\n";
    cout << "  print_stmt  →  print  ( expr )  ;\n";
    cout << "  input_stmt  →  input  ( ID )    ;\n";
    cout << "  check_stmt  →  check  ( cond )  block  ( otherwise block )?\n";
    cout << "  repeat_stmt →  repeat  block\n\n";

    cout << "  cond_expr   →  or_expr\n";
    cout << "  or_expr     →  and_expr  ( ||  and_expr )*\n";
    cout << "  and_expr    →  not_expr  ( &&  not_expr )*\n";
    cout << "  not_expr    →  !  not_expr  |  rel_expr\n";
    cout << "  rel_expr    →  expr  ( relop  expr )?\n";
    cout << "  relop       →  ==  |  !=  |  <  |  <=  |  >  |  >=\n\n";

    cout << "  expr        →  term  ( ( + | - )  term )*\n";
    cout << "  term        →  unary  ( ( * | / | % )  unary )*\n";
    cout << "  unary       →  -  unary  |  primary\n";
    cout << "  primary     →  NUM | CHAR_LIT | STRING_LIT | ID | ( expr )\n";
    cout << RESET << "\n";
}

// ─────────────────────────────────────────────────────────────
//  Show Parse Errors
// ─────────────────────────────────────────────────────────────

void printParseErrors(const vector<ParseError>& errors) {
    if (errors.empty()) {
        cout << GREEN << "  [SUCCESS] Syntax Analysis completed with 0 errors!\n" << RESET;
    } else {
        cout << RED << BOLD << "  SYNTAX ERRORS DETECTED (Panic-Mode Recovery Applied):\n" << RESET;
        printLine();
        cout << BOLD
             << left << setw(8)  << "  Line"
             << left << setw(45) << "Error Message"
             << "Found Token\n" << RESET;
        printLine();
        for (const auto& e : errors) {
            cout << RED
                 << left << setw(8)  << ("  " + to_string(e.line))
                 << left << setw(45) << e.message
                 << e.found << "\n" << RESET;
        }
        printLine();
        cout << RED << "  Total Syntax Errors : " << errors.size() << RESET << "\n";
        cout << YELLOW << "  Note: Parser recovered and continued after each error.\n" << RESET;
    }
    cout << "\n";
}

// ─────────────────────────────────────────────────────────────
//  PHASE 3: AST
// ─────────────────────────────────────────────────────────────

void printAST(const unique_ptr<ASTNode>& ast) {
    cout << BOLD << CYAN;
    printDoubleLine();
    cout << "  PHASE 3 — ABSTRACT SYNTAX TREE  (AST)\n";
    printDoubleLine();
    cout << RESET;

    cout << BOLD << "  AST Node Legend:\n" << RESET;
    cout << "  Program      → Whole EduLang program (enroll...graduate)\n";
    cout << "  Block        → A { } block of statements\n";
    cout << "  StmtList     → List of statements inside a block\n";
    cout << "  VarDecl      → Variable declaration   (score / grade)\n";
    cout << "  ConstDecl    → Constant declaration   (fixed)\n";
    cout << "  AssignStmt   → Assignment statement   (x = expr)\n";
    cout << "  CheckStmt    → If-else statement      (check / otherwise)\n";
    cout << "  RepeatStmt   → Loop statement         (repeat)\n";
    cout << "  PrintStmt    → Print statement        (print)\n";
    cout << "  InputStmt    → Input statement        (input)\n";
    cout << "  BinaryExpr   → Binary operation       (a + b, x >= 50)\n";
    cout << "  UnaryExpr    → Unary operation        (!x, -y)\n";
    cout << "  Identifier   → Variable name          (marks, result)\n";
    cout << "  NumLit       → Number value           (75, 50)\n";
    cout << "  CharLit      → Character value        ('P', 'F')\n";
    cout << "  StringLit    → String value           (\"hello\")\n";
    printLine();
    cout << "\n";

    if (ast) {
        ast->print(cout);
    } else {
        cout << RED << "  (No AST — critical errors prevented parsing)\n" << RESET;
    }
    cout << "\n";
}

// ─────────────────────────────────────────────────────────────
//  Final Summary
// ─────────────────────────────────────────────────────────────

void printSummary(const string& sourceLabel,
                  size_t tokenCount,
                  const vector<LexicalError>& lexErrors,
                  const vector<ParseError>& parseErrors) {
    printDoubleLine();
    cout << BOLD << "  COMPILER ANALYSIS SUMMARY\n" << RESET;
    printDoubleLine();

    cout << "  Source File       : " << BOLD << sourceLabel << RESET << "\n";
    cout << "  Tokens Produced   : " << BOLD << tokenCount  << RESET << "\n";
    cout << "  Lexical Errors    : ";
    if (lexErrors.empty())   cout << GREEN << BOLD << "0  [PASS]\n" << RESET;
    else                     cout << RED   << BOLD << lexErrors.size()   << "  [FAIL]\n" << RESET;

    cout << "  Syntax Errors     : ";
    if (parseErrors.empty()) cout << GREEN << BOLD << "0  [PASS]\n" << RESET;
    else                     cout << RED   << BOLD << parseErrors.size() << "  [FAIL]\n" << RESET;

    cout << "\n";

    cout << "  Techniques Used   :\n";
    cout << "    - DFA-based Lexical Analysis (Phase 1)\n";
    cout << "    - Recursive Descent Parsing  (Phase 2)\n";
    cout << "    - Panic-Mode Error Recovery  (Phase 2)\n";
    cout << "    - AST Construction           (Phase 3)\n";
    cout << "\n";

    int total = (int)lexErrors.size() + (int)parseErrors.size();
    printDoubleLine();
    if (total == 0) {
        cout << GREEN << BOLD;
        cout << "  RESULT : Program is Lexically and Syntactically VALID!\n";
        cout << RESET;
    } else {
        cout << RED << BOLD;
        cout << "  RESULT : " << total << " error(s) found. See details above.\n";
        cout << RESET;
    }
    printDoubleLine();
}

// ─────────────────────────────────────────────────────────────
//  MAIN PIPELINE
// ─────────────────────────────────────────────────────────────

void runPipeline(const string& sourceCode, const string& label, bool saveOutput) {

    // ── Phase 1: Lex ────────────────────────────────────────
    Lexer lexer(sourceCode);
    vector<Token> tokens = lexer.tokenize();
    printTokenTable(tokens, lexer.getErrors());

    // ── Phase 2: Parse ──────────────────────────────────────
    printGrammarRules();

    vector<Token> cleanTokens;
    for (const auto& tok : tokens)
        if (tok.type != TokenType::TOKEN_ERROR)
            cleanTokens.push_back(tok);

    Parser parser(cleanTokens);
    unique_ptr<ASTNode> ast = parser.parse();
    printParseErrors(parser.getErrors());

    // ── Phase 3: AST ────────────────────────────────────────
    printAST(ast);

    // ── Summary ─────────────────────────────────────────────
    printSummary(label, tokens.size() - 1, lexer.getErrors(), parser.getErrors());

    // ── Save output ─────────────────────────────────────────
    if (saveOutput && ast) {
        ofstream ofs("output/parse_output.txt");
        if (ofs) {
            ofs << "EduLang Recursive Descent Parser — Output\n";
            ofs << "Source: " << label << "\n\n";

            ofs << "=== TOKEN STREAM ===\n";
            ofs << left << setw(25) << "Lexeme"
                << left << setw(20) << "Token Type"
                << "Line\n";
            ofs << string(55, '-') << "\n";
            for (const auto& t : tokens) {
                if (t.type == TokenType::END_OF_FILE) continue;
                ofs << left << setw(25) << t.lexeme
                    << left << setw(20) << t.typeToString()
                    << t.line << "\n";
            }

            ofs << "\n=== LEXICAL ERRORS ===\n";
            if (lexer.getErrors().empty()) {
                ofs << "No lexical errors.\n";
            } else {
                for (const auto& e : lexer.getErrors())
                    ofs << "[Line " << e.line << "] " << e.message << "\n";
            }

            ofs << "\n=== SYNTAX ERRORS ===\n";
            if (parser.getErrors().empty()) {
                ofs << "No syntax errors.\n";
            } else {
                for (const auto& e : parser.getErrors())
                    ofs << "[Line " << e.line << "] " << e.message
                        << " | Found: " << e.found << "\n";
            }

            ofs << "\n=== ABSTRACT SYNTAX TREE ===\n";
            ast->print(ofs);

            cout << CYAN << "\n  Output saved to: output/parse_output.txt\n" << RESET;
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  File Loader
// ─────────────────────────────────────────────────────────────

bool loadFile(const string& path, string& out) {
    ifstream ifs(path);
    if (!ifs.is_open()) {
        cerr << RED << "  Error: Cannot open '" << path << "'\n" << RESET;
        return false;
    }
    ostringstream ss;
    ss << ifs.rdbuf();
    out = ss.str();
    return true;
}

// ─────────────────────────────────────────────────────────────
//  Interactive Input
// ─────────────────────────────────────────────────────────────

string readCodeFromConsole() {
    cout << CYAN;
    cout << "  Enter EduLang code below.\n";
    cout << "  Type END on a new line when done:\n" << RESET;
    printLine();
    string code, line;
    while (getline(cin, line)) {
        if (line == "END") break;
        code += line + "\n";
    }
    return code;
}

// ─────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
#ifdef _WIN32
    system("chcp 65001 > nul");
    system("");
#endif

    printHeader();

    // Direct file argument
    if (argc >= 2) {
        string src;
        if (loadFile(argv[1], src))
            runPipeline(src, argv[1], true);
        return 0;
    }

    // Interactive menu
    while (true) {
        cout << BOLD << CYAN;
        printDoubleLine();
        cout << "       EduLang Parser — Main Menu\n";
        printDoubleLine();
        cout << RESET;
        cout << "  1. Type / Paste EduLang code\n";
        cout << "  2. Load a .edu file\n";
        cout << "  3. Run  → test_program.edu   (Valid program)\n";
        cout << "  4. Run  → edge_cases.edu     (Complex expressions)\n";
        cout << "  5. Run  → syntax_errors.edu  (Error recovery demo)\n";
        cout << "  6. Exit\n";
        printLine();
        cout << "  Enter choice (1-6): ";

        int choice;
        if (!(cin >> choice)) break;
        cin.ignore();

        string source;
        cout << "\n";
        printHeader();

        switch (choice) {
            case 1:
                source = readCodeFromConsole();
                runPipeline(source, "<console input>", false);
                break;
            case 2: {
                cout << "  Enter file path: ";
                string path; getline(cin, path);
                if (loadFile(path, source))
                    runPipeline(source, path, true);
                break;
            }
            case 3:
                if (loadFile("tests/test_program.edu", source))
                    runPipeline(source, "tests/test_program.edu", true);
                break;
            case 4:
                if (loadFile("tests/edge_cases.edu", source))
                    runPipeline(source, "tests/edge_cases.edu", true);
                break;
            case 5:
                if (loadFile("tests/syntax_errors.edu", source))
                    runPipeline(source, "tests/syntax_errors.edu", true);
                break;
            case 6:
                cout << GREEN << "  Goodbye!\n" << RESET;
                return 0;
            default:
                cout << RED << "  Invalid choice.\n" << RESET;
        }
    }

    return 0;
}
