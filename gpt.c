#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 

#define MAX_LENGTH 1000
#define TABLE_SIZE 100

// Enum for Token Types
typedef enum {
    ID, KEYWORD, NUM, OPERATOR, SPECIAL, STRING, COMMENT, DIRECTIVE, INVALID
} TokenType;

// Structure for Tokens
typedef struct {
    char lexeme[MAX_LENGTH];
    TokenType type;
    int size;
    int row;
    int col;
    char value[MAX_LENGTH];
} Token;

Token symbolTable[TABLE_SIZE];

// Hash Function
int hashFunction(const char *lexeme) {
    int hash = 0;
    while (*lexeme) hash = (hash * 31 + *lexeme++) % TABLE_SIZE;
    return hash;
}

// Insert Symbol
int insertSymbol(const char *lexeme, TokenType type, int size) {
    int hash = hashFunction(lexeme);
    while (symbolTable[hash].lexeme[0] != '\0' && strcmp(symbolTable[hash].lexeme, lexeme) != 0) {
        hash = (hash + 1) % TABLE_SIZE;
    }
    strcpy(symbolTable[hash].lexeme, lexeme);
    symbolTable[hash].type = type;
    symbolTable[hash].size = size;
    return hash;
}

// Check if Keyword
bool isKeyword(const char *word) {
    const char *keywords[] = {"int", "float", "char", "if", "else", "while", "for", "return", "void", "main", NULL};
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) return true;
    }
    return false;
}

// Get Next Token
Token getNextToken(FILE *file, int *line, int *col) {
    Token token;
    token.type = INVALID;
    char ch, value[MAX_LENGTH];
    int valueIndex = 0;
    bool inString = false, inComment = false;

    while ((ch = fgetc(file)) != EOF) {
        if (isspace(ch) && !inString && !inComment) {
            if (ch == '\n') { (*line)++; *col = 1; } else (*col)++;
            continue;
        }
        if (!inString && !inComment && ch == '/' && (ch = fgetc(file)) == '/') {
            while ((ch = fgetc(file)) != '\n' && ch != EOF);
            (*line)++; *col = 1; continue;
        }
        if (ch == '"') {
            inString = !inString;
            token.type = STRING;
            value[valueIndex++] = ch;
            while ((ch = fgetc(file)) != '"' && ch != EOF) value[valueIndex++] = ch;
            value[valueIndex++] = ch;
            value[valueIndex] = '\0';
            token.row = *line;
            strcpy(token.value, value);
            (*col)++;
            return token;
        }
        if (isalpha(ch) || ch == '_') {
            value[valueIndex++] = ch;
            (*col)++;
            while ((ch = fgetc(file)) != EOF && (isalnum(ch) || ch == '_')) {
                value[valueIndex++] = ch;
                (*col)++;
            }
            ungetc(ch, file);
            value[valueIndex] = '\0';
            token.type = isKeyword(value) ? KEYWORD : ID;
            insertSymbol(value, token.type, 4);
            token.row = *line;
            strcpy(token.value, value);
            return token;
        }
        if (isdigit(ch)) {
            value[valueIndex++] = ch;
            (*col)++;
            while ((ch = fgetc(file)) != EOF && isdigit(ch)) {
                value[valueIndex++] = ch;
                (*col)++;
            }
            ungetc(ch, file);
            value[valueIndex] = '\0';
            token.type = NUM;
            token.row = *line;
            strcpy(token.value, value);
            return token;
        }
    }
    token.type = INVALID;
    return token;
}

// Display Symbol Table
void displaySymbolTable() {
    printf("\nSymbol Table:\n");
    printf("Index\tLexeme\t\tType\tSize\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (symbolTable[i].lexeme[0] != '\0') {
            printf("%d\t%s\t\t%d\t%d\n", i, symbolTable[i].lexeme, symbolTable[i].type, symbolTable[i].size);
        }
    }
}

// Main Function
int main() {
    FILE *file;
    char filename[MAX_LENGTH];
    int line = 1, col = 1;
    printf("Enter the C file name (with extension .c): ");
    scanf("%s", filename);
    file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        return 1;
    }
    Token token;
    while ((token = getNextToken(file, &line, &col)).type != INVALID) {
        printf("<%s, %d, %d, %s>\n",
               (token.type == ID) ? "id" : (token.type == KEYWORD) ? "keyword" : (token.type == NUM) ? "num" : (token.type == OPERATOR) ? "operator" : "invalid",
               token.row, token.col, token.value);
    }
    displaySymbolTable();
    fclose(file);
    return 0;
}
