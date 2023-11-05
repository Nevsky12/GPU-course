#include <iostream>
#include <cstdint>
#include <bit>

using u32 = std::uint32_t;

struct TraverseState
{
    u32 count;
    u32 node;
};
TraverseState initializeTraverse(u32 const count)
{
    return {count, 1u};
}

bool incomplete(TraverseState const state)
{
    return state.node != 0u;
}
u32 rightSibling(u32 const node)
{
    return node == 0u ? 0u : node + 1u;
}
TraverseState proceed(TraverseState const state)
{
    u32 const node = state.node < state.count
        ? state.node * 2u
        : rightSibling(state.node >> std::countr_zero(state.node + 1u));
    return {state.count, node};
}

int main()
{
    TraverseState traverse = initializeTraverse(7u);
    while(incomplete(traverse))
    {
        std::cout << traverse.node - 1u << ' ';
        traverse = proceed(traverse);
    }
    std::cout << std::endl;
}
