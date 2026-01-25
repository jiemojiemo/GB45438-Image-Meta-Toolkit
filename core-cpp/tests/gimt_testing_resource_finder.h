//
// Created by user on 1/25/26.
//

#ifndef GIMT_RESOURCE_FINDER_H
#define GIMT_RESOURCE_FINDER_H


#pragma once
#include <filesystem>
#include <string>
#include <vector>

#include "gimt_testing_global_paths.h"

namespace gimt_testing {
class ResourcePathFinder {
public:
  ResourcePathFinder()
  {
    paths_.emplace_back("./");

    if (std::filesystem::exists(GIMT_ROOT_DIR)) {
      paths_.emplace_back(GIMT_ROOT_DIR);
    }
    if (std::filesystem::exists(GIMT_RESOURCE_DIR)) {
      paths_.emplace_back(GIMT_RESOURCE_DIR);
    }
  }

  void append(std::string p) { paths_.emplace_back(p); }

  const std::vector<std::string> &getSearchPaths() const { return paths_; }

  std::string find(const std::string &file_name) const
  {
    if (std::filesystem::path(file_name).is_absolute()) {
      return file_name;
    }
    // try to find file name in search paths
    for (const auto &p : paths_) {
      auto try_find_path = std::filesystem::path(p) / file_name;
      if (std::filesystem::exists(try_find_path)) {
        return try_find_path.string();
      }
    }
    return {};
  }

private:
  std::vector<std::string> paths_;
};

}

#endif //GIMT_RESOURCE_FINDER_H
