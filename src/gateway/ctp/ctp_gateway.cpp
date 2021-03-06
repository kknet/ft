// Copyright [2020] <Copyright Kevin, kevin.lau.gd@gmail.com>

#include "gateway/ctp/ctp_gateway.h"

#include <ThostFtdcMdApi.h>
#include <ThostFtdcTraderApi.h>
#include <ft/base/contract_table.h>
#include <spdlog/spdlog.h>

namespace ft {

CtpGateway::CtpGateway() {}

CtpGateway::~CtpGateway() {}

bool CtpGateway::Login(BaseOrderManagementSystem *oms, const Config &config) {
  if (config.broker_id.size() > sizeof(TThostFtdcBrokerIDType) || config.broker_id.empty() ||
      config.investor_id.size() > sizeof(TThostFtdcUserIDType) || config.investor_id.empty() ||
      config.password.size() > sizeof(TThostFtdcPasswordType) || config.password.empty()) {
    spdlog::error("[CtpGateway::Login] Failed. Invalid Login config");
    return false;
  }

  if (config.trade_server_address.empty() && config.quote_server_address.empty()) {
    spdlog::warn("[CtpGateway::Login] 交易柜台和行情服务器地址都未设置");
    return false;
  }

  if (!ContractTable::Init(config.contracts_file)) {
    spdlog::error("初始化合约列表失败");
    return false;
  }

  if (!config.trade_server_address.empty()) {
    spdlog::debug("[CtpGateway::Login] Login into trading server");
    trade_api_ = std::make_unique<CtpTradeApi>(oms);
    if (!trade_api_->Login(config)) {
      spdlog::error("[CtpGateway::Login] Failed to Login into the counter");
      return false;
    }
  }

  if (!config.quote_server_address.empty()) {
    spdlog::debug("[CtpGateway::Login] Login into market data server");
    quote_api_ = std::make_unique<CtpQuoteApi>(oms);
    if (!quote_api_->Login(config)) {
      spdlog::error("[CtpGateway::Login] Failed to Login into the md server");
      return false;
    }
  }

  return true;
}

void CtpGateway::Logout() {
  trade_api_->Logout();
  quote_api_->Logout();
}

bool CtpGateway::SendOrder(const OrderRequest &order, uint64_t *privdata_ptr) {
  return trade_api_->SendOrder(order, privdata_ptr);
}

bool CtpGateway::CancelOrder(uint64_t order_id, uint64_t privdata) {
  return trade_api_->CancelOrder(order_id, privdata);
}

bool CtpGateway::Subscribe(const std::vector<std::string> &sub_list) {
  return quote_api_->Subscribe(sub_list);
}

bool CtpGateway::QueryContractList(std::vector<Contract> *result) {
  return trade_api_->QueryContractList(result);
}

bool CtpGateway::QueryPositionList(std::vector<Position> *result) {
  return trade_api_->QueryPositionList(result);
}

bool CtpGateway::QueryAccount(Account *result) { return trade_api_->QueryAccount(result); }

bool CtpGateway::QueryTradeList(std::vector<Trade> *result) {
  return trade_api_->QueryTradeList(result);
}

bool CtpGateway::QueryMarginRate(const std::string &ticker) {
  return trade_api_->QueryMarginRate(ticker);
}

REGISTER_GATEWAY(::ft::CtpGateway);

}  // namespace ft
