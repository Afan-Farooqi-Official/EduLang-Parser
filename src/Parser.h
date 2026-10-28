// ============================================================
//  Parser.h  —  Recursive-Descent Parser for EduLang
// ============================================================
#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <memory>
#include "Token.h"
#include "ASTNode.h"

// ── Parse Error record ────────────────────────────────────────
struct ParseError {
    std::string message;
    std::string found;      // what token was actually seen
    int line;
};

// ============================================================
//  Parser class
//
//  Grammar (EduLang):
//
//  program        → ENROLL block GRADUATE EOF
//  block          → LBRACE stmt_list RBRACE
//  stmt_list      → stmt*
//  stmt           → var_decl
//                 | const_decl
//                 | assign_stmt
//                 | print_stmt
//                 | input_stmt
//                 | check_stmt
//                 | repeat_stmt
//  var_decl       → DT ID (ASSIGN expr)? SEMI
//  const_decl     → RK DT ID ASSIGN expr SEMI
//  assign_stmt    → ID ASSIGN expr SEMI
//  print_stmt     → PRINT LPAREN expr RPAREN SEMI
//  input_stmt     → INPUT LPAREN ID RPAREN SEMI
//  check_stmt     → CHECK LPAREN cond_expr RPAREN block
//                   (OTHERWISE block)?
//  repeat_stmt    → REPEAT block
//
//  cond_expr      → or_expr
//  or_expr        → and_expr (OR and_expr)*
//  and_expr       → not_expr (AND not_expr)*
//  not_expr       → NOT not_expr | rel_expr
//  rel_expr       → expr (relop expr)?
//  relop          → EQUAL | NEQ | LT | LTE | GT | GTE
//
//  expr           → term ((PLUS | MINUS) term)*
//  term           → unary ((MUL | DIV | MOD) unary)*
//  unary          → MINUS unary | primary
//  primary        → NUM | CHAR_LIT | STRING_LIT | ID
//                 | LPAREN expr RPAREN
// ============================================================

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    // Main entry point — returns root AST node
    std::unique_ptr<ASTNode> parse();

    const std::vector<ParseError>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }

private:
    // ── Token stream management ───────────────────────────────
    std::vector<Token> tokens;
    size_t pos;

    Token peek() const;                         // current token
    Token peekNext() const;                     // lookahead by 1
    Token advance();                            // consume & return
    bool check(TokenType t) const;              // match without consuming
    bool match(TokenType t);                    // consume if match
    Token expect(TokenType t, const std::string& msg);  // consume or error

    bool isAtEnd() const;
    int currentLine() const;

    // ── Error handling & panic-mode recovery ──────────────────
    std::vector<ParseError> errors;
    void recordError(const std::string& msg, const Token& found);
    // Synchronise: skip tokens until a safe restart point
    void synchronize();

    // ── Grammar rules (each returns an AST node) ─────────────
    std::unique_ptr<ASTNode> parseProgram();
    std::unique_ptr<ASTNode> parseBlock();
    std::unique_ptr<ASTNode> parseStmtList();
    std::unique_ptr<ASTNode> parseStmt();

    std::unique_ptr<ASTNode> parseVarDecl();
    std::unique_ptr<ASTNode> parseConstDecl();
    std::unique_ptr<ASTNode> parseAssignStmt();
    std::unique_ptr<ASTNode> parsePrintStmt();
    std::unique_ptr<ASTNode> parseInputStmt();
    std::unique_ptr<ASTNode> parseCheckStmt();
    std::unique_ptr<ASTNode> parseRepeatStmt();

    // Expressions
    std::unique_ptr<ASTNode> parseCondExpr();
    std::unique_ptr<ASTNode> parseOrExpr();
    std::unique_ptr<ASTNode> parseAndExpr();
    std::unique_ptr<ASTNode> parseNotExpr();
    std::unique_ptr<ASTNode> parseRelExpr();

    std::unique_ptr<ASTNode> parseExpr();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseUnary();
    std::unique_ptr<ASTNode> parsePrimary();

    // Helpers
    bool isRelop(TokenType t) const;
    bool isStatementStart() const;
};

#endif // PARSER_H
