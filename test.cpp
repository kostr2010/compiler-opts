
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

    auto p0 = builder.NewInst(Opcode::PARAM, 0);
    auto p1 = builder.NewInst(Opcode::PARAM, 1);

    auto c0 = builder.NewInst(Opcode::CONST, 10);
    auto c1 = builder.NewInst(Opcode::CONST, 100);

    auto bb0 = builder.NewBlock();
    auto i0 = builder.NewInst(Opcode::ADD);
    auto i1 = builder.NewInst(Opcode::SUB);
    auto i2 = builder.NewInst(Opcode::MUL);

    builder.SetInstInputs(i1, p0, c0);
    builder.SetInstInputs(i2, p1, c1);
    builder.SetInstInputs(i0, i1, i2);

    Derived d;

    foo(&d);
    return 0;
}