#include "graph_builder.h"
#include "isa/isa.h"

GraphBuilder::GraphBuilder(Graph* g)
{
    SetGraph(g);
}

void GraphBuilder::SetGraph(Graph* g)
{
    ASSERT(g != nullptr);
    graph_ = g;

    bb_map_[g->BB_START_ID] = g->GetStartBasicBlock();

    cur_bb_ = g->GetStartBasicBlock();
    cur_bb_id_ = g->BB_START_ID;

    cfg_constructed = false;
    dfg_constructed = false;
}

IdType GraphBuilder::NewParameter()
{
    ASSERT(graph_ != nullptr);
    ASSERT(graph_->GetStartBasicBlock() != nullptr);
    auto last_inst = graph_->GetStartBasicBlock()->GetLastInst();
    if (last_inst != nullptr) {
        // make sure that parameters are passed one after another
        ASSERT(last_inst->IsParam());
    }

    auto inst = InstBase::NewInst<isa::inst::Opcode::PARAM>();

    ASSERT(inst != nullptr);
    auto id = inst->GetId();

    cur_inst_ = inst.get();
    inst_map_[id] = inst.get();

    graph_->GetStartBasicBlock()->PushBackInst(std::move(inst));

    return id;
}

IdType GraphBuilder::NewBlock()
{
    ASSERT(graph_ != nullptr);

    auto bb = graph_->NewBasicBlock();
    auto id = bb->GetId();

    cur_bb_ = bb;
    cur_bb_id_ = id;

    bb_map_[cur_bb_id_] = bb;

    return id;
}

void GraphBuilder::SetSuccessors(IdType bb_id, std::vector<IdType>&& succs)
{
    bb_succ_map_[bb_id] = succs;
}

void GraphBuilder::SetInputs(IdType id, std::vector<std::pair<IdType, IdType> >&& inputs)
{
    ASSERT(inst_map_.find(id) != inst_map_.end());
    ASSERT(inst_map_.at(id)->IsPhi());

    phi_inputs_map_[id].reserve(inputs.size());
    phi_inputs_map_.at(id) = std::move(inputs);
}

void GraphBuilder::SetType(IdType id, InstBase::DataType t)
{
    auto inst = inst_map_[id];
    ASSERT(inst != nullptr);
    inst->SetDataType(t);
}

template <typename ImmT>
static void SetImmediateT(InstBase* i, unsigned pos, ImmType imm)
{
    using NumImm = isa::InputValue<ImmT, isa::input::Type::IMM>;
    STATIC_ASSERT(NumImm::value > 0);
    STATIC_ASSERT(std::is_base_of_v<WithImm<NumImm::value>, ImmT>);

    static_cast<ImmT*>(i)->SetImmediate(pos, imm);
}

void GraphBuilder::SetImmediate(IdType id, unsigned pos, ImmType imm)
{
    auto inst = inst_map_[id];
    ASSERT(inst != nullptr);
    ASSERT(inst->GetNumImms() > 0);
    ASSERT(pos < inst->GetNumImms());
    auto opcode = inst->GetOpcode();

#define GENERATOR(OPCODE, TYPE, ...)                                                              \
    if constexpr (isa::InputValue<isa::inst_type::TYPE, isa::input::Type::IMM>::value > 0) {      \
        if (opcode == isa::inst::Opcode::OPCODE) {                                                \
            SetImmediateT<isa::inst_type::TYPE>(inst, pos, imm);                                  \
            return;                                                                               \
        }                                                                                         \
    }
    ISA_INSTRUCTION_LIST(GENERATOR);
#undef GENERATOR

    UNREACHABLE("trying to set immediate in an instruction with no immediates");
}

template <typename CondT>
static void SetConditionT(InstBase* i, Conditional::Type c)
{
    STATIC_ASSERT(isa::InputValue<CondT, isa::input::Type::COND>::value == true);
    STATIC_ASSERT(std::is_base_of_v<Conditional, CondT>);

    static_cast<CondT*>(i)->SetCondition(c);
}

void GraphBuilder::SetCondition(IdType id, Conditional::Type c)
{
    auto inst = inst_map_[id];
    ASSERT(inst != nullptr);
    ASSERT(inst->IsConditional());
    auto opcode = inst->GetOpcode();

#define GENERATOR(OPCODE, TYPE, ...)                                                              \
    if constexpr (isa::InputValue<isa::inst_type::TYPE, isa::input::Type::COND>::value == true) { \
        if (opcode == isa::inst::Opcode::OPCODE) {                                                \
            SetConditionT<isa::inst_type::TYPE>(inst, c);                                         \
            return;                                                                               \
        }                                                                                         \
    }
    ISA_INSTRUCTION_LIST(GENERATOR);
#undef GENERATOR

    UNREACHABLE("trying to set condition in an instruction with no condition");
}

using NumBranchesDefault =
    std::integral_constant<isa::flag::ValueT,
                           isa::flag::Flag<isa::flag::Type::BRANCH>::Value::ONE_SUCCESSOR>;

template <isa::inst::Opcode OP>
using GetNumBranches = isa::FlagValueOr<OP, isa::flag::Type::BRANCH, NumBranchesDefault::value>;

template <typename INST, typename ACC>
struct BranchNumAccumulator
{
    using BranchNum = GetNumBranches<INST::opcode>;
    static constexpr isa::flag::ValueT value =
        std::conditional_t < ACC::value<BranchNum::value, BranchNum, ACC>::value;
};

using MaxBranchNum = type_sequence::accumulate<isa::ISA, BranchNumAccumulator, NumBranchesDefault>;

void GraphBuilder::ConstructCFG()
{
    for (auto& [bb_id, succs] : bb_succ_map_) {
        ASSERT(succs.size() <= MaxBranchNum::value);
        auto bb = bb_map_.at(bb_id);
        for (unsigned i = 0; i < succs.size(); ++i) {
            auto succ = bb_map_.at(succs[i]);
            graph_->AddEdge(bb, succ, i);
        }
    }

    cfg_constructed = true;
}

void GraphBuilder::ConstructDFG()
{
    for (auto& [inst_id, inputs] : inst_inputs_map_) {
        ASSERT(inst_map_.find(inst_id) != inst_map_.end());
        auto inst = inst_map_.at(inst_id);
        ASSERT(!inst->IsPhi());

        unsigned input_idx = 0;
        for (auto input_id : inputs) {
            if (inst->IsTypeSensitive()) {
                inst->CheckInputType();
            }

            ASSERT(inst_map_.find(input_id) != inst_map_.end());
            auto input_inst = inst_map_.at(input_id);
            if (inst->IsDynamic()) {
                inst->AddInput(input_inst, input_inst->GetBasicBlock());
            } else {
                inst->SetInput(input_idx, input_inst);
            }
            ++input_idx;
        }
    }

    for (auto& [inst_id, inputs] : phi_inputs_map_) {
        ASSERT(inst_map_.find(inst_id) != inst_map_.end());
        auto inst = inst_map_.at(inst_id);
        ASSERT(inst->IsPhi());

        for (auto& input : inputs) {
            auto input_inst_id = input.first;
            auto input_inst_bb = input.second;

            ASSERT(inst_map_.find(input_inst_id) != inst_map_.end());
            auto input_inst = inst_map_.at(input_inst_id);
            auto input_bb = bb_map_.at(input_inst_bb);

            static_cast<isa::inst_type::PHI*>(inst)->AddInput(input_inst, input_bb);
        }
    }

    dfg_constructed = true;
}

bool GraphBuilder::RunChecks()
{
    if (!(cfg_constructed || dfg_constructed)) {
        LOG_ERROR("can't check graph without CFG or DFG!");
    }

    // check inputs of variable length
    auto analyser = graph_->GetPassManager();
    auto rpo = analyser->GetValidPass<RPO>()->GetBlocks();
    for (auto bb : rpo) {
        for (unsigned i = 0; i < bb->GetNumSuccessors(); ++i) {
            if (bb->GetSuccessor(i) == nullptr) {
                LOG_ERROR("BB: " << bb->GetId()
                                 << ", number of successors inferred from the last instruction is "
                                 << bb->GetNumSuccessors() << ". But " << i
                                 << "'th successor is nullptr!");
                return false;
            }
        }

        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            for (unsigned i = 0; i < inst->GetNumInputs(); ++i) {
                auto input = inst->GetInput(i);
                if (input.GetInst() == nullptr) {
                    if (inst->IsDynamic()) {
                        LOG_ERROR("BB: " << bb->GetId() << ", INST_ID: " << inst->GetId()
                                         << ", number of inputs inferred from  input's size is "
                                         << inst->GetNumInputs() << ". But " << i
                                         << "'th input is nullptr!");
                    } else {
                        LOG_ERROR("BB: " << bb->GetId() << ", INST_ID: " << inst->GetId()
                                         << ", number of inputs inferred from the ISA is "
                                         << inst->GetNumInputs() << ". But " << i
                                         << "'th input is nullptr!");
                    }

                    return false;
                }
            }

            for (const auto& user : inst->GetUsers()) {
                if (user.GetInst() == nullptr) {
                    LOG_ERROR("BB: " << bb->GetId() << ", INST_ID: " << inst->GetId() << ", "
                                     << user.GetIdx() << "'th user is nullptr!");

                    return false;
                }
            }
        }

        for (auto phi = bb->GetFirstPhi(); phi != nullptr; phi = phi->GetPrev()) {
            if (phi->GetNumInputs() > bb->GetNumPredecessors()) {
                LOG_ERROR("BB: " << bb->GetId() << ", INST_ID: " << phi->GetId()
                                 << ", number of phi inputs(" << phi->GetNumInputs()
                                 << ") > number of BB predecessors(" << bb->GetNumPredecessors()
                                 << ")");
                return false;
            }

            for (auto input : phi->GetInputs()) {
                if (!input.GetInst()->GetBasicBlock()->Dominates(input.GetSourceBB())) {
                    LOG_ERROR(
                        "BB: "
                        << bb->GetId() << ", INST_ID: " << phi->GetId()
                        << ", phi input's source BB must be dominated by input's original BB");
                    return false;
                }
            }
        }
    }

    // check types

    return true;
}

void GraphBuilder::AddInput(IdType i_id, IdType id)
{
    inst_inputs_map_[i_id].push_back(id);
}