#include "dce.h"
#include "bb.h"
#include "graph.h"
#include "marker.h"

bool DCE::RunPass()
{
    Mark();
    Sweep();
    ClearMarks();
    return true;
}

void DCE::Mark()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<PO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            if (inst->HasFlag(Inst::Flags::NO_DCE)) {
                MarkRecursively(inst);
            }
        }
    }
}

void DCE::MarkRecursively(Inst* inst)
{
    if (marking::Marker::GetMark<DCE, Marks::VISITED>(inst)) {
        return;
    }

    marking::Marker::SetMark<DCE, Marks::VISITED>(inst);

    for (const auto& i : inst->GetInputs()) {
        MarkRecursively(i.GetInst());
    }
}

void DCE::RemoveInst(Inst* inst)
{
    assert(inst != nullptr);

    for (const auto& i : inst->GetInputs()) {
        i.GetInst()->RemoveUser(inst);
    }

    inst->GetBasicBlock()->UnlinkInst(inst);
}

void DCE::Sweep()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<PO>()->GetBlocks()) {
        auto inst = bb->GetLastInst();
        while (inst != nullptr) {
            auto prev = inst->GetPrev();
            if (!marking::Marker::GetMark<DCE, Marks::VISITED>(inst)) {
                RemoveInst(inst);
            }
            inst = prev;
        }

        auto phi = bb->GetLastPhi();
        while (phi != nullptr) {
            auto prev = phi->GetPrev();
            if (!marking::Marker::GetMark<DCE, Marks::VISITED>(phi)) {
                RemoveInst(phi);
            }
            phi = prev;
        }
    }
}

void DCE::ClearMarks()
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<PO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            marking::Marker::ClearMark<DCE, VISITED>(inst);
        }

        for (auto inst = bb->GetFirstPhi(); inst != nullptr; inst = inst->GetNext()) {
            marking::Marker::ClearMark<DCE, VISITED>(inst);
        }
    }
}