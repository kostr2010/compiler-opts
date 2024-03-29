#define ISA_INPUT_TYPE_LIST(GENERATOR)                                                            \
    GENERATOR(VREG, 0U)                                                                           \
    GENERATOR(IMM, 0U)                                                                            \
    GENERATOR(COND, false)                                                                        \
    GENERATOR(DYN, false)

#define ISA_INSTRUCTION_TYPE_LIST(GENERATOR)                                                      \
    GENERATOR(NO_INPUT, /* NO INPUTS */)                                                          \
    GENERATOR(UNARY, (VREG, 1U))                                                                  \
    GENERATOR(BINARY, (VREG, 2U))                                                                 \
    GENERATOR(BIN_IMM, (VREG, 1U)(IMM, 1U))                                                       \
    GENERATOR(PHI, (DYN, true))                                                                   \
    GENERATOR(CALL, (DYN, true))                                                                  \
    GENERATOR(IF, (VREG, 2U)(COND, true))                                                         \
    GENERATOR(IF_IMM, (VREG, 1U)(IMM, 1U)(COND, true))                                            \
    GENERATOR(COMPARE, (VREG, 2U)(COND, true))                                                    \
    GENERATOR(CONST, /* NO INPUTS */)

#define ISA_FLAG_LIST(GENERATOR)                                                                  \
    GENERATOR(BRANCH, (NO_SUCCESSORS)(ONE_SUCCESSOR)(TWO_SUCCESSORS), ) /* inst controls CFG */   \
    GENERATOR(SYMMETRICAL, /* NO VALUES */, ) /* inst in1, in2 == inst in2, in1 */                \
    GENERATOR(CALL, /* NO VALUES */, )        /* instruction contains callee graph */             \
    GENERATOR(CHECK, /* NO VALUES */, )       /* instruction is check */                          \
    GENERATOR(NO_DCE, /* NO VALUES */, )      /* instruction can't be eliminated in dce */        \
    GENERATOR(NO_USE, /* NO VALUES */, )      /* instruction can't have users */

#define ISA_INSTRUCTION_LIST(GENERATOR)                                                           \
    GENERATOR(ADD, BINARY, (SYMMETRICAL), )                                                       \
    GENERATOR(SUB, BINARY, /* NO FLAGS */, )                                                      \
    GENERATOR(MUL, BINARY, (SYMMETRICAL))                                                         \
    GENERATOR(DIV, BINARY, /* NO FLAGS */, )                                                      \
    GENERATOR(MOD, BINARY, /* NO FLAGS */, )                                                      \
    GENERATOR(MIN, BINARY, (SYMMETRICAL))                                                         \
    GENERATOR(MAX, BINARY, (SYMMETRICAL))                                                         \
    GENERATOR(SHL, BINARY, /* NO FLAGS */, )                                                      \
    GENERATOR(SHR, BINARY, /* NO FLAGS */, )                                                      \
    GENERATOR(ASHR, BINARY, /* NO FLAGS */, )                                                     \
    GENERATOR(AND, BINARY, (SYMMETRICAL), )                                                       \
    GENERATOR(OR, BINARY, (SYMMETRICAL), )                                                        \
    GENERATOR(XOR, BINARY, (SYMMETRICAL), )                                                       \
    GENERATOR(ADDI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(SUBI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(MULI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(DIVI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(MODI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(MINI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(MAXI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(SHLI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(SHRI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(ASHRI, BIN_IMM, /* NO FLAGS */, )                                                   \
    GENERATOR(ANDI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(ORI, BIN_IMM, /* NO FLAGS */, )                                                     \
    GENERATOR(XORI, BIN_IMM, /* NO FLAGS */, )                                                    \
    GENERATOR(CMP, COMPARE, /* NO FLAGS */, )                                                     \
    GENERATOR(CHECK_ZERO, UNARY, (CHECK)(NO_DCE)(NO_USE), )                                       \
    GENERATOR(CHECK_NULL, UNARY, (CHECK)(NO_DCE)(NO_USE), )                                       \
    GENERATOR(CHECK_SIZE, BINARY, (CHECK)(NO_DCE)(NO_USE), )                                      \
    /*_(CAST, CastOp )*/                                                                          \
    /*_(CALL_DYNAMIC, CALL, (CALL) | (NO_DCE))*/                                                  \
    GENERATOR(CALL_STATIC, CALL, (CALL)(NO_DCE), )                                                \
    GENERATOR(PHI, PHI, /* NO FLAGS */, )                                                         \
    GENERATOR(CONST, CONST, /* NO FLAGS */, )                                                     \
    GENERATOR(PARAM, NO_INPUT, (NO_DCE), )                                                        \
    GENERATOR(RETURN, UNARY, (BRANCH, NO_SUCCESSORS)(NO_DCE)(NO_USE), )                           \
    GENERATOR(RETURN_VOID, NO_INPUT, (BRANCH, NO_SUCCESSORS)(NO_DCE)(NO_USE), )                   \
    GENERATOR(IF_IMM, IF_IMM, (BRANCH, TWO_SUCCESSORS)(NO_DCE)(NO_USE), )                         \
    GENERATOR(IF, IF, (BRANCH, TWO_SUCCESSORS)(NO_DCE)(NO_USE), )                                 \
    GENERATOR(JMP, NO_INPUT, (BRANCH, ONE_SUCCESSOR)(NO_DCE)(NO_USE), )
