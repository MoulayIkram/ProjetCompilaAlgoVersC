#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"

// Structure du lexer
typedef struct {
    const char* source;        // Code source
    int position;              // Position courante
    int ligne;                 // Ligne courante
    int colonne;               // Colonne courante
    Token* tokens;             // Tableau de tokens
    int nb_tokens;             // Nombre de tokens
    int capacite_tokens;       // Capacité du tableau
    char** messages_erreur;    // Messages d'erreur détaillés
    int nb_erreurs;            // Nombre d'erreurs
    int capacite_erreurs;      // Capacité du tableau d'erreurs
    char* nom_fichier;         // Nom du fichier source
    bool mode_strict;          // Mode strict (arrête à la première erreur)
} Lexer;

// Fonctions publiques
Lexer* creer_lexer(const char* source, const char* nom_fichier);
void detruire_lexer(Lexer* lexer);
bool analyser_lexicalement(Lexer* lexer);
Token* obtenir_tokens(Lexer* lexer, int* nb_tokens);
char** obtenir_messages_erreur(Lexer* lexer, int* nb_erreurs);
void afficher_tokens(Lexer* lexer);
void afficher_erreurs(Lexer* lexer);
int compter_tokens_erreur(Lexer* lexer);

#endif // LEXER_H