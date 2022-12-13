#define INSTRUCTION_LIST(_)                                                                       \
    _(ADD, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                      \
    _(SUB, BinaryOp, InstFlags::EMPTY)                                                            \
    _(MUL, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                      \
    _(DIV, BinaryOp, InstFlags::EMPTY)                                                            \
    _(MOD, BinaryOp, InstFlags::EMPTY)                                                            \
    _(MIN, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                      \
    _(MAX, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                      \
    _(SHL, BinaryOp, InstFlags::EMPTY)                                                            \
    _(SHR, BinaryOp, InstFlags::EMPTY)                                                            \
    _(ASHR, BinaryOp, InstFlags::EMPTY)                                                           \
    _(AND, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                      \
    _(OR, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                       \
    _(XOR, BinaryOp, InstFlags::EMPTY | InstFlags::SYMMETRY)                                      \
    _(ADDI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(SUBI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(MULI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(DIVI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(MODI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(MINI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(MAXI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(SHLI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(SHRI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(ASHRI, BinaryImmOp, InstFlags::EMPTY)                                                       \
    _(ANDI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(ORI, BinaryImmOp, InstFlags::EMPTY)                                                         \
    _(XORI, BinaryImmOp, InstFlags::EMPTY)                                                        \
    _(CMP, CompareOp, InstFlags::EMPTY)                                                           \
    /*_(CAST, CastOp )*/                                                                          \
    /*_(CHECK, CHECK )*/                                                                          \
    /*_(CALL, CALL )*/                                                                            \
    _(PHI, PhiOp, InstFlags::EMPTY)                                                               \
    _(CONST, ConstantOp, InstFlags::EMPTY)                                                        \
    _(PARAM, ParamOp, InstFlags::NO_DCE)                                                          \
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
    /*_(CHECK, CHECK )*/                                                                          \
    /*_(CALL, CALL )*/                                                                            \
    _(ConstantOp)                                                                                 \
    _(ParamOp)                                                                                    \
    _(FixedInputOp0)                                                                              \
    _(FixedInputOp1)                                                                              \
    _(PhiOp)                                                                                      \
    _(IfImmOp)                                                                                    \
    _(IfOp)

#define INSTUCTION_FLAGS(_) _(NO_DCE)