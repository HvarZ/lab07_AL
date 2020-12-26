// Copyright 2020 Zakhar Ushakov <hvarzahar@gmail.com>

#include "suggestions.hpp"

#include <google/protobuf/repeated_field.h>

#include <string>
#include <fstream>
#include <thread>

using grpc::Status;

typedef suggest::SuggestAnswer answer;
typedef google::protobuf::RepeatedPtrField<answer> answers;


namespace suggest {
void from_json(const json& j, answer& answer) {
  answer.set_text(j.at("name").get<std::string>());
  answer.set_position(j.at("cost").get<size_t>());
}
}

auto json_sort(const json& left,
               const json& right) -> bool {
  return left["cost"] < right["cost"];
}

Status SuggestServiceAnswer::Answer(grpc::ServerContext* context,
                              const suggest::SuggestRequest* request,
                                    suggest::SuggestResponse* response) {
  auto suggestions_sort = suggestions;
  std::sort(suggestions_sort.begin(), suggestions_sort.end(), json_sort);

  for (size_t i = 0; i < suggestions.size(); i++) {
    suggestions_sort[i]["cost"] = i + 1;
  }

  answers _answers;

  std::shared_lock<std::shared_mutex> lock(parse_mutex);
  for (const auto& suggestion : suggestions_sort) {
    if (request->input() == suggestion.at("id").get<std::string>()) {
      _answers.Add(suggestion.get<answer>());
    }
  }
  lock.unlock();

  *response->mutable_suggest_answer() = _answers;
  return Status::OK;
}

[[noreturn]] void SuggestServiceAnswer::ParseJson() {
  std::ifstream file;
  for (;;) {
    file.open(
"/Users/zahar/go/src/github.com/iu8-31-cpp-2020/lab07-HvarZ/suggestions.json");
    std::unique_lock<std::shared_mutex> lock(parse_mutex);
    suggestions = json::parse(file);
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
