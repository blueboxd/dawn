// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/dawn/node/binding/GPURenderPipeline.h"

#include <utility>

#include "src/dawn/node/binding/GPUBindGroupLayout.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPURenderPipeline
////////////////////////////////////////////////////////////////////////////////
GPURenderPipeline::GPURenderPipeline(const wgpu::RenderPipelineDescriptor& desc,
                                     wgpu::RenderPipeline pipeline)
    : pipeline_(std::move(pipeline)), label_(desc.label ? desc.label : "") {}

GPURenderPipeline::GPURenderPipeline(wgpu::RenderPipeline pipeline, std::string label)
    : pipeline_(std::move(pipeline)), label_(label) {}

interop::Interface<interop::GPUBindGroupLayout> GPURenderPipeline::getBindGroupLayout(
    Napi::Env env,
    uint32_t index) {
    wgpu::BindGroupLayoutDescriptor desc{};
    return interop::GPUBindGroupLayout::Create<GPUBindGroupLayout>(
        env, desc, pipeline_.GetBindGroupLayout(index));
}

std::string GPURenderPipeline::getLabel(Napi::Env) {
    return label_;
}

void GPURenderPipeline::setLabel(Napi::Env, std::string value) {
    pipeline_.SetLabel(value.c_str());
    label_ = value;
}

}  // namespace wgpu::binding
