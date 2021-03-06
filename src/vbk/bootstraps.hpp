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
        bootstrap.hash = hash.asVector();
        bootstrap.previousBlock = genesis.hashPrevBlock.asVector();
        bootstrap.height = 0; // pop is enabled starting at genesis
        bootstrap.timestamp = genesis.GetBlockTime();
    }

    altintegration::AltBlock getBootstrapBlock() const noexcept override
    {
        return bootstrap;
    }

    int64_t getIdentifier() const noexcept override { return 50657861; }

    std::vector<uint8_t>
    getHash(const std::vector<uint8_t>& bytes) const noexcept override;

    altintegration::AltBlock bootstrap;
};

void printConfig(const altintegration::Config& config);
void selectPopConfig(const ArgsManager& mgr);
void selectPopConfig(const std::string& btcnet, const std::string& vbknet, bool popautoconfig = true, int btcstart = 0, const std::string& btcblocks = {}, int vbkstart = 0, const std::string& vbkblocks = {});

} // namespace VeriBlock

#endif
