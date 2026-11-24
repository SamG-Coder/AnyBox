#include "Tests.hpp"
#include "../../src/fence/Fence.hpp"
#include "../../src/query/QueryHeap.hpp"

void AnyBox::Graphics::Tests::CheckQueryHeaps(Device* device) {
    auto create = Method<decltype(&Device::CreateQueryHeap)>(device, 39);
    void* output = nullptr;
    D3D12_QUERY_HEAP_DESC occlusion{D3D12_QUERY_HEAP_TYPE_OCCLUSION, 2, 0};
    Require(create(device, &occlusion, QueryHeapId, &output) == 0 && output, "Occlusion query heap creation failed");
    Require(Method<decltype(&QueryHeap::Release)>(static_cast<QueryHeap*>(output), 2)(static_cast<QueryHeap*>(output)) == 0, "Occlusion query heap final reference was not released");

    D3D12_QUERY_HEAP_DESC timestamps{D3D12_QUERY_HEAP_TYPE_TIMESTAMP, 4, 1};
    Require(create(device, &timestamps, QueryHeapId, &output) == 0 && output, "Timestamp query heap creation failed");
    auto* heap = static_cast<QueryHeap*>(output);
    auto query = Method<decltype(&QueryHeap::QueryInterface)>(heap, 0);
    auto release = Method<decltype(&QueryHeap::Release)>(heap, 2);
    for (const auto& id : {QueryHeapId, UnknownId, ObjectId, DeviceChildId, PageableId}) {
        void* identity = nullptr;
        Require(query(heap, id, &identity) == 0 && identity == heap, "Query heap COM identity changed");
        Require(release(static_cast<QueryHeap*>(identity)) == 1, "Query heap interface did not own its reference");
    }
    void* parent = nullptr;
    Require(Method<decltype(&QueryHeap::GetDevice)>(heap, 7)(heap, XboxDeviceId, &parent) == 0 && parent == device, "Query heap parent identity changed");
    Method<decltype(&Device::Release)>(device, 2)(static_cast<Device*>(parent));

    constexpr Guid key{0x6A2E7C10, 0x51B4, 0x4E1A, {0x9C, 0x2D, 0x3F, 0x41, 0x7B, 0x88, 0x20, 0x5D}};
    constexpr std::uint32_t stored = 0x51A7E;
    Require(Method<decltype(&QueryHeap::SetPrivateData)>(heap, 4)(heap, key, sizeof(stored), &stored) == 0, "Query heap private data was not stored");
    std::uint32_t loaded = 0;
    std::uint32_t size = sizeof(loaded);
    Require(Method<decltype(&QueryHeap::GetPrivateData)>(heap, 3)(heap, key, &size, &loaded) == 0 && size == sizeof(loaded) && loaded == stored, "Query heap private data did not reach its host heap");

    output = nullptr;
    Reject<std::invalid_argument>([&] { create(device, &timestamps, QueryHeapId, nullptr); });
    Reject<std::invalid_argument>([&] { create(device, nullptr, QueryHeapId, &output); });
    Reject<std::invalid_argument>([&] { create(device, &timestamps, FenceId, &output); });
    auto invalid = timestamps;
    invalid.NodeMask = 2;
    Reject<std::runtime_error>([&] { create(device, &invalid, QueryHeapId, &output); });
    invalid = timestamps;
    invalid.Type = static_cast<D3D12_QUERY_HEAP_TYPE>(6);
    Reject<std::runtime_error>([&] { create(device, &invalid, QueryHeapId, &output); });
    Require(!output, "Rejected query heap creation changed its output");
    Require(release(heap) == 0, "Query heap final reference was not released");
}
