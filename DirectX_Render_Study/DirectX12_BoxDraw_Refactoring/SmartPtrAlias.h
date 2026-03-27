#pragma once

#include <memory>

//Uniqueポインタ
template<typename T>
using UniquePtr = std::unique_ptr<T>;

//SharedPtr
template <class T>
using SharedPtr = std::shared_ptr<T>;

//WeakPtr
template <class T>
using WeakPtr = std::weak_ptr<T>;