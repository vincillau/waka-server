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

#include "heartbeat_service.hpp"

#include <spdlog/spdlog.h>

#include <common/pattern.hpp>
#include <common/uuid.hpp>
#include <model/heartbeat.hpp>
#include <unordered_map>

using std::out_of_range;
using std::string;
using std::stringstream;
using std::unordered_map;
using waka::bo::Summaries;
using waka::common::Config;
using waka::common::Date;
using waka::common::genUUIDv4;
using waka::common::parseUserAgent;
using waka::model::Heartbeat;

namespace waka::service {

static void stringToLower(string str) {
  for (char& ch : str) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = ch - 'A' + 'a';
    }
  }
}

static const unordered_map<string, string> kOSMap = {
    {"aix", "AIX"},              //
    {"android", "Android"},      //
    {"darwin", "macOS"},         //
    {"dragonfly", "DragonFly"},  //
    {"freebsd", "FreeBSD"},      //
    {"hurd", "Hurd"},            //
    {"illumos", "Illumos"},      //
    {"ios", "IOS"},              //
    {"js", "JavaScript"},        //
    {"linux", "Linux"},          //
    {"nacl", "NaCl"},            //
    {"netbsd", "NetBSD"},        //
    {"openbsd", "OpenBSD"},      //
    {"plan9", "Plan9"},          //
    {"solaris", "Solaris"},      //
    {"windows", "Windows"},      //
    {"zos", "Z/OS"},             //
    {"unknown", "Unknown"}};

static const unordered_map<string, string> kEditorMap = {
    {"vscode", "VS Code"},   //
    {"unknown", "Unknown"},  //
};

string HeartbeatService::save(bo::Heartbeat bo) const {
  Heartbeat model;
  model.id = genUUIDv4();

  if (!bo.branch.empty()) {
    model.branch = std::move(bo.branch);
  } else {
    model.branch = "Unknown";
  }

  model.entity = std::move(bo.entity);

  if (!bo.language.empty()) {
    model.language = std::move(bo.language);
  } else {
    model.language = "Unknown";
  }

  if (!bo.project.empty()) {
    model.project = std::move(bo.project);
  } else {
    model.project = "Unknown";
  }

  model.time = bo.time;

  auto pair = parseUserAgent(bo.user_agent);
  try {
    model.os = kOSMap.at(pair.first);
  } catch (const out_of_range& e) {
    model.os = pair.first;
  }
  try {
    model.editor = kEditorMap.at(pair.second);
  } catch (const out_of_range& e) {
    model.editor = pair.second;
  }

  mapper_.insert(model);
  return model.id;
}

int64_t HeartbeatService::today() const {
  auto lst = mapper_.listByDate(Date::today());
  if (lst.empty()) {
    return 0;
  }

  int timeout = Config::get().timeout();
  int64_t msec = 0;
  for (size_t i = 0; i < lst.size() - 1; i++) {
    int64_t duration = lst[i + 1].time - lst[i].time;
    if (duration <= timeout * 1000) {
      msec += duration;
    }
  }
  return msec;
}

Summaries HeartbeatService::summarize(const Date& start, const Date& end) {
  assert(start <= end);
  SPDLOG_DEBUG("summarize, start={}, end={}", start.toString(), end.toString());

  int timeout = Config::get().timeout();
  Summaries summaries;
  for (Date date = start; date <= end; date++) {
    auto lst = mapper_.listByDate(date);
    if (lst.empty()) {
      summaries.msec_per_day.push_back(0);
      continue;
    }

    int64_t msec_this_day = 0;
    for (size_t i = 0; i < lst.size() - 1; i++) {
      auto& h1 = lst[i];
      auto& h2 = lst[i + 1];
      int64_t duration = h2.time - h1.time;
      if (duration > timeout * 1000) {
        continue;
      }

      {
        auto it = summaries.editors.find(h1.editor);
        if (it != summaries.editors.end()) {
          it->second += duration;
        } else {
          summaries.editors.insert({std::move(h1.editor), duration});
        }
      }

      {
        auto it = summaries.languages.find(h1.language);
        if (it != summaries.languages.end()) {
          it->second += duration;
        } else {
          summaries.languages.insert({std::move(h1.language), duration});
        }
      }

      {
        auto it = summaries.oss.find(h1.os);
        if (it != summaries.oss.end()) {
          it->second += duration;
        } else {
          summaries.oss.insert({std::move(h1.os), duration});
        }
      }

      {
        auto it = summaries.projects.find(h1.project);
        if (it != summaries.projects.end()) {
          it->second += duration;
        } else {
          summaries.projects.insert({std::move(h1.project), duration});
        }
      }

      msec_this_day += duration;
    }

    summaries.msec_per_day.push_back(msec_this_day);
    summaries.total_msec += msec_this_day;
  }

  return summaries;
}

}  // namespace waka::service
