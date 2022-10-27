// Copyright 2022 Vincil Lau
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef WAKA_SRC_HTTP_WRAPPER_HPP_
#define WAKA_SRC_HTTP_WRAPPER_HPP_

#include <spdlog/spdlog.h>

#include <stdexcept>

#include "http.hpp"
#include "msg.hpp"

namespace waka::http {

template <typename Controller, Method kMethod>
class Wrapper {};

template <typename Controller>
class Wrapper<Controller, Method::kGet> {
 public:
  void operator()(const httplib::Request& req, httplib::Response& resp) const {
    HTTPLibRequest request{req};
    HTTPLibResponse response{resp};
    try {
      Controller{}.get(request, response);
    } catch (const std::exception& e) {
      SPDLOG_WARN("{} {} -- {}", request.getMethod(), request.getPath(),
                  e.what());
      response.setStatus(Status::kInternalServerError);
      response.setContent(jsonMsg(e.what()), "application");
    }
  }
};

template <typename Controller>
class Wrapper<Controller, Method::kPost> {
 public:
  void operator()(const httplib::Request& req, httplib::Response& resp) {
    HTTPLibRequest request{req};
    HTTPLibResponse response{resp};
    try {
      Controller{}.post(request, response);
    } catch (const std::exception& e) {
      SPDLOG_WARN("{} {} -- {}", request.getMethod(), request.getPath(),
                  e.what());
      response.setStatus(Status::kInternalServerError);
      response.setContent(jsonMsg(e.what()), "application");
    }
  }
};

template <typename Controller>
class Wrapper<Controller, Method::kPut> {
 public:
  void operator()(const httplib::Request& req, httplib::Response& resp) {
    HTTPLibRequest request{req};
    HTTPLibResponse response{resp};
    try {
      Controller{}.put(request, response);
    } catch (const std::exception& e) {
      SPDLOG_WARN("{} {} -- {}", request.getMethod(), request.getPath(),
                  e.what());
      response.setStatus(Status::kInternalServerError);
      response.setContent(jsonMsg(e.what()), "application");
    }
  }
};

}  // namespace waka::http

#endif  // WAKA_SRC_HTTP_WRAPPER_HPP_