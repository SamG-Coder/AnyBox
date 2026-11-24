#pragma once

#include "../object/Child.hpp"

namespace AnyBox::Graphics {
inline constexpr Guid QueryHeapId{0x0D9658AE, 0xED45, 0x469E, {0xA6, 0x1D, 0x97, 0x0E, 0xC5, 0x83, 0xCA, 0xB4}};
using QueryHeap = DeviceChild<ID3D12QueryHeap, QueryHeapId, true>;
}
