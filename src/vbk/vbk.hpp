#ifndef PEXA_SRC_VBK_VBK_HPP
#define PEXA_SRC_VBK_VBK_HPP

#include <array>

#include <uint256.h>

namespace VeriBlock {
using KeystoneArray = std::array<uint256, 2>;

const static int32_t POP_BLOCK_VERSION_BIT = 0x80000UL;

} // namespace VeriBlock

#endif