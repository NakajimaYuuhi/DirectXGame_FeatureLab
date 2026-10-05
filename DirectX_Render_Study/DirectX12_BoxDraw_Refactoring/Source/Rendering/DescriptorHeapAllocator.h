#pragma once
#include <d3d12.h>
#include "imgui.h"

class CDescriptorHeapAllocator
{
public:
    ID3D12DescriptorHeap* Heap = nullptr;
    D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu{ 0 };
    D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu{ 0 };
    UINT                        HeapHandleIncrement = 0;
    UINT                        NumDescriptors = 0;
    ImVector<int>               FreeIndices;

    void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
    {
        IM_ASSERT(Heap == nullptr && FreeIndices.empty());
        Heap = heap;
        D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
        HeapType = desc.Type;
        NumDescriptors = desc.NumDescriptors;
        HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
        HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
        HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
        FreeIndices.reserve((int)desc.NumDescriptors);
        for (int n = desc.NumDescriptors; n > 0; n--)
            FreeIndices.push_back(n - 1);
    }

    void Destroy()
    {
        Heap = nullptr;
        FreeIndices.clear();
        NumDescriptors = 0;
        HeapHandleIncrement = 0;
        HeapStartCpu.ptr = 0;
        HeapStartGpu.ptr = 0;
    }

    void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
    {
        if (FreeIndices.Size <= 0)
        {
            if (out_cpu_desc_handle) out_cpu_desc_handle->ptr = 0;
            if (out_gpu_desc_handle) out_gpu_desc_handle->ptr = 0;
            return;
        }
        int idx = FreeIndices.back();
        FreeIndices.pop_back();
        if (out_cpu_desc_handle) out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
        if (out_gpu_desc_handle) out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
    }

    void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
    {
        // 無効なハンドルやヒープ未初期化の場合は安全に無視
        if (Heap == nullptr || out_cpu_desc_handle.ptr == 0 || out_gpu_desc_handle.ptr == 0) return;
        if (HeapHandleIncrement == 0) return;
        if (out_cpu_desc_handle.ptr < HeapStartCpu.ptr || out_gpu_desc_handle.ptr < HeapStartGpu.ptr) return;

        int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
        int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);

        // インデックス不一致または範囲外の場合は破棄せず無視
        if (cpu_idx != gpu_idx || cpu_idx < 0 || (NumDescriptors > 0 && cpu_idx >= (int)NumDescriptors))
        {
            return;
        }

        FreeIndices.push_back(cpu_idx);
    }
};
