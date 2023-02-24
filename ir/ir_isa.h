#define INSTRUCTION_LIST(_)                                                                       \
    _(ADD, BinaryOp, InstFlags::SYMMETRY)                                                         \
    _(SUB, BinaryOp, {})                                                                          \
    _(MUL, BinaryOp, InstFlags::SYMMETRY)                                                         \
    _(DIV, BinaryOp, {})                                                                          \
    _(MOD, BinaryOp, {})                                                                          \
    _(MIN, BinaryOp, InstFlags::SYMMETRY)                                                         \
    _(MAX, BinaryOp, InstFlags::SYMMETRY)                                                         \
    _(SHL, BinaryOp, {})                                                                          \
    _(SHR, BinaryOp, {})                                                                          \
    _(ASHR, BinaryOp, {})                                                                         \
    _(AND, BinaryOp, InstFlags::SYMMETRY)                                                         \
    _(OR, BinaryOp, InstFlags::SYMMETRY)                                                          \
    _(XOR, BinaryOp, InstFlags::SYMMETRY)                                                         \
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
    _(CHECK_ZERO, FixedInputOp1, InstFlags::IS_CHECK | InstFlags::NO_DCE)                         \
    /*_(CAST, CastOp )*/                                                                          \
    /*_(CALL_DYNAMIC, CallOp, InstFlags::IS_CALL | InstFlags::NO_DCE)*/                           \
    _(CALL_STATIC, CallOp, InstFlags::IS_CALL | InstFlags::NO_DCE)                                \
    _(PHI, PhiOp, {})                                                                             \
    _(CONST, ConstantOp, {})                                                                      \
    _(PARAM, ParamOp, {})                                                                         \
    _(RETURN, FixedInputOp1, InstFlags::NO_DCE)                                                   \
    _(RETURN_VOID, FixedInputOp0, InstFlags::NO_DCE)                                              \
    _(IF, IfOp, InstFlags::NO_DCE)                                                                \
    _(IF_IMM, IfImmOp, InstFlags::NO_DCE)                                                         \
    _(JMP_IND, FixedInputOp1, InstFlags::NO_DCE)

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