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

void SetVariableUserPtr(Variable* var, int user_ptr)
{
    var->type = VARIABLETYPE_USERPTR;
    var->ptr = user_ptr;
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