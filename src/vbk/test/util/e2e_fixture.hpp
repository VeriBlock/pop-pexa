#ifndef PEXA_SRC_VBK_TEST_UTIL_E2E_FIXTURE_HPP
#define PEXA_SRC_VBK_TEST_UTIL_E2E_FIXTURE_HPP

#include <boost/test/unit_test.hpp>

#include <chainparams.h>
#include <consensus/validation.h>
#include <test/util/setup_common.h>
#include <txmempool.h>
#include <validation.h>

#include <vbk/bootstraps.hpp>
#include <vbk/pop_service.hpp>
#include <vbk/util.hpp>

#include <veriblock/alt-util.hpp>
#include <veriblock/mempool.hpp>
#include <veriblock/mock_miner.hpp>

using altintegration::ATV;
using altintegration::BtcBlock;
using altintegration::MockMiner;
using altintegration::PublicationData;
using altintegration::VbkBlock;
using altintegration::VTB;

struct TestLogger : public altintegration::Logger {
    ~TestLogger() override = default;

    void log(altintegration::LogLevel lvl, const std::string& msg) override
    {
        fmt::printf("[pop] [%s]\t%s\n", altintegration::LevelToString(lvl),
                    msg);
    }
};

struct E2eFixture : public TestChain100Setup {
    CScript cbKey = CScript()
                    << ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    MockMiner popminer;
    altintegration::ValidationState state_;
    altintegration::PopContext* pop;
    std::vector<uint8_t> defaultPayoutInfo = {1, 2, 3, 4, 5};

    E2eFixture()
    {
        altintegration::SetLogger<TestLogger>();
        altintegration::GetLogger().level = altintegration::LogLevel::warn;

        CScript scriptPubKey =
            CScript() << ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;

        while (!Params().isPopEnabled(ChainActive().Tip()->nHeight)) {
            CBlock b = CreateAndProcessBlock({}, scriptPubKey);
            m_coinbase_txns.push_back(b.vtx[0]);
        }

        pop = &VeriBlock::GetPop();
    }

    void InvalidateTestBlock(CBlockIndex* pblock)
    {
        BlockValidationState state;
        ChainstateActive().InvalidateBlock(state, Params(), pblock);
        ActivateBestChain(state, Params());
        mempool.clear();
    }

    void ReconsiderTestBlock(CBlockIndex* pblock)
    {
        BlockValidationState state;

        {
            LOCK(cs_main);
            ResetBlockFailureFlags(pblock);
        }
        ActivateBestChain(state, Params());
    }

    BtcBlock::hash_t getLastKnownBTCblock()
    {
        LOCK(cs_main);
        auto blocks = VeriBlock::getLastKnownBTCBlocks(1);
        BOOST_CHECK(blocks.size() == 1);
        return blocks[0];
    }

    VbkBlock::hash_t getLastKnownVBKblock()
    {
        LOCK(cs_main);
        auto blocks = VeriBlock::getLastKnownVBKBlocks(1);
        BOOST_CHECK(blocks.size() == 1);
        return blocks[0];
    }

    ATV endorseAltBlock(const uint256& hash, const std::vector<VTB>& vtbs, const std::vector<uint8_t>& payoutInfo)
    {
        CBlockIndex* endorsed = nullptr;
        {
            LOCK(cs_main);
            endorsed = LookupBlockIndex(hash);
            BOOST_CHECK(endorsed != nullptr);
        }

        auto publicationdata = createPublicationData(endorsed, payoutInfo);
        auto vbktx = popminer.createVbkTxEndorsingAltBlock(publicationdata);
        auto atv = popminer.applyATV(vbktx, state_);
        BOOST_CHECK(state_.IsValid());
        return atv;
    }

    ATV endorseAltBlock(const uint256& hash, const std::vector<VTB>& vtbs)
    {
        return endorseAltBlock(hash, vtbs, defaultPayoutInfo);
    }

    CBlock endorseAltBlockAndMine(const std::vector<uint256>& hashes,
                                  size_t generateVtbs = 0)
    {
        return endorseAltBlockAndMine(
            hashes, ChainActive().Tip()->GetBlockHash(), generateVtbs);
    }

    CBlock endorseAltBlockAndMine(const std::vector<uint256>& hashes,
                                  const uint256& prevBlock,
                                  size_t generateVtbs = 0)
    {
        return endorseAltBlockAndMine(hashes, prevBlock, defaultPayoutInfo,
                                      generateVtbs);
    }

    CBlock endorseAltBlockAndMine(const std::vector<uint256>& hashes,
                                  const uint256& prevBlock,
                                  const std::vector<uint8_t>& payoutInfo,
                                  size_t generateVtbs = 0)
    {
        std::vector<VTB> vtbs;
        vtbs.reserve(generateVtbs);
        std::generate_n(std::back_inserter(vtbs), generateVtbs,
                        [&]() { return endorseVbkTip(); });

        BOOST_CHECK_EQUAL(vtbs.size(), generateVtbs);

        std::vector<ATV> atvs;
        atvs.reserve(hashes.size());
        std::transform(hashes.begin(), hashes.end(), std::back_inserter(atvs),
                       [&](const uint256& hash) -> ATV {
                           return endorseAltBlock(hash, {}, payoutInfo);
                       });

        BOOST_CHECK_EQUAL(atvs.size(), hashes.size());
        auto& pop_mempool = *pop->mempool;

        for (const auto& vtb : vtbs) {
            BOOST_CHECK_MESSAGE(pop_mempool.submit(vtb, state_),
                                state_.toString());
        }

        for (const auto& atv : atvs) {
            BOOST_CHECK_MESSAGE(pop_mempool.submit(atv, state_),
                                state_.toString());
        }

        return CreateAndProcessBlock({}, cbKey);
    }

    CBlock endorseAltBlockAndMine(const uint256& hash, const uint256& prevBlock, const std::vector<uint8_t>& payoutInfo, size_t generateVtbs = 0)
    {
        return endorseAltBlockAndMine(std::vector<uint256>{hash}, prevBlock,
                                      payoutInfo, generateVtbs);
    }

    CBlock endorseAltBlockAndMine(const uint256& hash, size_t generateVtbs = 0)
    {
        return endorseAltBlockAndMine(hash, ChainActive().Tip()->GetBlockHash(),
                                      generateVtbs);
    }

    CBlock endorseAltBlockAndMine(const uint256& hash, const uint256& prevBlock, size_t generateVtbs = 0)
    {
        return endorseAltBlockAndMine(hash, prevBlock, defaultPayoutInfo,
                                      generateVtbs);
    }

    VTB endorseVbkTip()
    {
        auto best = popminer.vbk().getBestChain();
        auto tip = best.tip();
        BOOST_CHECK(tip != nullptr);
        return endorseVbkBlock(tip->getHeight());
    }

    VTB endorseVbkBlock(int height)
    {
        auto vbkbest = popminer.vbk().getBestChain();
        auto endorsed = vbkbest[height];
        if (!endorsed) {
            throw std::logic_error("can not find VBK block at height " +
                                   std::to_string(height));
        }

        auto btctx =
            popminer.createBtcTxEndorsingVbkBlock(endorsed->getHeader());
        auto* btccontaining = popminer.mineBtcBlocks(1);
        auto vbktx = popminer.createVbkPopTxEndorsingVbkBlock(
            btccontaining->getHeader(), btctx, endorsed->getHeader(),
            getLastKnownBTCblock());
        auto* vbkcontaining = popminer.mineVbkBlocks(1);

        auto vtbs = popminer.vbkPayloads[vbkcontaining->getHash()];
        BOOST_CHECK(vtbs.size() == 1);
        return vtbs[0];
    }

    PublicationData
    createPublicationData(CBlockIndex* endorsed,
                          const std::vector<uint8_t>& payoutInfo)
    {
        PublicationData p;

        auto& config = *VeriBlock::GetPop().config;
        p.identifier = config.alt->getIdentifier();
        p.payoutInfo = payoutInfo;

        // serialize block header
        CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
        stream << endorsed->GetBlockHeader();
        p.header = std::vector<uint8_t>{stream.begin(), stream.end()};

        return p;
    }

    PublicationData createPublicationData(CBlockIndex* endorsed)
    {
        return createPublicationData(endorsed, defaultPayoutInfo);
    }
};

#endif