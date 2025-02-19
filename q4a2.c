Using getNextToken( ) implemented in Lab No 3, design a Lexical Analyser to implement a single
symbol table.


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h> 


#define MAX_LENGTH 1000
#define TABLE_SIZE 100

typedef enum 
{
    ID, KEYWORD, NUM, OPERATOR, SPECIAL, STRING, COMMENT, DIRECTIVE,
    INVALID
} TokenType;
typedef struct 
{
    char lexeme[MAX_LENGTH];
    TokenType type;
    int size; // Size of the variable or array
    int row;
    int col;
    char value[MAX_LENGTH];
} Token;

Token symbolTable[TABLE_SIZE];
int hashFunction(const char *lexeme) 
{
    int hash = 0;
    while (*lexeme) 
    {
        hash = (hash * 31 + *lexeme++) % TABLE_SIZE;
    }
    return hash;
}

int insertSymbol(const char *lexeme, TokenType type, int size) 
{
    int hash = hashFunction(lexeme);
    int originalHash = hash;
    while (symbolTable[hash].lexeme[0] != '\0' && strcmp(symbolTable[hash].lexeme,
    lexeme) != 0) 
    {
        hash = (hash + 1) % TABLE_SIZE;
        if (hash == originalHash) 
        {
            printf("Error: Symbol table is full!\n");
            return -1;
        }
    }
    strcpy(symbolTable[hash].lexeme, lexeme);
    symbolTable[hash].type = type;
    symbolTable[hash].size = size;
    return hash;
}
bool isKeyword(const char *word) 
{
    const char *keywords[] = 
    {
        "int", "float", "char", "if", "else", "while", "for", "return", "void", "main", NULL
    };
    for (int i = 0; keywords[i] != NULL; i++) 
    {
        if (strcmp(word, keywords[i]) == 0)
        return true;
    }
    return false;
}
bool isIdentifier(const char *word) 
{
    if (!isalpha(word[0]) && word[0] != '_') 
    return false;
    for (int i = 1; word[i] != '\0'; i++) 
    {
        if (!isalnum(word[i]) && word[i] != '_') 
        return false;
    }
    return true;
}
Token getNextToken(FILE *file, int *line, int *col) 
    {
    Token token;
    token.type = INVALID;
    token.size = 0; // Default size is 0
    char ch;
    char value[MAX_LENGTH];
    int valueIndex = 0;
    bool inString = false;
    bool inComment = false;
    while ((ch = fgetc(file)) != EOF) 
    {
    if (isspace(ch) && !inString && !inComment) 
    {
    if (ch == '\n') 
    {
    (*line)++;
    *col = 1;
    } 
    else
    {
    (*col)++;
    }
continue;
}
if (!inString && !inComment && ch == '/' && (ch = fgetc(file)) == '/') 
{
inComment = true;
while ((ch = fgetc(file)) != '\n' && ch != EOF);
(*line)++;
*col = 1;
continue;
}
if (!inString && !inComment && ch == '/' && (ch = fgetc(file)) == '*') 
{
inComment = true;
while ((ch = fgetc(file)) != '*' && ch != EOF) 
{
if (ch == '\n') 
{
(*line)++;
*col = 1;
} 
else 
{
(*col)++;
}
}
if ((ch = fgetc(file)) == '/') 
{
inComment = false;
(*col)++;
continue;
}
}
if (!inString && !inComment && ch == '#') 
{
token.type = DIRECTIVE;
value[valueIndex++] = ch;
while ((ch = fgetc(file)) != '\n' && ch != EOF) 
{
value[valueIndex++] = ch;
}
value[valueIndex] = '\0';
token.col = *col;
token.row = *line;
strcpy(token.value, value);
(*line)++;
*col = 1;
return token;
}
if (ch == '"' && !inComment) 
{
inString = !inString;
token.type = STRING;
value[valueIndex++] = ch;
while ((ch = fgetc(file)) != '"' && ch != EOF) 
{
value[valueIndex++] = ch;
}
value[valueIndex++] = ch; // Add closing quote
value[valueIndex] = '\0';
token.col = *col;
token.row = *line;
strcpy(token.value, value);
(*col)++;
return token;
}
if (!inString && !inComment && (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' ||
ch == '!' || ch == '<' || ch == '>')) 
{
token.type = OPERATOR;
value[valueIndex++] = ch;
value[valueIndex] = '\0';
token.col = *col;
token.row = *line;
strcpy(token.value, value);
(*col)++;
return token;
}
if (isalpha(ch) || ch == '_') 
{
value[valueIndex++] = ch;
(*col)++;
while ((ch = fgetc(file)) != EOF && (isalnum(ch) || ch == '_' || ch == '[' || ch == ']')) 
{
value[valueIndex++] = ch;
(*col)++;
}
ungetc(ch, file);
value[valueIndex] = '\0';
if (isKeyword(value)) 
{
token.type = KEYWORD;
} 
else 
{
token.type = ID;
}
int size = 4; // Default size for int
if (token.type == KEYWORD)
{
if (strcmp(value, "int") == 0) 
{
size = 4; // int size
} else if (strcmp(value, "char") == 0) 
{
size = 1; // char size
} else if (strcmp(value, "float") == 0) 
{
size = 4; // float size
} else if (strcmp(value, "double") == 0) 
{
size = 8; // double size
}
}
if (strchr(value, '[')) 
{
char *arrayStart = strchr(value, '[');
int arraySize = 1;
if (arrayStart != NULL) 
{
arraySize = atoi(arrayStart + 1);
}
size = arraySize * 4;
}
insertSymbol(value, token.type, size);
token.row = *line;
strcpy(token.value, value);
return token;
}
if (isdigit(ch)) 
{
value[valueIndex++] = ch;
(*col)++;
while ((ch = fgetc(file)) != EOF && isdigit(ch)) 
{
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
void displaySymbolTable() 
{
printf("Symbol Table:\n");
printf("Index\tLexeme\t\tType\tSize\n");
for (int i = 0; i < TABLE_SIZE; i++) 
{
if (symbolTable[i].lexeme[0] != '\0') 
{
printf("%d\t%s\t\t", i, symbolTable[i].lexeme);
switch (symbolTable[i].type) 
{
case KEYWORD:
if (strcmp(symbolTable[i].lexeme, "int") == 0) 
{
printf("int\t");
} 
else if (strcmp(symbolTable[i].lexeme, "float") == 0) 
{
printf("float\t");
} 
else if (strcmp(symbolTable[i].lexeme, "char") == 0) 
{
printf("char\t");
} 
else if (strcmp(symbolTable[i].lexeme, "void") == 0) 
{
printf("void\t");
} 
else 
{
printf("keyword\t");
}
break;
case ID:
printf("int\t");
break;
case NUM:
printf("num\t");
break;
case OPERATOR:
printf("operator\t");
break;
case STRING:
printf("string\t");
break;
default:
printf("invalid\t");
}
printf("%d\n", symbolTable[i].size);
}
}
}


int main() 
{
    FILE *file;
    char filename[MAX_LENGTH];
    int line = 1, col = 1;
    printf("Enter the C file name (with extension .c): ");
    scanf("%s", filename);
    file = fopen(filename, "r");
    if (!file)
    {
        printf("Error opening file %s\n", filename);
        return 1;
    }
    // Read and process the file
    Token token;
    while ((token = getNextToken(file, &line, &col)).type != INVALID) 
    {
        if (token.type == ID || token.type == KEYWORD) 
        {
        printf("<%s, %d, %d, %s>\n",
        (token.type == ID) ? "id" :
        (token.type == KEYWORD) ? "keyword" :
        (token.type == NUM) ? "num" :
        (token.type == OPERATOR) ? "operator" :
        (token.type == STRING) ? "string" :
        (token.type == DIRECTIVE) ? "directive" :
        "comment",
        token.row, token.col, token.value);
        }
    }
    displaySymbolTable();
    fclose(file);
    return 0;
}
