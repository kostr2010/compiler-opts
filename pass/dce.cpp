#include "dce.h"
#include "ir/bb.h"
#include "ir/graph.h"

bool DCE::Run()
{
    Markers markers{};

    Mark(markers);
    Sweep(markers);

    return true;
}

void DCE::Mark(const Markers markers)
{
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<PO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            if (inst->HasFlag<isa::flag::Type::NO_DCE>()) {
                MarkRecursively(inst, markers);
            }
        }
    }
}

void DCE::MarkRecursively(InstBase* inst, const Markers markers)
{
    if (inst->SetMark(&markers[Marks::VISITED])) {
        return;
    }

    for (const auto& i : inst->GetInputs()) {
        MarkRecursively(i.GetInst(), markers);
    }
}

void DCE::Sweep(const Markers markers)
{
    for (const auto& bb : graph_->GetPassManager()->GetValidPass<PO>()->GetBlocks()) {
        std::set<InstBase*> to_remove{};
        for (auto inst = bb->GetLastInst(); inst != nullptr; inst = inst->GetPrev()) {
            if (!inst->ProbeMark(&markers[Marks::VISITED])) {
                for (const auto& i : inst->GetInputs()) {
                    i.GetInst()->RemoveUser(inst);
                }
                to_remove.insert(inst);
            }
        }

        for (auto phi = bb->GetLastPhi(); phi != nullptr; phi = phi->GetPrev()) {
            if (!phi->ProbeMark(&markers[Marks::VISITED])) {
                for (const auto& i : phi->GetInputs()) {
                    i.GetInst()->RemoveUser(phi);
                }
                to_remove.insert(phi);
            }
        }

        for (auto inst : to_remove) {
            ASSERT(inst != nullptr);
            bb->UnlinkInst(inst);
        }
    }
}
