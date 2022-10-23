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

#include "config.hpp"

#include <assert.h>

#include <service/meta_service.hpp>

using std::make_unique;
using std::unique_ptr;
using waka::service::MetaService;

namespace waka::common {

unique_ptr<Config> Config::config_;

// 在调用此函数前必须先调用initDB初始化数据库
void Config::init() {
  assert(!config_);
  Config config = MetaService{}.loadConfig();
  config_ = make_unique<Config>(std::move(config));
}

const Config& Config::get() {
  assert(config_);
  return *config_;
}

}  // namespace waka::common
