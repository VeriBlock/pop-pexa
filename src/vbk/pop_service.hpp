#ifndef PEXA_SRC_VBK_POP_SERVICE_HPP
#define PEXA_SRC_VBK_POP_SERVICE_HPP

#include <vbk/adaptors/block_batch_adaptor.hpp>
#include <vbk/adaptors/payloads_provider.hpp>
#include <vbk/pop_common.hpp>

class CBlockTreeDB;
class CDBIterator;
class CDBWrapper;

namespace VeriBlock {

void SetPop(CDBWrapper &db);

PayloadsProvider &GetPayloadsProvider();

//! returns true if all tips are stored in database, false otherwise
bool hasPopData(CBlockTreeDB &db);
altintegration::PopData getPopData();
void saveTrees(altintegration::BlockBatchAdaptor &batch);
bool loadTrees(CDBIterator &iter);

} // namespace VeriBlock


#endif