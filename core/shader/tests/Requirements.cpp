#include <anybox/shader/Requirements.hpp>
#include <anybox/graphics/container/View.hpp>
#include <anybox/ExceptionTest.hpp>
#include "Fixtures.hpp"
#include "NativeFixtures.hpp"
#include <array>

namespace {
using namespace Fixtures;
using namespace AnyBox::Graphics::Container;

std::vector<std::byte> MakeContainer(std::span<const std::byte> program, bool waves) {
    std::array<std::byte, 8> features{};
    if (waves) features[1] = std::byte{0x40};
    return BuildDxbc(std::array<Part, 2>{{{0x30494653, features}, {0x4C495844, program}}});
}

std::vector<std::byte> MakeNativeProgramPart(std::uint32_t firstWaveSize, std::uint32_t secondWaveSize) {
    auto pack = [](std::uint32_t waveSize) {
        auto program = MakeComputeProgram();
        Set(program, 0xD4, waveSize == 32 ? 0x20 : 0);
        return PackNativeProgram(program);
    };
    auto container = MakeNativeContainer(pack(firstWaveSize), pack(secondWaveSize));
    auto part = DxbcView(container).FindPart(0x4C495844);
    Require(part.has_value());
    return {part->data.begin(), part->data.end()};
}

void CheckNativeWaveSize() {
    using AnyBox::Shader::GetComputeWaveSize;
    Require(GetComputeWaveSize(MakeContainer(MakeNativeProgramPart(32, 32), true)) == 32u);
    Require(GetComputeWaveSize(MakeContainer(MakeNativeProgramPart(64, 64), true)) == 64u);
    Require(!GetComputeWaveSize(MakeContainer(MakeNativeProgramPart(64, 64), false)));
    Reject<std::invalid_argument>([] { GetComputeWaveSize(MakeContainer(MakeNativeProgramPart(32, 64), true)); });
}

void Check() {
    using AnyBox::Shader::GetComputeWaveSize;
    auto program = MakeXboxProgram();
    Require(GetComputeWaveSize(MakeContainer(program, true)) == 32u);
    Require(!GetComputeWaveSize(MakeContainer(program, false)));
    program[120] = std::byte{'0'};
    Require(GetComputeWaveSize(MakeContainer(program, true)) == 64u);
    Require(!GetComputeWaveSize(MakeContainer(program, false)));
    program[120] = std::byte{'2'};
    Reject<std::invalid_argument>([&] { GetComputeWaveSize(MakeContainer(program, true)); });
    Reject<std::invalid_argument>([&] { GetComputeWaveSize(MakeContainer(program, false)); });
    program = MakeXboxProgram();
    Set(program, 112, 999);
    Reject<std::runtime_error>([&] { GetComputeWaveSize(MakeContainer(program, true)); });
    Reject<std::runtime_error>([&] { GetComputeWaveSize(MakeContainer(program, false)); });
    program = MakeXboxProgram();
    Set(program, 28, 1);
    Require(!GetComputeWaveSize(MakeContainer(program, false)));
    Reject<std::runtime_error>([&] { GetComputeWaveSize(MakeContainer(program, true)); });
    auto missing = BuildDxbc(std::array<Part, 1>{{{0x4C495844, program}}});
    Reject<std::invalid_argument>([&] { GetComputeWaveSize(missing); });
    auto ordinary = MakeProgram();
    Require(!GetComputeWaveSize(BuildDxbc(std::array<Part, 1>{{{0x4C495844, ordinary}}})));
    Set(ordinary, 0, 0x65);
    Reject<std::invalid_argument>([&] { GetComputeWaveSize(BuildDxbc(std::array<Part, 1>{{{0x4C495844, ordinary}}})); });
}
}

int main() {
    return AnyBox::Tests::Run([] {
        Check();
        CheckNativeWaveSize();
    });
}
