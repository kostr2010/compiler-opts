#include "ir/passes/passes.h"
#include "utils/macros.h"

#include <iostream>
#include <utility>
#include <vector>

// forward-decl for classes
#define MAKE_PASSES_FORWARD_DECL(pass, ...) class pass;
PASSES(MAKE_PASSES_FORWARD_DECL)
#undef MAKE_PASSES_FORWARD_DECL

class Graph
{
  public:
    DEFAULT_CTOR(Graph);
    DEFAULT_DTOR(Graph);

    int a{ 10 };
};

class Pass
{
  public:
    Pass(Graph* g) : graph_{ g }
    {
    }
    DEFAULT_DTOR(Pass);

    virtual bool RunPass() = 0;

  protected:
    Graph* graph_;
};

class RPO : public Pass
{
  public:
    SINGLETON_GET_INSTANCE_IMPLEMENT(RPO);
    NO_COPY_CTOR(RPO);
    NO_DEFAULT_CTOR(RPO);
    DEFAULT_DTOR(RPO);

    bool RunPass() override
    {
        std::cout << "RPO\n";
        std::cout << graph_->a << "\n";
        return true;
    }

    void func_RPO()
    {
        std::cout << "rpo function\n";
    }

  private:
    SINGLETON_CTOR(RPO);
};

class DFS : public Pass
{
  public:
    SINGLETON_GET_INSTANCE_IMPLEMENT(DFS);
    NO_COPY_CTOR(DFS);
    NO_DEFAULT_CTOR(DFS);
    DEFAULT_DTOR(DFS);

    bool RunPass() override
    {
        std::cout << "DFS\n";
        std::cout << graph_->a << "\n";
        return true;
    }

    void func_DFS()
    {
        std::cout << "dfs function\n";
    }

  private:
    SINGLETON_CTOR(DFS);
};

template <typename... Types>
class Passes
{
  public:
    NO_DEFAULT_CTOR(Passes);
    NO_DEFAULT_DTOR(Passes);

    template <typename... Args>
    static std::vector<Pass*> Allocate(Graph* graph)
    {
        std::vector<Pass*> vec{};
        vec.reserve(sizeof...(Types));
        (vec.push_back(Types::SINGLETON_GET_INSTANCE(graph)), ...);
        return vec;
    }

    template <typename Type>
    static constexpr size_t GetIndex()
    {
        static_assert(HasPass<Type>());

        size_t i = 0;
        size_t res = 0;
        (((std::is_same<Type, Types>::value) ? (res = i) : (++i)), ...);
        return res;
    }

    template <typename Type>
    static constexpr bool HasPass()
    {
        return (std::is_same<Type, Types>::value || ...);
    }

  private:
};

using PassesList = Passes<RPO, DFS>;

class Analyser
{

  public:
    Analyser(Graph* graph)
    {
        passes_ = PassesList::Allocate(graph);
    }
    DEFAULT_DTOR(Analyser);

    template <typename PassT>
    PassT* GetPass()
    {
        static_assert(PassesList::HasPass<PassT>());

        return PassT::SINGLETON_GET_INSTANCE();
    }

    std::vector<Pass*> passes_;
};

int main()
{
    std::cout << "index of DFS:" << PassesList::GetIndex<DFS>() << "\n";
    std::cout << "index of RPO:" << PassesList::GetIndex<RPO>() << "\n";

    Graph g;
    Analyser a(&g);

    a.GetPass<RPO>()->func_RPO();
    a.GetPass<DFS>()->func_DFS();
    // a.GetPass<int>(); // failure of static assert

    for (auto pass : a.passes_) {
        pass->RunPass();
    }

    for (auto pass : a.passes_) {
        pass->RunPass();
    }

    for (auto pass : a.passes_) {
        pass->RunPass();
    }
}