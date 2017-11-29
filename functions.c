#include "header.h"
#include "parser.tab.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
extern int scope;
extern int STRUCT_DEC;
extern int linenumber;
extern int GLOBAL_ERROR;

extern int linenumber;

extern char function_name[ MAXLEN ];


extern int float_number, string_number;

int LS[ 1000 ];

int stack_now = 0;

char *printarray[]={"int","float","array","struct","function","typedef","void","error type"};

char *printtype(int x){
    switch (x){
    case INT_:
        return printarray[0];
    case FLOAT_:
        return printarray[1];
    case ARR_:
        return printarray[2];
    case STR_:
    case STR_VAR_:
        return printarray[3];
    case FUNC_:
        return printarray[4];
    case TYPEDEF_:
        return printarray[5];
    case VOID_:
        return printarray[6];
    default:
        return printarray[7];
    }
}

var_ref *relop_extm(var_ref * a, int b, var_ref *c){
    if(a->type==ERROR_||c->type==ERROR_)
        a->type=ERROR_;
    else if(a->type!=INT_||c->type!=INT_){
        if((a->type==INT_||a->type==FLOAT_)&&(c->type==INT_||c->type==FLOAT_)){
            //printf("warning %d: operator %s applied to non integer type\n",linenumber,b==OP_OR?"||":"&&");
            a->type=INT_;
        }
        else{
            printf("error %d: operator %s applied to non basic type\n",linenumber,b==OP_OR?"||":"&&");
            a->type=ERROR_;
        }
    }
    else
        a->type=INT_;
    free(c);
    return a;
}

var_ref* assign_ex(char * a, var_ref * b){
    symtab *PST;

    if((PST=lookup(a))==NULL){
        printf("error %d: undeclared variable %s\n",linenumber,a);
        b->type=ERROR_;
        GLOBAL_ERROR=1;
        return b;
    }
    if((PST->type==ERROR_)||(b->type==ERROR_)){
        b->type=ERROR_;
        return b;
    }
    if(PST->type!=b->type){
        if((PST->type==INT_||PST->type==FLOAT_)&&(b->type==INT_||b->type==FLOAT_))
            b->type=((PST->type==FLOAT_)||(b->type==FLOAT_))?FLOAT_:INT_;
        else{
            printf("error %d: incompatible assignment, %s %sto %s %s\n",linenumber,printtype(b->type),(b->name)?b->name:"",printtype(PST->type),PST->lexeme);
            b->type=ERROR_;
        }
    }
    else
        switch(PST->type){
            case STR_VAR_:
                if(PST->symtab_u.type_name!=b->var_ref_u.type_name){
                    printf("error %d: incompatible assignment, struct %s to struct %s of different types\n",linenumber,(b->name)?(b->name):"",a);
                    b->type=ERROR_;
                }
                break;
            case ARR_:
                printf("error %d: incompatible assignment array %s to array %s\n",linenumber,(b->name)?(b->name):"",a);
                b->type=ERROR_;
                break;
            case INT_:
                if ( b->type == INT_ )
                {
                    int reg = get_reg( b );
                    symtab *ptr_b = lookup ( b->name );

                    if ( PST->scope != 0 )
                    {
                        reg = gen_load_int (reg, b);
                        PST->place = reg;
                        gen_store( reg, PST->offset );
                        reset_reg();
                        float_reset_reg();
                    }
                    else
                    {
                        if ( b->name != NULL )
                        {
                            if ( ptr_b->scope != 0)
                                gen_load( reg, ptr_b->offset );
                            else
                                gen_load_global( reg, b->name );
                        }
                        else if ( b->place == 0 )
                            gen_load_const( reg, b->const_value.const_int );
                        PST->place = reg;
                        gen_store_global( reg, PST->lexeme );
                        reset_reg();
                        float_reset_reg();
                    }
                }
                else
                {
                    b->type = INT_;
                    b->const_value.const_int = b->const_value.const_float;
                }
            case FLOAT_:
                break;
            default:
                printf("CHECKER ERROR: unknown types, line %d\n",linenumber);
                break;
        }
    return b;
}

TYPE stmt_assign_ex(var_ref *a,var_ref *b){
    symtab *ptr_a = NULL;
    symtab *ptr_b = NULL;
    int reg;

    if((a->type==ERROR_)||(b->type==ERROR_))
        return ERROR_;

    if(a->type!=b->type && !(a->type==INT_||a->type==FLOAT_)&& !(b->type==INT_||b->type==FLOAT_) ){
        if((a->type==INT_||a->type==FLOAT_)&&(b->type==INT_||b->type==FLOAT_)){
            free(a);
            free(b);
            return ZERO_;
        }
        else{
            printf("error %d: incompatible assignment, %s %s to %s %s\n",linenumber,printtype(b->type),(b->name)?(b->name):"",printtype(a->type),(a->name)?a->name:"");
            free(a);
            free(b);
            return ERROR_;
        }
    }
    else
        switch(a->type){
            case STR_VAR_:
                if(a->var_ref_u.type_name!=b->var_ref_u.type_name){
                    printf("error %d: incomaptible assignment, struct %s to struct %s of different types\n",linenumber,(b->name)?(b->name):"",a->name);
                    free(a);
                    free(b);
                    return ERROR_;
                }
                break;
            case ARR_:
                printf("error %d: incompatible assignment array %s to array %s\n",linenumber,(b->name)?(b->name):"",a->name);
                free(a);
                free(b);
                return ERROR_;
                break;
            case INT_:
                ptr_a = lookup( a->name );

                if ( b->name != NULL )
                    ptr_b = lookup( b->name );

                if ( ptr_a->scope != 0 )
                {
                    if ( b->is_array != 1 )
                    {
                        reg = get_reg( b );
                        reg = gen_load_int( reg, b );
                    }
                    else
                    {
                        reg = gen_load_array( b, 4 );
                        printf( "\tlw\t$%d, 0($%d)\n", reg, reg);
                    }

                    if ( ptr_a->type != ARR_ )
                    {
                        ptr_a->place = reg;
                        gen_store( reg, ptr_a->offset );
                        reset_reg();
                        float_reset_reg();
                    }
                    else
                    {
                        printf( "\tsw\t$%d, 0($%d)\n", reg, gen_load_array( a, 4 ));
                        reset_reg();
                        float_reset_reg();
                    }
                }
                else
                {
                    reg = get_reg( b );
                    if ( b->name != NULL )
                        if ( b->is_array != 1 )
                        {
                            if ( ptr_b->scope != 0 )
                                gen_load( reg, ptr_b->offset );
                            else
                                gen_load_global( reg, b->name );
                        }
                        else
                            gen_load_array( b, 4 );

                    else if ( b->place >= 8 && b->place < MAX_REG )
                        reg = b->place;
                    else if ( b->place < 8 || b->place > MAX_REG )
                        gen_load_const( reg, b->const_value.const_int );

                    if ( ptr_a->type != ARR_ )
                    {
                        ptr_a->place = -1;
                        gen_store_global( reg, a->name );
                        reset_reg();
                        float_reset_reg();
                    }
                    else
                    {
                        printf( "\tsw\t$%d, 0($%d)\n", reg, gen_load_array( a, 4 ));
                        reset_reg();
                        float_reset_reg();
                    }
                }
                break;
            case FLOAT_:
                ptr_a = lookup( a->name );

                if ( b->name != NULL )
                    ptr_b = lookup ( b->name );

                if ( ptr_a->scope != 0 )
                {
                    if ( b->name != NULL )
                    {
                        if ( b->is_array != 1 )
                        {

                            if( ptr_b->offset < 0 )
                            {
                                reg = float_get_reg( b );

                                gen_load_const_float( reg, float_number );
                                set_float_data( float_number++, b->const_value.const_float );
                            }
                        }
                        else
                        {
                            reg = gen_load_array( b, 4 );
                            printf( "\tl.s\t$f%d, 0($%d)\n", reg, reg);
                        }
                    }
                    else if ( b->place >= 0 && b->place < REG_COUNT )
                    {
                        if ( b->const_value.const_float != 0.0 && b->place == 0 )
                        {
                            reg = float_get_reg( b );

                            gen_load_const_float( reg, float_number );
                            set_float_data( float_number++, b->const_value.const_float );
                        }
                        else
                            reg = b->place;
                    }
                    if ( ptr_a->type != ARR_ )
                    {
                        ptr_a->place = -1;
                        gen_store_float( reg, ptr_a->offset );
                        reset_reg();
                        float_reset_reg();
                    }
                    else
                    {
                        printf( "\ts.s\t$f%d, 0($%d)\n", reg, gen_load_array( a, 4 ));
                        reset_reg();
                        float_reset_reg();
                    }
                }
                else
                {
                    if ( b->name != NULL )
                    {
                        if (b->is_array != 1)
                        {
                            reg = float_get_result_reg();
                            gen_load_const_float( reg, float_number );
                            set_float_data( float_number++, b->const_value.const_float );
                        }
                        else
                            reg = gen_load_array( b, 4 );
                    }
                    else if ( b->is_return == 1)
                    {
                        reg = float_get_result_reg();
                        gen_load_reg( reg, b->place );
                    }
                    else if ( b->place == 0 )
                        if ( b->const_value.const_float != 0.0 )
                        {
                            reg = float_get_reg( b );

                            gen_load_const_float( reg, float_number );
                            set_float_data( float_number++, b->const_value.const_float );
                        }
                        else
                            reg = b->place;

                    if ( ptr_a->type != ARR_ )
                    {
                        ptr_a->place = -1;
                        gen_store_const_float( reg, a->name );
                        reset_reg();
                        float_reset_reg();
                    }
                    else
                    {
                        printf( "\ts.s\t$f%d, 0($%d)\n", reg, gen_load_array( a, 4 ));
                        reset_reg();
                        float_reset_reg();
                    }
                }
                free(a);
                free(b);
                return ZERO_;
                break;

            default:
                printf("CHECKER ERROR: unknown types, line %d\n",linenumber);
                free(a);
                free(b);
                return ERROR_;
                break;
        }
}

TYPE check_relop_ex_lst(TypeList *a){
    TypeList* b=a;
    if(a==NULL)
        return ZERO_;
    do{
        if(a->P_var_r->type==ERROR_)
            return ERROR_;
    }while(a=a->next);
    free(b);
    return ZERO_;
}


id_list* Create_Id_List (char *x){
    init_id* PII;
    id_list *RET;
    PII=Allocate(INIT_ID);
    PII->init_id_u.name=x;
    PII->type=ZERO_;
    RET=Allocate(ID_LIST);
    RET->P_ini_i=PII;
    RET->next=NULL;
    return RET;
}

id_list* Merge_Id_List (id_list * y,char *x){
    init_id* PII;
    id_list *RET;
    PII=Allocate(INIT_ID);
    PII->init_id_u.name=x;
    PII->type=ZERO_;
    RET=Allocate(ID_LIST);
    RET->P_ini_i=PII;
    RET->next=y;
    return RET;
}

id_list* Id_List_Array(id_list *a,char *b,ArrayInfo *c){
    init_id* PII;
    id_list *RET;
    int j;
    PII=Allocate(INIT_ID);
    PII->type=ARR_;
    PII->init_id_u.P_arr_s=Allocate(ARRAY_SEM);
    PII->init_id_u.P_arr_s->arr_info=Allocate(TYPE_ARR);
    PII->init_id_u.P_arr_s->name=b;
    PII->init_id_u. P_arr_s->arr_info->dim=c->dim;
    for (j=0; j< 10; j++)
    {
        PII->init_id_u. P_arr_s->arr_info->dim_limit[j] = c->dim_limit[j];
    }
    RET=Allocate(ID_LIST);
    RET->P_ini_i=PII;
    if(a==NULL)
        RET->next=NULL;
    else
        RET->next=a;
    return RET;
}

def_list *def_list_P(def_list *a,var_decl *b){
    def_list *PDL;
    PDL=Allocate(DEF_LIST);
    PDL->P_var_s=b;
    if(a==NULL)
        PDL->next=NULL;
    else
        PDL->next=a;
    return PDL;
}


struct token{
    char * name;
    struct token *left;
    struct token* right;
};

struct token *root;

void init(struct token *ptr){
    ptr->name=NULL;
    ptr->left=NULL;
    ptr->right=NULL;
    return;
}

int addtreeflag=0;
void addtree(struct token **TT,char *S){
    struct token *T;
    int i;
    T=*TT;
    if(T==NULL){
        T=(struct token*)malloc(sizeof(struct token));
        init(T);
        T->name=S;
        *TT=T;
        return;
    }
    i=strcmp(T->name,S);
    if(i==0){
        printf("error %d: duplicate tokens %s in structure definition\n",linenumber,S);
        addtreeflag=1;
    }
    else if(i<0)
        addtree(&(T->right),S);
    else
        addtree(&(T->left),S);
}

int check_duplicate(def_list *a){
    addtreeflag=0;
    root=NULL;
    var_decl *PVD;
    do{
        PVD=a->P_var_s;
        id_list *PIL=PVD->P_id_l;
        do{
            init_id *PII=PIL->P_ini_i;
            switch (PII->type){
                case ARR_:
                    addtree(&root,PII->init_id_u. P_arr_s->name);
                    break;
                default:
                    addtree(&root,PII->init_id_u.name);
                    break;
            }
        }while(PIL=PIL->next);
    }while(a=a->next);

    return addtreeflag;
}



/*return 0 on error else return 1*/
/* 0 means no entry added to symbol table*/
int struct_type_P(char *a,def_list* b){
    if(b==NULL){
        printf("error %d: structure declaration with no members\n",linenumber);
        GLOBAL_ERROR=1;
        free(b);
        return 0;
    }
    if(check_duplicate(b)){
        GLOBAL_ERROR=1;
        free(b);
        return 0;
    }

    /*there are no duplicates in the structure*/
    /*we enter all the elements in the def_list even of they have errors*/
    struct_semantic *ret=NULL;
    struct_semantic *PSS;
    def_list * c=b;
    do{
        var_decl *PVD=b->P_var_s;
        TYPE type=PVD->type;
        id_list *PIL=PVD->P_id_l;
        do{
            init_id *PII=PIL->P_ini_i;
            array_semantic *PAS;
            PSS=Allocate(STRUCT_SEM);
            switch (type){
                case INT_:
                case FLOAT_:
                    switch (PII->type){
                        case ZERO_:
                        case ERROR_:
                            PSS->struct_semantic_u.var_name=PII->init_id_u.name;
                            PSS->type=type;
                            break;
                        case ARR_:
                            PSS->struct_semantic_u.arr_sem=PII->init_id_u.P_arr_s;
                            PSS->type=ARR_;
                            PII->init_id_u.P_arr_s->arr_info->arrtype=type;
                            break;
                    }
                    break;
                case STR_VAR_:
                case STR_:
                    switch (PII->type){
                        case ZERO_:
                        case ERROR_:
                            PSS->type=type;
                            PSS->struct_semantic_u.str_info.str_var_name=PII->init_id_u.name;
                            PSS->struct_semantic_u.str_info.struct_type_name=PVD->type_name;
                            break;
                        case ARR_:
                            PSS->type=ARR_;
                            PSS->struct_semantic_u.arr_sem=PII->init_id_u.P_arr_s;
                            PII->init_id_u.P_arr_s->arr_info->arrtype=STR_;
                            PII->init_id_u.P_arr_s->arr_info->type_name=PVD->type_name;
                            break;
                    }
                    break;
                case ERROR_:
                    GLOBAL_ERROR=1;
                    continue;
            }
            if(!ret){
                PSS->next=NULL;
                ret=PSS;
            }else{
                PSS->next=ret;
                ret=PSS;
            }

        }while(PIL=PIL->next);
    }while(b=b->next);
    insert(a,STR_,ret,0);
    free(c);
    return 1;
}


char *buffer;
int tempnumber=0;
char *get_temp_name(){

    buffer=(char*)malloc(10);
    sprintf(buffer,"&&temp%d",tempnumber);
    tempnumber++;
    return buffer;

}

char *ArrayNewName(char *a){
    int i;
    char *c;
    i=strlen(a);
    c=(char*)malloc(i+2+1);
    strcpy(c,a);
    strcat(c,"[]");
    return c;
}

char * new_name(char *a){
    int i;
    char *c;
    i=strlen(a);
    c=(char*)malloc(i+2);
    *c='&';
    *(c+1)='\0';
    strcat(c,a);
    return c;
}

int if_user_name(char *x){
    if(*x=='&'&&*(x+1)=='&')
        return 0;
    return 1;
}

TYPE type_decl_enter_ST1(int a,id_list *b){
    /*enter the id_list into the ST*/
    char * name;
    symtab *entry;
    id_list * c=b;
    TYPE ret_type=ZERO_;
    do{
        name=(b->P_ini_i->type==ARR_)?b->P_ini_i->init_id_u.P_arr_s->name:b->P_ini_i->init_id_u.name;
        if(entry=lookup(name))
            if((entry)&&(entry->scope>=scope)){
                printf("error %d: variable or type redeclared, %s, declared earlier at line %d\n",linenumber,name,entry->line);
                ret_type=ERROR_;
            }
        if(b->P_ini_i->type==ARR_){
            b->P_ini_i->init_id_u.P_arr_s->arr_info->arrtype=a;
            insert(name,TYPEDEF_,b->P_ini_i->init_id_u.P_arr_s->arr_info,TYPEDEF_ARR);
        }
        else if(a==INT_)
            insert(b->P_ini_i->init_id_u.name,TYPEDEF_,NULL,TYPEDEF_INT);
        else
            insert(b->P_ini_i->init_id_u.name,TYPEDEF_,NULL,TYPEDEF_FLT);
    }while(b=b->next);
    free(c);
    return ret_type;
}

TYPE type_decl_enter_ST2(char *a,id_list* b){
    char * name;
    symtab *entry;
    TYPE ret_type=ZERO_;
    id_list * c=b;
    entry=lookup(a);
    if(entry==NULL){
        printf("error %d:  type undeclared, %s\n",linenumber,name);
        ret_type=ERROR_;
    }
    else
        do{
            name=(b->P_ini_i->type==ARR_)?b->P_ini_i->init_id_u.P_arr_s->name:b->P_ini_i->init_id_u.name;
            if(b->P_ini_i->type==ARR_){
                b->P_ini_i->init_id_u.P_arr_s->arr_info->arrtype=STR_;
                b->P_ini_i->init_id_u.P_arr_s->arr_info->type_name=a;
                insert(name,TYPEDEF_,b->P_ini_i->init_id_u.P_arr_s->arr_info,TYPEDEF_ARR);
            }
            else
                insert(name,TYPEDEF_,a,TYPEDEF_STR);
        }while(b=b->next);
    free(c);
    return ret_type;
}


void chk_insert(char *a,TYPE b,void *c,IS_TYPE_DEF d){
    symtab * PST;
    PST=lookup(a);
    if((PST)&&(PST->scope>=scope)){
        printf("error %d:redeclaration of variable %s declared previously at line %d\n",linenumber,a,PST->line);
        GLOBAL_ERROR=1;
    }
    else
        insert(a,b,c,d);
    return;
}



TYPE decl_enter_ST(var_decl *a){
    init_id * PII;
    id_list * PIL;
    TYPE ret=ZERO_;
    PIL=a->P_id_l;
    symtab *sym_ptr;

    if(a==NULL)
        return ERROR_;

    if(a->type==ERROR_){
        free(a);
        return ERROR_;
    }


    PIL=a->P_id_l;
    if(PIL==NULL)
        return ERROR_;

    do{
        PII=PIL->P_ini_i;
        switch(a->type){
            case INT_:
                if(PII->type==ARR_){
                    PII->init_id_u.P_arr_s->arr_info->arrtype=INT_;
                    chk_insert(PII->init_id_u.P_arr_s->name,ARR_,PII->init_id_u.P_arr_s->arr_info,0);
                    sym_ptr = lookup( PII->init_id_u.P_arr_s->name );
                    sym_ptr->offset = PII->offset;
                }else
                {
                    chk_insert(PII->init_id_u.name,INT_,NULL,0);
                    sym_ptr = lookup( PII->init_id_u.name );
                    sym_ptr->offset = PII->offset;
                }
                break;
            case FLOAT_:
                if(PII->type==ARR_){
                    PII->init_id_u.P_arr_s->arr_info->arrtype=FLOAT_;
                    chk_insert(PII->init_id_u.P_arr_s->name,ARR_,PII->init_id_u.P_arr_s->arr_info,0);
                    sym_ptr = lookup( PII->init_id_u.P_arr_s->name );
                    sym_ptr->offset = PII->offset;
                }else
                {
                    chk_insert(PII->init_id_u.name,FLOAT_,NULL,0);
                    sym_ptr = lookup( PII->init_id_u.name );
                    sym_ptr->offset = PII->offset;
                }
                break;
            case STR_VAR_:case STR_:
                if(PII->type==ARR_){
                    PII->init_id_u.P_arr_s->arr_info->arrtype=STR_;
                    PII->init_id_u.P_arr_s->arr_info->type_name=a->type_name;
                    chk_insert(PII->init_id_u.P_arr_s->name,ARR_,PII->init_id_u.P_arr_s->arr_info,0);
                }else
                    chk_insert(PII->init_id_u.name,STR_VAR_,a->type_name,0);
                break;
            case ERROR_:
                ret=ERROR_;
                break;
        }
    }while(PIL=PIL->next);
    free(a);
    return ret;
}

char * Add_Str(char *a,char *b){
    int i;
    char *c;
    i=strlen(a)+strlen(b);
    c=(char*)malloc(i+1+4);
    strcpy(c,b);
    strcat(c," in ");
    strcat(c,a);
    return c;
}

struct_semantic * search(char *a,char *b){
    struct_semantic *PSS1;
    char c[32];
    int i=0;

    while(((c[i]=*b++)!=' ')&&(c[i]!='\0'))
        i++;
    c[i]='\0';

    symtab * PST, *PST1;
    if((PST=lookup(a))==NULL){
        printf("error %d: unknown struct type variable %s\n",linenumber,a);
        GLOBAL_ERROR=1;
        return NULL;
        //exit(0);
    }

    if(PST->scope>scope){
        printf("error %d: undeclared struct type variable %s\n",linenumber,a);
        GLOBAL_ERROR=1;
        return NULL;
    }
    PSS1=PST->symtab_u.st_struct;

    while(PSS1){
        switch(PSS1->type){
            case ARR_:
                if(!strcmp(c,PSS1->struct_semantic_u. arr_sem->name))
                    return PSS1;
                break;
            case STR_:
                if(!strcmp(c,PSS1->struct_semantic_u.str_info.str_var_name)){
                    if(((PST1=lookup(PSS1->struct_semantic_u.str_info.struct_type_name))==NULL)||(PST1->scope>scope))
                        return NULL;
                    else
                        return PSS1;
                }
                break;
            default:
                if(!strcmp(c,PSS1->struct_semantic_u.var_name))
                    return PSS1;
                break;
        }
        PSS1=PSS1->next;
    }
    return PSS1;
}


TYPE param_P(param *a,char *b){
    Type_arr * PTA;
    symtab *PST;
    if(a==NULL)
        return ZERO_;


    if(((PST=lookup(b))!=NULL)&&(PST->scope>=scope)){
        printf("error %d: redeclaration of variable %s in parameter list\n",linenumber,b);
        free(a);
        return ERROR_;
    }

    else{
        switch (a->type){
            case INT_:
                insert(b,INT_,NULL,0);
                break;
            case FLOAT_:
                insert(b,FLOAT_,NULL,0);
                break;
            case ARR_:
                PTA=Allocate(TYPE_ARR);
                PTA->dim=a->dim;
                PTA->arrtype=a->arrtype;
                insert(b,ARR_,PTA,0);
                break;
        }
    }
    return ZERO_;
}

param_list *MakeParamList(param_list *a,param *b){
    param_list *c,*d=a;


    if(a==NULL){
        a=Allocate(STRINGLIST);
        a->PPAR=b;
        a->next=NULL;
        return a;
    }

    else{
        do{
            c=a;
        }while(a=a->next);

        c->next=Allocate(PARAM_LIST);
        c->next->PPAR=b;
        c->next->next=NULL;
        if(b==NULL)
            GLOBAL_ERROR=1;
    }
    return d;
}


TYPE func_enter_ST(TYPE a,char *b,param_list *c){
    ST_func *PSF;
    symtab *PST;
    TYPE ret =ZERO_;
    int i=0;
    if((PST=lookup(b))!=NULL){
        printf("error %d: redeclaration of symbol %s  as function, declared previously as %s on line %d\n",linenumber,b,printtype(PST->type),PST->line);
        return ERROR_;
    }

    PSF=Allocate(ST_FUNC);
    PSF->PL=c;
    int param_offset = 8;
    symtab *param;
    while(c){
        i++;
        if(c->PPAR==NULL)
            ret=ERROR_;
        else
        {
            param = lookup (c->PPAR->name);
            if ( param != NULL )
            {
                param->offset = param_offset;
                param_offset += 4;
            }
        }
        c=c->next;
    }
    PSF->params=i;
    switch(a){
        case INT_:
        case FLOAT_:
        case VOID_:
            PSF->ret_type=a;
            break;
        case STR_:
            PSF->ret_type=ERROR_;
            ret=ERROR_;
            break;
        default:
            PSF->ret_type=ERROR_;
            ret=ERROR_;
    }

    insert(b,FUNC_,PSF,0);

    strcpy( function_name, b );
    return ret;
}



var_ref* check_function(char * a, TypeList *b){
    symtab * PST;
    var_ref *PVR;
    int i=0, reg;
    TypeList *c=b;
    TypeList *d=b;
    while(c){
        i++;
        c=c->next;
    }

    PVR=Allocate(VAR_REF);

    if((PST=lookup(a))==NULL){
        printf("error %d: undefined function %s\n",linenumber,a);
        PVR->type=ERROR_;
        GLOBAL_ERROR=1;
    }else if(PST->type!=FUNC_){
        printf("error %d: %s is not a function\n",linenumber,a);
        PVR->type=ERROR_;
        GLOBAL_ERROR=1;
    }else if(i>PST->symtab_u.st_func->params){
        printf("error %d: too many arguments to function %s\n",linenumber,a);
        PVR->type=ERROR_;
        GLOBAL_ERROR=1;
    }else if(i<PST->symtab_u.st_func->params){
        printf("error %d: too few arguments to function %s\n",linenumber,a);
        PVR->type=ERROR_;
        GLOBAL_ERROR=1;
    }else{
        ST_func *x;

        param_list *y = reverse_param_list (PST->symtab_u.st_func->PL);
        param_list *new_param_head = y;

        TypeList *t = reverse_TypeList (b);
        TypeList *new_typelist_head = t;


        //x=PST->symtab_u.st_func;
        var_ref * PVR1;
        //y=x->PL;
        i=0;
        PVR->type=PST->symtab_u.st_func->ret_type;

        while(y){
            PVR1=t->P_var_r;
            if((y->PPAR==NULL)||(PVR1->type==ERROR_)){
                y=y->next;
                t=t->next;
                PVR->type=ERROR_;
                i++;
                continue;
            }
            switch(y->PPAR->type){
                case INT_:
                    if((PVR1->type!=INT_)&&(PVR1->type!=FLOAT_)){
                        printf("error %d: function arg %d, expects scalar, passed %s of type %s\n",linenumber,i,(PVR1->name)?(PVR1->name):"",printtype(PVR1->type));
                        PVR->type=ERROR_;
                    }
                    else if ( PVR1->place == 0 )
                    {
                        reg = get_result_reg();
                        gen_load_int( reg, PVR1 );
                    }
                    else
                    {
                        reg = PVR1->place;
                        if(PVR1->is_array)
                        {
                            reg = gen_load_array (PVR1, 4);
                            printf( "\tlw\t$%d, 0($%d)\n", reg, reg );
                        }
                    }
                    printf( "\tsw\t$%d, ($sp)\n", reg );
                    printf( "\tsub\t$sp, $sp, 4\n" );
                    free_reg (reg);
                    PVR1->place = -1;
                    PVR->place=reg;
                    break;
                case FLOAT_:
                    if((PVR1->type!=INT_)&&(PVR1->type!=FLOAT_)){
                        printf("error %d: function arg %d, expects scalar, passed %s of type %s\n",linenumber,i,(PVR1->name)?(PVR1->name):"",printtype(PVR1->type));
                        PVR->type=ERROR_;
                    }
                    else if ( PVR1->place == 0 )
                    {
                        reg = get_result_reg();
                        gen_load_float( reg, PVR1 );
                    }
                    else
                    {
                        if( PVR1->is_array )
                        {
                            reg = gen_load_array( PVR1, 4 );
                            printf ("\tlw\t$f%d, 0($f%d)\n", reg, reg);
                        }
                        else
                            reg = PVR1->place;
                    }
                    printf( "\ts.s\t$f%d, ($sp)\n", reg );
                    printf( "\tsub\t$sp, $sp, 4\n");
                    float_free_reg (reg);
                    PVR1->place = -1;
                    PVR->place=reg;
                    break;
                case ARR_:
                    if((PVR1->type==INT_)||(PVR1->type==FLOAT_)){
                        printf("error %d: function arg %d, scalar %s passed, expects a %dD array\n",linenumber,i,(PVR1->name)?(PVR1->name):"",PST->symtab_u.st_arr->dim);
                        PVR->type=ERROR_;
                    }else if(PVR1->type==ARR_){
                        if((y->PPAR->arrtype!=PVR1->var_ref_u.arr_info->arrtype)||(y->PPAR->dim!=PVR1->var_ref_u.arr_info->dim))
                            //printf("warning %d: passing arg %d of %s from incompatible pointer type\n",linenumber,i,a);
                            printf("not support!!\n");
                    }
                    else{
                        printf("error %d: function arg %d, expects array, passed a struct %s\n",linenumber,i,(PVR1->name)?(PVR1->name):"");
                        PVR->type=ERROR_;
                    }
                    break;
            }
            i++;
            y=y->next;
            t=t->next;
        }
        reverse_param_list (new_param_head);
        reverse_TypeList (new_typelist_head);

    }
    free(d);
    if (PVR->name !=NULL)
        PVR->name = NULL;
    //printf("error\n");
    return PVR;
}

void put_read_ST(){
    ST_func *PSF;

    PSF=Allocate(ST_FUNC);
    PSF->ret_type=INT_;
    PSF->params=0;
    PSF->PL=NULL;

    insert("read",FUNC_,PSF,0);

    PSF=Allocate(ST_FUNC);
    PSF->params=0;
    PSF->ret_type=FLOAT_;
    PSF->PL=NULL;

    insert("fread",FUNC_,PSF,0);

    strcpy( frame_data.string, ".data" );
    return;
}

TYPE check_return(int flag,TYPE type){
    if(flag)
        return ZERO_;
    else{
        printf("error %d: missing return statement\n",linenumber);
        return ERROR_;
    }
}

/* new add */
int get_offset( char *name )
{
    symtab *ptr = lookup (name);


    if ( ptr->type == INT_ || ptr->type == FLOAT_ )
        return ptr->offset;
    else if (ARR_ == ptr->type)
        ;//no need to do

    return ptr->offset;
}

int set_offset( var_decl *value, int offset )
{

    init_id *PII = NULL;
    id_list *PIL = NULL;

    PIL = value->P_id_l;

    symtab *ptr_a;
    var_ref *b;
    int reg;

    do
    {
        PII = PIL->P_ini_i;

        switch ( value->type )
        {
            case INT_:
                PII->offset = offset;
                if (ARR_ == PII->type)
                {
                    int i;
                    for (i = 0; i <  PII->init_id_u.P_arr_s->arr_info->dim; i++)
                        if (i == 0)
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                        else
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->size * PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                    offset -= PII->init_id_u.P_arr_s->arr_info->size * 4;
                }
                else
                {
                    offset -= 4;
                    ptr_a = lookup( PII->init_id_u.name );
                    if ( PII->initial_type != 0 )
                    {
                        if ( PII->place == 0 )
                        {
                            b = Allocate( VAR_REF );
                            b->type = INT_;
                            b->place = 0;
                            b->const_value.const_int = PII->value.value_int;
                            reg = gen_load_int( get_reg( b ), b );
                        }
                        else
                            reg = PII->place;
                        gen_store( reg, PII->offset );
                        reset_reg();
                        float_reset_reg();
                    }
                }
                break;
            case FLOAT_:
                PII->offset = offset;
                if (ARR_ == PII->type)
                {
                    int i;
                    for (i = 0; i <  PII->init_id_u.P_arr_s->arr_info->dim; i++)
                        if (i == 0)
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                        else
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->size * PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                    offset -= PII->init_id_u.P_arr_s->arr_info->size * 4;
                }
                else
                {
                    offset -= 4;
                    ptr_a = lookup( PII->init_id_u.name );
                    if ( PII->initial_type != 0 )
                    {
                        if ( PII->place == 0 )
                        {
                            b = Allocate( VAR_REF );
                            b->type = FLOAT_;
                            b->place = 0;
                            b->const_value.const_float = PII->value.value_float;
                            reg = gen_load_float( float_get_reg( b ), b );
                        }
                        else
                            reg = PII->place;
                        gen_store_float( reg, PII->offset );
                        reset_reg();
                        float_reset_reg();
                    }
                }
                break;
        }
    }
    while ( PIL = PIL->next );

    return offset;
}

void set_global( var_decl *value )
{

    init_id *PII = NULL;
    id_list *PIL = NULL;

    PIL = value->P_id_l;

    do
    {
        PII = PIL->P_ini_i;

        char data_set[ MAXLEN ];

        switch ( value->type )
        {
            case INT_:
                if (ARR_ == PII->type)
                {
                    int i;
                    for (i = 0; i <  PII->init_id_u.P_arr_s->arr_info->dim; i++)
                        if (i == 0)
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                        else
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->size * PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                    sprintf( data_set, "_%s: .space %d", PII->init_id_u.P_arr_s->name, ( int ) PII->init_id_u.P_arr_s->arr_info->size * 4 );
                }
                else
                    sprintf( data_set, "_%s: .word %d", PII->init_id_u.name, ( int ) PII->value.value_int );
                break;
            case FLOAT_:
                if (ARR_ == PII->type)
                {
                    int i;
                    for (i = 0; i <  PII->init_id_u.P_arr_s->arr_info->dim; i++)
                        if (i == 0)
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                        else
                            PII->init_id_u.P_arr_s->arr_info->size = PII->init_id_u.P_arr_s->arr_info->size * PII->init_id_u.P_arr_s->arr_info->dim_limit[i];
                    sprintf( data_set, "_%s: .space %d", PII->init_id_u.P_arr_s->name, ( int ) PII->init_id_u.P_arr_s->arr_info->size );
                }
                else
                    sprintf( data_set, "_%s: .float %f", PII->init_id_u.name, ( float ) PII->value.value_float );
                break;
        }
        struct string_list *ptr = &frame_data;
        while ( ptr->next != NULL )
            ptr = ptr->next;

        ptr->next = ( void* ) malloc( sizeof( struct string_list ) );
        strcpy( ptr->next->string, data_set );
    }
    while ( PIL = PIL->next );
}

void set_float_data ( int number, float value )
{
    char data_set[ MAXLEN ];
    sprintf( data_set, "_fp%d: .float %f", number, value );

    struct string_list *ptr = &frame_data;
    while ( ptr->next != NULL )
        ptr = ptr->next;

    ptr->next = ( void* ) malloc( sizeof( struct string_list ) );
    strcpy( ptr->next->string, data_set );
}

void set_string_data( int number, char* value )
{
    char data_set[ MAXLEN ];
    sprintf( data_set, "_m%d: .asciiz %s", number, value );

    struct string_list *ptr = &frame_data;
    while ( ptr->next != NULL )
        ptr = ptr->next;

    ptr->next = ( void* ) malloc( sizeof( struct string_list ) );
    strcpy( ptr->next->string, data_set );
}

void count_dim( var_ref *v_ptr, var_ref *value )
{
    symtab *STP;
    int reg;
    if ( v_ptr->name != NULL )
    {
        STP=lookup(v_ptr->name);

        if ( value->place == 0 )
            if ( value->name != NULL )
            {
                if ( value->type == ARR_ )
                    value->place = gen_load_array( value, 4 );
                else
                    value->place = gen_load_int( get_result_reg(), value );
            }
    }

    v_ptr->var_ref_u.arr_info->dim_limit[STP->symtab_u.st_arr->dim - v_ptr->var_ref_u.arr_info->dim] = value->const_value.const_int;
    v_ptr->var_ref_u.arr_info->dim_place[STP->symtab_u.st_arr->dim - v_ptr->var_ref_u.arr_info->dim] = value->place;
}


param_list* reverse_param_list( param_list *p )
{
    param_list *rv = NULL;
    while ( p )
    {
        param_list *tmp = p->next;
        p->next = rv;
        rv = p;
        p = tmp;
    }
    return rv;
}

TypeList* reverse_TypeList( TypeList *t )
{
    TypeList *rv = NULL;
    while ( t )
    {
        TypeList *tmp = t->next;
        t->next = rv;
        rv = t;
        t = tmp;
    }
    return rv;
}

void stack_push(int value )
{
    LS[ stack_now++ ] = value;
}

int stack_pop()
{
    return LS[ --stack_now ];
}

int stack_top( int value )
{
    if ( value == -1 )
        return LS[ stack_now - 1 ];
    else
        LS[ stack_now - 1 ] = value;
    return value;
}

void print_stack()
{
    int i;
    for ( i = stack_now - 1; i >= 0; i-- )
        printf("%d ", LS[ i ]);
    printf("\n");


}
