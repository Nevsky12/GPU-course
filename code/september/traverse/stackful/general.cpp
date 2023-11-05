#include <iostream>
#include <cstdint>
#include <stack>

using u32 = std::uint32_t;

int main()
{
    u32 const leafCount = 7u;
    
    std::stack<u32> trail;

    trail.push(1u);
    while(!trail.empty())
    {
        u32 const i = trail.top();
        trail.pop();

        std::cout << i - 1u << ' ';
        if(i >= leafCount)
            continue;

        bool const goLeft  = (i % 4u) != 0u;
        bool const goRight = (i % 3u) != 0u;
        if(goRight)
            trail.push(2u * i + 1u);
        if(goLeft)
            trail.push(2u * i);
    }
    std::cout << std::endl;
}
