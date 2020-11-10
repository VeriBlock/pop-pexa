#ifndef PEXA_SRC_VBK_POP_COMMON_HPP
#define PEXA_SRC_VBK_POP_COMMON_HPP

#include <veriblock/pop_context.hpp>

namespace VeriBlock {
altintegration::PopContext& GetPop();

void SetPopConfig(const altintegration::Config& config);

void SetPop(std::shared_ptr<altintegration::PayloadsProvider>& db);

std::string toPrettyString(const altintegration::PopContext& pop);

} // namespace VeriBlock


#endif