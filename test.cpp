
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/graph_builder.h"

#include <iostream>

class Base
{
  public:
    virtual void kek()
    {
        std::cout << "base\n";
    }
};

class Derived : public Base
{
  public:
    void kek()
    {
        std::cout << "derived\n";
    }
};

void foo(Base* b)
{
    b->kek();
}

int main()
{
    Graph g;
    GraphBuilder builder(&g);

    auto p0 = builder.NewParameter(0);
    auto p1 = builder.NewParameter(1);

    auto c0 = builder.NewConst(10);
    auto c1 = builder.NewConst(100);

    auto b0 = builder.NewBlock();
    auto i0 = builder.NewInst(Opcode::ADD);
    auto i1 = builder.NewInst(Opcode::SUBI);
    builder.SetInstImm(i1, 10);
    auto i2 = builder.NewInst(Opcode::MUL);

    builder.SetInstInputs(i1, p0, c0);
    builder.SetInstInputs(i2, p1, c1);
    builder.SetInstInputs(i0, i1, i2);

    auto b1 = builder.NewBlock();
    builder.SetSuccessors(b0, { b1 });
    // auto i3 = builder.NewInst(Opcode::MULI);

    auto b2 = builder.NewBlock();
    builder.SetSuccessors(b2, { b1, b0 });
    // auto i4 = builder.NewInst(Opcode::RETURN);

    builder.ConstructCFG();

    Derived d;

    foo(&d);
    return 0;
}