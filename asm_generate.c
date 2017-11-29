#include "header.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

extern int ARoffset;
extern char function_name[ MAXLEN ];

extern int float_number, string_number;

void gen_head( const char *name )
{
    printf( "\n.text\n" );
    printf( "%s:\n\n", name );
    strcpy( function_name, name );
}

void gen_prologue( const char *name )
{
    printf( "# Prologue\n" );
    printf( "\tsw\t$ra, 0($sp)\n" );
    printf( "\tsw\t$fp, -4($sp)\n" );
    printf( "\tadd\t$fp, $sp, -4\n" );
    printf( "\tadd\t$sp, $sp, -8\n" );
    printf( "\tlw\t$2, _framesize_of_%s\n", name );
    printf( "\tsub\t$sp, $sp, $2\n" );
    push_reg();
    printf( "_begin_%s:\n", name );
}

void gen_epilogue( const char *name )
{
    printf( "# Epilogue\n" );
    printf( "_end_%s:\n", name );
    int i;
    for ( i = 8; i < MAX_REG; i++ )
        free_reg(i);

    pop_reg();
    printf( "\tlw\t$ra, 4($fp)\n" );
    printf( "\tadd\t$sp, $fp, 4\n" );
    printf( "\tlw\t$fp, 0($fp)\n" );
    if ( strcmp ( name, "main" ) == 0 )
    {
        printf( "\tli\t$v0, 10\n" );
        printf( "\tsyscall\n" );
    }
    else
        printf( "\tjr\t$ra\n" );
    printf( ".data\n" );
    printf( "_framesize_of_%s: .word %d\n", name, abs( ARoffset ) + 72 );
}

int is_reverse_cond = 0;
int FLOAT_COMPARE = 0;

int gen_relop_factor( var_ref *a, var_ref *b, int op )
{
    is_reverse_cond = 0;
    TYPE value_type = INT_;

    int regA;
    int regB;
    int dist_reg;

    if ( a->type == FLOAT_ || b->type == FLOAT_ )
    {
        regA = float_get_reg( a );
        regB = float_get_reg( b );

        regA = gen_load_float( regA, a );

        if ( regB != regA )
            regB = gen_load_float( regB, b );
        value_type = FLOAT_;
    }
    else
    {
        regA = get_reg( a );
        regB = get_reg( b );

        regA = gen_load_int( regA, a );

        if ( regB != regA )
            regB = gen_load_int( regB, b );
    }

    //reset_reg();
    //float_reset_reg();

    if ( value_type == INT_ )
        dist_reg = get_result_reg();
    else
        dist_reg = get_result_reg();

    if ( b != NULL )
    {
        if ( value_type == INT_ )
            switch ( op )
            {
                case OP_GT:
                    printf("\tsgt\t$%d, $%d, $%d\n", dist_reg, regA, regB );
                    break;
                case OP_GE:
                    printf("\tsge\t$%d, $%d, $%d\n", dist_reg, regA, regB );
                    break;
                case OP_LT:
                    printf("\tslt\t$%d, $%d, $%d\n", dist_reg, regA, regB );
                    break;
                case OP_LE:
                    printf("\tsle\t$%d, $%d, $%d\n", dist_reg, regA, regB );
                    break;
                case OP_NE:
                    printf("\tsne\t$%d, $%d, $%d\n", dist_reg, regA, regB );
                    break;
                case OP_EQ:
                    printf("\tseq\t$%d, $%d, $%d\n", dist_reg, regA, regB );
                    break;
                default:
                    break;
            }
        else
        {   switch (op)
            {
                case OP_GT:
                    printf("\tc.le.s\t$f%d, $f%d\n", regA, regB );
                    is_reverse_cond = 1;
                    break;
                case OP_GE:
                    printf("\tc.lt.s\t$f%d, $f%d\n", regA, regB );
                    is_reverse_cond = 1;
                    break;
                case OP_LT:
                    printf("\tc.lt.s\t$f%d, $f%d\n", regA, regB );
                    break;
                case OP_LE:
                    printf("\tc.le.s\t$f%d, $f%d\n", regA, regB );
                    break;
                case OP_NE:
                    printf("\tc.eq.s\t$f%d, $f%d\n", regA, regB );
                    is_reverse_cond = 1;
                    break;
                case OP_EQ:
                    printf("\tc.eq.s\t$f%d, $f%d\n", regA, regB );
                    break;
                default:
                    break;
            }

            if ( OP_NE == op || OP_GT == op || OP_GE == op )
                printf("\tli\t$%d, 1\n", dist_reg );
            else
                printf("\tli\t$%d, 0\n", dist_reg );
            printf( "\tbc1f\t_FLOAT_COMPARE%d\n", FLOAT_COMPARE );
            if ( OP_NE == op || OP_GT == op || OP_GE == op )
                printf("\tli\t$%d, 0\n", dist_reg );
            else
                printf("\tli\t$%d, 1\n", dist_reg );
            printf("_FLOAT_COMPARE%d:\n", FLOAT_COMPARE++ );
        }
    }
    if ( value_type == INT_ )
        save_reg( dist_reg );
    return dist_reg;
}

int gen_expr( var_ref *a, var_ref *b, int op )
{
    TYPE value_type = INT_;

    int regA;
    int regB;
    int dist_reg;

    if ( a->type == FLOAT_ || b->type == FLOAT_ )
    {
        regA = float_get_reg( a );
        regB = float_get_reg( b );

        regA = gen_load_float( regA, a );

        if ( regB != regA )
            regB = gen_load_float( regB, b );
        value_type = FLOAT_;
    }
    else
    {
        regA = get_reg( a );
        regB = get_reg( b );

        regA = gen_load_int( regA, a );

        if ( regB != regA )
            regB = gen_load_int( regB, b );
    }

    //reset_reg();
    //float_reset_reg();

    if ( value_type == INT_ )
        dist_reg = get_result_reg();
    else
        dist_reg = float_get_result_reg();

    switch ( op )
    {
        case OP_PLUS:
            if ( a->type == INT_ && b->type  == INT_ )
                printf( "\tadd\t$%d, $%d, $%d\n", dist_reg, regA, regB );
            else
                printf( "\tadd.s\t$f%d, $f%d, $f%d\n", dist_reg, regA, regB);
            break;
        case OP_MINUS:
            if ( a->type == INT_ && b->type  == INT_ )
                printf( "\tsub\t$%d, $%d, $%d\n", dist_reg, regA, regB );
            else
                printf( "\tsub.s\t$f%d, $f%d, $f%d\n", dist_reg, regA, regB);
            break;
    }

    a->is_array = 0;
    if ( value_type == INT_ )
        save_reg( dist_reg );
    else
        float_save_reg( dist_reg );
    //print_regs();

    return dist_reg;
}

int gen_term (var_ref *a, var_ref *b, int op )
{
    TYPE value_type = INT_;

    int regA;
    int regB;
    int dist_reg;

    if ( a->type == FLOAT_ || b->type == FLOAT_ )
    {
        regA = float_get_reg( a );
        regB = float_get_reg( b );

        regA = gen_load_float( regA, a );

        if ( regB != regA )
            regB = gen_load_float( regB, b );
        value_type = FLOAT_;
    }
    else
    {
        regA = get_reg( a );
        regB = get_reg( b );

        regA = gen_load_int( regA, a );

        if ( regB != regA )
            regB = gen_load_int( regB, b );
    }

    //reset_reg();
    //float_reset_reg();

    if ( value_type == INT_ )
        dist_reg = get_result_reg();
    else
        dist_reg = float_get_result_reg();

    switch ( op )
    {
        case OP_TIMES:
            if ( a->type == INT_ && b->type  == INT_ )
                printf( "\tmul\t$%d, $%d, $%d\n", dist_reg, regA, regB );
            else
                printf( "\tmul.s\t$f%d, $f%d, $f%d\n", dist_reg, regA, regB);
            break;
        case OP_DIVIDE:
            if ( a->type == INT_ && b->type  == INT_ )
                printf( "\tdiv\t$%d, $%d, $%d\n", dist_reg, regA, regB );
            else
                printf( "\tdiv.s\t$f%d, $f%d, $f%d\n", dist_reg, regA, regB);
            break;
    }

    a->is_array = 0;
    if ( value_type == INT_ )
        save_reg( dist_reg );
    else
        float_save_reg( dist_reg );
    return dist_reg;
}

void gen_function( const char *name )
{
    printf("\tjal\t%s\n", name );
}

void gen_return( var_ref *value )
{
    if( value->type == INT_ )
    {
        int reg = get_result_reg();
        reg = gen_load_int( reg, value );
        printf( "\tmove\t$v0, $%d\n", reg );
    }
    else if ( value->type == FLOAT_ )
    {
        int reg = float_get_result_reg();
        reg = gen_load_float( reg, value );
        printf( "\tmov.s\t$f0, $f%d\n",  reg );
    }
    printf( "\tj _end_%s\n", function_name );
}

void gen_result( var_ref *value )
{
    int reg;
    if( value->type == INT_ )
    {
        reg = get_result_reg();
        printf( "\tmove\t$%d, $v0\n", reg );
        value->place = reg;
    }
    else if( value->type == FLOAT_ )
    {
        reg = float_get_result_reg();
        printf( "\tmov.s $f%d, $f0\n", reg );
        value->place = reg;
    }
    value->is_return = 1;
}

void gen_store( int reg, int offset )
{
    printf( "\tsw\t$%d, %d($fp)\n", reg, offset );
    //print_regs();
    reset_reg();
    float_reset_reg();
}

void gen_store_float( int reg, int offset )
{
    printf( "\ts.s\t$f%d, %d($fp)\n", reg, offset );
    reset_reg();
    float_reset_reg();
}

void gen_store_const_float( int reg, const char *name )
{
    printf( "\ts.s\t$f%d, _%s\n", reg, name );
    reset_reg();
    float_reset_reg();
}

void gen_store_global( int reg, const char *name )
{
    printf( "\tsw\t$%d, _%s\n", reg, name );
    reset_reg();
    float_reset_reg();
}

void gen_load( int reg, int offset )
{
    printf( "\tlw\t$%d, %d($fp)\n", reg, offset );
}

int gen_load_int( int reg, var_ref *value )
{
    symtab *ptr = NULL;
    if ( value->type == INT_ )
    {
        if ( value->name != NULL )
        {
            ptr = lookup( value->name );
            if ( ptr->type != ARR_ )
                if ( ptr->scope > 0 )
                    printf( "\tlw\t$%d, %d($fp)\n", reg, ptr->offset );
                else if ( value->is_return == 1 )
                    printf( "\tmove\t$%d, $%d\n", reg, value->place );
                else
                    printf( "\tlw\t$%d, _%s\n", reg, value->name );
            else
                printf( "\tlw\t$%d, 0($%d)\n", reg, gen_load_array( value, 4 ) );
        }
        else
            if ( value->place == 0 )
                printf( "\tli\t$%d, %d\n", reg, value->const_value.const_int );
            else
                reg = value->place;
    }
    else
    {
        reg = gen_load_float( reg, value );
        printf( "\tcvt.w.s\t$f%d, $f%d\n", reg, reg );
        printf( "\tmfc1\t$%d, $f%d\n", reg, reg );
    }
    return reg;
}

int gen_load_float( int reg, var_ref *value )
{
    symtab *ptr = NULL;

    if ( value->type == FLOAT_ )
        if ( value->name != NULL )
        {
            ptr = lookup (value->name);
            if ( ptr->type != ARR_ )
                if (ptr->scope > 0)
                    printf( "\tl.s\t$f%d, %d($fp)\n", reg, ptr->offset );
                else if ( value->is_return == 1 )
                    printf( "\tmove\t$%d, $%d\n", reg, value->place );
                else
                    printf( "\tl.s\t$f%d, _%s\n", reg, value->name );
            else
                printf( "\tl.s\t$f%d, 0($%d)\n", reg, gen_load_array( value, 4 ) );
        }
        else
            if ( value->place == 0 )
            {
                gen_load_const_float( reg, float_number );
                set_float_data( float_number++, value->const_value.const_float );
            }
            else
                reg = value->place;
    else
    {
        reg = gen_load_int( reg, value );
        printf( "\tmtc1\t$%d, $f%d\n", reg, reg );
        printf( "\tcvt.s.w\t$f%d, $f%d\n", reg, reg );
    }
    return reg;
}

void gen_load_const( int reg, int value )
{
    printf( "\tli\t$%d, %d\n", reg, value );
}

void gen_load_const_float( int reg, int number )
{
    printf( "\tl.s\t$f%d, _fp%d\n", reg, number );
}

void gen_load_reg( int reg_a, int reg_b )
{
    printf( "\tmove\t$%d, $%d\n", reg_a, reg_b );
}

void gen_load_global( int reg, const char *name )
{
    printf( "\tlw\t$%d, _%s\n", reg, name );
}

int gen_load_array( var_ref *v_ptr, int width )
{
    symtab *STP;

    int base_reg;
    int reg;
    int access_reg;
    int dim_reg;
    int i;

    STP = lookup( v_ptr->name );

    for (i = 0; i < STP->symtab_u.st_arr->dim; i++)
    {
        if ( i != 0 )
        {
            dim_reg = get_result_reg();
            gen_load_const( dim_reg, STP->symtab_u.st_arr->dim_limit[i] );

            printf( "\tmul\t$%d, $%d, $%d\n", reg, reg, dim_reg );
            save_reg( reg );
            free_reg( dim_reg );
            dim_reg = reg;
        }

        if ( v_ptr->var_ref_u.arr_info->dim_place[i] == 0 )
        {
            reg = get_result_reg();
            gen_load_const( reg, v_ptr->var_ref_u.arr_info->dim_limit[i] );
        }
        else
            reg = v_ptr->var_ref_u.arr_info->dim_place[i];

        //gen_load_const( reg, v_ptr->var_ref_u.arr_info->dim_limit[i] );

        if ( i != 0 )
        {
            printf( "\tadd\t$%d, $%d, $%d\n", reg, reg, dim_reg );
            save_reg( reg );
            free_reg( dim_reg );
        }
    }
    printf( "\tmul\t$%d, $%d, 4\n", reg, reg );

    base_reg = get_reg( v_ptr );


    if (STP->scope == 0)
        printf( "\tla\t$%d, _%s\n", base_reg, v_ptr->name );
    else
        printf( "\tla\t$%d, %d($fp)\n", base_reg, STP->offset );

    printf( "\tadd\t$%d, $%d, $%d\n", reg, reg, base_reg );
    notsave_reg( reg );
    free_reg( base_reg );
    /*
       for (i = 0; i < STP->symtab_u.st_arr->dim; i++)
       printf("%d ", v_ptr->var_ref_u.arr_info->dim_limit[i]);
       printf("\n");
       for (i = 0; i < STP->symtab_u.st_arr->dim; i++)
       printf("%d ", v_ptr->var_ref_u.arr_info->dim_place[i]);
       printf("\n");
       */
    return reg;
}


void gen_control_while( int label_num )
{
    printf( "\tj\t_Ltest%d\t\n", label_num );
}

void gen_control_endlabel( int label_num )
{
    printf( "_Lexit%d:\t\n", label_num );
}

void gen_control_jump_else( int label_num )
{
    printf ( "\tj\t_Ljoin%d\t\n", label_num );
}

void gen_control_else( int label_num )
{
    printf ( "_Ljoin%d:\t\n", label_num );
}

void gen_control_start( int label_num )
{
    printf( "_Ltest%d:\n", label_num );
}

int gen_or_tail( int reg, int label_num )
{
    printf( "F%d:\n\tli\t$%d, 0\n", label_num, reg );
    printf( "\tj\tE%d\n",label_num );
    printf( "T%d:\n\tli\t$%d, 1\n", label_num, reg );
    printf( "E%d:\n",label_num );
    return reg;
};

int gen_and_tail( int reg, int label_num )
{
    printf( "T%d:\n\tli\t$%d, 1\n", label_num, reg );
    printf( "\tj\tE%d\n",label_num );
    printf( "F%d:\n\tli\t$%d, 0\n", label_num, reg );
    printf( "E%d:\n",label_num );
    return reg;
};


void gen_control_test( var_ref *value, int label_num )
{
    if ( value->name == NULL )
    {
        int reg = get_result_reg();
        switch ( value->type )
        {
            case INT_:
                if ( value->place < 1 || value->place >= REG_COUNT )
                {
                    printf("3\n");printf( "\tli\t$%d, %d\n", reg, value->const_value.const_int );
                }
                else
                    reg = value->place;
                break;
            case FLOAT_:
                gen_load_const_float( reg, float_number );
                set_float_data( float_number++, value->const_value.const_float );
                break;
        }
        printf( "\tbeqz\t$%d, _Lexit%d\n", reg, label_num );
    }
    else if ( value->place > 0 && value->place < REG_COUNT )
        switch ( value->type )
        {
            case INT_:
                printf( "\tbeqz\t$%d, _Lexit%d\n", value->place, label_num );
                break;
            case FLOAT_:
                if ( is_reverse_cond )
                    printf( "\tbc1t\t_Lexit%d\n", label_num );
                else
                    printf( "\tbc1f\t_Lexit%d\n", label_num );
                break;
        }
    else
    {
        printf( "\tli\t$%d, %d\n", value->place, value->const_value.const_int );
        printf( "\tbeqz\t$%d, _Lexit%d\n", value->place, label_num );

    }
    reset_reg();
    float_reset_reg();
}

void gen_frame_data()
{
    struct string_list *ptr = &frame_data;
    while ( ptr->next != NULL )
    {
        printf( "%s\n", ptr->string );
        ptr = ptr->next;
    }
    printf( "%s\n", ptr->string );
}

void gen_write( TypeList * id_list )
{
    symtab *sym_ptr = NULL;

    if ( id_list->P_var_r->name != NULL )
        sym_ptr = lookup( id_list->P_var_r->name );

    if ( id_list->P_var_r->type == INT_ )
    {
        if ( sym_ptr != NULL )
        {
            if ( sym_ptr->type != ARR_ )
                if ( sym_ptr->scope != 0)
                    printf( "\tlw\t$a0, %d($fp)\n", sym_ptr->offset );
                else
                    printf( "\tlw\t$a0, _%s\n", id_list->P_var_r->name );
            else
                printf( "\tlw\t$a0, 0($%d)\n", gen_load_array( id_list->P_var_r, 4 ));
        }
        else
            printf( "\tmove\t$a0, $%d\n", id_list->P_var_r->place );
        printf( "\tli\t$v0, 1\n" );
    }
    else if ( id_list->P_var_r->type == FLOAT_ )
    {
        if ( sym_ptr != NULL )
        {
            if ( sym_ptr->type != ARR_ )
                if ( sym_ptr->scope != 0)
                    printf( "\tl.s\t$f12, %d($fp)\n", sym_ptr->offset );
                else
                    printf( "\tl.s\t$f12, _%s\n", sym_ptr->lexeme );
            else
                printf( "\tl.s\t$f12, 0($%d)\n", gen_load_array( id_list->P_var_r, 4 ));

        }
        else
            printf( "\tmov.s\t$f12, $f%d\n", id_list->P_var_r->place );
        printf( "\tli\t$v0, 2\n" );
    }
    else
    {
        printf( "\tli\t$v0, 4\n" );
        set_string_data( string_number, id_list->P_var_r->const_value.const_char );
        printf( "\tla\t$a0, _m%d\n", string_number++ );
    }
    printf( "\tsyscall\n" );
    //reset_reg();
    //float_reset_reg();
}

void gen_read()
{
    printf ( "\tli\t$v0, 5\n" );
    printf ( "\tsyscall\n" );
}

void gen_read_float()
{
    printf ( "\tli\t$v0, 6\n" );
    printf ( "\tsyscall\n" );
}

void gen_pop_params( TypeList *b )
{
    TypeList *c=b;
    while(c){
        c=c->next;
        printf( "\tadd\t$sp, $sp, 4\n");
    }
}

