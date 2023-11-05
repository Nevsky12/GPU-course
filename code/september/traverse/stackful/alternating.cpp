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

        if((i % 2u) != 0u)
        {
            trail.push(2u * i + 1u);
            trail.push(2u * i);
        }
        else
        {
            trail.push(2u * i);
            trail.push(2u * i + 1u);
        }
    }
    std::cout << std::endl;
}
