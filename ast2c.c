// ast2c.c - Génération de code C "haut niveau" depuis AST
// C11, UTF-8 OK

#include "ast2c.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "token.h"

#define STR_SIZE 1024

// =========================
// String builder
// =========================
typedef struct {
    char* data;
    int len;
    int cap;
} SB;

static void sb_init(SB* s) { s->data = NULL; s->len = 0; s->cap = 0; }
static void sb_free(SB* s) { free(s->data); s->data = NULL; s->len = 0; s->cap = 0; }

static void sb_reserve(SB* s, int need) {
    if (need <= s->cap) return;
    int ncap = (s->cap == 0) ? 1024 : s->cap;
    while (ncap < need) ncap *= 2;
    char* n = (char*)realloc(s->data, (size_t)ncap);
    if (!n) return;
    s->data = n;
    s->cap = ncap;
}
static void sb_add(SB* s, const char* txt) {
    if (!txt) return;
    int n = (int)strlen(txt);
    sb_reserve(s, s->len + n + 1);
    memcpy(s->data + s->len, txt, (size_t)n);
    s->len += n;
    s->data[s->len] = 0;
}
static void sb_addf(SB* s, const char* fmt, ...) {
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    sb_add(s, buf);
}

static char* xstrdup(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* r = (char*)malloc(n + 1);
    if (!r) return NULL;
    memcpy(r, s, n + 1);
    return r;
}

// =========================
// C string escaping
// =========================
static void sb_add_c_escaped(SB* out, const char* s) {
    sb_add(out, "\"");
    if (s) {
        for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
            unsigned char c = *p;
            if (c == '\\') sb_add(out, "\\\\");
            else if (c == '\"') sb_add(out, "\\\"");
            else if (c == '\n') sb_add(out, "\\n");
            else if (c == '\r') sb_add(out, "\\r");
            else if (c == '\t') sb_add(out, "\\t");
            else {
                char b[2] = {(char)c, 0};
                sb_add(out, b);
            }
        }
    }
    sb_add(out, "\"");
}

// =========================
// Type system (pour printf/scanf + déclarations)
// =========================
typedef enum {
    TY_UNKNOWN = 0,
    TY_INT,
    TY_REAL,
    TY_BOOL,
    TY_CHAR,
    TY_STRING,
    TY_STRUCT,
    TY_ARRAY
} TypeKind;

typedef struct {
    TypeKind kind;
    TypeKind leaf_kind;       // si TY_ARRAY
    int dims;                 // nb dimensions restantes
    const char* struct_name;  // si TY_STRUCT ou array de struct
} CType;

static CType ct_unknown(void){ CType t; memset(&t,0,sizeof(t)); t.kind=TY_UNKNOWN; return t; }
static CType ct_scalar(TypeKind k){ CType t; memset(&t,0,sizeof(t)); t.kind=k; return t; }
static CType ct_struct(const char* n){ CType t; memset(&t,0,sizeof(t)); t.kind=TY_STRUCT; t.struct_name=n; return t; }
static CType ct_array(TypeKind leaf, int dims, const char* sn){ CType t; memset(&t,0,sizeof(t)); t.kind=TY_ARRAY; t.leaf_kind=leaf; t.dims=dims; t.struct_name=sn; return t; }

static TypeKind prim_to_typekind(PrimitiveType pt) {
    switch (pt) {
        case TYPE_ENTIER:    return TY_INT;
        case TYPE_REEL:      return TY_REAL;
        case TYPE_BOOLEEN:   return TY_BOOL;
        case TYPE_CARACTERE: return TY_CHAR;
        case TYPE_CHAINE:    return TY_STRING;
        default:             return TY_UNKNOWN;
    }
}

static CType type_from_type_node(ASTNode* tn) {
    if (!tn) return ct_unknown();

    if (tn->kind == AST_TYPE_PRIMITIVE) {
        return ct_scalar(prim_to_typekind(tn->as.type_prim.prim));
    }
    if (tn->kind == AST_TYPE_NAMED) {
        return ct_struct(tn->as.type_named.name);
    }
    if (tn->kind == AST_TYPE_ARRAY) {
        int d = tn->as.type_array.dims.count;   // peut être 0 pour "[]"
        CType base = type_from_type_node(tn->as.type_array.elem_type);
        if (base.kind == TY_STRUCT) return ct_array(TY_STRUCT, d, base.struct_name);
        if (base.kind == TY_ARRAY)  return ct_array(base.leaf_kind, d + base.dims, base.struct_name);
        return ct_array(base.kind, d, NULL);
    }
    return ct_unknown();
}

// =========================
// Struct table (types champs)
// =========================
typedef struct {
    char* field;
    ASTNode* type_node;
} Field;

typedef struct {
    char* name;
    Field* fields;
    int count;
    int cap;
} StructInfo;

typedef struct {
    StructInfo* items;
    int count;
    int cap;
} StructTable;

static void st_init(StructTable* t){ memset(t,0,sizeof(*t)); }
static void st_free(StructTable* t){
    for(int i=0;i<t->count;i++){
        free(t->items[i].name);
        for(int j=0;j<t->items[i].count;j++) free(t->items[i].fields[j].field);
        free(t->items[i].fields);
    }
    free(t->items);
    memset(t,0,sizeof(*t));
}
static StructInfo* st_find(StructTable* t, const char* name){
    if(!t||!name) return NULL;
    for(int i=0;i<t->count;i++) if(t->items[i].name && strcmp(t->items[i].name,name)==0) return &t->items[i];
    return NULL;
}
static StructInfo* st_add(StructTable* t, const char* name){
    if(!t||!name) return NULL;
    if(t->count>=t->cap){
        int ncap = (t->cap==0)?8:t->cap*2;
        StructInfo* n = (StructInfo*)realloc(t->items,(size_t)ncap*sizeof(StructInfo));
        if(!n) return NULL;
        t->items=n; t->cap=ncap;
    }
    StructInfo* si=&t->items[t->count++];
    memset(si,0,sizeof(*si));
    si->name=xstrdup(name);
    return si;
}
static void si_add_field(StructInfo* si, const char* field, ASTNode* type_node){
    if(!si||!field) return;
    if(si->count>=si->cap){
        int ncap = (si->cap==0)?8:si->cap*2;
        Field* n = (Field*)realloc(si->fields,(size_t)ncap*sizeof(Field));
        if(!n) return;
        si->fields=n; si->cap=ncap;
    }
    si->fields[si->count].field = xstrdup(field);
    si->fields[si->count].type_node = type_node;
    si->count++;
}
static ASTNode* si_field_type(StructInfo* si, const char* field){
    if(!si||!field) return NULL;
    for(int i=0;i<si->count;i++){
        if(si->fields[i].field && strcmp(si->fields[i].field,field)==0) return si->fields[i].type_node;
    }
    return NULL;
}

static void st_build_from_program(StructTable* st, ASTNode* program){
    if(!st||!program||program->kind!=AST_PROGRAM) return;
    for(int i=0;i<program->as.program.defs.count;i++){
        ASTNode* d = program->as.program.defs.items[i];
        if(!d || d->kind!=AST_DEF_STRUCT) continue;
        StructInfo* si = st_add(st, d->as.def_struct.name);
        if(!si) continue;
        for(int f=0; f<d->as.def_struct.fields.count; f++){
            ASTNode* fld = d->as.def_struct.fields.items[f];
            if(!fld || fld->kind!=AST_FIELD) continue;
            si_add_field(si, fld->as.field.name, fld->as.field.type);
        }
    }
}

// =========================
// Signature table (return type)
// =========================
typedef struct {
    char* name;
    bool is_func;
    ASTNode* ret_type;
} Sig;

typedef struct {
    Sig* items;
    int count;
    int cap;
} SigTable;

static void sig_init(SigTable* t){ memset(t,0,sizeof(*t)); }
static void sig_free(SigTable* t){
    for(int i=0;i<t->count;i++) free(t->items[i].name);
    free(t->items);
    memset(t,0,sizeof(*t));
}
static void sig_add(SigTable* t, const char* name, bool is_func, ASTNode* ret){
    if(!t||!name) return;
    if(t->count>=t->cap){
        int ncap=(t->cap==0)?16:t->cap*2;
        Sig* n=(Sig*)realloc(t->items,(size_t)ncap*sizeof(Sig));
        if(!n) return;
        t->items=n; t->cap=ncap;
    }
    t->items[t->count].name = xstrdup(name);
    t->items[t->count].is_func = is_func;
    t->items[t->count].ret_type = ret;
    t->count++;
}
static Sig* sig_find(SigTable* t, const char* name){
    if(!t||!name) return NULL;
    for(int i=0;i<t->count;i++){
        if(t->items[i].name && strcmp(t->items[i].name,name)==0) return &t->items[i];
    }
    return NULL;
}
static ASTNode* sig_ret_type(SigTable* t, const char* name){
    Sig* s = sig_find(t,name);
    return s ? s->ret_type : NULL;
}
static void sig_build_from_program(SigTable* t, ASTNode* program){
    if(!t||!program||program->kind!=AST_PROGRAM) return;
    for(int i=0;i<program->as.program.defs.count;i++){
        ASTNode* d = program->as.program.defs.items[i];
        if(!d) continue;
        if(d->kind==AST_DEF_FUNC) sig_add(t, d->as.def_func.name, true, d->as.def_func.return_type);
        else if(d->kind==AST_DEF_PROC) sig_add(t, d->as.def_proc.name, false, NULL);
    }
}

// =========================
// Symbol env (types des ident)
// =========================
typedef struct {
    char* name;
    ASTNode* type_node;  // scalaire OU elem_type si array
    bool is_array;
    int array_dims;
    ASTList array_dim_exprs; // dimensions (si connues)
} Sym;

typedef struct Env {
    struct Env* parent;
    Sym* items;
    int count;
    int cap;
} Env;

static void astlist_shallow_copy(ASTList* dst, ASTList* src){
    dst->items = src ? src->items : NULL;
    dst->count = src ? src->count : 0;
    dst->cap   = src ? src->cap : 0;
}

static Env* env_push(Env* cur){
    Env* e=(Env*)calloc(1,sizeof(Env));
    if(!e) return cur;
    e->parent=cur;
    return e;
}
static Env* env_pop(Env* cur){
    if(!cur) return NULL;
    Env* p=cur->parent;
    for(int i=0;i<cur->count;i++) free(cur->items[i].name);
    free(cur->items);
    free(cur);
    return p;
}
static Sym* env_lookup(Env* e, const char* name){
    for(Env* it=e; it; it=it->parent){
        for(int i=0;i<it->count;i++){
            if(it->items[i].name && strcmp(it->items[i].name,name)==0) return &it->items[i];
        }
    }
    return NULL;
}
static Sym* env_add(Env* e, const char* name, ASTNode* type_node, bool is_array, int dims, ASTList* dim_exprs){
    if(!e||!name) return NULL;
    if(e->count>=e->cap){
        int ncap=(e->cap==0)?32:e->cap*2;
        Sym* n=(Sym*)realloc(e->items,(size_t)ncap*sizeof(Sym));
        if(!n) return NULL;
        e->items=n; e->cap=ncap;
    }
    Sym* s=&e->items[e->count++];
    memset(s,0,sizeof(*s));
    s->name=xstrdup(name);
    s->type_node=type_node;
    s->is_array=is_array;
    s->array_dims=dims;
    if(dim_exprs) astlist_shallow_copy(&s->array_dim_exprs, dim_exprs);
    return s;
}

// =========================
// Pretty-print types C
// =========================
static void emit_indent(FILE* out, int ind){
    for(int i=0;i<ind;i++) fputc(' ', out);
}

static void emit_c_type(FILE* out, ASTNode* type_node, bool as_field) {
    // as_field : pour string => char[STR_SIZE] dans struct
    if(!type_node){ fputs("int", out); return; }

    if(type_node->kind==AST_TYPE_PRIMITIVE){
        switch(type_node->as.type_prim.prim){
            case TYPE_ENTIER: fputs("int", out); return;
            case TYPE_REEL: fputs("double", out); return;
            case TYPE_BOOLEEN: fputs("bool", out); return;
            case TYPE_CARACTERE: fputs("char", out); return;
            case TYPE_CHAINE:
                if(as_field) fputs("char", out);
                else fputs("char", out);
                return;
            default: fputs("int", out); return;
        }
    }
    if(type_node->kind==AST_TYPE_NAMED){
        fputs(type_node->as.type_named.name ? type_node->as.type_named.name : "int", out);
        return;
    }
    if(type_node->kind==AST_TYPE_ARRAY){
        // On ne met pas les dims ici (c'est géré par déclarations)
        emit_c_type(out, type_node->as.type_array.elem_type, as_field);
        return;
    }
    fputs("int", out);
}

// =========================
// Expr to C string + Type inference (juste assez pour IO)
// =========================
static void expr_to_c(SB* out, ASTNode* e, StructTable* st, SigTable* sigs, Env* env);

static const char* normalize_real_tmp(const char* txt, char* buf, size_t nbuf){
    if(!txt){ snprintf(buf, nbuf, "0.0"); return buf; }
    size_t j=0;
    for(size_t i=0; txt[i] && j+1<nbuf; i++){
        buf[j++] = (txt[i]==',') ? '.' : txt[i];
    }
    buf[j]=0;
    return buf;
}

static CType typeof_ident(Env* env, const char* name){
    Sym* s = env_lookup(env, name);
    if(!s) return ct_unknown();

    if(s->is_array){
        CType base = type_from_type_node(s->type_node);
        if(base.kind==TY_STRUCT) return ct_array(TY_STRUCT, s->array_dims, base.struct_name);
        if(base.kind==TY_ARRAY)  return ct_array(base.leaf_kind, s->array_dims + base.dims, base.struct_name);
        return ct_array(base.kind, s->array_dims, NULL);
    }
    return type_from_type_node(s->type_node);
}

static CType typeof_expr(ASTNode* e, StructTable* st, SigTable* sigs, Env* env){
    if(!e) return ct_unknown();

    switch(e->kind){
        case AST_LITERAL_INT: return ct_scalar(TY_INT);
        case AST_LITERAL_REAL: return ct_scalar(TY_REAL);
        case AST_LITERAL_BOOL: return ct_scalar(TY_BOOL);
        case AST_LITERAL_STRING: return ct_scalar(TY_STRING);

        case AST_IDENT:
            return typeof_ident(env, e->as.ident.name);

        case AST_INDEX: {
            CType bt = typeof_expr(e->as.index.base, st, sigs, env);
            if(bt.kind==TY_ARRAY && bt.dims>0){
                int nd = bt.dims-1;
                if(nd==0){
                    if(bt.leaf_kind==TY_STRUCT) return ct_struct(bt.struct_name);
                    return ct_scalar(bt.leaf_kind);
                }
                return ct_array(bt.leaf_kind, nd, bt.struct_name);
            }
            return ct_unknown();
        }

        case AST_FIELD_ACCESS: {
            CType bt = typeof_expr(e->as.field_access.base, st, sigs, env);
            if(bt.kind!=TY_STRUCT || !bt.struct_name) return ct_unknown();
            StructInfo* si = st_find(st, bt.struct_name);
            if(!si) return ct_unknown();
            ASTNode* ftn = si_field_type(si, e->as.field_access.field);
            return type_from_type_node(ftn);
        }

        case AST_CALL: {
            const char* name=NULL;
            if(e->as.call.callee && e->as.call.callee->kind==AST_IDENT) name=e->as.call.callee->as.ident.name;
            ASTNode* rt = sig_ret_type(sigs, name?name:"");
            return type_from_type_node(rt);
        }

        case AST_UNARY: {
            if(e->as.unary.op==TOK_NON) return ct_scalar(TY_BOOL);
            // -x : type de x
            return typeof_expr(e->as.unary.expr, st, sigs, env);
        }

        case AST_BINARY: {
            TokenType op = e->as.binary.op;

            // comparaisons + logique => bool
            if(op==TOK_ET || op==TOK_OU ||
               op==TOK_EGAL || op==TOK_DIFFERENT ||
               op==TOK_INFERIEUR || op==TOK_INFERIEUR_EGAL ||
               op==TOK_SUPERIEUR || op==TOK_SUPERIEUR_EGAL){
                return ct_scalar(TY_BOOL);
            }

            CType a = typeof_expr(e->as.binary.lhs, st, sigs, env);
            CType b = typeof_expr(e->as.binary.rhs, st, sigs, env);

            if(op==TOK_DIVISE) return ct_scalar(TY_REAL);
            if(a.kind==TY_REAL || b.kind==TY_REAL) return ct_scalar(TY_REAL);
            if(a.kind==TY_INT && b.kind==TY_INT) return ct_scalar(TY_INT);

            return ct_unknown();
        }

        default:
            return ct_unknown();
    }
}

static int op_prec(TokenType op){
    // plus le nombre est grand => plus forte priorité
    switch(op){
        case TOK_OU: return 1;
        case TOK_ET: return 2;
        case TOK_EGAL: case TOK_DIFFERENT:
        case TOK_INFERIEUR: case TOK_INFERIEUR_EGAL:
        case TOK_SUPERIEUR: case TOK_SUPERIEUR_EGAL: return 3;
        case TOK_PLUS: case TOK_MOINS: return 4;
        case TOK_FOIS: case TOK_DIVISE: case TOK_DIV_ENTIER: case TOK_MODULO: return 5;
        case TOK_PUISSANCE: return 6;
        default: return 0;
    }
}

static void expr_to_c_prec(SB* out, ASTNode* e, StructTable* st, SigTable* sigs, Env* env, int parent_prec);

static void expr_to_c(SB* out, ASTNode* e, StructTable* st, SigTable* sigs, Env* env){
    expr_to_c_prec(out, e, st, sigs, env, 0);
}

static void expr_to_c_prec(SB* out, ASTNode* e, StructTable* st, SigTable* sigs, Env* env, int parent_prec){
    if(!e){ sb_add(out, "0"); return; }

    switch(e->kind){
        case AST_LITERAL_INT:
            sb_addf(out, "%lld", e->as.lit_int.value);
            return;

        case AST_LITERAL_REAL: {
            char buf[128];
            sb_add(out, normalize_real_tmp(e->as.lit_real.text, buf, sizeof(buf)));
            return;
        }

        case AST_LITERAL_BOOL:
            sb_add(out, e->as.lit_bool.value ? "true" : "false");
            return;

        case AST_LITERAL_STRING:
            sb_add_c_escaped(out, e->as.lit_string.text ? e->as.lit_string.text : "");
            return;

        case AST_IDENT:
            sb_add(out, e->as.ident.name ? e->as.ident.name : "<?>");
            return;

        case AST_INDEX:
            expr_to_c_prec(out, e->as.index.base, st, sigs, env, 10);
            sb_add(out, "[");
            expr_to_c(out, e->as.index.index, st, sigs, env);
            sb_add(out, "]");
            return;

        case AST_FIELD_ACCESS:
            expr_to_c_prec(out, e->as.field_access.base, st, sigs, env, 10);
            sb_add(out, ".");
            sb_add(out, e->as.field_access.field ? e->as.field_access.field : "<?>");
            return;

        case AST_CALL: {
            const char* name = NULL;
            if(e->as.call.callee && e->as.call.callee->kind==AST_IDENT) name=e->as.call.callee->as.ident.name;
            sb_add(out, name ? name : "<?>");
            sb_add(out, "(");
            for(int i=0;i<e->as.call.args.count;i++){
                if(i) sb_add(out, ", ");
                expr_to_c(out, e->as.call.args.items[i], st, sigs, env);
            }
            sb_add(out, ")");
            return;
        }

        case AST_UNARY: {
            if(e->as.unary.op==TOK_NON){
                sb_add(out, "(!");
                expr_to_c(out, e->as.unary.expr, st, sigs, env);
                sb_add(out, ")");
            } else if(e->as.unary.op==TOK_MOINS){
                sb_add(out, "(-");
                expr_to_c(out, e->as.unary.expr, st, sigs, env);
                sb_add(out, ")");
            } else {
                sb_add(out, "(");
                expr_to_c(out, e->as.unary.expr, st, sigs, env);
                sb_add(out, ")");
            }
            return;
        }

        case AST_BINARY: {
            TokenType op = e->as.binary.op;
            int myp = op_prec(op);
            bool paren = (myp < parent_prec);

            if(paren) sb_add(out, "(");

            // puissance => pow(...)
            if(op==TOK_PUISSANCE){
                CType rt = typeof_expr(e, st, sigs, env);
                if(rt.kind==TY_INT){
                    sb_add(out, "(int)pow((double)");
                    expr_to_c(out, e->as.binary.lhs, st, sigs, env);
                    sb_add(out, ", (double)");
                    expr_to_c(out, e->as.binary.rhs, st, sigs, env);
                    sb_add(out, ")");
                } else {
                    sb_add(out, "pow((double)");
                    expr_to_c(out, e->as.binary.lhs, st, sigs, env);
                    sb_add(out, ", (double)");
                    expr_to_c(out, e->as.binary.rhs, st, sigs, env);
                    sb_add(out, ")");
                }
                if(paren) sb_add(out, ")");
                return;
            }

            expr_to_c_prec(out, e->as.binary.lhs, st, sigs, env, myp);
            sb_add(out, " ");

            switch(op){
                case TOK_PLUS: sb_add(out, "+"); break;
                case TOK_MOINS: sb_add(out, "-"); break;
                case TOK_FOIS: sb_add(out, "*"); break;
                case TOK_DIVISE: sb_add(out, "/"); break;
                case TOK_DIV_ENTIER: sb_add(out, "/"); break;
                case TOK_MODULO: sb_add(out, "%"); break;

                case TOK_EGAL: sb_add(out, "=="); break;
                case TOK_DIFFERENT: sb_add(out, "!="); break;
                case TOK_INFERIEUR: sb_add(out, "<"); break;
                case TOK_INFERIEUR_EGAL: sb_add(out, "<="); break;
                case TOK_SUPERIEUR: sb_add(out, ">"); break;
                case TOK_SUPERIEUR_EGAL: sb_add(out, ">="); break;

                case TOK_ET: sb_add(out, "&&"); break;
                case TOK_OU: sb_add(out, "||"); break;

                default: sb_add(out, "/*op?*/"); break;
            }

            sb_add(out, " ");
            expr_to_c_prec(out, e->as.binary.rhs, st, sigs, env, myp+1);

            if(paren) sb_add(out, ")");
            return;
        }

        default:
            sb_add(out, "0");
            return;
    }
}

// =========================
// Codegen context + statements
// =========================
typedef struct {
    FILE* out;
    int indent;
    StructTable st;
    SigTable sigs;
    Env* env;
    int tmp_id;
    bool need_math_pow;
    bool need_string_h;
} CG;

static void emitln(CG* cg, const char* fmt, ...) {
    emit_indent(cg->out, cg->indent);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(cg->out, fmt, ap);
    va_end(ap);
    fputc('\n', cg->out);
}

static void cg_push(CG* cg){ cg->env = env_push(cg->env); }
static void cg_pop(CG* cg){ cg->env = env_pop(cg->env); }

static void lvalue_to_c(SB* out, ASTNode* lv, StructTable* st, SigTable* sigs, Env* env){
    // même rendu que expr mais seulement lvalue
    expr_to_c(out, lv, st, sigs, env);
}

static CType typeof_lvalue(ASTNode* lv, StructTable* st, SigTable* sigs, Env* env){
    return typeof_expr(lv, st, sigs, env);
}

// ---------- Declarations ----------
static void cg_decl_var(CG* cg, ASTNode* d, bool is_global, bool as_param){
    // d: AST_DECL_VAR / AST_DECL_CONST / AST_DECL_ARRAY / AST_PARAM / AST_FIELD

    if(!d) return;

    // VAR
    if(d->kind==AST_DECL_VAR){
        const char* name = d->as.decl_var.name;
        ASTNode* tn = d->as.decl_var.type;

        bool is_array=false;
        int dims=0;
        ASTList* dim_exprs=NULL;
        ASTNode* elem_type = tn;

        if(tn && tn->kind==AST_TYPE_ARRAY){
            is_array=true;
            dims = tn->as.type_array.dims.count;
            dim_exprs = &tn->as.type_array.dims;
            elem_type = tn->as.type_array.elem_type;
        }

        env_add(cg->env, name, elem_type, is_array, dims, dim_exprs);

        // string => char name[STR_SIZE];
        CType t = type_from_type_node(elem_type);
        if(is_array){
            // tableau : type + dims
            emit_indent(cg->out, cg->indent);
            emit_c_type(cg->out, elem_type, false);
            fprintf(cg->out, " %s", name);

            // dims connus ?
            if(dim_exprs && dim_exprs->count>0){
                for(int i=0;i<dim_exprs->count;i++){
                    SB ex; sb_init(&ex);
                    expr_to_c(&ex, dim_exprs->items[i], &cg->st, &cg->sigs, cg->env);
                    fprintf(cg->out, "[%s]", ex.data?ex.data:"0");
                    sb_free(&ex);
                }
            } else {
                // dims inconnues -> pointeurs (fallback)
                // en global on ne peut pas écrire "int a[];" sans taille: on met un commentaire
                fprintf(cg->out, " /* TODO: tailles tableau */");
            }
            fprintf(cg->out, ";\n");
            return;
        }

        if(t.kind==TY_STRING){
            emitln(cg, "char %s[%d];", name, STR_SIZE);
            cg->need_string_h = true;
            return;
        }

        emit_indent(cg->out, cg->indent);
        emit_c_type(cg->out, elem_type, false);
        fprintf(cg->out, " %s;\n", name);
        return;
    }

    // CONST
    if(d->kind==AST_DECL_CONST){
        const char* name = d->as.decl_const.name;
        ASTNode* tn = d->as.decl_const.type;
        ASTNode* v  = d->as.decl_const.value;

        env_add(cg->env, name, tn, false, 0, NULL);

        CType t = type_from_type_node(tn);
        emit_indent(cg->out, cg->indent);
        fprintf(cg->out, "const ");
        if(t.kind==TY_STRING){
            fprintf(cg->out, "char* %s = ", name);
            cg->need_string_h = true;
        } else {
            emit_c_type(cg->out, tn, false);
            fprintf(cg->out, " %s = ", name);
        }

        SB ex; sb_init(&ex);
        expr_to_c(&ex, v, &cg->st, &cg->sigs, cg->env);
        fprintf(cg->out, "%s;\n", ex.data?ex.data:"0");
        sb_free(&ex);
        return;
    }

    // DECL_ARRAY (forme dédiée)
    if(d->kind==AST_DECL_ARRAY){
        const char* name = d->as.decl_array.name;
        ASTNode* elem = d->as.decl_array.elem_type;
        ASTList* dims = &d->as.decl_array.dims;

        env_add(cg->env, name, elem, true, dims?dims->count:0, dims);

        emit_indent(cg->out, cg->indent);
        emit_c_type(cg->out, elem, false);
        fprintf(cg->out, " %s", name);

        if(dims && dims->count>0){
            for(int i=0;i<dims->count;i++){
                SB ex; sb_init(&ex);
                expr_to_c(&ex, dims->items[i], &cg->st, &cg->sigs, cg->env);
                fprintf(cg->out, "[%s]", ex.data?ex.data:"0");
                sb_free(&ex);
            }
        } else {
            fprintf(cg->out, " /* TODO: tailles tableau */");
        }
        fprintf(cg->out, ";\n");
        return;
    }

    // PARAM
    if(d->kind==AST_PARAM){
        // géré dans signature (pas ici)
        (void)is_global; (void)as_param;
        return;
    }

    // FIELD
    if(d->kind==AST_FIELD){
        // géré dans struct
        (void)is_global; (void)as_param;
        return;
    }
}

// ---------- Statements forward ----------
static void cg_stmt(CG* cg, ASTNode* st);

static void cg_block(CG* cg, ASTNode* block){
    if(!block || block->kind!=AST_BLOCK) return;

    emitln(cg, "{");
    cg->indent += 4;
    cg_push(cg);

    // on génère dans l’ordre (C11 accepte déclarations au milieu)
    for(int i=0;i<block->as.block.stmts.count;i++){
        ASTNode* s = block->as.block.stmts.items[i];
        if(!s) continue;

        if(s->kind==AST_DECL_VAR || s->kind==AST_DECL_CONST || s->kind==AST_DECL_ARRAY){
            cg_decl_var(cg, s, false, false);
        } else {
            cg_stmt(cg, s);
        }
    }

    cg_pop(cg);
    cg->indent -= 4;
    emitln(cg, "}");
}

static void cg_stmt_assign(CG* cg, ASTNode* st){
    SB lhs; sb_init(&lhs);
    SB rhs; sb_init(&rhs);

    lvalue_to_c(&lhs, st->as.assign.target, &cg->st, &cg->sigs, cg->env);
    expr_to_c(&rhs, st->as.assign.value, &cg->st, &cg->sigs, cg->env);

    CType lt = typeof_lvalue(st->as.assign.target, &cg->st, &cg->sigs, cg->env);

    if(lt.kind==TY_STRING){
        // strncpy(lhs, rhs, STR_SIZE-1); lhs[STR_SIZE-1]=0;
        emitln(cg, "strncpy(%s, %s, %d-1);", lhs.data?lhs.data:"", rhs.data?rhs.data:"", STR_SIZE);
        emitln(cg, "%s[%d-1] = '\\0';", lhs.data?lhs.data:"", STR_SIZE);
        cg->need_string_h = true;
    } else {
        emitln(cg, "%s = %s;", lhs.data?lhs.data:"", rhs.data?rhs.data:"0");
    }

    sb_free(&lhs); sb_free(&rhs);
}

static void cg_stmt_write(CG* cg, ASTNode* st){
    for(int i=0;i<st->as.write_stmt.args.count;i++){
        ASTNode* a = st->as.write_stmt.args.items[i];
        CType t = typeof_expr(a, &cg->st, &cg->sigs, cg->env);

        SB ex; sb_init(&ex);
        expr_to_c(&ex, a, &cg->st, &cg->sigs, cg->env);

        switch(t.kind){
            case TY_INT:    emitln(cg, "printf(\"%%d\", %s);", ex.data?ex.data:"0"); break;
            case TY_REAL:   emitln(cg, "printf(\"%%g\", %s);", ex.data?ex.data:"0"); break;
            case TY_BOOL:   emitln(cg, "printf(\"%%d\", (%s) ? 1 : 0);", ex.data?ex.data:"0"); break;
            case TY_CHAR:   emitln(cg, "printf(\"%%c\", %s);", ex.data?ex.data:"'?'"); break;
            case TY_STRING: emitln(cg, "printf(\"%%s\", %s);", ex.data?ex.data:"\"\""); break;
            default:        emitln(cg, "printf(\"%%d\", %s);", ex.data?ex.data:"0"); break;
        }

        sb_free(&ex);
    }
}

static void cg_stmt_read(CG* cg, ASTNode* st){
    for(int i=0;i<st->as.read_stmt.targets.count;i++){
        ASTNode* lv = st->as.read_stmt.targets.items[i];
        CType t = typeof_lvalue(lv, &cg->st, &cg->sigs, cg->env);

        SB lvs; sb_init(&lvs);
        lvalue_to_c(&lvs, lv, &cg->st, &cg->sigs, cg->env);

        if(t.kind==TY_STRING){
            // scanf(" %1023s", lv);
            emitln(cg, "scanf(\" %%%ds\", %s);", STR_SIZE-1, lvs.data?lvs.data:"");
        } else if(t.kind==TY_CHAR){
            emitln(cg, "scanf(\" %%c\", &%s);", lvs.data?lvs.data:"");
        } else if(t.kind==TY_REAL){
            emitln(cg, "scanf(\" %%lf\", &%s);", lvs.data?lvs.data:"");
        } else {
            // int/bool/unknown
            emitln(cg, "scanf(\" %%d\", &%s);", lvs.data?lvs.data:"");
        }

        sb_free(&lvs);
    }
}

static void cg_stmt_return(CG* cg, ASTNode* st){
    if(st->as.ret_stmt.value){
        SB ex; sb_init(&ex);
        expr_to_c(&ex, st->as.ret_stmt.value, &cg->st, &cg->sigs, cg->env);
        emitln(cg, "return %s;", ex.data?ex.data:"0");
        sb_free(&ex);
    } else {
        emitln(cg, "return;");
    }
}

static void cg_stmt_call(CG* cg, ASTNode* st){
    ASTNode* call = st->as.call_stmt.call;
    if(!call || call->kind!=AST_CALL) return;
    SB ex; sb_init(&ex);
    expr_to_c(&ex, call, &cg->st, &cg->sigs, cg->env);
    emitln(cg, "%s;", ex.data?ex.data:"");
    sb_free(&ex);
}

static void cg_stmt_if(CG* cg, ASTNode* st){
    SB c; sb_init(&c);
    expr_to_c(&c, st->as.if_stmt.cond, &cg->st, &cg->sigs, cg->env);
    emitln(cg, "if (%s)", c.data?c.data:"0");
    sb_free(&c);

    cg_block(cg, st->as.if_stmt.then_block);

    for(int i=0;i<st->as.if_stmt.elif_conds.count;i++){
        SB ec; sb_init(&ec);
        expr_to_c(&ec, st->as.if_stmt.elif_conds.items[i], &cg->st, &cg->sigs, cg->env);
        emitln(cg, "else if (%s)", ec.data?ec.data:"0");
        sb_free(&ec);
        cg_block(cg, st->as.if_stmt.elif_blocks.items[i]);
    }

    if(st->as.if_stmt.else_block){
        emitln(cg, "else");
        cg_block(cg, st->as.if_stmt.else_block);
    }
}

static void cg_stmt_while(CG* cg, ASTNode* st){
    SB c; sb_init(&c);
    expr_to_c(&c, st->as.while_stmt.cond, &cg->st, &cg->sigs, cg->env);
    emitln(cg, "while (%s)", c.data?c.data:"0");
    sb_free(&c);
    cg_block(cg, st->as.while_stmt.body);
}

static void cg_stmt_repeat(CG* cg, ASTNode* st){
    emitln(cg, "do");
    cg_block(cg, st->as.repeat_stmt.body);
    if(st->as.repeat_stmt.until_cond){
        SB c; sb_init(&c);
        expr_to_c(&c, st->as.repeat_stmt.until_cond, &cg->st, &cg->sigs, cg->env);
        emitln(cg, "while (!(%s));", c.data?c.data:"0");
        sb_free(&c);
    } else {
        emitln(cg, "while (0);");
    }
}

static void cg_stmt_for(CG* cg, ASTNode* st){
    const char* var = st->as.for_stmt.var;

    // on force un step (si NULL => 1)
    int id = ++cg->tmp_id;
    char step_name[64];
    snprintf(step_name, sizeof(step_name), "__step%d", id);

    SB start; sb_init(&start);
    SB end;   sb_init(&end);
    SB step;  sb_init(&step);

    expr_to_c(&start, st->as.for_stmt.start, &cg->st, &cg->sigs, cg->env);
    expr_to_c(&end,   st->as.for_stmt.end,   &cg->st, &cg->sigs, cg->env);
    if(st->as.for_stmt.step) expr_to_c(&step, st->as.for_stmt.step, &cg->st, &cg->sigs, cg->env);
    else sb_add(&step, "1");

    // bloc pour porter __step
    emitln(cg, "{");
    cg->indent += 4;

    emitln(cg, "int %s = (int)(%s);", step_name, step.data?step.data:"1");
    emitln(cg, "for (%s = (int)(%s); (%s >= 0) ? (%s <= (int)(%s)) : (%s >= (int)(%s)); %s += %s)",
           var, start.data?start.data:"0",
           step_name,
           var, end.data?end.data:"0",
           var, end.data?end.data:"0",
           var, step_name);
    cg_block(cg, st->as.for_stmt.body);

    cg->indent -= 4;
    emitln(cg, "}");

    sb_free(&start); sb_free(&end); sb_free(&step);
}

static void cg_stmt_switch(CG* cg, ASTNode* st){
    SB ex; sb_init(&ex);
    expr_to_c(&ex, st->as.switch_stmt.expr, &cg->st, &cg->sigs, cg->env);

    emitln(cg, "switch (%s)", ex.data?ex.data:"0");
    sb_free(&ex);

    emitln(cg, "{");
    cg->indent += 4;

    for(int i=0;i<st->as.switch_stmt.cases.count;i++){
        ASTNode* c = st->as.switch_stmt.cases.items[i];
        if(!c || c->kind!=AST_CASE) continue;

        // plusieurs valeurs => plusieurs "case"
        for(int v=0; v<c->as.case_stmt.values.count; v++){
            SB vv; sb_init(&vv);
            expr_to_c(&vv, c->as.case_stmt.values.items[v], &cg->st, &cg->sigs, cg->env);
            emitln(cg, "case %s:", vv.data?vv.data:"0");
            sb_free(&vv);
        }

        // corps
        cg->indent += 4;
        if(c->as.case_stmt.body) {
            // on génère les statements du block sans refaire un switch-block énorme
            // on garde un vrai block C pour la portée
            cg_block(cg, c->as.case_stmt.body);
        }
        emitln(cg, "break;");
        cg->indent -= 4;
    }

    if(st->as.switch_stmt.default_block){
        emitln(cg, "default:");
        cg->indent += 4;
        cg_block(cg, st->as.switch_stmt.default_block);
        emitln(cg, "break;");
        cg->indent -= 4;
    }

    cg->indent -= 4;
    emitln(cg, "}");
}

static void cg_stmt(CG* cg, ASTNode* st){
    if(!st) return;

    switch(st->kind){
        case AST_ASSIGN: cg_stmt_assign(cg, st); return;
        case AST_WRITE:  cg_stmt_write(cg, st);  return;
        case AST_READ:   cg_stmt_read(cg, st);   return;
        case AST_RETURN: cg_stmt_return(cg, st); return;
        case AST_CALL_STMT: cg_stmt_call(cg, st); return;

        case AST_IF:     cg_stmt_if(cg, st);     return;
        case AST_WHILE:  cg_stmt_while(cg, st);  return;
        case AST_FOR:    cg_stmt_for(cg, st);    return;
        case AST_REPEAT: cg_stmt_repeat(cg, st); return;

        case AST_SWITCH: cg_stmt_switch(cg, st); return;

        case AST_BREAK:
        case AST_QUIT_FOR:
            emitln(cg, "break;");
            return;

        default:
            // ignore decls inside stmt here (gérés dans cg_block)
            return;
    }
}

// =========================
// Definitions: struct / func / proc
// =========================
static void cg_emit_struct(CG* cg, ASTNode* def){
    const char* name = def->as.def_struct.name ? def->as.def_struct.name : "Unnamed";

    emitln(cg, "typedef struct %s {", name);
    cg->indent += 4;

    for(int i=0;i<def->as.def_struct.fields.count;i++){
        ASTNode* fld = def->as.def_struct.fields.items[i];
        if(!fld || fld->kind!=AST_FIELD) continue;

        ASTNode* tn = fld->as.field.type;
        const char* fn = fld->as.field.name;

        // string field => char fn[STR_SIZE];
        CType t = type_from_type_node(tn);
        if(t.kind==TY_STRING){
            emitln(cg, "char %s[%d];", fn, STR_SIZE);
            cg->need_string_h = true;
        } else if(t.kind==TY_ARRAY){
            // champs array : rare, fallback simple (pas complet)
            emit_indent(cg->out, cg->indent);
            emit_c_type(cg->out, tn->as.type_array.elem_type, true);
            fprintf(cg->out, " %s", fn);
            int dims = tn->as.type_array.dims.count;
            if(dims>0){
                for(int d=0; d<dims; d++){
                    // tailles inconnues côté struct -> interdit en C standard (VLA interdit en struct)
                    fprintf(cg->out, "/*[?]*/");
                }
            }
            fprintf(cg->out, ";\n");
        } else {
            emit_indent(cg->out, cg->indent);
            emit_c_type(cg->out, tn, true);
            fprintf(cg->out, " %s;\n", fn);
        }
    }

    cg->indent -= 4;
    emitln(cg, "} %s;", name);
    emitln(cg, "");
}

static void cg_emit_params(CG* cg, ASTList* params){
    for(int i=0;i<params->count;i++){
        ASTNode* p = params->items[i];
        if(!p || p->kind!=AST_PARAM) continue;
        const char* pn = p->as.param.name;
        ASTNode* tn = p->as.param.type;

        // si param est tableau => on passe pointeur (fallback)
        if(tn && tn->kind==AST_TYPE_ARRAY){
            ASTNode* elem = tn->as.type_array.elem_type;
            CType bt = type_from_type_node(elem);

            if(bt.kind==TY_STRING){
                fprintf(cg->out, "char* %s", pn);
                cg->need_string_h = true;
            } else {
                emit_c_type(cg->out, elem, false);
                fprintf(cg->out, "* %s", pn);
            }

            env_add(cg->env, pn, elem, true, tn->as.type_array.dims.count, &tn->as.type_array.dims);
        } else {
            CType t = type_from_type_node(tn);
            if(t.kind==TY_STRING){
                fprintf(cg->out, "char %s[%d]", pn, STR_SIZE);
                cg->need_string_h = true;
            } else {
                emit_c_type(cg->out, tn, false);
                fprintf(cg->out, " %s", pn);
            }
            env_add(cg->env, pn, tn, false, 0, NULL);
        }

        if(i+1<params->count) fprintf(cg->out, ", ");
    }
}

static void cg_emit_func(CG* cg, ASTNode* def){
    const char* name = def->as.def_func.name ? def->as.def_func.name : "Func";

    // return type
    CType rt = type_from_type_node(def->as.def_func.return_type);
    emit_indent(cg->out, cg->indent);
    if(rt.kind==TY_STRING){
        // très dur de retourner string "by value" proprement -> on retourne char*
        fprintf(cg->out, "char* %s(", name);
        cg->need_string_h = true;
    } else {
        emit_c_type(cg->out, def->as.def_func.return_type, false);
        fprintf(cg->out, " %s(", name);
    }

    cg_push(cg);
    cg_emit_params(cg, &def->as.def_func.params);
    cg_pop(cg);

    fprintf(cg->out, ")\n");

    cg_push(cg);
    cg_block(cg, def->as.def_func.body);
    cg_pop(cg);

    emitln(cg, "");
}

static void cg_emit_proc(CG* cg, ASTNode* def){
    const char* name = def->as.def_proc.name ? def->as.def_proc.name : "Proc";

    emit_indent(cg->out, cg->indent);
    fprintf(cg->out, "void %s(", name);

    cg_push(cg);
    cg_emit_params(cg, &def->as.def_proc.params);
    cg_pop(cg);

    fprintf(cg->out, ")\n");

    cg_push(cg);
    cg_block(cg, def->as.def_proc.body);
    cg_pop(cg);

    emitln(cg, "");
}

// =========================
// Public API
// =========================
bool ast2c_generate_stream(ASTNode* program, FILE* out){
    if(!program || program->kind!=AST_PROGRAM || !out) return false;

    CG cg;
    memset(&cg,0,sizeof(cg));
    cg.out = out;
    cg.indent = 0;
    cg.env = env_push(NULL);
    st_init(&cg.st);
    sig_init(&cg.sigs);

    st_build_from_program(&cg.st, program);
    sig_build_from_program(&cg.sigs, program);

    // Première passe : détecter pow potentiel (TOK_PUISSANCE)
    // (simple : on regarde defs + main)
    // => ici on active pow dès qu'on rencontre TOK_PUISSANCE en expr_to_c (on n'a pas de hook),
    // donc on inclut math.h par défaut : plus simple.
    cg.need_math_pow = true;

    // includes (on met tout ce qui est nécessaire)
    fputs("#include <stdio.h>\n", out);
    fputs("#include <stdlib.h>\n", out);
    fputs("#include <stdbool.h>\n", out);
    fputs("#include <string.h>\n", out);
    fputs("#include <math.h>\n\n", out);

    fputs("#ifndef STR_SIZE\n#define STR_SIZE 1024\n#endif\n\n", out);

    // structs d'abord
    for(int i=0;i<program->as.program.defs.count;i++){
        ASTNode* d = program->as.program.defs.items[i];
        if(d && d->kind==AST_DEF_STRUCT){
            cg_emit_struct(&cg, d);
        }
    }

    // globals
    emitln(&cg, "/* ===== GLOBALS ===== */");
    for(int i=0;i<program->as.program.decls.count;i++){
        ASTNode* d = program->as.program.decls.items[i];
        if(!d) continue;
        cg_decl_var(&cg, d, true, false);
    }
    emitln(&cg, "");

    // defs func/proc
    for(int i=0;i<program->as.program.defs.count;i++){
        ASTNode* d = program->as.program.defs.items[i];
        if(!d) continue;
        if(d->kind==AST_DEF_FUNC) cg_emit_func(&cg, d);
        else if(d->kind==AST_DEF_PROC) cg_emit_proc(&cg, d);
    }

    // main
    emitln(&cg, "int main(void)");
    cg_push(&cg);
    cg_block(&cg, program->as.program.main_block);
    cg_pop(&cg);
    emitln(&cg, "");

    // cleanup
    while(cg.env) cg.env = env_pop(cg.env);
    st_free(&cg.st);
    sig_free(&cg.sigs);

    return true;
}

bool ast2c_generate_file(ASTNode* program, const char* out_c_path){
    if(!out_c_path) return false;
    FILE* f = fopen(out_c_path, "wb");
    if(!f) return false;
    bool ok = ast2c_generate_stream(program, f);
    fclose(f);
    return ok;
}