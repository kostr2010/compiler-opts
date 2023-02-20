#include "dce.h"
#include "bb.h"
#include "graph.h"

bool DCE::RunPass()
{
    Mark();
    Sweep();
    ClearMarks();
    return true;
}

void DCE::Mark()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            if (inst->HasFlag(InstFlags::NO_DCE)) {
                MarkRecursively(inst);
            }
        }
    }
}

void DCE::MarkRecursively(Inst* inst)
{
    if (marking::Marker::GetMark<DCE, Marks::VISITED>(*(inst->GetMarkHolder()))) {
        return;
    }

    marking::Marker::SetMark<DCE, Marks::VISITED>(inst->GetMarkHolder());

    for (const auto& i : inst->GetInputs()) {
        MarkRecursively(i.GetInst());
    }
}

void DCE::RemoveInst(Inst* inst)
{
    assert(inst != nullptr);

    for (const auto& i : inst->GetInputs()) {
        if (i.GetInst() != nullptr) {
            i.GetInst()->RemoveUser(inst);
        }
    }

    for (const auto& u : inst->GetUsers()) {
        u.GetInst()->ClearInput(inst);
    }

    inst->GetBasicBlock()->UnlinkInst(inst);
}

void DCE::Sweep()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        auto inst = bb->GetFirstInst();
        while (inst != nullptr) {
            auto next = inst->GetNext();
            if (!marking::Marker::GetMark<DCE, Marks::VISITED>(*(inst->GetMarkHolder()))) {
                RemoveInst(inst);
            }
            inst = next;
        }

        auto phi = bb->GetFirstPhi();
        while (phi != nullptr) {
            auto next = phi->GetNext();
            if (!marking::Marker::GetMark<DCE, Marks::VISITED>(*(phi->GetMarkHolder()))) {
                RemoveInst(phi);
            }
            phi = next;
        }
    }
}

void DCE::ClearMarks()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<RPO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            marking::Marker::ClearMark<DCE, VISITED>(inst->GetMarkHolder());
        }

        for (auto inst = bb->GetFirstPhi(); inst != nullptr; inst = inst->GetNext()) {
            marking::Marker::ClearMark<DCE, VISITED>(inst->GetMarkHolder());
        }
    }
}