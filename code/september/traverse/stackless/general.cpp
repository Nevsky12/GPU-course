#include <iostream>
#include <cstdint>
#include <bit>

using u32 = std::uint32_t;

struct TraverseState
{
    u32 count;
    u32 node;
    u32 trail;
};
TraverseState initializeTraverse(u32 const count)
{
    return {count, 1u, 0u};
}

bool incomplete(TraverseState const state)
{
    return state.node != 0u;
}
u32 sibling(u32 const node)
{
    return node == 0u
        ? 0u
        : node + 1u - ((node & 1u) << 1u);
}
TraverseState up(TraverseState const state)
{
    u32 const trail = state.trail + 1u;
    u32 const up = std::countr_zero(trail);
    return
    {
        .count = state.count,
        .node  = sibling(state.node >> up),
        .trail = trail >> up,
    };
}
TraverseState proceed(TraverseState const state)
{
    if(state.node < state.count)
    {
        bool const goLeft  = (state.node % 4u) != 0u;
        bool const goRight = (state.node % 3u) != 0u;

        if(goLeft || goRight)
        {
            u32 const node = goLeft
                ? state.node * 2u
                : state.node * 2u + 1u;
            return
            {
                .count = state.count,
                .node  = node,
                .trail = 2u * state.trail + (goLeft xor goRight ? 1u : 0u),
            };
        }
    }
    return up(state);
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
