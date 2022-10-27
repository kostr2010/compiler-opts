
#include "ir/bb.h"
#include "ir/graph.h"
#include "ir/graph_builder.h"

#include <bitset>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

// file for tomfoolery and experiments

template <uint8_t N_BITS>
struct Mks
{
    NO_DEFAULT_CTOR(Mks);
    NO_DEFAULT_DTOR(Mks);
    static constexpr uint8_t LEN = N_BITS;

    template <uint8_t OFFT, uint8_t N_BIT>
    static constexpr void SetMark(MarkHolderT* ptr)
    {
        static_assert(N_BIT < N_BITS);
        static_assert((OFFT + N_BIT) <= (sizeof(MarkHolderT) * BITS_IN_BYTE));
        (*ptr) |= (1ULL << (OFFT + N_BIT));
    }

    template <uint8_t OFFT, uint8_t N_BIT>
    static constexpr void ClearMark(MarkHolderT* ptr)
    {
        static_assert(N_BIT < N_BITS);
        static_assert((OFFT + N_BIT) <= (sizeof(MarkHolderT) * BITS_IN_BYTE));
        (*ptr) &= ~(1ULL << (OFFT + N_BIT));
    }

    template <uint8_t OFFT, uint8_t N_BIT>
    static constexpr bool GetMark(const MarkHolderT& ptr)
    {
        static_assert(N_BIT < N_BITS);
        static_assert((OFFT + N_BIT) <= (sizeof(MarkHolderT) * BITS_IN_BYTE));
        return (ptr) & (1ULL << (OFFT + N_BIT));
    }
};

struct Base
{
    DEFAULT_CTOR(Base);
    DEFAULT_DTOR(Base);
    virtual void kek() = 0;
};

struct A : public Base
{
    using Marks = Mks<1>;
    DEFAULT_CTOR(A);
    DEFAULT_DTOR(A);
    void kek() override
    {
        std::cout << "A\n";
    }
};

struct B : public Base
{
    using Marks = Mks<2>;
    DEFAULT_CTOR(B);
    DEFAULT_DTOR(B);
    void kek() override
    {
        std::cout << "B\n";
    }
};

struct C : public Base
{
    using Marks = Mks<0>;
    DEFAULT_CTOR(C);
    DEFAULT_DTOR(C);
    void kek() override
    {
        std::cout << "C\n";
    }
};

struct D : public Base
{
    using Marks = Mks<4>;
    DEFAULT_CTOR(D);
    DEFAULT_DTOR(D);
    void kek() override
    {
        std::cout << "D\n";
    }
};

template <typename... Types>
class Psss
{
  public:
    NO_DEFAULT_CTOR(Psss);
    NO_DEFAULT_DTOR(Psss);

    static std::vector<Base*> Allocate()
    {
        std::vector<Base*> vec{};
        vec.reserve(sizeof...(Types));
        (vec.push_back(new Types()), ...);
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
    static constexpr size_t GetMkOfft()
    {
        static_assert(HasPass<Type>());
        size_t i = 0;
        size_t res = 0;
        (((std::is_same<Type, Types>::value) ? (res = i) : (i += Types::Marks::LEN)), ...);
        return res;
    }

    template <typename MkT>
    static constexpr bool HasMk()
    {
        return (std::is_same<MkT, typename Types::Marks>::value || ...);
    }

    template <typename Type>
    static constexpr bool HasPass()
    {
        return (std::is_same<Type, Types>::value || ...);
    }

  private:
};

using PsssList = Psss<A, B, C, D>;

class Ansr
{
  public:
    Ansr()
    {
        bases_ = PsssList::Allocate();
    }
    DEFAULT_DTOR(Ansr);

    template <typename PassT>
    PassT* GetPass()
    {
        static_assert(PsssList::HasPass<PassT>());

        return static_cast<PassT*>(bases_[PsssList::GetIndex<PassT>()]);
    }

    template <typename PassT>
    void RunPass()
    {
        static_assert(PsssList::HasPass<PassT>());

        bases_[PsssList::GetIndex<PassT>()]->kek();
    }

    template <typename PassT, size_t N>
    void SetMark(MarkHolderT* ptr)
    {
        static_assert(PsssList::HasPass<PassT>());
        static_assert(PsssList::HasMk<typename PassT::Marks>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PsssList::GetMkOfft<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PsssList::GetMkOfft<PassT>();
        PassT::Marks::template SetMark<OFFT, N>(ptr);
    }

    template <typename PassT, size_t N>
    void ClearMark(MarkHolderT* ptr)
    {
        static_assert(PsssList::HasPass<PassT>());
        static_assert(PsssList::HasMk<typename PassT::Marks>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PsssList::GetMkOfft<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PsssList::GetMkOfft<PassT>();
        PassT::Marks::template ClearMark<OFFT, N>(ptr);
    }

    template <typename PassT, size_t N>
    bool GetMark(const MarkHolderT& ptr)
    {
        static_assert(PsssList::HasPass<PassT>());
        static_assert(PsssList::HasMk<typename PassT::Marks>());
        static_assert(N < PassT::Marks::LEN);
        static_assert(PsssList::GetMkOfft<PassT>() + N < sizeof(MarkHolderT) * BITS_IN_BYTE);

        constexpr auto OFFT = PsssList::GetMkOfft<PassT>();
        return PassT::Marks::template GetMark<OFFT, N>(ptr);
    }

  private:
    std::vector<Base*> bases_;
};

int main()
{
    MarkHolderT mark{};

    Mks<2>::SetMark<4, 1>(&mark);

    std::cout << mark << "\n";

    Mks<2>::ClearMark<4, 1>(&mark);

    std::cout << mark << "\n";

    Ansr an;

    an.RunPass<A>();
    an.RunPass<B>();
    an.RunPass<C>();
    an.RunPass<D>();

    std::cout << "C: " << PsssList::GetIndex<C>() << "\n";
    std::cout << "A: " << PsssList::GetIndex<A>() << "\n";
    std::cout << "B: " << PsssList::GetIndex<B>() << "\n";
    std::cout << "D: " << PsssList::GetIndex<D>() << "\n";

    std::cout << PsssList::GetMkOfft<A>() << "\n";
    std::cout << PsssList::GetMkOfft<B>() << "\n";
    std::cout << PsssList::GetMkOfft<C>() << "\n";
    std::cout << PsssList::GetMkOfft<D>() << "\n";

    an.SetMark<A, 0>(&mark);
    std::cout << std::bitset<64>{ mark } << "\n";
    an.ClearMark<A, 0>(&mark);
    std::cout << std::bitset<64>{ mark } << "\n";

    an.SetMark<B, 1>(&mark);
    std::cout << std::bitset<64>{ mark } << "\n";

    an.SetMark<D, 2>(&mark);
    std::cout << std::bitset<64>{ mark } << "\n";
    an.ClearMark<D, 2>(&mark);
    std::cout << std::bitset<64>{ mark } << "\n";

    std::cout << an.GetMark<A, 0>(mark) << "\n";
    std::cout << an.GetMark<B, 1>(mark) << "\n";
    std::cout << an.GetMark<D, 2>(mark) << "\n";

    return 0;
}

#pragma GCC diagnostic pop