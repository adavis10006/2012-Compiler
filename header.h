/* need to define */
#define MAXLEN 1024
#define REG_COUNT 32
#define MAX_REG 26

/* Constant Types */
typedef enum {INTEGERC,FLOATC,STRINGC} C_type;

typedef enum {ZERO_,INT_,FLOAT_,ARR_,STR_,VOID_,ERROR_,CONST_,FUNC_,POINTER_,STR_VAR_,STRING_,TYPEDEF_} TYPE;

typedef enum{INTNUMBER,STRING,TYPE_STRING,TYPELIST,STRINGLIST,ARRAY_SEM,STRUCT_SEM,ID_LIST,VAR_DECL,INIT_ID,DEF_LIST,VAR_REF,PARAM,PARAM_LIST,ST_ARR,ST_FUNC,ST_STRUCT,ST_BASIC,SYMTAB,TYPE_ARR,CONST_REC} ALL_TYPE;

typedef struct {
    C_type  const_type;
    union {
        int     intval;
        double  fval;
        char    *sc; }
    const_u;
} CON_Type;

typedef struct MyArrInfo {
    int dim;
    int dim_limit[10];
} ArrayInfo;

typedef struct {
    char * name;
}String;

typedef struct{
    TYPE type;
    char * name;
}Type_String;


struct Stringlist{
    char * name;
    struct Stringlist *next;
};
typedef struct Stringlist Stringlist;

typedef struct{
    int dim;
    int dim_limit[10];
    int dim_place[10];
    int size;
    TYPE arrtype;
    char * type_name;   /*in case of array of structs*/
    /*or in case of array of array through typedef*/
}Type_arr;

typedef struct{
    Type_arr * arr_info;
    char *name; /*name of array in symbol table, filled in init_id or id_list*/
}array_semantic;


struct struct_semantic{
    TYPE type;
    union{
        char * var_name;            /*name of the variable of type type. */
        array_semantic * arr_sem;   /*info if the element is an array*/
        struct{
            char *str_var_name;     /*name of variable in case of nested struct*/
            char * struct_type_name;/*if TYPE is STR_R, this is the name of
                                      type of struct in symbol table
                                      this name is a temporary name since structs
                                      do not have tags, but if they do have tags,
                                      it is handled in the same way*/
        }str_info;
    }struct_semantic_u;
    struct struct_semantic *next;
};
typedef struct struct_semantic struct_semantic;

typedef struct{
    TYPE type;
    char *name;
    int place;
    int is_return;
    int is_array;
    int is_tail;
    int label_num;
    union{
        char *type_name;
        Type_arr *arr_info;
    }var_ref_u;
    union
    {
        int const_int;
        float const_float;
        char *const_char;
    }const_value;
}var_ref;

struct TypeList{
    var_ref *P_var_r;
    struct TypeList *next;
};
typedef struct TypeList TypeList;



typedef struct{
    TYPE type;  /*the type tells only if an array or not*/
    /*we cannot have struct types here.*/
    int offset;
    int initial_type;
    int place;
    union{
        char * name;
        array_semantic * P_arr_s;
    }init_id_u;
    union
    {
        int value_int;
        float value_float;
    } value;
}init_id;

/*explanations above in id_list structure*/

struct id_list{
    init_id *P_ini_i;
    struct id_list * next;
};

typedef struct id_list id_list;

typedef struct{
    TYPE type;
    char *type_name;/*if type is STR_, this is the name of struct type in ST*/
    /*if a typedef of struct, name of ST entry
      if a typedef of array, name of ST entry*/


    id_list * P_id_l;
    int linnum; /*line number of declaration for
                  error messages;*/
}var_decl;



struct def_list{
    var_decl* P_var_s;
    struct def_list * next;
};
typedef struct def_list def_list;


typedef struct{
    TYPE type;      //type, could be basic type, array or error
    TYPE arrtype;   //if type is ARR_, this the type of array
    int dim;        //if array number of dimensions
    int dim_limit[10];
    char *name;
    int offset;
}param;

struct param_list{
    param* PPAR;
    struct param_list *next;
};
typedef struct param_list param_list;


/*Symbol table entries for basic types, arrays, functions, and structs*/

typedef Type_arr ST_arr;

typedef struct{
    TYPE ret_type;
    int params;
    param_list * PL;
}ST_func;

typedef struct_semantic ST_struct;

typedef enum {TYPEDEF_INT,TYPEDEF_FLT,TYPEDEF_ARR,TYPEDEF_STR} IS_TYPE_DEF;

struct symtab{
    char *lexeme;
    TYPE type;
    IS_TYPE_DEF type_when_def;
    union{
        ST_arr *st_arr;     /*for arrays not nested inside a struct*/
        ST_func *st_func;   /*for function declaration*/
        ST_struct *st_struct;   /*for structure declaration*/
        char * type_name;   /*for structure variables*/
        /*also name of struct type in case of
          typedef structs*/
    }symtab_u;
    struct symtab *front;
    struct symtab *back;
    int scope;
    int line;
    int offset;
    int place;
};
typedef struct symtab symtab;

/* float data */
struct string_list
{
    char string[ MAXLEN ];
    struct string_list *next;
} frame_data;

symtab * lookup(char *name);
void insert(char *name,TYPE type,void * P,IS_TYPE_DEF TypeDef);
int delete_scope(int scp);
void * Allocate(ALL_TYPE);
char *printtype(int x);
char *ArrayNewName(char *a);
var_ref *relop_extm(var_ref * a, int b, var_ref *c);
var_ref* assign_ex(char * a, var_ref * b);
TYPE check_relop_ex_lst(TypeList *a);
id_list* Create_Id_List(char *x);
id_list* Merge_Id_List(id_list * y,char *x);
id_list* Id_List_Array(id_list *a,char *b,ArrayInfo *c);
def_list *def_list_P(def_list *a,var_decl *b);
int struct_type_P(char *a,def_list* b);
char *get_temp_name();
char * new_name(char *a);
int if_user_name(char *x);
TYPE type_decl_enter_ST1(int a,id_list *b);
TYPE type_decl_enter_ST2(char *a,id_list* b);
TYPE decl_enter_ST(var_decl *a);
TYPE stmt_assign_ex(var_ref *a,var_ref *b);
char * Add_Str(char *a,char *b);
struct_semantic * search(char *a,char *b);
var_ref* check_function(char * a, TypeList *b);
TYPE func_enter_ST(TYPE a,char *b,param_list *c);
param_list *MakeParamList(param_list *a,param *b);
TYPE param_P(param *a,char *b);
void put_read_ST();
TYPE check_return(int flag,TYPE type);

/* function.c */
int get_offset( char *name );
int set_offset( var_decl *value, int offset );
void set_global( var_decl *value );
void set_float_data ( int number, float value );
void set_string_data( int number, char* value );

void start_dim();
void count_dim( var_ref* v_ptr, var_ref* value );
void finish_dim();
param_list* reverse_param_list( param_list *p );
TypeList* reverse_TypeList( TypeList *t );

void stack_push(int value );
int stack_pop();
int stack_top( int value );
void print_stack();

//void set_array_data( int number, char* value );

/*  registers.c */
int check_reg( const char *name );
int get_reg( var_ref *v_ptr );
int get_result_reg();
void free_reg( int reg );
void notsave_reg( int reg );
void save_reg( int reg );
void reset_reg();
void float_clean_reg( var_ref* v_ptr );
int float_check_reg( const char *name );
int float_get_reg( var_ref *v_ptr );
int float_get_result_reg();
void float_free_reg( int reg );
void float_notsave_reg( int reg );
void float_save_reg( int reg );
void float_reset_reg();
void float_clean_reg( var_ref* v_ptr );
void push_reg();
void pop_reg();
void print_regs();

/* asm_generate.c */
void gen_head( const char *name );
void gen_prologue( const char *name );
void gen_epilogue( const char *name );

int gen_relop_factor(var_ref *a, var_ref *b, int op);
int gen_expr( var_ref *a, var_ref *b, int op );
int gen_term (var_ref *a, var_ref *b, int op );
void gen_function( const char *name );
void gen_return( var_ref *value );
void gen_result( var_ref *value );

void gen_store( int reg, int offset );
void gen_store_const_float( int reg, const char *name );
void gen_store_float( int reg, int offset );
void gen_store_global( int reg, const char *name );

void gen_load( int reg, int offset );
int gen_load_int( int reg, var_ref *value );
int gen_load_float( int reg, var_ref *value );
void gen_load_const( int reg, int value );
void gen_load_const_float( int reg, int value );
void gen_load_reg( int reg_a, int reg_b );
void gen_load_global( int reg, const char *name );
int gen_load_array( var_ref *v_ptr, int width );

void gen_control_while( int label_num );
void gen_control_endlabel( int label_num );
void gen_control_jump_else( int label_num );
void gen_control_else( int label_num );
void gen_control_start( int label_num );
void gen_control_test( var_ref *value, int label_num );

int gen_or_tail( int reg, int label_num );
int gen_and_tail( int reg,int label_num );

void gen_frame_data();

void gen_write();
void gen_read();
void gen_read_float();

void gen_pop_params( TypeList *b );
