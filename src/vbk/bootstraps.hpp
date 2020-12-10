#ifndef PEXA_SRC_VBK_BOOTSTRAPS_HPP
#define PEXA_SRC_VBK_BOOTSTRAPS_HPP

#include <string>
#include <vector>

#include <primitives/block.h>
#include <util/system.h> // for gArgs
#include <veriblock/config.hpp>

namespace VeriBlock {

extern const int testnetVBKstartHeight;
extern const std::vector<std::string> testnetVBKblocks;

extern const int testnetBTCstartHeight;
extern const std::vector<std::string> testnetBTCblocks;

struct AltChainParamsPEXA : public altintegration::AltChainParams {
    ~AltChainParamsPEXA() override = default;

    AltChainParamsPEXA(const CBlock& genesis)
    {
        auto hash = genesis.GetHash();
        bootstrap.hash = std::vector<uint8_t>{hash.begin(), hash.end()};
        bootstrap.previousBlock = std::vector<uint8_t>{genesis.hashPrevBlock.begin(), genesis.hashPrevBlock.end()};
        bootstrap.height = 0; // pop is enabled starting at genesis
        bootstrap.timestamp = genesis.GetBlockTime();
    }

    altintegration::AltBlock getBootstrapBlock() const noexcept override
    {
        return bootstrap;
    }

    int64_t getIdentifier() const noexcept override { return 0x3ae6ca; }

    std::vector<uint8_t>
    getHash(const std::vector<uint8_t>& bytes) const noexcept override;

    altintegration::AltBlock bootstrap;
};

void printConfig(const altintegration::Config& config);
void selectPopConfig(const ArgsManager& mgr);
void selectPopConfig(const std::string& btcnet, const std::string& vbknet, bool popautoconfig = true, int btcstart = 0, const std::string& btcblocks = {}, int vbkstart = 0, const std::string& vbkblocks = {});

} // namespace VeriBlock

#endif