// Copyright 2020 Zakhar Ushakov <hvarzahar@gmail.com>

#ifndef INCLUDE_SUGGESTIONS_HPP_
#define INCLUDE_SUGGESTIONS_HPP_

#include <suggest.grpc.pb.h>
#include <shared_mutex>
#include <nlohmann/json.hpp>

using nlohmann::json;

class SuggestServiceAnswer : public suggest::Suggest::Service {
 private:
  std::shared_mutex parse_mutex;
  json suggestions;

  grpc::Status Answer(grpc::ServerContext* context,
                      const suggest::SuggestRequest* request,
                      suggest::SuggestResponse* response) override;

  [[noreturn]] void ParseJson();

 public:
  SuggestServiceAnswer();
};

#endif  // INCLUDE_SUGGESTIONS_HPP_
