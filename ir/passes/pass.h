#ifndef __PASS_H_INCLUDED__
#define __PASS_H_INCLUDED__

#include "macros.h"
#include "typedefs.h"

class Graph;

class Pass
{
  public:
    template <uint8_t N_BITS>
    struct MarksT
    {
        NO_DEFAULT_CTOR(MarksT);
        NO_DEFAULT_DTOR(MarksT);
        static constexpr uint8_t LEN = N_BITS;

        template <uint8_t OFFT, uint8_t N_TH_BIT>
        static constexpr void Set(MarkHolderT* ptr)
        {
            static_assert(N_TH_BIT < N_BITS);
            static_assert((OFFT + N_TH_BIT) <= (sizeof(MarkHolderT) * BITS_IN_BYTE));
            constexpr MarkHolderT MASK = (1ULL << (OFFT + N_TH_BIT));
            (*ptr) |= MASK;
        }

        template <uint8_t OFFT, uint8_t N_TH_BIT>
        static constexpr void Clear(MarkHolderT* ptr)
        {
            static_assert(N_TH_BIT < N_BITS);
            static_assert((OFFT + N_TH_BIT) <= (sizeof(MarkHolderT) * BITS_IN_BYTE));
            constexpr MarkHolderT MASK = ~(1ULL << (OFFT + N_TH_BIT));
            (*ptr) &= MASK;
        }

        template <uint8_t OFFT, uint8_t N_TH_BIT>
        static constexpr bool Get(const MarkHolderT& ptr)
        {
            static_assert(N_TH_BIT < N_BITS);
            static_assert((OFFT + N_TH_BIT) <= (sizeof(MarkHolderT) * BITS_IN_BYTE));
            constexpr MarkHolderT MASK = (1ULL << (OFFT + N_TH_BIT));
            return ptr & MASK;
        }
    };

    using Marks = MarksT<0>;

    Pass(Graph* g) : graph_{ g }
    {
    }
    DEFAULT_DTOR(Pass);
    GETTER_SETTER(Valid, bool, is_valid_);

    virtual bool RunPass() = 0;

  protected:
    Graph* graph_;

    bool is_valid_ = false;
};

#endif