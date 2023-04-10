#include "dce.h"
#include "bb.h"
#include "graph.h"
#include "marker_factory.h"

bool DCE::RunPass()
{
    Markers markers = { marking::MarkerFactory::AcquireMarker() };

    Mark(markers);
    Sweep(markers);

    return true;
}

void DCE::Mark(const Markers markers)
{
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<PO>()->GetBlocks()) {
        for (auto inst = bb->GetFirstInst(); inst != nullptr; inst = inst->GetNext()) {
            if (inst->HasFlag(Inst::Flags::NO_DCE)) {
                MarkRecursively(inst, markers);
            }
        }
    }
}

void DCE::MarkRecursively(Inst* inst, const Markers markers)
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
    for (const auto& bb : graph_->GetAnalyser()->GetValidPass<PO>()->GetBlocks()) {
        auto inst = bb->GetLastInst();
        while (inst != nullptr) {
            auto prev = inst->GetPrev();
            if (!inst->ProbeMark(&markers[Marks::VISITED])) {
                RemoveInst(inst);
            }
            inst = prev;
        }

        auto phi = bb->GetLastPhi();
        while (phi != nullptr) {
            auto prev = phi->GetPrev();
            if (!phi->ProbeMark(&markers[Marks::VISITED])) {
                RemoveInst(phi);
            }
            phi = prev;
        }
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
