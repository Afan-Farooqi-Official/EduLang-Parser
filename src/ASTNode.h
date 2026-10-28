// ============================================================
//  ASTNode.h  —  Abstract Syntax Tree node definitions
//  EduLang Recursive-Descent Parser
// ============================================================
#ifndef ASTNODE_H
#define ASTNODE_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

// ── Node kinds ───────────────────────────────────────────────
enum class NodeKind {
    // Program
    PROGRAM,

    // Statements
    STMT_LIST,
    VAR_DECL,          // score x = expr;  /  grade ch;
    CONST_DECL,        // fixed score x = expr;
    ASSIGN_STMT,       // x = expr;
    PRINT_STMT,        // print(expr);
    INPUT_STMT,        // input(id);
    CHECK_STMT,        // check(cond) { ... } otherwise { ... }
    REPEAT_STMT,       // repeat { ... }
    BLOCK,             // { stmt* }

    // Expressions
    BINARY_EXPR,       // left OP right
    UNARY_EXPR,        // !expr  /  -expr
    NUM_LIT,           // 75
    CHAR_LIT,          // 'P'
    STRING_LIT,        // "hello"
    IDENTIFIER,        // marks
};

// ── Pretty name for debugging ─────────────────────────────────
inline std::string nodeKindName(NodeKind k) {
    switch (k) {
        case NodeKind::PROGRAM:      return "Program";
        case NodeKind::STMT_LIST:    return "StmtList";
        case NodeKind::VAR_DECL:     return "VarDecl";
        case NodeKind::CONST_DECL:   return "ConstDecl";
        case NodeKind::ASSIGN_STMT:  return "AssignStmt";
        case NodeKind::PRINT_STMT:   return "PrintStmt";
        case NodeKind::INPUT_STMT:   return "InputStmt";
        case NodeKind::CHECK_STMT:   return "CheckStmt";
        case NodeKind::REPEAT_STMT:  return "RepeatStmt";
        case NodeKind::BLOCK:        return "Block";
        case NodeKind::BINARY_EXPR:  return "BinaryExpr";
        case NodeKind::UNARY_EXPR:   return "UnaryExpr";
        case NodeKind::NUM_LIT:      return "NumLit";
        case NodeKind::CHAR_LIT:     return "CharLit";
        case NodeKind::STRING_LIT:   return "StringLit";
        case NodeKind::IDENTIFIER:   return "Identifier";
        default:                     return "Unknown";
    }
}

// ── Base AST Node ─────────────────────────────────────────────
struct ASTNode {
    NodeKind kind;
    std::string value;          // lexeme / operator string (optional)
    int line;
    std::vector<std::unique_ptr<ASTNode>> children;

    ASTNode(NodeKind k, std::string v = "", int ln = 0)
        : kind(k), value(std::move(v)), line(ln) {}

    // Convenience: add a child
    void addChild(std::unique_ptr<ASTNode> child) {
        children.push_back(std::move(child));
    }

    // ── Pretty-print the AST tree ─────────────────────────────
    void print(std::ostream& out, const std::string& prefix = "",
               bool isLast = true) const {
        out << prefix;
        out << (isLast ? "└── " : "├── ");

        out << nodeKindName(kind);
        if (!value.empty()) {
            out << " [" << value << "]";
        }
        if (line > 0) {
            out << "  (line " << line << ")";
        }
        out << "\n";

        std::string childPrefix = prefix + (isLast ? "    " : "│   ");
        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i]) {
                children[i]->print(out, childPrefix, i == children.size() - 1);
            }
        }
    }
};

// ── Factory helpers (make_unique wrappers) ────────────────────
inline std::unique_ptr<ASTNode> makeNode(NodeKind k, const std::string& v = "", int ln = 0) {
    return std::make_unique<ASTNode>(k, v, ln);
}

#endif // ASTNODE_H
