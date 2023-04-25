#define DISPATCH_TABLE_NAME __DISPATCH_TABLE__

#define GEN_BLOCK_ORDER_FUNCTION(CLASS, PASS)                                                     \
    std::vector<BasicBlock*> CLASS ::BlockOrder() const                                           \
    {                                                                                             \
        return graph_->GetAnalyser()->GetValidPass<PASS>()->GetBlocks();                          \
    }

#define GEN_VISIT_FUNCTIONS(CLASS)                                                                \
    void CLASS ::VisitInstruction(InstBase* inst)                                                 \
    {                                                                                             \
        DISPATCH_TABLE_NAME[static_cast<unsigned>(inst->GetOpcode())](this, inst);                \
    }                                                                                             \
                                                                                                  \
    void CLASS ::VisitBasicBlock(BasicBlock* bb)                                                  \
    {                                                                                             \
        for (auto i = bb->GetFirstInst(); i != nullptr; i = i->GetNext()) {                       \
            VisitInstruction(i);                                                                  \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    void CLASS ::VisitGraph()                                                                     \
    {                                                                                             \
        for (const auto& bb : BlockOrder()) {                                                     \
            VisitBasicBlock(bb);                                                                  \
        }                                                                                         \
    }

#define GEN_VISIT_FUNCTIONS_WITH_BLOCK_ORDER(CLASS, PASS)                                         \
    void CLASS ::VisitInstruction(InstBase* inst)                                                 \
    {                                                                                             \
        DISPATCH_TABLE_NAME[static_cast<unsigned>(inst->GetOpcode())](this, inst);                \
    }                                                                                             \
                                                                                                  \
    void CLASS ::VisitBasicBlock(BasicBlock* bb)                                                  \
    {                                                                                             \
        for (auto i = bb->GetFirstInst(); i != nullptr; i = i->GetNext()) {                       \
            VisitInstruction(i);                                                                  \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    void CLASS ::VisitGraph()                                                                     \
    {                                                                                             \
        for (const auto& bb : BlockOrder()) {                                                     \
            VisitBasicBlock(bb);                                                                  \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    std::vector<BasicBlock*> CLASS ::BlockOrder() const                                           \
    {                                                                                             \
        return graph_->GetAnalyser()->GetValidPass<PASS>()->GetBlocks();                          \
    }

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
