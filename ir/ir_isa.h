#define INSTRUCTION_LIST(_)                                                                       \
    _(ADD, BinaryOp, Inst::Flags::SYMMETRY)                                                       \
    _(SUB, BinaryOp, {})                                                                          \
    _(MUL, BinaryOp, Inst::Flags::SYMMETRY)                                                       \
    _(DIV, BinaryOp, {})                                                                          \
    _(MOD, BinaryOp, {})                                                                          \
    _(MIN, BinaryOp, Inst::Flags::SYMMETRY)                                                       \
    _(MAX, BinaryOp, Inst::Flags::SYMMETRY)                                                       \
    _(SHL, BinaryOp, {})                                                                          \
    _(SHR, BinaryOp, {})                                                                          \
    _(ASHR, BinaryOp, {})                                                                         \
    _(AND, BinaryOp, Inst::Flags::SYMMETRY)                                                       \
    _(OR, BinaryOp, Inst::Flags::SYMMETRY)                                                        \
    _(XOR, BinaryOp, Inst::Flags::SYMMETRY)                                                       \
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
    _(CHECK_ZERO, FixedInputOp1, Inst::Flags::CHECK | Inst::Flags::NO_DCE)                        \
    _(CHECK_NULL, FixedInputOp1, Inst::Flags::CHECK | Inst::Flags::NO_DCE)                        \
    _(CHECK_SIZE, BinaryOp, Inst::Flags::CHECK | Inst::Flags::NO_DCE)                             \
    /*_(CAST, CastOp )*/                                                                          \
    /*_(CALL_DYNAMIC, CallOp, Inst::Flags::CALL | Inst::Flags::NO_DCE)*/                          \
    _(CALL_STATIC, CallOp, Inst::Flags::CALL | Inst::Flags::NO_DCE)                               \
    _(PHI, PhiOp, {})                                                                             \
    _(CONST, ConstantOp, {})                                                                      \
    _(PARAM, ParamOp, {})                                                                         \
    _(RETURN, FixedInputOp1, Inst::Flags::NO_DCE)                                                 \
    _(RETURN_VOID, FixedInputOp0, Inst::Flags::NO_DCE)                                            \
    _(IF, IfOp, Inst::Flags::NO_DCE)                                                              \
    _(IF_IMM, IfImmOp, Inst::Flags::NO_DCE)                                                       \
    _(JMP_IND, FixedInputOp1, Inst::Flags::NO_DCE)

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