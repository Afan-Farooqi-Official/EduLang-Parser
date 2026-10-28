// ============================================================
//  Parser.cpp  —  Recursive-Descent Parser implementation
//  EduLang — Compiler Construction Project
// ============================================================
#include "Parser.h"
#include <stdexcept>
#include <sstream>

// ═══════════════════════════════════════════════════════════
//  Constructor
// ═══════════════════════════════════════════════════════════
Parser::Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}

// ═══════════════════════════════════════════════════════════
//  Token stream helpers
// ═══════════════════════════════════════════════════════════

Token Parser::peek() const {
    if (pos < tokens.size()) return tokens[pos];
    return Token{TokenType::END_OF_FILE, "", 0};
}

Token Parser::peekNext() const {
    if (pos + 1 < tokens.size()) return tokens[pos + 1];
    return Token{TokenType::END_OF_FILE, "", 0};
}

Token Parser::advance() {
    if (!isAtEnd()) pos++;
    return tokens[pos - 1];
}

bool Parser::check(TokenType t) const {
    return peek().type == t;
}

bool Parser::match(TokenType t) {
    if (check(t)) {
        advance();
        return true;
    }
    return false;
}

// Consume the expected token type; record an error if it's missing.
Token Parser::expect(TokenType t, const std::string& msg) {
    if (check(t)) {
        return advance();
    }
    // Error — record but keep the current token for recovery
    recordError(msg, peek());
    // Return a synthetic token so callers don't crash
    return Token{t, "", currentLine()};
}

bool Parser::isAtEnd() const {
    return pos >= tokens.size() || tokens[pos].type == TokenType::END_OF_FILE;
}

int Parser::currentLine() const {
    return peek().line;
}

// ═══════════════════════════════════════════════════════════
//  Error handling & panic-mode recovery
// ═══════════════════════════════════════════════════════════

void Parser::recordError(const std::string& msg, const Token& found) {
    std::string foundStr = "'" + found.lexeme + "' (" + found.typeToString() + ")";
    errors.push_back(ParseError{msg, foundStr, found.line});
}

// Panic-mode synchronisation:
// Skip tokens until we find something that could begin a new statement,
// or a closing brace, or EOF.
void Parser::synchronize() {
    while (!isAtEnd()) {
        TokenType t = peek().type;

        // A semicolon ends the current statement — skip past it and stop
        if (t == TokenType::SEMI) {
            advance();
            return;
        }

        // A closing brace ends a block — don't consume it, let the caller handle it
        if (t == TokenType::RBRACE) return;

        // Any statement-starter is a safe restart point
        if (t == TokenType::DT      ||
            t == TokenType::RK      ||
            t == TokenType::ID      ||
            t == TokenType::PRINT   ||
            t == TokenType::INPUT   ||
            t == TokenType::CHECK   ||
            t == TokenType::REPEAT  ||
            t == TokenType::GRADUATE) {
            return;
        }

        advance(); // skip the offending token
    }
}

// ═══════════════════════════════════════════════════════════
//  Top-level entry point
// ═══════════════════════════════════════════════════════════

// program → ENROLL block GRADUATE EOF
std::unique_ptr<ASTNode> Parser::parse() {
    return parseProgram();
}

std::unique_ptr<ASTNode> Parser::parseProgram() {
    auto node = makeNode(NodeKind::PROGRAM, "program", currentLine());

    // Expect 'enroll'
    if (!match(TokenType::ENROLL)) {
        recordError("Expected 'enroll' at the start of program", peek());
        synchronize();
    }

    // Parse the main block { ... }
    if (check(TokenType::LBRACE)) {
        node->addChild(parseBlock());
    } else {
        recordError("Expected '{' after 'enroll'", peek());
        synchronize();
    }

    // Expect 'graduate'
    if (!match(TokenType::GRADUATE)) {
        recordError("Expected 'graduate' at the end of program", peek());
    }

    // Expect EOF
    if (!isAtEnd()) {
        recordError("Unexpected tokens after 'graduate'", peek());
    }

    return node;
}

// ═══════════════════════════════════════════════════════════
//  Block  { stmt* }
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseBlock() {
    int ln = currentLine();
    expect(TokenType::LBRACE, "Expected '{' to open block");
    auto block = makeNode(NodeKind::BLOCK, "{}", ln);
    block->addChild(parseStmtList());
    expect(TokenType::RBRACE, "Expected '}' to close block");
    return block;
}

// ═══════════════════════════════════════════════════════════
//  Statement list  stmt*
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseStmtList() {
    auto list = makeNode(NodeKind::STMT_LIST, "", currentLine());

    while (!isAtEnd() && !check(TokenType::RBRACE) && !check(TokenType::GRADUATE)) {
        try {
            auto stmt = parseStmt();
            if (stmt) list->addChild(std::move(stmt));
        } catch (...) {
            // Safety net — should not normally reach here
            synchronize();
        }
    }

    return list;
}

// ═══════════════════════════════════════════════════════════
//  Statement dispatcher
// ═══════════════════════════════════════════════════════════

bool Parser::isStatementStart() const {
    TokenType t = peek().type;
    return t == TokenType::DT     ||
           t == TokenType::RK     ||
           t == TokenType::ID     ||
           t == TokenType::PRINT  ||
           t == TokenType::INPUT  ||
           t == TokenType::CHECK  ||
           t == TokenType::REPEAT;
}

std::unique_ptr<ASTNode> Parser::parseStmt() {
    TokenType t = peek().type;

    if (t == TokenType::RK) {
        return parseConstDecl();        // fixed score x = ...;
    }
    if (t == TokenType::DT) {
        return parseVarDecl();          // score x = ...;  /  grade ch;
    }
    if (t == TokenType::ID) {
        return parseAssignStmt();       // x = expr;
    }
    if (t == TokenType::PRINT) {
        return parsePrintStmt();        // print(expr);
    }
    if (t == TokenType::INPUT) {
        return parseInputStmt();        // input(id);
    }
    if (t == TokenType::CHECK) {
        return parseCheckStmt();        // check(...) { } otherwise { }
    }
    if (t == TokenType::REPEAT) {
        return parseRepeatStmt();       // repeat { }
    }

    // Unknown token — error + synchronize
    recordError("Unexpected token; expected a statement", peek());
    advance(); // consume the bad token
    synchronize();
    return nullptr;
}

// ═══════════════════════════════════════════════════════════
//  Variable Declaration
//  var_decl → DT ID (ASSIGN expr)? SEMI
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseVarDecl() {
    int ln = currentLine();
    Token dt = advance();  // consume DT (score / grade)

    auto node = makeNode(NodeKind::VAR_DECL, dt.lexeme, ln);

    Token id = expect(TokenType::ID, "Expected identifier after data type '" + dt.lexeme + "'");
    node->addChild(makeNode(NodeKind::IDENTIFIER, id.lexeme, id.line));

    // Optional initialiser
    if (match(TokenType::ASSIGN)) {
        node->addChild(parseExpr());
    }

    expect(TokenType::SEMI, "Expected ';' after variable declaration");
    return node;
}

// ═══════════════════════════════════════════════════════════
//  Constant Declaration
//  const_decl → RK DT ID ASSIGN expr SEMI
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseConstDecl() {
    int ln = currentLine();
    advance();  // consume 'fixed'

    auto node = makeNode(NodeKind::CONST_DECL, "fixed", ln);

    Token dt = expect(TokenType::DT, "Expected data type ('score'/'grade') after 'fixed'");
    node->addChild(makeNode(NodeKind::IDENTIFIER, dt.lexeme, dt.line));

    Token id = expect(TokenType::ID, "Expected constant name after '" + dt.lexeme + "'");
    node->addChild(makeNode(NodeKind::IDENTIFIER, id.lexeme, id.line));

    expect(TokenType::ASSIGN, "Expected '=' after constant name (constants must be initialised)");
    node->addChild(parseExpr());

    expect(TokenType::SEMI, "Expected ';' after constant declaration");
    return node;
}

// ═══════════════════════════════════════════════════════════
//  Assignment Statement
//  assign_stmt → ID ASSIGN expr SEMI
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseAssignStmt() {
    int ln = currentLine();
    Token id = advance();  // consume ID

    auto node = makeNode(NodeKind::ASSIGN_STMT, "=", ln);
    node->addChild(makeNode(NodeKind::IDENTIFIER, id.lexeme, id.line));

    expect(TokenType::ASSIGN, "Expected '=' after identifier '" + id.lexeme + "'");
    node->addChild(parseExpr());

    expect(TokenType::SEMI, "Expected ';' after assignment");
    return node;
}

// ═══════════════════════════════════════════════════════════
//  Print Statement
//  print_stmt → PRINT LPAREN expr RPAREN SEMI
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parsePrintStmt() {
    int ln = currentLine();
    advance();  // consume 'print'

    auto node = makeNode(NodeKind::PRINT_STMT, "print", ln);

    expect(TokenType::LPAREN, "Expected '(' after 'print'");
    node->addChild(parseExpr());
    expect(TokenType::RPAREN, "Expected ')' after print expression");
    expect(TokenType::SEMI, "Expected ';' after print statement");

    return node;
}

// ═══════════════════════════════════════════════════════════
//  Input Statement
//  input_stmt → INPUT LPAREN ID RPAREN SEMI
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseInputStmt() {
    int ln = currentLine();
    advance();  // consume 'input'

    auto node = makeNode(NodeKind::INPUT_STMT, "input", ln);

    expect(TokenType::LPAREN, "Expected '(' after 'input'");
    Token id = expect(TokenType::ID, "Expected identifier inside input()");
    node->addChild(makeNode(NodeKind::IDENTIFIER, id.lexeme, id.line));
    expect(TokenType::RPAREN, "Expected ')' after input variable");
    expect(TokenType::SEMI, "Expected ';' after input statement");

    return node;
}

// ═══════════════════════════════════════════════════════════
//  Check (if) Statement
//  check_stmt → CHECK LPAREN cond_expr RPAREN block (OTHERWISE block)?
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseCheckStmt() {
    int ln = currentLine();
    advance();  // consume 'check'

    auto node = makeNode(NodeKind::CHECK_STMT, "check", ln);

    expect(TokenType::LPAREN, "Expected '(' after 'check'");
    node->addChild(parseCondExpr());          // condition
    expect(TokenType::RPAREN, "Expected ')' after check condition");

    node->addChild(parseBlock());             // then-block

    if (match(TokenType::OTHERWISE)) {        // optional else
        node->addChild(parseBlock());
    }

    return node;
}

// ═══════════════════════════════════════════════════════════
//  Repeat (loop) Statement
//  repeat_stmt → REPEAT block
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseRepeatStmt() {
    int ln = currentLine();
    advance();  // consume 'repeat'

    auto node = makeNode(NodeKind::REPEAT_STMT, "repeat", ln);
    node->addChild(parseBlock());

    return node;
}

// ═══════════════════════════════════════════════════════════
//  Condition Expressions  (Boolean)
//
//  cond_expr → or_expr
//  or_expr   → and_expr (OR and_expr)*
//  and_expr  → not_expr (AND not_expr)*
//  not_expr  → NOT not_expr | rel_expr
//  rel_expr  → expr (relop expr)?
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseCondExpr() {
    return parseOrExpr();
}

std::unique_ptr<ASTNode> Parser::parseOrExpr() {
    auto left = parseAndExpr();

    while (check(TokenType::OR)) {
        Token op = advance();
        auto right = parseAndExpr();
        auto node = makeNode(NodeKind::BINARY_EXPR, op.lexeme, op.line);
        node->addChild(std::move(left));
        node->addChild(std::move(right));
        left = std::move(node);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseAndExpr() {
    auto left = parseNotExpr();

    while (check(TokenType::AND)) {
        Token op = advance();
        auto right = parseNotExpr();
        auto node = makeNode(NodeKind::BINARY_EXPR, op.lexeme, op.line);
        node->addChild(std::move(left));
        node->addChild(std::move(right));
        left = std::move(node);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseNotExpr() {
    if (check(TokenType::NOT)) {
        Token op = advance();
        auto node = makeNode(NodeKind::UNARY_EXPR, op.lexeme, op.line);
        node->addChild(parseNotExpr());
        return node;
    }
    return parseRelExpr();
}

bool Parser::isRelop(TokenType t) const {
    return t == TokenType::EQUAL ||
           t == TokenType::NEQ   ||
           t == TokenType::LT    ||
           t == TokenType::LTE   ||
           t == TokenType::GT    ||
           t == TokenType::GTE;
}

std::unique_ptr<ASTNode> Parser::parseRelExpr() {
    auto left = parseExpr();

    if (isRelop(peek().type)) {
        Token op = advance();
        auto right = parseExpr();
        auto node = makeNode(NodeKind::BINARY_EXPR, op.lexeme, op.line);
        node->addChild(std::move(left));
        node->addChild(std::move(right));
        return node;
    }

    return left;
}

// ═══════════════════════════════════════════════════════════
//  Arithmetic Expressions
//
//  expr  → term ((PLUS | MINUS) term)*
//  term  → unary ((MUL | DIV | MOD) unary)*
//  unary → MINUS unary | primary
//  primary → NUM | CHAR_LIT | STRING_LIT | ID | ( expr )
// ═══════════════════════════════════════════════════════════

std::unique_ptr<ASTNode> Parser::parseExpr() {
    auto left = parseTerm();

    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        Token op = advance();
        auto right = parseTerm();
        auto node = makeNode(NodeKind::BINARY_EXPR, op.lexeme, op.line);
        node->addChild(std::move(left));
        node->addChild(std::move(right));
        left = std::move(node);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseUnary();

    while (check(TokenType::MUL) || check(TokenType::DIV) || check(TokenType::MOD)) {
        Token op = advance();
        auto right = parseUnary();
        auto node = makeNode(NodeKind::BINARY_EXPR, op.lexeme, op.line);
        node->addChild(std::move(left));
        node->addChild(std::move(right));
        left = std::move(node);
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseUnary() {
    if (check(TokenType::MINUS)) {
        Token op = advance();
        auto node = makeNode(NodeKind::UNARY_EXPR, op.lexeme, op.line);
        node->addChild(parseUnary());
        return node;
    }
    return parsePrimary();
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    Token t = peek();

    // Number literal
    if (t.type == TokenType::NUM) {
        advance();
        return makeNode(NodeKind::NUM_LIT, t.lexeme, t.line);
    }

    // Character literal
    if (t.type == TokenType::CHAR_LIT) {
        advance();
        return makeNode(NodeKind::CHAR_LIT, t.lexeme, t.line);
    }

    // String literal
    if (t.type == TokenType::STRING_LIT) {
        advance();
        return makeNode(NodeKind::STRING_LIT, t.lexeme, t.line);
    }

    // Identifier
    if (t.type == TokenType::ID) {
        advance();
        return makeNode(NodeKind::IDENTIFIER, t.lexeme, t.line);
    }

    // Grouped expression  ( expr )
    if (t.type == TokenType::LPAREN) {
        advance();
        auto expr = parseExpr();
        expect(TokenType::RPAREN, "Expected ')' to close grouped expression");
        return expr;
    }

    // Anything else is an error
    recordError("Expected an expression (number, identifier, or '(')", t);
    advance(); // consume the bad token to avoid infinite loop
    synchronize();
    // Return a placeholder node so callers don't crash
    return makeNode(NodeKind::NUM_LIT, "0", t.line);
}
