#include <iostream>
#include <stack>

using u32 = unsigned int;
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

        if(i < leafCount)
        {
            trail.push(2u * i + 1u); // right child
            trail.push(2u * i     ); // left  child
        }
    }
    std::cout << std::endl;
}
