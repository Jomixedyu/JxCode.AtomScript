#include "Variable.h"
#include <memory.h>

void SetVariableUndefined(Variable* var)
{
    var->type = VARIABLETYPE_UNDEFINED;
    var->num = 0;
}

void SetVariableNumber(Variable* var, float num)
{
    var->type = VARIABLETYPE_NUMBER;
    var->num = num;
}

void SetVariableStrPtr(Variable* var, int ptr)
{
    var->type = VARIABLETYPE_STRPTR;
    var->ptr = ptr;
}

void SetVariableFuncPtr(Variable* var, int ptr)
{
    var->type = VARIABLETYPE_FUNCPTR;
    var->ptr = ptr;
}

void SetVariableTablePtr(Variable* var, int ptr)
{
    var->type = VARIABLETYPE_TABLEPTR;
    var->ptr = ptr;
}

void SetVariableUserPtr(Variable* var, int ptr)
{
    var->type = VARIABLETYPE_USERPTR;
    var->ptr = ptr;
}

Variable GetVariableNumber(float num)
{
    Variable v;
    SetVariableNumber(&v, num);
    return v;
}
Variable GetVariableStrPtr(int ptr)
{
    Variable var;
    SetVariableStrPtr(&var, ptr);
    return var;
}
Variable GetVariableFuncPtr(int ptr)
{
    Variable v;
    SetVariableFuncPtr(&v, ptr);
    return v;
}
Variable GetVariableTablePtr(int ptr)
{
    Variable v;
    SetVariableTablePtr(&v, ptr);
    return v;
}
Variable GetVariableUserPtr(int ptr)
{
    Variable v;
    SetVariableUserPtr(&v, ptr);
    return v;
}
void SerializeVariable(Variable* var, char out[8])
{
    memcpy(out, var, sizeof(Variable));
}
Variable DeserializeVariable(char value[8])
{
    Variable var;
    memcpy(&var, value, 8);
    return var;
}