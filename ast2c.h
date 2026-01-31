#ifndef AST2C_H
#define AST2C_H

#include <stdio.h>
#include <stdbool.h>

#include "ast.h"

// Génère du C vers un flux (stdout ou fichier déjà ouvert)
bool ast2c_generate_stream(ASTNode* program, FILE* out);

// Génère du C vers un fichier (crée/écrase)
bool ast2c_generate_file(ASTNode* program, const char* out_c_path);

#endif