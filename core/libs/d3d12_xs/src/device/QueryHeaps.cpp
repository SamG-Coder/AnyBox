#include "Device.hpp"
#include "../query/QueryHeap.hpp"
#include <memory>
#include <stdexcept>

namespace {
struct QueryHeapDeleter {
    void operator()(ID3D12QueryHeap* value) const noexcept { value->Release(); }
};

bool IsHostQueryHeapType(D3D12_QUERY_HEAP_TYPE type) noexcept {
    switch (type) {
    case D3D12_QUERY_HEAP_TYPE_OCCLUSION:
    case D3D12_QUERY_HEAP_TYPE_TIMESTAMP:
    case D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS:
    case D3D12_QUERY_HEAP_TYPE_SO_STATISTICS:
    case D3D12_QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS:
    case D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP:
    case D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS1:
        return true;
    default:
        return false;
    }
}
}

HRESULT AnyBox::Graphics::Device::CreateQueryHeap(Device* self, const D3D12_QUERY_HEAP_DESC* description, const Guid& interfaceId, void** heap) {
    if (!heap) throw std::invalid_argument("Query heap output is null");
    *heap = nullptr;
    if (!description || !QueryHeap::Supports(interfaceId)) throw std::invalid_argument("Query heap description or interface is invalid");
    if (description->NodeMask > 1) throw std::runtime_error("Query heap node mode is not implemented");
    if (!IsHostQueryHeapType(description->Type)) throw std::runtime_error("Query heap type is not implemented");
    void* host = nullptr;
    auto result = self->host->CreateQueryHeap(description, NativeGuid(QueryHeapId), &host);
    CheckGraphicsResult(result, "ID3D12Device::CreateQueryHeap");
    if (!host) throw std::runtime_error("D3D12 query heap creation returned no interface");
    std::unique_ptr<ID3D12QueryHeap, QueryHeapDeleter> ownedHost(static_cast<ID3D12QueryHeap*>(host));
    auto wrapper = std::make_unique<QueryHeap>(ownedHost.get(), self);
    ownedHost.release();
    *heap = wrapper.release();
    return result;
}
