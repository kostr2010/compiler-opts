#define DISPATCH_TABLE_NAME __DISPATCH_TABLE__

#define GEN_BLOCK_ORDER_FUNCTION(CLASS, PASS)                                                     \
    std::vector<BasicBlock*> CLASS ::BlockOrder() const                                           \
    {                                                                                             \
        return graph_->GetPassManager()->GetValidPass<PASS>()->GetBlocks();                       \
    }

#define GEN_DEFAULT_VISIT_INSTRUCTION_FUNCTION(CLASS)                                             \
    void CLASS ::VisitInstruction(InstBase* inst)                                                 \
    {                                                                                             \
        DISPATCH_TABLE_NAME[static_cast<unsigned>(inst->GetOpcode())](this, inst);                \
    }

#define GEN_DEFAULT_VISIT_BASIC_BLOCK_FUNCTION(CLASS)                                             \
    void CLASS ::VisitBasicBlock(BasicBlock* bb)                                                  \
    {                                                                                             \
        for (auto i = bb->GetFirstInst(); i != nullptr; i = i->GetNext()) {                       \
            VisitInstruction(i);                                                                  \
        }                                                                                         \
        for (auto i = bb->GetFirstPhi(); i != nullptr; i = i->GetNext()) {                        \
            VisitInstruction(i);                                                                  \
        }                                                                                         \
    }

#define GEN_DEFAULT_VISIT_GRAPH_FUNCTION(CLASS)                                                   \
    void CLASS ::VisitGraph()                                                                     \
    {                                                                                             \
        for (const auto& bb : BlockOrder()) {                                                     \
            VisitBasicBlock(bb);                                                                  \
        }                                                                                         \
    }

#define GEN_DEFAULT_VISIT_FUNCTIONS(CLASS, PASS)                                                  \
    GEN_DEFAULT_VISIT_INSTRUCTION_FUNCTION(CLASS);                                                \
    GEN_DEFAULT_VISIT_BASIC_BLOCK_FUNCTION(CLASS);                                                \
    GEN_DEFAULT_VISIT_GRAPH_FUNCTION(CLASS);                                                      \
    GEN_BLOCK_ORDER_FUNCTION(CLASS, PASS);

public:
void VisitInstruction(InstBase* inst) override;
void VisitBasicBlock(BasicBlock* bb) override;
void VisitGraph() override;
std::vector<BasicBlock*> BlockOrder() const override;

private:
using VisitFunc = void (*)(GraphVisitor*, InstBase*);
static constexpr VisitFunc DISPATCH_TABLE_NAME[] = {
#define GEN_DISPATCH_TABLE(OPCODE, ...) Visit##OPCODE,
    ISA_INSTRUCTION_LIST(GEN_DISPATCH_TABLE)
#undef GEN_DISPATCH_TABLE
};
