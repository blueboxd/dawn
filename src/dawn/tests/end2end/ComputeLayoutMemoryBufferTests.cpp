// Copyright 2021 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <algorithm>
#include <array>
#include <functional>
#include <string>
#include <vector>

#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {

// Helper for replacing all occurrences of substr in str with replacement
std::string ReplaceAll(std::string str, const std::string& substr, const std::string& replacement) {
    size_t pos = 0;
    while ((pos = str.find(substr, pos)) != std::string::npos) {
        str.replace(pos, substr.length(), replacement);
        pos += replacement.length();
    }
    return str;
}

// AddressSpace is an enumerator of address spaces used by ComputeLayoutMemoryBufferTests.Fields
enum class AddressSpace {
    Uniform,
    Storage,
};

std::ostream& operator<<(std::ostream& o, AddressSpace addressSpace) {
    switch (addressSpace) {
        case AddressSpace::Uniform:
            o << "uniform";
            break;
        case AddressSpace::Storage:
            o << "storage";
            break;
    }
    return o;
}

// Host-sharable scalar types
enum class ScalarType {
    f32,
    i32,
    u32,
    f16,
};

std::string ScalarTypeName(ScalarType scalarType) {
    switch (scalarType) {
        case ScalarType::f32:
            return "f32";
        case ScalarType::i32:
            return "i32";
        case ScalarType::u32:
            return "u32";
        case ScalarType::f16:
            return "f16";
    }
    UNREACHABLE();
    return "";
}

size_t ScalarTypeSize(ScalarType scalarType) {
    switch (scalarType) {
        case ScalarType::f32:
        case ScalarType::i32:
        case ScalarType::u32:
            return 4;
        case ScalarType::f16:
            return 2;
    }
    UNREACHABLE();
    return 0;
}

// MemoryDataBuilder records and performs operations of following types on a memory buffer `buf`:
//   1. "Align": Align to a alignment `alignment`, which will ensure
//      `buf.size() % alignment == 0` by adding padding bytes into the buffer
//      if necessary;
//   2. "Data": Add `size` bytes of data bytes into buffer;
//   3. "Padding": Add `size` bytes of padding bytes into buffer;
//   4. "FillingFixed": Fill all `size` given (fixed) bytes into the memory buffer.
// Note that data bytes and padding bytes are generated seperatedly and designed to
// be distinguishable, i.e. data bytes have MSB set to 0 while padding bytes 1.
class MemoryDataBuilder {
  public:
    // Record a "Align" operation
    MemoryDataBuilder& AlignTo(uint32_t alignment) {
        mOperations.push_back({OperationType::Align, alignment, {}});
        return *this;
    }

    // Record a "Data" operation
    MemoryDataBuilder& AddData(size_t size) {
        mOperations.push_back({OperationType::Data, size, {}});
        return *this;
    }

    // Record a "Padding" operation
    MemoryDataBuilder& AddPadding(size_t size) {
        mOperations.push_back({OperationType::Padding, size, {}});
        return *this;
    }

    // Record a "FillingFixed" operation
    MemoryDataBuilder& AddFixedBytes(std::vector<uint8_t>& bytes) {
        mOperations.push_back({OperationType::FillingFixed, bytes.size(), bytes});
        return *this;
    }

    // A helper function to record a "FillingFixed" operation with all four bytes of a given U32
    MemoryDataBuilder& AddFixedU32(uint32_t u32) {
        std::vector<uint8_t> bytes;
        bytes.emplace_back((u32 >> 0) & 0xff);
        bytes.emplace_back((u32 >> 8) & 0xff);
        bytes.emplace_back((u32 >> 16) & 0xff);
        bytes.emplace_back((u32 >> 24) & 0xff);
        return AddFixedBytes(bytes);
    }

    // Record all operations that `builder` recorded
    MemoryDataBuilder& AddSubBuilder(MemoryDataBuilder builder) {
        mOperations.insert(mOperations.end(), builder.mOperations.begin(),
                           builder.mOperations.end());
        return *this;
    }

    // Apply all recorded operations, one by one, on a given memory buffer.
    // dataXorKey and paddingXorKey controls the generated data and padding bytes seperatedly, make
    // it possible to, for example, generate two buffers that have different data bytes but
    // identical padding bytes, thus can be used as initializer and expectation bytes of the copy
    // destination buffer, expecting data bytes are changed while padding bytes are left unchanged.
    void ApplyOperationsToBuffer(std::vector<uint8_t>& buffer,
                                 uint8_t dataXorKey,
                                 uint8_t paddingXorKey) {
        uint8_t dataByte = 0x0u;
        uint8_t paddingByte = 0x2u;
        // Get a data byte with MSB set to 0.
        auto NextDataByte = [&]() {
            dataByte += 0x11u;
            return static_cast<uint8_t>((dataByte ^ dataXorKey) & 0x7fu);
        };
        // Get a padding byte with MSB set to 1, distinguished from data bytes.
        auto NextPaddingByte = [&]() {
            paddingByte += 0x13u;
            return static_cast<uint8_t>((paddingByte ^ paddingXorKey) | 0x80u);
        };
        for (auto& operation : mOperations) {
            switch (operation.mType) {
                case OperationType::FillingFixed: {
                    ASSERT(operation.mOperand == operation.mFixedFillingData.size());
                    buffer.insert(buffer.end(), operation.mFixedFillingData.begin(),
                                  operation.mFixedFillingData.end());
                    break;
                }
                case OperationType::Align: {
                    size_t targetSize = Align(buffer.size(), operation.mOperand);
                    size_t paddingSize = targetSize - buffer.size();
                    for (size_t i = 0; i < paddingSize; i++) {
                        buffer.push_back(NextPaddingByte());
                    }
                    break;
                }
                case OperationType::Data: {
                    for (size_t i = 0; i < operation.mOperand; i++) {
                        buffer.push_back(NextDataByte());
                    }
                    break;
                }
                case OperationType::Padding: {
                    for (size_t i = 0; i < operation.mOperand; i++) {
                        buffer.push_back(NextPaddingByte());
                    }
                    break;
                }
            }
        }
    }

    // Create a empty memory buffer and apply all recorded operations one by one on it.
    std::vector<uint8_t> CreateBufferAndApplyOperations(uint8_t dataXorKey = 0u,
                                                        uint8_t paddingXorKey = 0u) {
        std::vector<uint8_t> buffer;
        ApplyOperationsToBuffer(buffer, dataXorKey, paddingXorKey);
        return buffer;
    }

  protected:
    enum class OperationType {
        Align,
        Data,
        Padding,
        FillingFixed,
    };
    struct Operation {
        OperationType mType;
        // mOperand is `alignment` for Align operation, and `size` for Data, Padding, and
        // FillingFixed.
        size_t mOperand;
        // The data that will be filled into buffer if the segment type is FillingFixed. Otherwise
        // for Padding and Data segment, the filling bytes are byte-wise generated based on xor
        // keys.
        std::vector<uint8_t> mFixedFillingData;
    };

    std::vector<Operation> mOperations;
};

// DataMatcherCallback is the callback function by DataMatcher.
// It is called for each contiguous sequence of bytes that should be checked
// for equality.
// offset and size are in units of bytes.
using DataMatcherCallback = std::function<void(uint32_t offset, uint32_t size)>;

// Field describe a type that has contiguous data bytes, e.g. `i32`, `vec2<f32>`, `mat4x4<f32>` or
// `array<f32, 5>`, or have a fixed data stride, e.g. `mat3x3<f32>` or `array<vec3<f32>, 4>`.
// `@size` and `@align` attributes, when used as a struct member, can also described by this struct.
class Field {
  public:
    // Constructor with WGSL type name, natural alignment and natural size. Set mStrideDataBytes to
    // natural size and mStridePaddingBytes to 0 by default to indicate continious data part.
    Field(std::string wgslType, size_t align, size_t size)
        : mWGSLType(wgslType),
          mAlign(align),
          mSize(size),
          mStrideDataBytes(size),
          mStridePaddingBytes(0) {}

    const std::string& GetWGSLType() const { return mWGSLType; }
    size_t GetAlign() const { return mAlign; }
    // The natural size of this field type, i.e. the size without @size attribute
    size_t GetUnpaddedSize() const { return mSize; }
    // The padded size determined by @size attribute if existed, otherwise the natural size
    size_t GetPaddedSize() const { return mHasSizeAttribute ? mPaddedSize : mSize; }

    // Applies a @size attribute, sets the mPaddedSize to value.
    // Returns this Field so calls can be chained.
    Field& SizeAttribute(size_t value) {
        ASSERT(value >= mSize);
        mHasSizeAttribute = true;
        mPaddedSize = value;
        return *this;
    }

    bool HasSizeAttribute() const { return mHasSizeAttribute; }

    // Applies a @align attribute, sets the align to value.
    // Returns this Field so calls can be chained.
    Field& AlignAttribute(size_t value) {
        ASSERT(value >= mAlign);
        ASSERT(IsPowerOfTwo(value));
        mAlign = value;
        mHasAlignAttribute = true;
        return *this;
    }

    bool HasAlignAttribute() const { return mHasAlignAttribute; }

    // Mark that the data part of this field is strided, and record given mStrideDataBytes and
    // mStridePaddingBytes. Returns this Field so calls can be chained.
    Field& Strided(size_t bytesData, size_t bytesPadding) {
        // Check that stride pattern cover the whole data part, i.e. the data part contains N x
        // whole data bytes and N or (N-1) x whole padding bytes.
        ASSERT((mSize % (bytesData + bytesPadding) == 0) ||
               ((mSize + bytesPadding) % (bytesData + bytesPadding) == 0));
        mStrideDataBytes = bytesData;
        mStridePaddingBytes = bytesPadding;
        return *this;
    }

    // Marks that this should only be used for storage buffer tests.
    // Returns this Field so calls can be chained.
    Field& StorageBufferOnly() {
        mStorageBufferOnly = true;
        return *this;
    }

    bool IsStorageBufferOnly() const { return mStorageBufferOnly; }

    // Call the DataMatcherCallback `callback` for continious or strided data bytes, based on the
    // strided information of this field. The callback may be called once or multiple times. Note
    // that padding bytes introduced by @size attribute are not tested.
    void CheckData(DataMatcherCallback callback) const {
        // Calls `callback` with the strided intervals of length mStrideDataBytes, skipping
        // mStridePaddingBytes. For example, for a field of mSize = 18, mStrideDataBytes = 2,
        // and mStridePaddingBytes = 4, calls `callback` with the intervals: [0, 2), [6, 8),
        // [12, 14). If the data is continious, i.e. mStrideDataBytes = 18 and
        // mStridePaddingBytes = 0, `callback` would be called only once with the whole interval
        // [0, 18).
        size_t offset = 0;
        while (offset < mSize) {
            callback(offset, mStrideDataBytes);
            offset += mStrideDataBytes + mStridePaddingBytes;
        }
    }

    // Get a MemoryDataBuilder that do alignment, place data bytes and padding bytes, according to
    // field's alignment, size, padding, and stride information. This MemoryDataBuilder can be used
    // by other MemoryDataBuilder as needed.
    MemoryDataBuilder GetDataBuilder() const {
        MemoryDataBuilder builder;
        builder.AlignTo(mAlign);
        // Check that stride pattern cover the whole data part, i.e. the data part contains N x
        // whole data bytes and N or (N-1) x whole padding bytes. Note that this also handle
        // continious data, i.e. mStrideDataBytes == mSize and mStridePaddingBytes == 0, correctly.
        ASSERT((mSize % (mStrideDataBytes + mStridePaddingBytes) == 0) ||
               ((mSize + mStridePaddingBytes) % (mStrideDataBytes + mStridePaddingBytes) == 0));
        size_t offset = 0;
        while (offset < mSize) {
            builder.AddData(mStrideDataBytes);
            offset += mStrideDataBytes;
            if (offset < mSize) {
                builder.AddPadding(mStridePaddingBytes);
                offset += mStridePaddingBytes;
            }
        }
        if (mHasSizeAttribute) {
            builder.AddPadding(mPaddedSize - mSize);
        }
        return builder;
    }

    // Helper function to build a Field describing a scalar type.
    static Field Scalar(ScalarType type) {
        return Field(ScalarTypeName(type), ScalarTypeSize(type), ScalarTypeSize(type));
    }

    // Helper function to build a Field describing a vector type.
    static Field Vector(uint32_t n, ScalarType type) {
        ASSERT(2 <= n && n <= 4);
        size_t elementSize = ScalarTypeSize(type);
        size_t vectorSize = n * elementSize;
        size_t vectorAlignment = (n == 3 ? 4 : n) * elementSize;
        return Field{"vec" + std::to_string(n) + "<" + ScalarTypeName(type) + ">", vectorAlignment,
                     vectorSize};
    }

    // Helper function to build a Field describing a matrix type.
    static Field Matrix(uint32_t col, uint32_t row, ScalarType type) {
        ASSERT(2 <= col && col <= 4);
        ASSERT(2 <= row && row <= 4);
        ASSERT(type == ScalarType::f32 || type == ScalarType::f16);
        size_t elementSize = ScalarTypeSize(type);
        size_t colVectorSize = row * elementSize;
        size_t colVectorAlignment = (row == 3 ? 4 : row) * elementSize;
        Field field = Field{"mat" + std::to_string(col) + "x" + std::to_string(row) + "<" +
                                ScalarTypeName(type) + ">",
                            colVectorAlignment, col * colVectorAlignment};
        if (colVectorSize != colVectorAlignment) {
            field.Strided(colVectorSize, colVectorAlignment - colVectorSize);
        }
        return field;
    }

  private:
    const std::string mWGSLType;  // Friendly WGSL name of the type of the field
    size_t mAlign;       // Alignment of the type in bytes, can be change by @align attribute
    const size_t mSize;  // Natural size of the type in bytes

    bool mHasAlignAttribute = false;
    bool mHasSizeAttribute = false;
    // Decorated size of the type in bytes indicated by @size attribute, if existed
    size_t mPaddedSize = 0;
    // Whether this type doesn't meet the layout constraints for uniform buffer and thus should only
    // be used for storage buffer tests
    bool mStorageBufferOnly = false;

    // Describe the striding pattern of data part (i.e. the "natural size" part). Note that
    // continious types are described as mStrideDataBytes == mSize and mStridePaddingBytes == 0.
    size_t mStrideDataBytes;
    size_t mStridePaddingBytes;
};

std::ostream& operator<<(std::ostream& o, Field field) {
    o << "@align(" << field.GetAlign() << ") @size(" << field.GetPaddedSize() << ") "
      << field.GetWGSLType();
    return o;
}

// Create a compute pipeline with all buffer in bufferList binded in order starting from slot 0, and
// run the given shader.
void RunComputeShaderWithBuffers(const wgpu::Device& device,
                                 const wgpu::Queue& queue,
                                 const std::string& shader,
                                 std::initializer_list<wgpu::Buffer> bufferList) {
    // Set up shader and pipeline
    auto module = utils::CreateShaderModule(device, shader.c_str());

    wgpu::ComputePipelineDescriptor csDesc;
    csDesc.compute.module = module;
    csDesc.compute.entryPoint = "main";

    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&csDesc);

    // Set up bind group and issue dispatch
    std::vector<wgpu::BindGroupEntry> entries;
    uint32_t bufferSlot = 0;
    for (const wgpu::Buffer& buffer : bufferList) {
        wgpu::BindGroupEntry entry;
        entry.binding = bufferSlot++;
        entry.buffer = buffer;
        entry.offset = 0;
        entry.size = wgpu::kWholeSize;
        entries.push_back(entry);
    }

    wgpu::BindGroupDescriptor descriptor;
    descriptor.layout = pipeline.GetBindGroupLayout(0);
    descriptor.entryCount = static_cast<uint32_t>(entries.size());
    descriptor.entries = entries.data();

    wgpu::BindGroup bindGroup = device.CreateBindGroup(&descriptor);

    wgpu::CommandBuffer commands;
    {
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
        pass.SetPipeline(pipeline);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
        pass.End();

        commands = encoder.Finish();
    }

    queue.Submit(1, &commands);
}

DAWN_TEST_PARAM_STRUCT(ComputeLayoutMemoryBufferTestParams, AddressSpace, Field);

class ComputeLayoutMemoryBufferTests
    : public DawnTestWithParams<ComputeLayoutMemoryBufferTestParams> {
    void SetUp() override { DawnTestBase::SetUp(); }
};

// Align returns the WGSL decoration for an explicit structure field alignment
std::string AlignDeco(uint32_t value) {
    return "@align(" + std::to_string(value) + ") ";
}

// Test different types used as a struct member
TEST_P(ComputeLayoutMemoryBufferTests, StructMember) {
    // Sentinel value markers codes used to check that the start and end of
    // structures are correctly aligned. Each of these codes are distinct and
    // are not likely to be confused with data.
    constexpr uint32_t kDataHeaderCode = 0xa0b0c0a0u;
    constexpr uint32_t kDataFooterCode = 0x40302010u;
    constexpr uint32_t kInputHeaderCode = 0x91827364u;
    constexpr uint32_t kInputFooterCode = 0x19283764u;

    // Status codes returned by the shader.
    constexpr uint32_t kStatusBadInputHeader = 100u;
    constexpr uint32_t kStatusBadInputFooter = 101u;
    constexpr uint32_t kStatusBadDataHeader = 102u;
    constexpr uint32_t kStatusBadDataFooter = 103u;
    constexpr uint32_t kStatusOk = 200u;

    const Field& field = GetParam().mField;

    const bool isUniform = GetParam().mAddressSpace == AddressSpace::Uniform;

    std::string shader = R"(
struct Data {
    header : u32,
    @align({field_align}) @size({field_size}) field : {field_type},
    footer : u32,
}

struct Input {
    header : u32,
    {data_align}data : Data,
    {footer_align}footer : u32,
}

struct Output {
    data : {field_type}
}

struct Status {
    code : u32
}

@group(0) @binding(0) var<{input_qualifiers}> input : Input;
@group(0) @binding(1) var<storage, read_write> output : Output;
@group(0) @binding(2) var<storage, read_write> status : Status;

@compute @workgroup_size(1,1,1)
fn main() {
    if (input.header != {input_header_code}u) {
        status.code = {status_bad_input_header}u;
    } else if (input.footer != {input_footer_code}u) {
        status.code = {status_bad_input_footer}u;
    } else if (input.data.header != {data_header_code}u) {
        status.code = {status_bad_data_header}u;
    } else if (input.data.footer != {data_footer_code}u) {
        status.code = {status_bad_data_footer}u;
    } else {
        status.code = {status_ok}u;
        output.data = input.data.field;
    }
})";

    // https://www.w3.org/TR/WGSL/#alignment-and-size
    // Structure size: roundUp(AlignOf(S), OffsetOf(S, L) + SizeOf(S, L))
    // https://www.w3.org/TR/WGSL/#storage-class-constraints
    // RequiredAlignOf(S, uniform): roundUp(16, max(AlignOf(T0), ..., AlignOf(TN)))
    uint32_t dataAlign = isUniform ? std::max(size_t(16u), field.GetAlign()) : field.GetAlign();

    // https://www.w3.org/TR/WGSL/#structure-layout-rules
    // Note: When underlying the target is a Vulkan device, we assume the device does not support
    // the scalarBlockLayout feature. Therefore, a data value must not be placed in the padding at
    // the end of a structure or matrix, nor in the padding at the last element of an array.
    uint32_t footerAlign = isUniform ? 16 : 4;

    shader = ReplaceAll(shader, "{data_align}", isUniform ? AlignDeco(dataAlign) : "");
    shader = ReplaceAll(shader, "{field_align}", std::to_string(field.GetAlign()));
    shader = ReplaceAll(shader, "{footer_align}", isUniform ? AlignDeco(footerAlign) : "");
    shader = ReplaceAll(shader, "{field_size}", std::to_string(field.GetPaddedSize()));
    shader = ReplaceAll(shader, "{field_type}", field.GetWGSLType());
    shader = ReplaceAll(shader, "{input_header_code}", std::to_string(kInputHeaderCode));
    shader = ReplaceAll(shader, "{input_footer_code}", std::to_string(kInputFooterCode));
    shader = ReplaceAll(shader, "{data_header_code}", std::to_string(kDataHeaderCode));
    shader = ReplaceAll(shader, "{data_footer_code}", std::to_string(kDataFooterCode));
    shader = ReplaceAll(shader, "{status_bad_input_header}", std::to_string(kStatusBadInputHeader));
    shader = ReplaceAll(shader, "{status_bad_input_footer}", std::to_string(kStatusBadInputFooter));
    shader = ReplaceAll(shader, "{status_bad_data_header}", std::to_string(kStatusBadDataHeader));
    shader = ReplaceAll(shader, "{status_bad_data_footer}", std::to_string(kStatusBadDataFooter));
    shader = ReplaceAll(shader, "{status_ok}", std::to_string(kStatusOk));
    shader = ReplaceAll(shader, "{input_qualifiers}",
                        isUniform ? "uniform"  //
                                  : "storage, read_write");

    // Build the input and expected data.
    MemoryDataBuilder inputDataBuilder;  // The whole SSBO data
    {
        inputDataBuilder.AddFixedU32(kInputHeaderCode);  // Input.header
        inputDataBuilder.AlignTo(dataAlign);             // Input.data
        {
            inputDataBuilder.AddFixedU32(kDataHeaderCode);           // Input.data.header
            inputDataBuilder.AddSubBuilder(field.GetDataBuilder());  // Input.data.field
            inputDataBuilder.AddFixedU32(kDataFooterCode);           // Input.data.footer
            inputDataBuilder.AlignTo(field.GetAlign());              // Input.data padding
        }
        inputDataBuilder.AlignTo(footerAlign);           // Input.footer @align
        inputDataBuilder.AddFixedU32(kInputFooterCode);  // Input.footer
        inputDataBuilder.AlignTo(256);                   // Input padding
    }

    MemoryDataBuilder expectedDataBuilder;  // The expected data to be copied by the shader
    expectedDataBuilder.AddSubBuilder(field.GetDataBuilder());

    // Expectation and input buffer have identical data bytes but different padding bytes.
    // Initializes the dst buffer with data bytes different from input and expectation, and padding
    // bytes identical to expectation but different from input.
    constexpr uint8_t dataKeyForInputAndExpectation = 0x00u;
    constexpr uint8_t dataKeyForDstInit = 0xffu;
    constexpr uint8_t paddingKeyForInput = 0x3fu;
    constexpr uint8_t paddingKeyForDstInitAndExpectation = 0x77u;

    std::vector<uint8_t> inputData = inputDataBuilder.CreateBufferAndApplyOperations(
        dataKeyForInputAndExpectation, paddingKeyForInput);
    std::vector<uint8_t> expectedData = expectedDataBuilder.CreateBufferAndApplyOperations(
        dataKeyForInputAndExpectation, paddingKeyForDstInitAndExpectation);
    std::vector<uint8_t> initData = expectedDataBuilder.CreateBufferAndApplyOperations(
        dataKeyForDstInit, paddingKeyForDstInitAndExpectation);

    // Set up input storage buffer
    wgpu::Buffer inputBuf = utils::CreateBufferFromData(
        device, inputData.data(), inputData.size(),
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
            (isUniform ? wgpu::BufferUsage::Uniform : wgpu::BufferUsage::Storage));

    // Set up output storage buffer
    wgpu::Buffer outputBuf = utils::CreateBufferFromData(
        device, initData.data(), initData.size(),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

    // Set up status storage buffer
    wgpu::BufferDescriptor statusDesc;
    statusDesc.size = 4u;
    statusDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer statusBuf = device.CreateBuffer(&statusDesc);

    RunComputeShaderWithBuffers(device, queue, shader, {inputBuf, outputBuf, statusBuf});

    // Check the status
    EXPECT_BUFFER_U32_EQ(kStatusOk, statusBuf, 0) << "status code error" << std::endl
                                                  << "Shader: " << shader;

    // Check the data
    field.CheckData([&](uint32_t offset, uint32_t size) {
        EXPECT_BUFFER_U8_RANGE_EQ(expectedData.data() + offset, outputBuf, offset, size)
            << "offset: " << offset;
    });
}

// Test different types that used directly as buffer type
TEST_P(ComputeLayoutMemoryBufferTests, NonStructMember) {
    auto params = GetParam();
    Field& field = params.mField;
    // @size and @align attribute only apply to struct members, skip them
    if (field.HasSizeAttribute() || field.HasAlignAttribute()) {
        return;
    }

    const bool isUniform = GetParam().mAddressSpace == AddressSpace::Uniform;

    std::string shader = R"(
@group(0) @binding(0) var<{input_qualifiers}> input : {field_type};
@group(0) @binding(1) var<storage, read_write> output : {field_type};

@compute @workgroup_size(1,1,1)
fn main() {
        output = input;
})";

    shader = ReplaceAll(shader, "{field_type}", field.GetWGSLType());
    shader = ReplaceAll(shader, "{input_qualifiers}",
                        isUniform ? "uniform"  //
                                  : "storage, read_write");

    // Build the input and expected data.
    MemoryDataBuilder dataBuilder;
    dataBuilder.AddSubBuilder(field.GetDataBuilder());

    // Expectation and input buffer have identical data bytes but different padding bytes.
    // Initializes the dst buffer with data bytes different from input and expectation, and padding
    // bytes identical to expectation but different from input.
    constexpr uint8_t dataKeyForInputAndExpectation = 0x00u;
    constexpr uint8_t dataKeyForDstInit = 0xffu;
    constexpr uint8_t paddingKeyForInput = 0x3fu;
    constexpr uint8_t paddingKeyForDstInitAndExpectation = 0x77u;

    std::vector<uint8_t> inputData = dataBuilder.CreateBufferAndApplyOperations(
        dataKeyForInputAndExpectation, paddingKeyForInput);
    std::vector<uint8_t> expectedData = dataBuilder.CreateBufferAndApplyOperations(
        dataKeyForInputAndExpectation, paddingKeyForDstInitAndExpectation);
    std::vector<uint8_t> initData = dataBuilder.CreateBufferAndApplyOperations(
        dataKeyForDstInit, paddingKeyForDstInitAndExpectation);

    // Set up input storage buffer
    wgpu::Buffer inputBuf = utils::CreateBufferFromData(
        device, inputData.data(), inputData.size(),
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
            (isUniform ? wgpu::BufferUsage::Uniform : wgpu::BufferUsage::Storage));
    EXPECT_BUFFER_U8_RANGE_EQ(inputData.data(), inputBuf, 0, inputData.size());

    // Set up output storage buffer
    wgpu::Buffer outputBuf = utils::CreateBufferFromData(
        device, initData.data(), initData.size(),
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);
    EXPECT_BUFFER_U8_RANGE_EQ(initData.data(), outputBuf, 0, initData.size());

    RunComputeShaderWithBuffers(device, queue, shader, {inputBuf, outputBuf});

    // Check the data
    field.CheckData([&](uint32_t offset, uint32_t size) {
        EXPECT_BUFFER_U8_RANGE_EQ(expectedData.data() + offset, outputBuf, offset, size)
            << "offset: " << offset;
    });
}

auto GenerateParams() {
    auto params = MakeParamGenerator<ComputeLayoutMemoryBufferTestParams>(
        {
            D3D12Backend(),
            MetalBackend(),
            VulkanBackend(),
            OpenGLBackend(),
            OpenGLESBackend(),
        },
        {AddressSpace::Storage, AddressSpace::Uniform},
        {
            // See https://www.w3.org/TR/WGSL/#alignment-and-size
            // Scalar types with no custom alignment or size
            Field::Scalar(ScalarType::f32),
            Field::Scalar(ScalarType::i32),
            Field::Scalar(ScalarType::u32),

            // Scalar types with custom alignment
            Field::Scalar(ScalarType::f32).AlignAttribute(16),
            Field::Scalar(ScalarType::i32).AlignAttribute(16),
            Field::Scalar(ScalarType::u32).AlignAttribute(16),

            // Scalar types with custom size
            Field::Scalar(ScalarType::f32).SizeAttribute(24),
            Field::Scalar(ScalarType::i32).SizeAttribute(24),
            Field::Scalar(ScalarType::u32).SizeAttribute(24),

            // Vector types with no custom alignment or size
            Field::Vector(2, ScalarType::f32),
            Field::Vector(3, ScalarType::f32),
            Field::Vector(4, ScalarType::f32),
            Field::Vector(2, ScalarType::i32),
            Field::Vector(3, ScalarType::i32),
            Field::Vector(4, ScalarType::i32),
            Field::Vector(2, ScalarType::u32),
            Field::Vector(3, ScalarType::u32),
            Field::Vector(4, ScalarType::u32),

            // Vector types with custom alignment
            Field::Vector(2, ScalarType::f32).AlignAttribute(32),
            Field::Vector(3, ScalarType::f32).AlignAttribute(32),
            Field::Vector(4, ScalarType::f32).AlignAttribute(32),
            Field::Vector(2, ScalarType::i32).AlignAttribute(32),
            Field::Vector(3, ScalarType::i32).AlignAttribute(32),
            Field::Vector(4, ScalarType::i32).AlignAttribute(32),
            Field::Vector(2, ScalarType::u32).AlignAttribute(32),
            Field::Vector(3, ScalarType::u32).AlignAttribute(32),
            Field::Vector(4, ScalarType::u32).AlignAttribute(32),

            // Vector types with custom size
            Field::Vector(2, ScalarType::f32).SizeAttribute(24),
            Field::Vector(3, ScalarType::f32).SizeAttribute(24),
            Field::Vector(4, ScalarType::f32).SizeAttribute(24),
            Field::Vector(2, ScalarType::i32).SizeAttribute(24),
            Field::Vector(3, ScalarType::i32).SizeAttribute(24),
            Field::Vector(4, ScalarType::i32).SizeAttribute(24),
            Field::Vector(2, ScalarType::u32).SizeAttribute(24),
            Field::Vector(3, ScalarType::u32).SizeAttribute(24),
            Field::Vector(4, ScalarType::u32).SizeAttribute(24),

            // Matrix types with no custom alignment or size
            Field::Matrix(2, 2, ScalarType::f32),
            Field::Matrix(3, 2, ScalarType::f32),
            Field::Matrix(4, 2, ScalarType::f32),
            Field::Matrix(2, 3, ScalarType::f32),
            Field::Matrix(3, 3, ScalarType::f32),
            Field::Matrix(4, 3, ScalarType::f32),
            Field::Matrix(2, 4, ScalarType::f32),
            Field::Matrix(3, 4, ScalarType::f32),
            Field::Matrix(4, 4, ScalarType::f32),

            // Matrix types with custom alignment
            Field::Matrix(2, 2, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(3, 2, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(4, 2, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(2, 3, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(3, 3, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(4, 3, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(2, 4, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(3, 4, ScalarType::f32).AlignAttribute(32),
            Field::Matrix(4, 4, ScalarType::f32).AlignAttribute(32),

            // Matrix types with custom size
            Field::Matrix(2, 2, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(3, 2, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(4, 2, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(2, 3, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(3, 3, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(4, 3, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(2, 4, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(3, 4, ScalarType::f32).SizeAttribute(128),
            Field::Matrix(4, 4, ScalarType::f32).SizeAttribute(128),

            // Array types with no custom alignment or size.
            // Note: The use of StorageBufferOnly() is due to UBOs requiring 16 byte alignment
            // of array elements. See https://www.w3.org/TR/WGSL/#storage-class-constraints
            Field("array<u32, 1>", /* align */ 4, /* size */ 4).StorageBufferOnly(),
            Field("array<u32, 2>", /* align */ 4, /* size */ 8).StorageBufferOnly(),
            Field("array<u32, 3>", /* align */ 4, /* size */ 12).StorageBufferOnly(),
            Field("array<u32, 4>", /* align */ 4, /* size */ 16).StorageBufferOnly(),
            Field("array<vec2<u32>, 1>", /* align */ 8, /* size */ 8).StorageBufferOnly(),
            Field("array<vec2<u32>, 2>", /* align */ 8, /* size */ 16).StorageBufferOnly(),
            Field("array<vec2<u32>, 3>", /* align */ 8, /* size */ 24).StorageBufferOnly(),
            Field("array<vec2<u32>, 4>", /* align */ 8, /* size */ 32).StorageBufferOnly(),
            Field("array<vec3<u32>, 1>", /* align */ 16, /* size */ 16).Strided(12, 4),
            Field("array<vec3<u32>, 2>", /* align */ 16, /* size */ 32).Strided(12, 4),
            Field("array<vec3<u32>, 3>", /* align */ 16, /* size */ 48).Strided(12, 4),
            Field("array<vec3<u32>, 4>", /* align */ 16, /* size */ 64).Strided(12, 4),
            Field("array<vec4<u32>, 1>", /* align */ 16, /* size */ 16),
            Field("array<vec4<u32>, 2>", /* align */ 16, /* size */ 32),
            Field("array<vec4<u32>, 3>", /* align */ 16, /* size */ 48),
            Field("array<vec4<u32>, 4>", /* align */ 16, /* size */ 64),

            // Array types with custom alignment
            Field("array<u32, 1>", /* align */ 4, /* size */ 4)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<u32, 2>", /* align */ 4, /* size */ 8)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<u32, 3>", /* align */ 4, /* size */ 12)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<u32, 4>", /* align */ 4, /* size */ 16)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<vec2<u32>, 1>", /* align */ 8, /* size */ 8)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<vec2<u32>, 2>", /* align */ 8, /* size */ 16)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<vec2<u32>, 3>", /* align */ 8, /* size */ 24)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<vec2<u32>, 4>", /* align */ 8, /* size */ 32)
                .AlignAttribute(32)
                .StorageBufferOnly(),
            Field("array<vec3<u32>, 1>", /* align */ 16, /* size */ 16)
                .AlignAttribute(32)
                .Strided(12, 4),
            Field("array<vec3<u32>, 2>", /* align */ 16, /* size */ 32)
                .AlignAttribute(32)
                .Strided(12, 4),
            Field("array<vec3<u32>, 3>", /* align */ 16, /* size */ 48)
                .AlignAttribute(32)
                .Strided(12, 4),
            Field("array<vec3<u32>, 4>", /* align */ 16, /* size */ 64)
                .AlignAttribute(32)
                .Strided(12, 4),
            Field("array<vec4<u32>, 1>", /* align */ 16, /* size */ 16).AlignAttribute(32),
            Field("array<vec4<u32>, 2>", /* align */ 16, /* size */ 32).AlignAttribute(32),
            Field("array<vec4<u32>, 3>", /* align */ 16, /* size */ 48).AlignAttribute(32),
            Field("array<vec4<u32>, 4>", /* align */ 16, /* size */ 64).AlignAttribute(32),

            // Array types with custom size
            Field("array<u32, 1>", /* align */ 4, /* size */ 4)
                .SizeAttribute(128)
                .StorageBufferOnly(),
            Field("array<u32, 2>", /* align */ 4, /* size */ 8)
                .SizeAttribute(128)
                .StorageBufferOnly(),
            Field("array<u32, 3>", /* align */ 4, /* size */ 12)
                .SizeAttribute(128)
                .StorageBufferOnly(),
            Field("array<u32, 4>", /* align */ 4, /* size */ 16)
                .SizeAttribute(128)
                .StorageBufferOnly(),
            Field("array<vec3<u32>, 4>", /* align */ 16, /* size */ 64)
                .SizeAttribute(128)
                .Strided(12, 4),
        });

    std::vector<ComputeLayoutMemoryBufferTestParams> filtered;
    for (auto param : params) {
        if (param.mAddressSpace != AddressSpace::Storage && param.mField.IsStorageBufferOnly()) {
            continue;
        }
        filtered.emplace_back(param);
    }
    return filtered;
}

INSTANTIATE_TEST_SUITE_P(,
                         ComputeLayoutMemoryBufferTests,
                         ::testing::ValuesIn(GenerateParams()),
                         DawnTestBase::PrintToStringParamName("ComputeLayoutMemoryBufferTests"));
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ComputeLayoutMemoryBufferTests);

}  // namespace
