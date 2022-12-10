#define INSTRUCTION_LIST(_)                                                                       \
    _(ADD, BinaryOp, /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                           \
    _(SUB, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(MUL, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(DIV, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(MOD, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(MIN, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(MAX, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(SHL, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(SHR, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(ASHR, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                           \
    _(AND, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(OR, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                             \
    _(XOR, BinaryOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                            \
    _(ADDI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(SUBI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(MULI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(DIVI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(MODI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(MINI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(MAXI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(SHLI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(SHRI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(ASHRI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                       \
    _(ANDI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(ORI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                         \
    _(XORI, BinaryImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(CMP, CompareOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                           \
    /*_(CAST, CastOp )*/                                                                          \
    /*_(CHECK, CHECK )*/                                                                          \
    /*_(CALL, CALL )*/                                                                            \
    _(CONST, ConstantOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                        \
    _(PARAM, ParamOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                           \
    _(RETURN, FixedInputOp1 /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                    \
    _(RETURN_VOID, FixedInputOp0 /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)               \
    _(PHI, PhiOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                               \
    _(IF, IfOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                                 \
    _(IF_IMM, IfImmOp /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)                          \
    _(JMP_IND, FixedInputOp1 /*place info here, f.ex NO_DCE, ALLOCATION, etc*/)

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