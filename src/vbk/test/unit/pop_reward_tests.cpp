// Copyright (c) 2019-2020 Xenios SEZC
// https://www.veriblock.org
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>
#include <script/interpreter.h>
#include <vbk/test/util/e2e_fixture.hpp>

struct PopRewardsTestFixture : public E2eFixture {
};

BOOST_AUTO_TEST_SUITE(pop_reward_tests)

BOOST_FIXTURE_TEST_CASE(addPopPayoutsIntoCoinbaseTx_test, PopRewardsTestFixture)
{
    CScript scriptPubKey = CScript() << ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;

    auto tip = ChainActive().Tip();
    BOOST_CHECK(tip != nullptr);
    std::vector<uint8_t> payoutInfo{scriptPubKey.begin(), scriptPubKey.end()};
    CBlock block = endorseAltBlockAndMine(tip->GetBlockHash(), tip->GetBlockHash(), payoutInfo, 0);
    {
        LOCK(cs_main);
        BOOST_CHECK(ChainActive().Tip()->GetBlockHash() == block.GetHash());
    }

    // Generate a chain whith rewardInterval of blocks
    int rewardInterval = (int)VeriBlock::GetPop().config->alt->getPayoutParams().getPopPayoutDelay();
    // do not add block with rewards
    // do not add block before block with rewards
    for (int i = 0; i < (rewardInterval - 2); i++) {
        CBlock b = CreateAndProcessBlock({}, scriptPubKey);
        m_coinbase_txns.push_back(b.vtx[0]);
    }

    CBlock beforePayoutBlock = CreateAndProcessBlock({}, scriptPubKey);

    int n = 0;
    for (const auto& out : beforePayoutBlock.vtx[0]->vout) {
        if (out.nValue > 0) n++;
    }
    BOOST_CHECK(n == 1);

    CBlock payoutBlock = CreateAndProcessBlock({}, scriptPubKey);
    n = 0;
    for (const auto& out : payoutBlock.vtx[0]->vout) {
        if (out.nValue > 0) n++;
    }

    // we've got additional coinbase out
    BOOST_CHECK(n > 1);

    // assume POP reward is the output after the POW reward
    BOOST_CHECK(payoutBlock.vtx[0]->vout[1].scriptPubKey == scriptPubKey);
    BOOST_CHECK(payoutBlock.vtx[0]->vout[1].nValue > 0);

    CMutableTransaction spending;
    spending.nVersion = 1;
    spending.vin.resize(1);
    spending.vin[0].prevout.hash = payoutBlock.vtx[0]->GetHash();
    // use POP payout as an input
    spending.vin[0].prevout.n = 1;
    spending.vout.resize(1);
    spending.vout[0].nValue = 100;
    spending.vout[0].scriptPubKey = scriptPubKey;

    std::vector<unsigned char> vchSig;
    uint256 hash = SignatureHash(scriptPubKey, spending, 0, SIGHASH_ALL, 0, SigVersion::BASE);
    BOOST_CHECK(coinbaseKey.Sign(hash, vchSig));
    vchSig.push_back((unsigned char)SIGHASH_ALL);
    spending.vin[0].scriptSig << vchSig;

    CBlock spendingBlock;
    // make sure we cannot spend till coinbase maturity
    spendingBlock = CreateAndProcessBlock({spending}, scriptPubKey);
    {
        LOCK(cs_main);
        BOOST_CHECK(ChainActive().Tip()->GetBlockHash() != spendingBlock.GetHash());
    }

    for (int i = 0; i < COINBASE_MATURITY; i++) {
        CBlock b = CreateAndProcessBlock({}, scriptPubKey);
        m_coinbase_txns.push_back(b.vtx[0]);
    }

    spendingBlock = CreateAndProcessBlock({spending}, scriptPubKey);
    {
        LOCK(cs_main);
        BOOST_CHECK(ChainActive().Tip()->GetBlockHash() == spendingBlock.GetHash());
    }
}

BOOST_AUTO_TEST_SUITE_END()
