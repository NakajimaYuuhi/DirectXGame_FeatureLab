#include "Component.h"
#include <cstdlib>
#include <new>

void* CComponent::operator new(size_t size)
{
    if (void* ptr = std::malloc(size))
    {
        return ptr;
    }
    throw std::bad_alloc();
}

void CComponent::operator delete(void* ptr) noexcept
{
    std::free(ptr);
}
