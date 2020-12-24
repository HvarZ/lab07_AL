// Copyright 2018 Your Name <your_email>

#include "suggestions.hpp"

#include <google/protobuf/repeated_field.h>

#include <string>
#include <fstream>
#include <thread>

#define forever for (;;)

namespace suggest {
void from_json(const nlohmann::json& j, suggest::SuggestAnswer& answer) {
  answer.set_text(j.at("name").get<std::string>());
  answer.set_position(j.at("cost").get<uint>());
}
}

grpc::Status SuggestServiceAnswer::Answer(grpc::ServerContext* context,
                                          const suggest::SuggestRequest* request,
                                          suggest::SuggestResponse* response) {
  nlohmann::json suggestions_sort = suggestions;
  std::sort(suggestions_sort.begin(), suggestions_sort.end(),
            [](const nlohmann::json& left, const nlohmann::json& right)
            {return left["cost"] < right["cost"];});

  for (uint i = 0; i < suggestions_sort.size(); i++) {
    suggestions_sort[i]["cost"] = i + 1;
  }

  google::protobuf::RepeatedPtrField<suggest::SuggestAnswer> answers;

  std::shared_lock<std::shared_mutex> lock(parse_mutex);
  for (const auto& suggestion : suggestions_sort) {
    if (request->input() == suggestion.at("id").get<std::string>()) {
      answers.Add(suggestion.get<suggest::SuggestAnswer>());
    }
  }
  lock.unlock();

  *response->mutable_suggest_answer() = answers;
  return grpc::Status::OK;
}

[[noreturn]] void SuggestServiceAnswer::ParseJson() {
  std::ifstream file;
  forever {
    file.open("/Users/zahar/go/src/github.com/iu8-31-cpp-2020/lab07-HvarZ/suggestions.json");
    std::unique_lock<std::shared_mutex> lock(parse_mutex);
    suggestions = nlohmann::json::parse(file);
    lock.unlock();
    file.close();
    std::cout << "Sleeping for 15 mins" << std::endl;
    std::this_thread::sleep_for(std::chrono::minutes(15));
  }
}

SuggestServiceAnswer::SuggestServiceAnswer() {
  std::thread suggest(&SuggestServiceAnswer::ParseJson, this);
  suggest.detach();
}