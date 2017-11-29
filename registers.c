#include "header.h"
#include "parser.tab.h"
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

int reg_number = 8;

typedef enum
{
    FREE = 0,
    NOTSAVE = 1,
    SAVE = 2
}reg_status;

struct reg_info
{
    int is_temp;
    char name[ MAXLEN ];
    reg_status status;
} regs_status[ REG_COUNT ], float_regs_status[ REG_COUNT ];

int check_reg( const char *name )
{
    int i;
    for ( i = 8; i < MAX_REG; i++ )
        if ( regs_status[ i ].status != FREE && regs_status[ i ].is_temp == 0 )
        {
            symtab *STP;
            STP = lookup( name );
            if ( STP->type == ARR_ )
                continue;
            if ( strcmp( regs_status[ i ].name, name ) == 0 )
                return i;
        }
    return -1;
}

int get_reg( var_ref *v_ptr )
{
    if ( v_ptr == NULL || v_ptr->place < 1 || v_ptr->place > REG_COUNT )
    {
        int reg, check;
        if ( v_ptr->name != NULL )
        {
            check = check_reg( v_ptr->name );
            if ( check == -1 )
            {
                reg = get_result_reg();
                regs_status[ reg ].is_temp = 0;
                strcpy( regs_status[ reg ].name, v_ptr->name );
            }
            else
                return check;
        }
        else
            reg = get_result_reg();
        return reg;
    }
    else if ( v_ptr != NULL && v_ptr->place < MAX_REG && v_ptr->place > 1 )
    {
        regs_status[ v_ptr->place ].status = NOTSAVE;
        return v_ptr->place;
    }
}

int get_result_reg()
{
    int i;
    for ( i = 8; i < MAX_REG; i++ )
        if ( regs_status[ i ].status == FREE )
        {
            regs_status[ i ].is_temp = 1;
            regs_status[ i ].status = NOTSAVE;
            return i;
        }

    for ( i = 8; i < MAX_REG; i++ )
        if ( regs_status[ i ].status == NOTSAVE )
        {
            regs_status[ i ].is_temp = 1;
            regs_status[ i ].status = SAVE;
            return i;
        }

    for ( i = 8; i < MAX_REG; i++ )
        if ( regs_status[ i ].status == SAVE )
            return i;
}

void free_reg( int reg )
{
    if ( reg >= 8 && reg < MAX_REG )
        regs_status[ reg ].status = FREE;
}

void notsave_reg( int reg )
{
    if ( reg >= 8 && reg < MAX_REG )
        regs_status[ reg ].status = NOTSAVE;
}

void save_reg ( int reg )
{
    if ( reg >= 8 && reg < MAX_REG )
        regs_status[ reg ].status = SAVE;
}

void reset_reg()
{
    int i;
    for ( i = 8; i < MAX_REG; i++ )
        if ( regs_status[ i ].status != SAVE )
            free_reg( i );
}

int float_check_reg( const char *name )
{
    int i;
    for ( i = 2; i < REG_COUNT; i+=2 )
        if ( float_regs_status[ i ].status != FREE && float_regs_status[ i ].is_temp == 0 )
            if ( strcmp( float_regs_status[ i ].name, name ) == 0 )
                return i;
    return -1;
}

int float_get_reg( var_ref *v_ptr )
{
    if ( v_ptr == NULL || v_ptr->place < 1 || v_ptr->place > REG_COUNT )
    {
        int reg, check;
        if ( v_ptr->name != NULL )
        {
            check = float_check_reg( v_ptr->name );
            if ( check == -1 )
            {
                reg = float_get_result_reg();
                float_regs_status[ reg ].is_temp = 0;
                strcpy( float_regs_status[ reg ].name, v_ptr->name );
            }
            else
                return check;
        }
        else
            reg = float_get_result_reg();
        return reg;
    }
    else if ( v_ptr != NULL && v_ptr->place < MAX_REG && v_ptr->place > 1 )
    {
        float_regs_status[ v_ptr->place ].status = NOTSAVE;
        return v_ptr->place;
    }
}

int float_get_result_reg()
{
    int i;
    for ( i = 2; i < REG_COUNT; i+=2 )
        if ( float_regs_status[ i ].status == FREE )
        {
            float_regs_status[ i ].is_temp = 1;
            float_regs_status[ i ].status = NOTSAVE;
            return i;
        }

    for ( i = 2; i < REG_COUNT; i+=2 )
        if ( float_regs_status[ i ].status == NOTSAVE )
        {
            float_regs_status[ i ].is_temp = 1;
            float_regs_status[ i ].status = SAVE;
            return i;
        }

    for ( i = 2; i < REG_COUNT; i+=2 )
        if ( float_regs_status[ i ].status == SAVE )
            return i;
}

void float_free_reg( int reg )
{
    if ( reg >= 2 && reg < REG_COUNT )
        float_regs_status[ reg ].status = FREE;
}

void float_notsave_reg( int reg )
{
    if ( reg >= 2 && reg < REG_COUNT )
        float_regs_status[ reg ].status = NOTSAVE;
}

void float_save_reg ( int reg )
{
    if ( reg >= 2 && reg < REG_COUNT )
        float_regs_status[ reg ].status = SAVE;
}

void float_reset_reg()
{
    int i;
    for ( i = 2; i < REG_COUNT; i+=2 )
        if ( float_regs_status[ i ].status != SAVE )
            float_free_reg( i );
}

void push_reg()
{
    int i;
    for ( i = 8; i < MAX_REG; i++ )
        printf( "\tsw\t$%i, %i($sp)\n", i, ( MAX_REG - i ) * 4 );
}

void pop_reg()
{
    int i;
    for ( i = 8; i < MAX_REG; i++ )
        printf( "\tlw\t$%i, %i($sp)\n", i, ( MAX_REG - i ) * 4 );
}

void print_regs()
{
    int i;
    printf("regs:\n");
    for ( i = 8; i < MAX_REG; i++ )
        if ( regs_status[ i ].status != FREE )
            printf ( "$%d\t\tname:%s\t\tstatus:%s\n", i, regs_status[ i ].is_temp ? "TEMP" : regs_status[ i ].name, regs_status[ i ].status == NOTSAVE ? "NOTSAVE" : "SAVE" );
    printf("\n");
}
