#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer.h"

// ============================================================================
// STRUCTURES ET DONNÉES INTERNES
// ============================================================================

// Structure pour les mots-clés avec leurs tokens d'erreur correspondants
typedef struct {
    const char* mot;
    TokenType type;
    TokenType type_erreur;
} MotCleAvecErreur;

// Table complète des mots-clés avec leurs tokens d'erreur
static const MotCleAvecErreur MOTS_CLES_AVEC_ERREUR[] = {
    // === NOUVEAUX TOKENS AJOUTÉS ===
    {"Procédure", TOK_PROCEDURE, TOK_PROCEDURE_ERR},
    {"procedure", TOK_PROCEDURE, TOK_PROCEDURE_ERR},
    {"FinProc", TOK_FIN_PROC, TOK_FIN_PROC_ERR},
    {"finproc", TOK_FIN_PROC, TOK_FIN_PROC_ERR},
    {"Fonction", TOK_FONCTION, TOK_FONCTION_ERR},
    {"fonction", TOK_FONCTION, TOK_FONCTION_ERR},
    {"FinFonct", TOK_FIN_FONCT, TOK_FIN_FONCT_ERR},
    {"finfonct", TOK_FIN_FONCT, TOK_FIN_FONCT_ERR},
    {"Retourner", TOK_RETOURNER, TOK_RETOURNER_ERR},
    {"retourner", TOK_RETOURNER, TOK_RETOURNER_ERR},
    {"Répéter", TOK_REPETER, TOK_REPETER_ERR},
    {"repeter", TOK_REPETER, TOK_REPETER_ERR},
    {"répéter", TOK_REPETER, TOK_REPETER_ERR},
    
    // === Mots-clés de structure ===
    {"Algorithme", TOK_ALGORITHME, TOK_ALG_ERR},
    {"Variables", TOK_VARIABLES, TOK_VARS_ERR},
    {"Début", TOK_DEBUT, TOK_BEGIN_ERR},
    {"Fin", TOK_FIN, TOK_END_ERR},
    
    // === Déclarations ===
    {"Variable", TOK_VARIABLE, TOK_VAR_ERR},
    {"Constante", TOK_CONSTANTE, TOK_CONST_ERR},
    
    // === Types ===
    {"entier", TOK_ENTIER, TOK_INT_TYPE_ERR},
    {"réel", TOK_REEL, TOK_REAL_TYPE_ERR},
    {"reel", TOK_REEL, TOK_REAL_TYPE_ERR},
    {"caractère", TOK_CARACTERE, TOK_CHAR_TYPE_ERR},
    {"caractere", TOK_CARACTERE, TOK_CHAR_TYPE_ERR},
    {"Chaine", TOK_CHAINE, TOK_STRING_TYPE_ERR},
    {"chaine", TOK_CHAINE, TOK_STRING_TYPE_ERR},
    {"booleen", TOK_BOOLEEN, TOK_BOOL_TYPE_ERR},
    {"booléen", TOK_BOOLEEN, TOK_BOOL_TYPE_ERR},
    {"tableau", TOK_TABLEAU, TOK_ARRAY_ERR},
    {"de", TOK_DE, TOK_OF_ERR},
    {"Structure", TOK_STRUCTURE, TOK_STRUCT_ERR},
    {"structure", TOK_STRUCTURE, TOK_STRUCT_ERR},
    {"Fin-struct", TOK_FIN_STRUCT, TOK_ENDSTRUCT_ERR},
    {"fin-struct", TOK_FIN_STRUCT, TOK_ENDSTRUCT_ERR},
    
    // === Entrées/Sorties ===
    {"Ecrire", TOK_ECRIRE, TOK_WRITE_ERR},
    {"ecrire", TOK_ECRIRE, TOK_WRITE_ERR},
    {"Lire", TOK_LIRE, TOK_READ_ERR},
    {"lire", TOK_LIRE, TOK_READ_ERR},
    {"Retour", TOK_RETOUR, TOK_NEWLINE_ERR},
    {"retour", TOK_RETOUR, TOK_NEWLINE_ERR},
    
    // === Constantes logiques ===
    {"Vrai", TOK_VRAI, TOK_TRUE_ERR},
    {"vrai", TOK_VRAI, TOK_TRUE_ERR},
    {"Faux", TOK_FAUX, TOK_FALSE_ERR},
    {"faux", TOK_FAUX, TOK_FALSE_ERR},
    {"Et", TOK_ET, TOK_AND_ERR},
    {"et", TOK_ET, TOK_AND_ERR},
    {"Ou", TOK_OU, TOK_OR_ERR},
    {"ou", TOK_OU, TOK_OR_ERR},
    {"Non", TOK_NON, TOK_NOT_ERR},
    {"non", TOK_NON, TOK_NOT_ERR},
    
    // === Structures de contrôle ===
    {"Si", TOK_SI, TOK_IF_ERR},
    {"si", TOK_SI, TOK_IF_ERR},
    {"Alors", TOK_ALORS, TOK_THEN_ERR},
    {"alors", TOK_ALORS, TOK_THEN_ERR},
    {"Sinon", TOK_SINON, TOK_ELSE_ERR},
    {"sinon", TOK_SINON, TOK_ELSE_ERR},
    {"FinSi", TOK_FIN_SI, TOK_ENDIF_ERR},
    {"finsi", TOK_FIN_SI, TOK_ENDIF_ERR},
    {"Selon", TOK_SELON, TOK_SWITCH_ERR},
    {"selon", TOK_SELON, TOK_SWITCH_ERR},
    {"FinSelon", TOK_FIN_SELON, TOK_ENDSWITCH_ERR},
    {"finselon", TOK_FIN_SELON, TOK_ENDSWITCH_ERR},
    {"Sortir", TOK_SORTIR, TOK_BREAK_SWITCH_ERR},
    {"sortir", TOK_SORTIR, TOK_BREAK_SWITCH_ERR},
    {"Pour", TOK_POUR, TOK_FOR_ERR},
    {"pour", TOK_POUR, TOK_FOR_ERR},
    {"jusqu'à", TOK_JUSQUA, TOK_TO_ERR},
    {"jusqua", TOK_JUSQUA, TOK_TO_ERR},
    {"pas", TOK_PAS, TOK_STEP_ERR},
    {"FinPour", TOK_FIN_POUR, TOK_ENDFOR_ERR},
    {"finpour", TOK_FIN_POUR, TOK_ENDFOR_ERR},
    {"Quitter", TOK_QUITTER_POUR, TOK_BREAK_FOR_ERR},
    {"quitter", TOK_QUITTER_POUR, TOK_BREAK_FOR_ERR},
    {"TantQue", TOK_TANTQUE, TOK_WHILE_ERR},
    {"tantque", TOK_TANTQUE, TOK_WHILE_ERR},
    
    // === Opérateurs spéciaux ===
    {"Div", TOK_DIV_ENTIER, TOK_INTDIV_ERR},
    {"div", TOK_DIV_ENTIER, TOK_INTDIV_ERR},
    {"Mod", TOK_MODULO, TOK_MOD_ERR},
    {"mod", TOK_MODULO, TOK_MOD_ERR},
    
    {NULL, TOK_EOF, TOK_ERREUR_GENERIQUE} // Marqueur de fin
};

// Structure pour la correspondance erreur/token
typedef struct {
    TokenType token_normal;
    TokenType token_erreur;
    const char* description;
} CorrespondanceErreur;

// Table de correspondance erreur/token
static const CorrespondanceErreur CORRESPONDANCES_ERREUR[] = {
    // Nouveaux tokens
    {TOK_GUILLEMET, TOK_GUILLEMET_ERR, "Guillemet mal formé"},
    {TOK_ID, TOK_ID_ERR, "Identifiant mal formé"},
    {TOK_CONST_ENTIERE, TOK_CONST_ENTIERE_ERR, "Constante entière mal formée"},
    {TOK_CONST_REEL, TOK_CONST_REEL_ERR, "Constante réelle mal formée"},
    {TOK_CONST_CHAINE, TOK_CONST_CHAINE_ERR, "Constante chaîne mal formée"},
    {TOK_PROCEDURE, TOK_PROCEDURE_ERR, "Procédure mal formée"},
    {TOK_FIN_PROC, TOK_FIN_PROC_ERR, "FinProc mal formé"},
    {TOK_FONCTION, TOK_FONCTION_ERR, "Fonction mal formée"},
    {TOK_FIN_FONCT, TOK_FIN_FONCT_ERR, "FinFonct mal formé"},
    {TOK_RETOURNER, TOK_RETOURNER_ERR, "Retourner mal formé"},
    {TOK_REPETER, TOK_REPETER_ERR, "Répéter mal formé"},
    
    // Tokens existants
    {TOK_ALGORITHME, TOK_ALG_ERR, "Algorithme mal formé"},
    {TOK_VARIABLES, TOK_VARS_ERR, "Variables mal formé"},
    {TOK_DEBUT, TOK_BEGIN_ERR, "Début mal formé"},
    {TOK_FIN, TOK_END_ERR, "Fin mal formé"},
    {TOK_VARIABLE, TOK_VAR_ERR, "Variable mal formée"},
    {TOK_CONSTANTE, TOK_CONST_ERR, "Constante mal formée"},
    {TOK_ENTIER, TOK_INT_TYPE_ERR, "Type entier mal formé"},
    {TOK_REEL, TOK_REAL_TYPE_ERR, "Type réel mal formé"},
    {TOK_CARACTERE, TOK_CHAR_TYPE_ERR, "Type caractère mal formé"},
    {TOK_CHAINE, TOK_STRING_TYPE_ERR, "Type chaine mal formé"},
    {TOK_BOOLEEN, TOK_BOOL_TYPE_ERR, "Type booléen mal formé"},
    {TOK_TABLEAU, TOK_ARRAY_ERR, "Tableau mal formé"},
    {TOK_DE, TOK_OF_ERR, "'de' mal formé"},
    {TOK_STRUCTURE, TOK_STRUCT_ERR, "Structure mal formée"},
    {TOK_FIN_STRUCT, TOK_ENDSTRUCT_ERR, "Fin-struct mal formé"},
    {TOK_ECRIRE, TOK_WRITE_ERR, "Ecrire mal formé"},
    {TOK_LIRE, TOK_READ_ERR, "Lire mal formé"},
    {TOK_RETOUR, TOK_NEWLINE_ERR, "Retour mal formé"},
    {TOK_VRAI, TOK_TRUE_ERR, "Vrai mal formé"},
    {TOK_FAUX, TOK_FALSE_ERR, "Faux mal formé"},
    {TOK_ET, TOK_AND_ERR, "Et mal formé"},
    {TOK_OU, TOK_OR_ERR, "Ou mal formé"},
    {TOK_NON, TOK_NOT_ERR, "Non mal formé"},
    {TOK_INFERIEUR, TOK_LT_ERR, "< mal formé"},
    {TOK_INFERIEUR_EGAL, TOK_LE_ERR, "<= mal formé"},
    {TOK_SUPERIEUR, TOK_GT_ERR, "> mal formé"},
    {TOK_SUPERIEUR_EGAL, TOK_GE_ERR, ">= mal formé"},
    {TOK_EGAL, TOK_EQ_ERR, "= mal formé"},
    {TOK_DIFFERENT, TOK_NEQ_ERR, "<> mal formé"},
    {TOK_AFFECTATION, TOK_ASSIGN_ERR, "<- mal formé"},
    {TOK_DEUX_POINTS, TOK_COLON_ERR, ": mal formé"},
    {TOK_VIRGULE, TOK_COMMA_ERR, ", mal formé"},
    {TOK_PAREN_OUVRANTE, TOK_LPAREN_ERR, "( mal formé"},
    {TOK_PAREN_FERMANTE, TOK_RPAREN_ERR, ") mal formé"},
    {TOK_CROCHET_OUVRANT, TOK_LBRACK_ERR, "[ mal formé"},
    {TOK_CROCHET_FERMANT, TOK_RBRACK_ERR, "] mal formé"},
    {TOK_POINTS, TOK_DOTDOT_ERR, ".. mal formé"},
    {TOK_PLUS, TOK_PLUS_ERR, "+ mal formé"},
    {TOK_MOINS, TOK_MINUS_ERR, "- mal formé"},
    {TOK_FOIS, TOK_MUL_ERR, "* mal formé"},
    {TOK_DIVISE, TOK_DIV_ERR, "/ mal formé"},
    {TOK_DIV_ENTIER, TOK_INTDIV_ERR, "Div mal formé"},
    {TOK_MODULO, TOK_MOD_ERR, "Mod mal formé"},
    {TOK_PUISSANCE, TOK_POW_ERR, "^ mal formé"},
    {TOK_SI, TOK_IF_ERR, "Si mal formé"},
    {TOK_ALORS, TOK_THEN_ERR, "Alors mal formé"},
    {TOK_SINON, TOK_ELSE_ERR, "Sinon mal formé"},
    {TOK_FIN_SI, TOK_ENDIF_ERR, "FinSi mal formé"},
    {TOK_SELON, TOK_SWITCH_ERR, "Selon mal formé"},
    {TOK_FIN_SELON, TOK_ENDSWITCH_ERR, "FinSelon mal formé"},
    {TOK_SORTIR, TOK_BREAK_SWITCH_ERR, "Sortir mal formé"},
    {TOK_POUR, TOK_FOR_ERR, "Pour mal formé"},
    {TOK_JUSQUA, TOK_TO_ERR, "jusqu'à mal formé"},
    {TOK_PAS, TOK_STEP_ERR, "pas mal formé"},
    {TOK_FIN_POUR, TOK_ENDFOR_ERR, "FinPour mal formé"},
    {TOK_QUITTER_POUR, TOK_BREAK_FOR_ERR, "Quitter Pour mal formé"},
    {TOK_TANTQUE, TOK_WHILE_ERR, "TantQue mal formé"},
    
    {TOK_EOF, TOK_ERREUR_GENERIQUE, "Fin de fichier"}
};

// ============================================================================
// FONCTIONS STATIQUES AUXILIAIRES
// ============================================================================

// Vérifie si on est à la fin du source
static bool est_fin_source(Lexer* lexer) {
    return lexer->position >= (int)strlen(lexer->source);
}

// Retourne le caractère courant
static char caractere_courant(Lexer* lexer) {
    if (est_fin_source(lexer)) return '\0';
    return lexer->source[lexer->position];
}

// Retourne le caractère suivant (avec décalage)
static char caractere_suivant(Lexer* lexer, int offset) {
    int pos = lexer->position + offset;
    if (pos >= (int)strlen(lexer->source)) return '\0';
    return lexer->source[pos];
}

// Avance de n caractères dans le source
static void avancer(Lexer* lexer, int n) {
    for (int i = 0; i < n; i++) {
        if (caractere_courant(lexer) == '\n') {
            lexer->ligne++;
            lexer->colonne = 1;
        } else {
            lexer->colonne++;
        }
        lexer->position++;
    }
}

// Vérifie si un caractère est un espace blanc
static bool est_blanc(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

// Vérifie si un caractère est un chiffre
static bool est_chiffre(char c) {
    return c >= '0' && c <= '9';
}

// Vérifie si un caractère est une lettre (inclut accents)
static bool est_lettre(char c) {
    return (c >= 'a' && c <= 'z') || 
           (c >= 'A' && c <= 'Z') || 
           c == '_' || 
           c == 'é' || c == 'è' || c == 'ê' || c == 'ë' ||
           c == 'à' || c == 'â' || c == 'ä' ||
           c == 'î' || c == 'ï' ||
           c == 'ô' || c == 'ö' ||
           c == 'ù' || c == 'û' || c == 'ü' ||
           c == 'ç';
}

// Vérifie si un caractère est alphanumérique
static bool est_alphanumerique(char c) {
    return est_lettre(c) || est_chiffre(c) || c == '_';
}

// Ignore les espaces blancs
static void ignorer_espaces(Lexer* lexer) {
    while (!est_fin_source(lexer) && est_blanc(caractere_courant(lexer))) {
        avancer(lexer, 1);
    }
}

// Trouve un mot-clé dans la table (insensible à la casse)
static TokenType trouver_mot_cle(const char* mot, TokenType* type_erreur) {
    for (int i = 0; MOTS_CLES_AVEC_ERREUR[i].mot != NULL; i++) {
        if (strcasecmp(MOTS_CLES_AVEC_ERREUR[i].mot, mot) == 0) {
            if (type_erreur) *type_erreur = MOTS_CLES_AVEC_ERREUR[i].type_erreur;
            return MOTS_CLES_AVEC_ERREUR[i].type;
        }
    }
    return TOK_IDENTIFIANT; // Si pas trouvé, c'est un identifiant
}

// Obtient le token d'erreur correspondant à un token normal
static TokenType obtenir_token_erreur_correspondant(TokenType token_normal) {
    for (int i = 0; CORRESPONDANCES_ERREUR[i].token_normal != TOK_EOF; i++) {
        if (CORRESPONDANCES_ERREUR[i].token_normal == token_normal) {
            return CORRESPONDANCES_ERREUR[i].token_erreur;
        }
    }
    return TOK_ERREUR_GENERIQUE;
}

// Ajoute un token au tableau
static void ajouter_token(Lexer* lexer, TokenType type, const char* valeur, bool est_erreur) {
    // Redimensionner si nécessaire
    if (lexer->nb_tokens >= lexer->capacite_tokens) {
        lexer->capacite_tokens *= 2;
        lexer->tokens = realloc(lexer->tokens, lexer->capacite_tokens * sizeof(Token));
    }
    
    Token* token = &lexer->tokens[lexer->nb_tokens++];
    token->type = type;
    token->valeur = strdup(valeur);
    token->ligne = lexer->ligne;
    token->colonne = lexer->colonne - strlen(valeur);
    if (token->colonne < 1) token->colonne = 1;
    token->est_erreur = est_erreur;
}

// Ajoute un message d'erreur
static void ajouter_message_erreur(Lexer* lexer, const char* message) {
    // Redimensionner si nécessaire
    if (lexer->nb_erreurs >= lexer->capacite_erreurs) {
        lexer->capacite_erreurs *= 2;
        lexer->messages_erreur = realloc(lexer->messages_erreur, 
                                        lexer->capacite_erreurs * sizeof(char*));
    }
    
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s:%d:%d: %s", 
             lexer->nom_fichier, lexer->ligne, lexer->colonne, message);
    
    lexer->messages_erreur[lexer->nb_erreurs++] = strdup(buffer);
}

// Ajoute un token d'erreur et un message
static void ajouter_erreur_lexicale(Lexer* lexer, TokenType type_erreur, 
                                    const char* valeur, const char* message) {
    ajouter_token(lexer, type_erreur, valeur, true);
    ajouter_message_erreur(lexer, message);
    
    // Si mode strict, on pourrait arrêter ici
    if (lexer->mode_strict && lexer->nb_erreurs >= 1) {
        // On pourrait ajouter une logique pour arrêter l'analyse
    }
}

// ============================================================================
// FONCTIONS DE LECTURE SPÉCIFIQUES
// ============================================================================

// Lit un identifiant ou mot-clé (utilise TOK_ID pour les identifiants)
static void lire_identifiant(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_col = lexer->colonne;
    
    // Premier caractère doit être une lettre
    if (!est_lettre(caractere_courant(lexer))) {
        char msg[64];
        snprintf(msg, sizeof(msg), "Identifiant invalide commençant par '%c'", 
                 caractere_courant(lexer));
        ajouter_erreur_lexicale(lexer, TOK_ID_ERR, 
                               &lexer->source[start_pos], msg);
        avancer(lexer, 1);
        return;
    }
    
    // Lire tous les caractères valides
    while (!est_fin_source(lexer) && 
           (est_alphanumerique(caractere_courant(lexer)) ||
            caractere_courant(lexer) == '\'' ||
            caractere_courant(lexer) == '-')) {
        avancer(lexer, 1);
    }
    
    // Extraire le lexème
    int length = lexer->position - start_pos;
    char* lexeme = malloc(length + 1);
    strncpy(lexeme, &lexer->source[start_pos], length);
    lexeme[length] = '\0';
    
    // Vérifier si c'est un mot-clé ou un identifiant
    TokenType type_erreur;
    TokenType type = trouver_mot_cle(lexeme, &type_erreur);
    
    // Traitement spécial pour "Quitter Pour"
    if (strcasecmp(lexeme, "Quitter") == 0 || strcasecmp(lexeme, "quitter") == 0) {
        ignorer_espaces(lexer);
        int sauvegarde_pos = lexer->position;
        int sauvegarde_ligne = lexer->ligne;
        int sauvegarde_col = lexer->colonne;
        
        char suivant[10];
        int i = 0;
        while (!est_fin_source(lexer) && i < 9 && 
               est_lettre(caractere_courant(lexer))) {
            suivant[i++] = caractere_courant(lexer);
            avancer(lexer, 1);
        }
        suivant[i] = '\0';
        
        if (strcasecmp(suivant, "Pour") == 0) {
            // C'est "Quitter Pour"
            char* combinaison = malloc(strlen(lexeme) + strlen(suivant) + 2);
            sprintf(combinaison, "%s %s", lexeme, suivant);
            ajouter_token(lexer, TOK_QUITTER_POUR, combinaison, false);
            free(combinaison);
            free(lexeme);
            return;
        } else {
            // Juste "Quitter" seul
            lexer->position = sauvegarde_pos;
            lexer->ligne = sauvegarde_ligne;
            lexer->colonne = sauvegarde_col;
        }
    }
    
    if (type != TOK_IDENTIFIANT) {
        // C'est un mot-clé
        ajouter_token(lexer, type, lexeme, false);
    } else {
        // C'est un identifiant normal - utiliser TOK_ID
        ajouter_token(lexer, TOK_ID, lexeme, false);
    }
    
    free(lexeme);
}

// Lit un nombre (utilise TOK_CONST_ENTIERE ou TOK_CONST_REEL)
static void lire_nombre(Lexer* lexer) {
    int start_pos = lexer->position;
    int start_col = lexer->colonne;
    bool est_reel = false;
    bool erreur = false;
    
    // Vérifier qu'il y a au moins un chiffre
    if (!est_chiffre(caractere_courant(lexer))) {
        erreur = true;
    }
    
    // Partie entière
    while (!est_fin_source(lexer) && est_chiffre(caractere_courant(lexer))) {
        avancer(lexer, 1);
    }
    
    // Point décimal
    if (caractere_courant(lexer) == '.') {
        est_reel = true;
        avancer(lexer, 1);
        
        // Doit être suivi de chiffres pour un nombre réel valide
        if (!est_chiffre(caractere_courant(lexer))) {
            erreur = true;
        }
        
        // Partie fractionnaire
        while (!est_fin_source(lexer) && est_chiffre(caractere_courant(lexer))) {
            avancer(lexer, 1);
        }
    }
    
    // Notation scientifique
    if (caractere_courant(lexer) == 'e' || caractere_courant(lexer) == 'E') {
        est_reel = true;
        avancer(lexer, 1);
        
        // Signe optionnel
        if (caractere_courant(lexer) == '+' || caractere_courant(lexer) == '-') {
            avancer(lexer, 1);
        }
        
        // Doit être suivi de chiffres
        if (!est_chiffre(caractere_courant(lexer))) {
            erreur = true;
        }
        
        // Chiffres de l'exposant
        while (!est_fin_source(lexer) && est_chiffre(caractere_courant(lexer))) {
            avancer(lexer, 1);
        }
    }
    
    // Extraire le nombre
    int length = lexer->position - start_pos;
    char* nombre = malloc(length + 1);
    strncpy(nombre, &lexer->source[start_pos], length);
    nombre[length] = '\0';
    
    if (erreur) {
        // Utiliser les nouveaux tokens d'erreur
        if (est_reel) {
            ajouter_erreur_lexicale(lexer, TOK_CONST_REEL_ERR,
                                   nombre, "Constante réelle invalide");
        } else {
            ajouter_erreur_lexicale(lexer, TOK_CONST_ENTIERE_ERR,
                                   nombre, "Constante entière invalide");
        }
    } else {
        // Utiliser les nouveaux tokens
        if (est_reel) {
            ajouter_token(lexer, TOK_CONST_REEL, nombre, false);
        } else {
            ajouter_token(lexer, TOK_CONST_ENTIERE, nombre, false);
        }
    }
    
    free(nombre);
}

// Lit une chaîne de caractères (utilise TOK_CONST_CHAINE et TOK_GUILLEMET)
static void lire_chaine(Lexer* lexer) {
    char delimiteur = caractere_courant(lexer);
    int start_ligne = lexer->ligne;
    int start_col = lexer->colonne;
    
    // Vérifier que c'est un délimiteur valide
    if (delimiteur != '"' && delimiteur != '\'') {
        ajouter_erreur_lexicale(lexer, TOK_GUILLEMET_ERR, 
                               (char[]){delimiteur, '\0'}, 
                               "Délimiteur de chaîne invalide");
        avancer(lexer, 1);
        return;
    }
    
    // Ajouter le guillemet ouvrant
    avancer(lexer, 1);
    ajouter_token(lexer, TOK_GUILLEMET, (char[]){delimiteur, '\0'}, false);
    
    int start_pos = lexer->position;
    bool escape = false;
    
    // Lire le contenu de la chaîne
    while (!est_fin_source(lexer)) {
        if (escape) {
            escape = false;
            avancer(lexer, 1);
            continue;
        }
        
        if (caractere_courant(lexer) == '\\') {
            escape = true;
            avancer(lexer, 1);
            continue;
        }
        
        if (caractere_courant(lexer) == delimiteur) {
            break;
        }
        
        if (caractere_courant(lexer) == '\n') {
            // Chaîne non fermée - erreur
            break;
        }
        
        avancer(lexer, 1);
    }
    
    // Extraire le contenu
    int length = lexer->position - start_pos;
    char* contenu = malloc(length + 1);
    strncpy(contenu, &lexer->source[start_pos], length);
    contenu[length] = '\0';
    
    // Ajouter le contenu comme constante chaîne ou caractère
    if (delimiteur == '"') {
        // C'est une chaîne de caractères
        ajouter_token(lexer, TOK_CONST_CHAINE, contenu, false);
    } else {
        // C'est un caractère
        if (length == 1 || (length == 2 && contenu[0] == '\\')) {
            ajouter_token(lexer, TOK_CARACTERE_LITTERAL, contenu, false);
        } else {
            ajouter_erreur_lexicale(lexer, TOK_CONST_CHAINE_ERR, 
                                   contenu, "Caractère littéral invalide");
        }
    }
    
    free(contenu);
    
    // Vérifier la fermeture de la chaîne
    if (est_fin_source(lexer)) {
        ajouter_erreur_lexicale(lexer, TOK_GUILLEMET_ERR, 
                               "", "Chaîne non fermée");
    } else {
        // Ajouter le guillemet fermant
        avancer(lexer, 1);
        ajouter_token(lexer, TOK_GUILLEMET, (char[]){delimiteur, '\0'}, false);
    }
}

// Lit un commentaire de ligne
static void lire_commentaire_ligne(Lexer* lexer) {
    avancer(lexer, 2); // Passer "//"
    
    int start_pos = lexer->position;
    
    // Lire jusqu'à la fin de la ligne
    while (!est_fin_source(lexer) && caractere_courant(lexer) != '\n') {
        avancer(lexer, 1);
    }
    
    // Extraire le commentaire (optionnel)
    int length = lexer->position - start_pos;
    if (length > 0) {
        char* commentaire = malloc(length + 1);
        strncpy(commentaire, &lexer->source[start_pos], length);
        commentaire[length] = '\0';
        ajouter_token(lexer, TOK_COMMENTAIRE, commentaire, false);
        free(commentaire);
    }
    
    // Passer le saut de ligne
    if (!est_fin_source(lexer) && caractere_courant(lexer) == '\n') {
        avancer(lexer, 1);
    }
}

// Lit un commentaire de bloc
static void lire_commentaire_bloc(Lexer* lexer) {
    avancer(lexer, 2); // Passer "/*"
    
    int start_pos = lexer->position;
    int start_ligne = lexer->ligne;
    int start_col = lexer->colonne;
    
    // Lire jusqu'à trouver "*/"
    while (!est_fin_source(lexer)) {
        if (caractere_courant(lexer) == '*' && caractere_suivant(lexer, 1) == '/') {
            break;
        }
        avancer(lexer, 1);
    }
    
    if (est_fin_source(lexer)) {
        ajouter_erreur_lexicale(lexer, TOK_ERREUR_GENERIQUE, "", 
                               "Commentaire bloc non fermé");
        return;
    }
    
    // Extraire le commentaire
    int length = lexer->position - start_pos;
    if (length > 0) {
        char* commentaire = malloc(length + 1);
        strncpy(commentaire, &lexer->source[start_pos], length);
        commentaire[length] = '\0';
        ajouter_token(lexer, TOK_COMMENTAIRE, commentaire, false);
        free(commentaire);
    }
    
    avancer(lexer, 2); // Passer "*/"
}

// Traite les opérateurs et symboles
static void traiter_operateurs(Lexer* lexer) {
    char courant = caractere_courant(lexer);
    char suivant = caractere_suivant(lexer, 1);
    
    // D'abord vérifier si c'est un délimiteur de chaîne
    if (courant == '"' || courant == '\'') {
        lire_chaine(lexer);
        return;
    }
    
    switch (courant) {
        // Affectation < ou opérateur <
        case '<':
            if (suivant == '-') {
                avancer(lexer, 2);
                ajouter_token(lexer, TOK_AFFECTATION, "<-", false);
            } else if (suivant == '=') {
                avancer(lexer, 2);
                ajouter_token(lexer, TOK_INFERIEUR_EGAL, "<=", false);
            } else if (suivant == '>') {
                avancer(lexer, 2);
                ajouter_token(lexer, TOK_DIFFERENT, "<>", false);
            } else {
                avancer(lexer, 1);
                ajouter_token(lexer, TOK_INFERIEUR, "<", false);
            }
            break;
            
        // Supérieur > ou >=
        case '>':
            if (suivant == '=') {
                avancer(lexer, 2);
                ajouter_token(lexer, TOK_SUPERIEUR_EGAL, ">=", false);
            } else {
                avancer(lexer, 1);
                ajouter_token(lexer, TOK_SUPERIEUR, ">", false);
            }
            break;
            
        // Égalité =
        case '=':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_EGAL, "=", false);
            break;
            
        // Points ..
        case '.':
            if (suivant == '.') {
                avancer(lexer, 2);
                ajouter_token(lexer, TOK_POINTS, "..", false);
            } else {
                // Point simple - erreur dans ce contexte
                ajouter_erreur_lexicale(lexer, TOK_DOTDOT_ERR, ".", 
                                       "Point isolé non valide");
                avancer(lexer, 1);
            }
            break;
            
        // Opérateurs arithmétiques
        case '+':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_PLUS, "+", false);
            break;
            
        case '-':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_MOINS, "-", false);
            break;
            
        case '*':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_FOIS, "*", false);
            break;
            
        case '/':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_DIVISE, "/", false);
            break;
            
        case '^':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_PUISSANCE, "^", false);
            break;
            
        // Ponctuation
        case ':':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_DEUX_POINTS, ":", false);
            break;
            
        case ',':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_VIRGULE, ",", false);
            break;
            
        case '(':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_PAREN_OUVRANTE, "(", false);
            break;
            
        case ')':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_PAREN_FERMANTE, ")", false);
            break;
            
        case '[':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_CROCHET_OUVRANT, "[", false);
            break;
            
        case ']':
            avancer(lexer, 1);
            ajouter_token(lexer, TOK_CROCHET_FERMANT, "]", false);
            break;
            
        default:
            // Caractère inconnu
            char msg[64];
            snprintf(msg, sizeof(msg), "Caractère inconnu: '%c' (0x%02x)", 
                     courant, courant);
            ajouter_erreur_lexicale(lexer, TOK_ERREUR_GENERIQUE, 
                                   (char[]){courant, '\0'}, msg);
            avancer(lexer, 1);
            break;
    }
}

// ============================================================================
// FONCTIONS PUBLIQUES
// ============================================================================

// Crée un nouveau lexer
Lexer* creer_lexer(const char* source, const char* nom_fichier) {
    Lexer* lexer = malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->source = source;
    lexer->position = 0;
    lexer->ligne = 1;
    lexer->colonne = 1;
    
    lexer->nb_tokens = 0;
    lexer->capacite_tokens = 256;
    lexer->tokens = malloc(lexer->capacite_tokens * sizeof(Token));
    
    lexer->nb_erreurs = 0;
    lexer->capacite_erreurs = 16;
    lexer->messages_erreur = malloc(lexer->capacite_erreurs * sizeof(char*));
    
    lexer->nom_fichier = strdup(nom_fichier ? nom_fichier : "stdin");
    lexer->mode_strict = false;
    
    return lexer;
}

// Détruit un lexer et libère la mémoire
void detruire_lexer(Lexer* lexer) {
    if (!lexer) return;
    
    // Libérer les tokens
    for (int i = 0; i < lexer->nb_tokens; i++) {
        free(lexer->tokens[i].valeur);
    }
    free(lexer->tokens);
    
    // Libérer les messages d'erreur
    for (int i = 0; i < lexer->nb_erreurs; i++) {
        free(lexer->messages_erreur[i]);
    }
    free(lexer->messages_erreur);
    
    free(lexer->nom_fichier);
    free(lexer);
}

// Analyse lexicale principale
bool analyser_lexicalement(Lexer* lexer) {
    if (!lexer || !lexer->source) return false;
    
    while (!est_fin_source(lexer)) {
        char courant = caractere_courant(lexer);
        
        // Ignorer les espaces
        if (est_blanc(courant)) {
            ignorer_espaces(lexer);
            continue;
        }
        
        // Commentaires de ligne
        if (courant == '/' && caractere_suivant(lexer, 1) == '/') {
            lire_commentaire_ligne(lexer);
            continue;
        }
        
        // Commentaires de bloc
        if (courant == '/' && caractere_suivant(lexer, 1) == '*') {
            lire_commentaire_bloc(lexer);
            continue;
        }
        
        // Nombres (doivent être avant les identifiants)
        if (est_chiffre(courant)) {
            lire_nombre(lexer);
            continue;
        }
        
        // Identifiants et mots-clés
        if (est_lettre(courant)) {
            lire_identifiant(lexer);
            continue;
        }
        
        // Opérateurs et symboles (inclut les guillemets)
        traiter_operateurs(lexer);
    }
    
    // Ajouter le token EOF
    ajouter_token(lexer, TOK_EOF, "", false);
    
    return lexer->nb_erreurs == 0;
}

// Retourne les tokens générés
Token* obtenir_tokens(Lexer* lexer, int* nb_tokens) {
    if (nb_tokens) *nb_tokens = lexer->nb_tokens;
    return lexer->tokens;
}

// Retourne les messages d'erreur
char** obtenir_messages_erreur(Lexer* lexer, int* nb_erreurs) {
    if (nb_erreurs) *nb_erreurs = lexer->nb_erreurs;
    return lexer->messages_erreur;
}

// Affiche tous les tokens
void afficher_tokens(Lexer* lexer) {
    printf("=== Tokens (%d) ===\n", lexer->nb_tokens);
    for (int i = 0; i < lexer->nb_tokens; i++) {
        printf("%4d: ", i);
        afficher_token(&lexer->tokens[i]);
    }
}

// Affiche les erreurs
void afficher_erreurs(Lexer* lexer) {
    if (lexer->nb_erreurs == 0) {
        printf("✅ Aucune erreur lexicale détectée.\n");
        return;
    }
    
    printf("=== Erreurs lexicales (%d) ===\n", lexer->nb_erreurs);
    for (int i = 0; i < lexer->nb_erreurs; i++) {
        printf("❌ %s\n", lexer->messages_erreur[i]);
    }
}

// Compte les tokens d'erreur
int compter_tokens_erreur(Lexer* lexer) {
    int count = 0;
    for (int i = 0; i < lexer->nb_tokens; i++) {
        if (lexer->tokens[i].est_erreur) {
            count++;
        }
    }
    return count;
}

// Active/désactive le mode strict
void set_mode_strict(Lexer* lexer, bool strict) {
    if (lexer) {
        lexer->mode_strict = strict;
    }
}