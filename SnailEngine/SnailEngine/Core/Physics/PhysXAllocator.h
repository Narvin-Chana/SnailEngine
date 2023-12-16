#pragma once
#include <memory>

namespace Snail {

using PhysXDeleter = decltype([](auto* p) {
	p->release();
});

template<class T>
using PhysXUniquePtr = std::unique_ptr<T, PhysXDeleter>;
}
