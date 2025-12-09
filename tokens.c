#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tokens.h"

// Conversion du type de token en chaîne
const char* token_type_to_string(TokenType type) {
    switch (type) {
        // Mots-clés de structure
        case TOK_ALGORITHME: return "ALGORITHME";
        case TOK_VARIABLES: return "VARIABLES";
        case TOK_DEBUT: return "DEBUT";
        case TOK_FIN: return "FIN";
        
        // Déclarations, types et constantes
        case TOK_VARIABLE: return "VARIABLE";
        case TOK_CONSTANTE: return "CONSTANTE";
        case TOK_ENTIER: return "ENTIER";
        case TOK_REEL: return "REEL";
        case TOK_CARACTERE: return "CARACTERE";
        case TOK_CHAINE: return "CHAINE";
        case TOK_BOOLEEN: return "BOOLEEN";
        case TOK_TABLEAU: return "TABLEAU";
        case TOK_DE: return "DE";
        case TOK_STRUCTURE: return "STRUCTURE";
        case TOK_FIN_STRUCT: return "FIN_STRUCT";
        
        // Entrées/Sorties
        case TOK_ECRIRE: return "ECRIRE";
        case TOK_LIRE: return "LIRE";
        case TOK_RETOUR: return "RETOUR";
        
        // Constantes logiques
        case TOK_VRAI: return "VRAI";
        case TOK_FAUX: return "FAUX";
        case TOK_ET: return "ET";
        case TOK_OU: return "OU";
        case TOK_NON: return "NON";
        
        // Comparateurs
        case TOK_INFERIEUR: return "INFERIEUR";
        case TOK_INFERIEUR_EGAL: return "INFERIEUR_EGAL";
        case TOK_SUPERIEUR: return "SUPERIEUR";
        case TOK_SUPERIEUR_EGAL: return "SUPERIEUR_EGAL";
        case TOK_EGAL: return "EGAL";
        case TOK_DIFFERENT: return "DIFFERENT";
        
        // Affectation et ponctuation
        case TOK_AFFECTATION: return "AFFECTATION";
        case TOK_DEUX_POINTS: return "DEUX_POINTS";
        case TOK_VIRGULE: return "VIRGULE";
        case TOK_PAREN_OUVRANTE: return "PAREN_OUVRANTE";
        case TOK_PAREN_FERMANTE: return "PAREN_FERMANTE";
        case TOK_CROCHET_OUVRANT: return "CROCHET_OUVRANT";
        case TOK_CROCHET_FERMANT: return "CROCHET_FERMANT";
        case TOK_POINTS: return "POINTS";
        
        // Opérateurs arithmétiques
        case TOK_PLUS: return "PLUS";
        case TOK_MOINS: return "MOINS";
        case TOK_FOIS: return "FOIS";
        case TOK_DIVISE: return "DIVISE";
        case TOK_DIV_ENTIER: return "DIV_ENTIER";
        case TOK_MODULO: return "MODULO";
        case TOK_PUISSANCE: return "PUISSANCE";
        
        // Structures de contrôle
        case TOK_SI: return "SI";
        case TOK_ALORS: return "ALORS";
        case TOK_SINON: return "SINON";
        case TOK_FIN_SI: return "FIN_SI";
        case TOK_SELON: return "SELON";
        case TOK_FIN_SELON: return "FIN_SELON";
        case TOK_SORTIR: return "SORTIR";
        case TOK_POUR: return "POUR";
        case TOK_JUSQUA: return "JUSQUA";
        case TOK_PAS: return "PAS";
        case TOK_FIN_POUR: return "FIN_POUR";
        case TOK_QUITTER_POUR: return "QUITTER_POUR";
        case TOK_TANTQUE: return "TANTQUE";
        
        // Identifiants et littéraux
        case TOK_IDENTIFIANT: return "IDENTIFIANT";
        case TOK_NOMBRE_ENTIER: return "NOMBRE_ENTIER";
        case TOK_NOMBRE_REEL: return "NOMBRE_REEL";
        case TOK_CHAINE_LITTERALE: return "CHAINE_LITTERALE";
        case TOK_CARACTERE_LITTERAL: return "CARACTERE_LITTERAL";
        
        // Nouveaux tokens
        case TOK_GUILLEMET: return "GUILLEMET";
        case TOK_GUILLEMET_ERR: return "GUILLEMET_ERR";
        case TOK_ID: return "ID";
        case TOK_ID_ERR: return "ID_ERR";
        case TOK_CONST_ENTIERE: return "CONST_ENTIERE";
        case TOK_CONST_ENTIERE_ERR: return "CONST_ENTIERE_ERR";
        case TOK_CONST_REEL: return "CONST_REEL";
        case TOK_CONST_REEL_ERR: return "CONST_REEL_ERR";
        case TOK_CONST_CHAINE: return "CONST_CHAINE";
        case TOK_CONST_CHAINE_ERR: return "CONST_CHAINE_ERR";
        case TOK_PROCEDURE: return "PROCEDURE";
        case TOK_PROCEDURE_ERR: return "PROCEDURE_ERR";
        case TOK_FIN_PROC: return "FIN_PROC";
        case TOK_FIN_PROC_ERR: return "FIN_PROC_ERR";
        case TOK_FONCTION: return "FONCTION";
        case TOK_FONCTION_ERR: return "FONCTION_ERR";
        case TOK_FIN_FONCT: return "FIN_FONCT";
        case TOK_FIN_FONCT_ERR: return "FIN_FONCT_ERR";
        case TOK_RETOURNER: return "RETOURNER";
        case TOK_RETOURNER_ERR: return "RETOURNER_ERR";
        case TOK_REPETER: return "REPETER";
        case TOK_REPETER_ERR: return "REPETER_ERR";
        
        // Spéciaux
        case TOK_COMMENTAIRE: return "COMMENTAIRE";
        case TOK_ERREUR: return "ERREUR";
        case TOK_EOF: return "EOF";
        
        default: return "INCONNU";
    }
}

bool est_token_erreur(TokenType type) {
    switch (type) {
        case TOK_ALG_ERR:
        case TOK_VARS_ERR:
        case TOK_BEGIN_ERR:
        case TOK_END_ERR:
        case TOK_VAR_ERR:
        case TOK_CONST_ERR:
        case TOK_INT_TYPE_ERR:
        case TOK_REAL_TYPE_ERR:
        case TOK_CHAR_TYPE_ERR:
        case TOK_STRING_TYPE_ERR:
        case TOK_BOOL_TYPE_ERR:
        case TOK_ARRAY_ERR:
        case TOK_OF_ERR:
        case TOK_STRUCT_ERR:
        case TOK_ENDSTRUCT_ERR:
        case TOK_WRITE_ERR:
        case TOK_READ_ERR:
        case TOK_NEWLINE_ERR:
        case TOK_TRUE_ERR:
        case TOK_FALSE_ERR:
        case TOK_AND_ERR:
        case TOK_OR_ERR:
        case TOK_NOT_ERR:
        case TOK_LT_ERR:
        case TOK_LE_ERR:
        case TOK_GT_ERR:
        case TOK_GE_ERR:
        case TOK_EQ_ERR:
        case TOK_NEQ_ERR:
        case TOK_ASSIGN_ERR:
        case TOK_COLON_ERR:
        case TOK_COMMA_ERR:
        case TOK_LPAREN_ERR:
        case TOK_RPAREN_ERR:
        case TOK_LBRACK_ERR:
        case TOK_RBRACK_ERR:
        case TOK_DOTDOT_ERR:
        case TOK_GUILLEMET_ERR:
        case TOK_ID_ERR:
        case TOK_CONST_ENTIERE_ERR:
        case TOK_CONST_REEL_ERR:
        case TOK_CONST_CHAINE_ERR:
        case TOK_PROCEDURE_ERR:
        case TOK_FIN_PROC_ERR:
        case TOK_FONCTION_ERR:
        case TOK_FIN_FONCT_ERR:
        case TOK_RETOURNER_ERR:
        case TOK_REPETER_ERR:
        case TOK_PLUS_ERR:
        case TOK_MINUS_ERR:
        case TOK_MUL_ERR:
        case TOK_DIV_ERR:
        case TOK_INTDIV_ERR:
        case TOK_MOD_ERR:
        case TOK_POW_ERR:
        case TOK_IF_ERR:
        case TOK_THEN_ERR:
        case TOK_ELSE_ERR:
        case TOK_ENDIF_ERR:
        case TOK_SWITCH_ERR:
        case TOK_ENDSWITCH_ERR:
        case TOK_BREAK_SWITCH_ERR:
        case TOK_FOR_ERR:
        case TOK_TO_ERR:
        case TOK_STEP_ERR:
        case TOK_ENDFOR_ERR:
        case TOK_BREAK_FOR_ERR:
        case TOK_WHILE_ERR:
            return true;
        default:
            return false;
    }
}

// Affichage d'un token
void afficher_token(Token* token) {
    if (!token) return;
    
    const char* type_str = token_type_to_string(token->type);
    const char* prefix = token->est_erreur ? "❌ " : "   ";
    
    printf("%sToken(%s, \"%s\", ligne: %d, colonne: %d)\n",
           prefix, type_str, token->valeur, token->ligne, token->colonne);
}