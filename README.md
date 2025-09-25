# Simple-Smart-Pointer-Implementation

A custom implementation of smart pointers in C++, featuring `SharedPtr` and `WeakPtr` with reference counting. This project demonstrates the core mechanics behind smart pointers, similar to `std::shared_ptr` and `std::weak_ptr`.

## Features

- **SharedPtr**: Manages the lifetime of an object using reference counting.
- **WeakPtr**: Provides a non-owning reference to an object managed by `SharedPtr`.
- **Control Block**: Handles shared and weak reference counts.
- **Exception Safety**: Basic safety guarantees for operations.
- **Move Semantics**: Support for move construction and assignment.

## Code Example

```cpp
#include "smart_ptr.h"
#include <iostream>

int main() {
    // Create a SharedPtr
    SharedPtr ptr1(new std::string("Hello, World!"));

    // Copy constructor increases ref count
    SharedPtr ptr2 = ptr1;

    // WeakPtr does not increase ref count
    WeakPtr weak = ptr1;

    // Create a SharedPtr from WeakPtr if object exists
    if (SharedPtr locked = weak.Lock()) {
        std::cout << *locked << std::endl; // Output: Hello, World!
    }

    // Reset ptr1 and ptr2, object is destroyed
    ptr1.Reset();
    ptr2.Reset();

    // WeakPtr is now expired
    std::cout << "Weak expired: " << weak.IsExpired() << std::endl; // Output: 1 (true)

    return 0;
}
```
