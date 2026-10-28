# 🌳 EduLang Recursive-Descent Parser

A **hand-written Recursive-Descent Parser** for **EduLang** that performs full syntax analysis, builds an **Abstract Syntax Tree (AST)**, and recovers from errors using **panic-mode recovery**.

---

## ⚡ Quick Start

### 1. Compile
```bash
g++ -std=c++17 src/main.cpp src/Parser.cpp src/Lexer.cpp -o edulang_parser.exe
```

### 2. Run
```powershell
# Parse a file directly:
.\edulang_parser.exe tests/test_program.edu

# Interactive menu:
.\edulang_parser.exe
```

---

## 📁 Project Structure

```
EduLang-Parser/
├── src/
│   ├── Token.h          ← Token types & structs (from Lexer project)
│   ├── Keywords.h       ← Keyword lookup table  (from Lexer project)
│   ├── Lexer.h/.cpp     ← DFA-based Lexer       (from Lexer project)
│   ├── ASTNode.h        ← AST node definitions & pretty-printer   ← NEW
│   ├── Parser.h         ← Recursive-Descent Parser interface      ← NEW
│   ├── Parser.cpp       ← Parser implementation                   ← NEW
│   └── main.cpp         ← Interactive driver & output             ← NEW
├── tests/
│   ├── test_program.edu    ← Valid student grading program
│   ├── edge_cases.edu      ← Signed numbers, logical ops, loops
│   └── syntax_errors.edu   ← Intentional errors (error recovery test)
└── output/
    └── parse_output.txt    ← Auto-saved AST + error report
```

---

## 📜 EduLang Grammar (Formal)

```
program     → ENROLL block GRADUATE EOF

block       → { stmt_list }
stmt_list   → stmt*

stmt        → var_decl
            | const_decl
            | assign_stmt
            | print_stmt
            | input_stmt
            | check_stmt
            | repeat_stmt

var_decl    → DT ID ( = expr )? ;
const_decl  → fixed DT ID = expr ;
assign_stmt → ID = expr ;
print_stmt  → print ( expr ) ;
input_stmt  → input ( ID ) ;
check_stmt  → check ( cond_expr ) block ( otherwise block )?
repeat_stmt → repeat block

cond_expr   → or_expr
or_expr     → and_expr ( || and_expr )*
and_expr    → not_expr ( && not_expr )*
not_expr    → ! not_expr | rel_expr
rel_expr    → expr ( relop expr )?
relop       → == | != | < | <= | > | >=

expr        → term ( ( + | - ) term )*
term        → unary ( ( * | / | % ) unary )*
unary       → - unary | primary
primary     → NUM | CHAR_LIT | STRING_LIT | ID | ( expr )
```

---

## 🌳 AST Node Types

| Node Kind     | Description                          | Example              |
|---------------|--------------------------------------|----------------------|
| `Program`     | Root node                            | entire program       |
| `Block`       | `{ stmt* }`                         | `{ ... }`            |
| `StmtList`    | List of statements                   | children of a block  |
| `VarDecl`     | Variable declaration                 | `score x = 5;`       |
| `ConstDecl`   | Constant declaration                 | `fixed score x = 5;` |
| `AssignStmt`  | Assignment                           | `x = expr;`          |
| `PrintStmt`   | Print                                | `print(x);`          |
| `InputStmt`   | Input                                | `input(x);`          |
| `CheckStmt`   | If / else                            | `check(...) {...}`   |
| `RepeatStmt`  | Loop                                 | `repeat {...}`       |
| `BinaryExpr`  | Binary operation                     | `a + b`, `x >= 50`   |
| `UnaryExpr`   | Unary operation                      | `!x`, `-y`           |
| `NumLit`      | Number literal                       | `75`, `-20`, `+100`  |
| `CharLit`     | Character literal                    | `'P'`, `'F'`         |
| `StringLit`   | String literal                       | `"hello"`            |
| `Identifier`  | Variable / constant name             | `marks`, `result`    |

---

## 🚨 Error Handling (Panic-Mode Recovery)

The parser uses **panic-mode error recovery**:

1. When a syntax error is detected → it is **recorded with line number**
2. Tokens are **skipped** until a safe restart point is found (`;`, `}`, or a statement keyword)
3. Parsing **continues** — multiple errors are caught in one run

| Error Type              | Trigger Example                          |
|-------------------------|------------------------------------------|
| Missing `enroll`        | Program doesn't start with `enroll`      |
| Missing `graduate`      | Program doesn't end with `graduate`      |
| Missing `;`             | `score x = 5` ← no semicolon            |
| Missing expression      | `score x = ;`                            |
| Missing `{` or `}`      | Unclosed block                           |
| Missing `(` or `)`      | `check marks >= 50`                      |
| Unexpected token        | Any token that can't start a statement   |

---

## 📊 Sample Output

### Input (`test_program.edu`)
```
enroll
{
    score marks = 75;
    grade result;
    check (marks >= 50)
    {
        result = 'P';
    }
    otherwise
    {
        result = 'F';
    }
    print(result);
}
graduate
```

### AST Output
```
└── Program [program]
    └── Block [{}]
        └── StmtList
            ├── VarDecl [score]
            │   ├── Identifier [marks]
            │   └── NumLit [75]
            ├── VarDecl [grade]
            │   └── Identifier [result]
            ├── CheckStmt [check]
            │   ├── BinaryExpr [>=]
            │   │   ├── Identifier [marks]
            │   │   └── NumLit [50]
            │   ├── Block [{}]  ← then-branch
            │   │   └── StmtList
            │   │       └── AssignStmt [=]
            │   │           ├── Identifier [result]
            │   │           └── CharLit ['P']
            │   └── Block [{}]  ← otherwise-branch
            │       └── StmtList
            │           └── AssignStmt [=]
            │               ├── Identifier [result]
            │               └── CharLit ['F']
            └── PrintStmt [print]
                └── Identifier [result]

✔  SUCCESS — Program is syntactically valid!
```

---

## 🛠️ Requirements

- **C++ Compiler**: `g++` with C++17 (`-std=c++17`)
- **OS**: Windows / Linux / macOS
- **Dependencies**: None (standard C++ library only)
