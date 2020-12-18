#ifndef PEXA_SRC_VBK_POP_SERVICE_HPP
#define PEXA_SRC_VBK_POP_SERVICE_HPP

#include <consensus/validation.h>
#include <util/ref.h>
#include <vbk/adaptors/block_batch_adaptor.hpp>
#include <vbk/adaptors/payloads_provider.hpp>
#include <vbk/pop_common.hpp>
#include <vbk/util.hpp>

typedef int64_t CAmount;

class CBlockIndex;
class CBlock;
class CScript;
class CBlockTreeDB;
class CDBIterator;
class CDBWrapper;
class CChainParams;

namespace VeriBlock {

using BlockBytes = std::vector<uint8_t>;
using PopRewards = std::map<CScript, CAmount>;

void SetPop(const util::Ref& context, CDBWrapper& db);

PayloadsProvider& GetPayloadsProvider();

//! returns true if all tips are stored in database, false otherwise
bool hasPopData(CBlockTreeDB& db);
void saveTrees(altintegration::BlockBatchAdaptor& batch);
bool loadTrees(CDBIterator& iter);

//! alttree methods
bool acceptBlock(const CBlockIndex& indexNew, BlockValidationState& state);
bool addAllBlockPayloads(const CBlock& block, BlockValidationState& state);
bool setState(const uint256& hash, altintegration::ValidationState& state);

//! mempool methods
altintegration::PopData getPopData();
void removePayloadsFromMempool(const altintegration::PopData& popData);
void updatePopMempoolForReorg();
void addDisconnectedPopData(const altintegration::PopData& popData);

std::vector<BlockBytes> getLastKnownVBKBlocks(size_t blocks);
std::vector<BlockBytes> getLastKnownBTCBlocks(size_t blocks);

PopRewards getPopRewards(const CBlockIndex& pindexPrev, const CChainParams& params);
void addPopPayoutsIntoCoinbaseTx(CMutableTransaction& coinbaseTx, const CBlockIndex& pindexPrev, const CChainParams& params);
bool checkCoinbaseTxWithPopRewards(const CTransaction& tx, const CAmount& nFees, const CBlockIndex& pindexPrev, const CChainParams& params, BlockValidationState& state);
CAmount getCoinbaseSubsidy(const CAmount& subsidy, int32_t height, const CChainParams& params);

CBlockIndex* compareTipToBlock(CBlockIndex* candidate);
int compareForks(const CBlockIndex& left, const CBlockIndex& right);

} // namespace VeriBlock


#endif