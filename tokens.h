#ifndef TOKENS_H
#define TOKENS_H

#include <stdbool.h>

// Enumération des types de tokens (normaux et erreurs)
typedef enum {
    // === Mots-clés de structure ===
    TOK_ALGORITHME,          // ALG_TOKEN
    TOK_ALG_ERR,            // ALG_ERR
    TOK_VARIABLES,           // VARS_TOKEN
    TOK_VARS_ERR,           // VARS_ERR
    TOK_DEBUT,               // BEGIN_TOKEN
    TOK_BEGIN_ERR,          // BEGIN_ERR
    TOK_FIN,                 // END_TOKEN
    TOK_END_ERR,            // END_ERR
    
    // === Déclarations, types et constantes ===
    TOK_VARIABLE,            // VAR_TOKEN
    TOK_VAR_ERR,            // VAR_ERR
    TOK_CONSTANTE,           // CONST_TOKEN
    TOK_CONST_ERR,          // CONST_ERR
    TOK_ENTIER,              // INT_TYPE_TOKEN
    TOK_INT_TYPE_ERR,       // INT_TYPE_ERR
    TOK_REEL,                // REAL_TYPE_TOKEN
    TOK_REAL_TYPE_ERR,      // REAL_TYPE_ERR
    TOK_CARACTERE,           // CHAR_TYPE_TOKEN
    TOK_CHAR_TYPE_ERR,      // CHAR_TYPE_ERR
    TOK_CHAINE,              // STRING_TYPE_TOKEN
    TOK_STRING_TYPE_ERR,    // STRING_TYPE_ERR
    TOK_BOOLEEN,             // BOOL_TYPE_TOKEN
    TOK_BOOL_TYPE_ERR,      // BOOL_TYPE_ERR
    TOK_TABLEAU,             // ARRAY_TOKEN
    TOK_ARRAY_ERR,          // ARRAY_ERR
    TOK_DE,                  // OF_TOKEN
    TOK_OF_ERR,             // OF_ERR
    TOK_STRUCTURE,           // STRUCT_TOKEN
    TOK_STRUCT_ERR,         // STRUCT_ERR
    TOK_FIN_STRUCT,          // ENDSTRUCT_TOKEN
    TOK_ENDSTRUCT_ERR,      // ENDSTRUCT_ERR
    
    // === Entrées/Sorties ===
    TOK_ECRIRE,              // WRITE_TOKEN
    TOK_WRITE_ERR,          // WRITE_ERR
    TOK_LIRE,                // READ_TOKEN
    TOK_READ_ERR,           // READ_ERR
    TOK_RETOUR,              // NEWLINE_TOKEN
    TOK_NEWLINE_ERR,        // NEWLINE_ERR
    
    // === Constantes logiques et opérateurs ===
    TOK_VRAI,                // TRUE_TOKEN
    TOK_TRUE_ERR,           // TRUE_ERR
    TOK_FAUX,                // FALSE_TOKEN
    TOK_FALSE_ERR,          // FALSE_ERR
    TOK_ET,                  // AND_TOKEN
    TOK_AND_ERR,            // AND_ERR
    TOK_OU,                  // OR_TOKEN
    TOK_OR_ERR,             // OR_ERR
    TOK_NON,                 // NOT_TOKEN
    TOK_NOT_ERR,            // NOT_ERR
    
    // === Comparateurs ===
    TOK_INFERIEUR,           // LT_TOKEN
    TOK_LT_ERR,             // LT_ERR
    TOK_INFERIEUR_EGAL,      // LE_TOKEN
    TOK_LE_ERR,             // LE_ERR
    TOK_SUPERIEUR,           // GT_TOKEN
    TOK_GT_ERR,             // GT_ERR
    TOK_SUPERIEUR_EGAL,      // GE_TOKEN
    TOK_GE_ERR,             // GE_ERR
    TOK_EGAL,                // EQ_TOKEN
    TOK_EQ_ERR,             // EQ_ERR
    TOK_DIFFERENT,           // NEQ_TOKEN
    TOK_NEQ_ERR,            // NEQ_ERR
    
    // === Affectation, séparateurs, ponctuation ===
    TOK_AFFECTATION,         // ASSIGN_TOKEN
    TOK_ASSIGN_ERR,         // ASSIGN_ERR
    TOK_DEUX_POINTS,         // COLON_TOKEN
    TOK_COLON_ERR,          // COLON_ERR
    TOK_VIRGULE,             // COMMA_TOKEN
    TOK_COMMA_ERR,          // COMMA_ERR
    TOK_PAREN_OUVRANTE,      // LPAREN_TOKEN
    TOK_LPAREN_ERR,         // LPAREN_ERR
    TOK_PAREN_FERMANTE,      // RPAREN_TOKEN
    TOK_RPAREN_ERR,         // RPAREN_ERR
    TOK_CROCHET_OUVRANT,     // LBRACK_TOKEN
    TOK_LBRACK_ERR,         // LBRACK_ERR
    TOK_CROCHET_FERMANT,     // RBRACK_TOKEN
    TOK_RBRACK_ERR,         // RBRACK_ERR
    TOK_POINTS,              // DOTDOT_TOKEN
    TOK_DOTDOT_ERR,         // DOTDOT_ERR
    
    // === Opérateurs arithmétiques ===
    TOK_PLUS,                // PLUS_TOKEN
    TOK_PLUS_ERR,           // PLUS_ERR
    TOK_MOINS,               // MINUS_TOKEN
    TOK_MINUS_ERR,          // MINUS_ERR
    TOK_FOIS,                // MUL_TOKEN
    TOK_MUL_ERR,            // MUL_ERR
    TOK_DIVISE,              // DIV_TOKEN
    TOK_DIV_ERR,            // DIV_ERR
    TOK_DIV_ENTIER,          // INTDIV_TOKEN
    TOK_INTDIV_ERR,         // INTDIV_ERR
    TOK_MODULO,              // MOD_TOKEN
    TOK_MOD_ERR,            // MOD_ERR
    TOK_PUISSANCE,           // POW_TOKEN
    TOK_POW_ERR,            // POW_ERR
    
    // === Structures de contrôle ===
    TOK_SI,                  // IF_TOKEN
    TOK_IF_ERR,             // IF_ERR
    TOK_ALORS,               // THEN_TOKEN
    TOK_THEN_ERR,           // THEN_ERR
    TOK_SINON,               // ELSE_TOKEN
    TOK_ELSE_ERR,           // ELSE_ERR
    TOK_FIN_SI,              // ENDIF_TOKEN
    TOK_ENDIF_ERR,          // ENDIF_ERR
    TOK_SELON,               // SWITCH_TOKEN
    TOK_SWITCH_ERR,         // SWITCH_ERR
    TOK_FIN_SELON,           // ENDSWITCH_TOKEN
    TOK_ENDSWITCH_ERR,      // ENDSWITCH_ERR
    TOK_SORTIR,              // BREAK_SWITCH_TOKEN
    TOK_BREAK_SWITCH_ERR,   // BREAK_SWITCH_ERR
    TOK_POUR,                // FOR_TOKEN
    TOK_FOR_ERR,            // FOR_ERR
    TOK_JUSQUA,              // TO_TOKEN
    TOK_TO_ERR,             // TO_ERR
    TOK_PAS,                 // STEP_TOKEN
    TOK_STEP_ERR,           // STEP_ERR
    TOK_FIN_POUR,            // ENDFOR_TOKEN
    TOK_ENDFOR_ERR,         // ENDFOR_ERR
    TOK_QUITTER_POUR,        // BREAK_FOR_TOKEN
    TOK_BREAK_FOR_ERR,      // BREAK_FOR_ERR
    TOK_TANTQUE,             // WHILE_TOKEN
    TOK_WHILE_ERR,          // WHILE_ERR
    
    // === Identifiants et littéraux ===
    TOK_IDENTIFIANT,
    TOK_NOMBRE_ENTIER,
    TOK_NOMBRE_REEL,
    TOK_CHAINE_LITTERALE,
    TOK_CARACTERE_LITTERAL,

    // === Nouveaux tokens ajoutés ===
    TOK_GUILLEMET,           // QUOTE_TOKEN
    TOK_GUILLEMET_ERR,       // QUOTE_ERR
    TOK_ID,                  // ID_TOKEN
    TOK_ID_ERR,              // ID_ERR
    TOK_CONST_ENTIERE,       // INT_CONST_TOKEN
    TOK_CONST_ENTIERE_ERR,   // INT_CONST_ERR
    TOK_CONST_REEL,          // REAL_CONST_TOKEN
    TOK_CONST_REEL_ERR,      // REAL_CONST_ERR
    TOK_CONST_CHAINE,        // STRING_CONST_TOKEN
    TOK_CONST_CHAINE_ERR,    // STRING_CONST_ERR
    TOK_PROCEDURE,           // PROCEDURE_TOKEN
    TOK_PROCEDURE_ERR,       // PROCEDURE_ERR
    TOK_FIN_PROC,            // END_PROCEDURE_TOKEN
    TOK_FIN_PROC_ERR,        // END_PROCEDURE_ERR
    TOK_FONCTION,            // FUNCTION_TOKEN
    TOK_FONCTION_ERR,        // FUNCTION_ERR
    TOK_FIN_FONCT,           // END_FUNCTION_TOKEN
    TOK_FIN_FONCT_ERR,       // END_FUNCTION_ERR
    TOK_RETOURNER,           // RETURN_TOKEN
    TOK_RETOURNER_ERR,       // RETURN_ERR
    TOK_REPETER,             // REPEAT_TOKEN
    TOK_REPETER_ERR,         // REPEAT_ERR
    
    // === Spéciaux ===
    TOK_COMMENTAIRE,
    TOK_ERREUR_GENERIQUE,
    TOK_EOF
} TokenType;

// Structure d'un token
typedef struct {
    TokenType type;
    char* valeur;
    int ligne;
    int colonne;
    bool est_erreur;  // Indique si c'est un token d'erreur
} Token;

// Fonctions pour les tokens
const char* token_type_to_string(TokenType type);
bool est_token_erreur(TokenType type);
void afficher_token(Token* token);

#endif // TOKENS_H