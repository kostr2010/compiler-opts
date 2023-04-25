#define INSTRUCTION_LIST(_)                                                                       \
    _(ADD, BinaryOp, isa::flag::TypeSYMMETRY)                                                     \
    _(SUB, BinaryOp, {})                                                                          \
    _(MUL, BinaryOp, isa::flag::TypeSYMMETRY)                                                     \
    _(DIV, BinaryOp, {})                                                                          \
    _(MOD, BinaryOp, {})                                                                          \
    _(MIN, BinaryOp, isa::flag::TypeSYMMETRY)                                                     \
    _(MAX, BinaryOp, isa::flag::TypeSYMMETRY)                                                     \
    _(SHL, BinaryOp, {})                                                                          \
    _(SHR, BinaryOp, {})                                                                          \
    _(ASHR, BinaryOp, {})                                                                         \
    _(AND, BinaryOp, isa::flag::TypeSYMMETRY)                                                     \
    _(OR, BinaryOp, isa::flag::TypeSYMMETRY)                                                      \
    _(XOR, BinaryOp, isa::flag::TypeSYMMETRY)                                                     \
    _(ADDI, BinaryImmOp, {})                                                                      \
    _(SUBI, BinaryImmOp, {})                                                                      \
    _(MULI, BinaryImmOp, {})                                                                      \
    _(DIVI, BinaryImmOp, {})                                                                      \
    _(MODI, BinaryImmOp, {})                                                                      \
    _(MINI, BinaryImmOp, {})                                                                      \
    _(MAXI, BinaryImmOp, {})                                                                      \
    _(SHLI, BinaryImmOp, {})                                                                      \
    _(SHRI, BinaryImmOp, {})                                                                      \
    _(ASHRI, BinaryImmOp, {})                                                                     \
    _(ANDI, BinaryImmOp, {})                                                                      \
    _(ORI, BinaryImmOp, {})                                                                       \
    _(XORI, BinaryImmOp, {})                                                                      \
    _(CMP, CompareOp, {})                                                                         \
    _(CHECK_ZERO, FixedInputOp1, isa::flag::TypeCHECK | isa::flag::TypeNO_DCE)                    \
    _(CHECK_NULL, FixedInputOp1, isa::flag::TypeCHECK | isa::flag::TypeNO_DCE)                    \
    _(CHECK_SIZE, BinaryOp, isa::flag::TypeCHECK | isa::flag::TypeNO_DCE)                         \
    /*_(CAST, CastOp )*/                                                                          \
    /*_(CALL_DYNAMIC, CallOp, isa::flag::TypeCALL | isa::flag::TypeNO_DCE)*/                      \
    _(CALL_STATIC, CallOp, isa::flag::TypeCALL | isa::flag::TypeNO_DCE)                           \
    _(PHI, PhiOp, {})                                                                             \
    _(CONST, ConstantOp, {})                                                                      \
    _(PARAM, ParamOp, {})                                                                         \
    _(RETURN, FixedInputOp1, isa::flag::TypeNO_DCE)                                               \
    _(RETURN_VOID, FixedInputOp0, isa::flag::TypeNO_DCE)                                          \
    _(IF, IfOp, isa::flag::TypeNO_DCE)                                                            \
    _(IF_IMM, IfImmOp, isa::flag::TypeNO_DCE)                                                     \
    _(JMP, FixedInputOp1, isa::flag::TypeNO_DCE)

#define INSTRUCTION_TYPES(_)                                                                      \
    _(BinaryOp)                                                                                   \
    _(BinaryImmOp)                                                                                \
    _(CompareOp)                                                                                  \
    /*_(CAST, CAST )*/                                                                            \
    /*_(CALL, CALL )*/                                                                            \
    _(ConstantOp)                                                                                 \
    _(ParamOp)                                                                                    \
    _(FixedInputOp0)                                                                              \
    _(FixedInputOp1)                                                                              \
    _(VariableInputOp)                                                                            \
    _(CallOp)                                                                                     \
    _(PhiOp)                                                                                      \
    _(IfImmOp)                                                                                    \
    _(IfOp)

#define INSTUCTION_FLAGS(_) _(NO_DCE)