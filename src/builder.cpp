/*
 * Brief: Hobby build system.
 * Copyright (C) 2015 Jean-Baptiste "Jiboo" Lepesme
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <iostream>

#include "brief/builder.hpp"
#include "brief/context.hpp"

namespace brief {

namespace fs = boost::filesystem;

void Builder::buildCache(const boost::filesystem::path &_repodesc, const std::vector<std::string> &_flavors) {
  std::ifstream src(_repodesc.string());
  std::string buf;

  src.seekg(0, std::ios::end);
  buf.reserve(static_cast<size_t>(src.tellg()));
  src.seekg(0, std::ios::beg);
  buf.assign((std::istreambuf_iterator<char>(src)), std::istreambuf_iterator<char>());
  src.close();

  Tokenizer tokenizer(std::begin(buf).base(), std::end(buf).base());

  json<Repository>::parse(tokenizer, repo_);

  // TODO Preprocess task and strings
  //  Remove optional task if one of their dependency isn't present, merge the others
  //  Remove disabled experimental features, pass the other in flavors

  const fs::path cachePath = _repodesc.string() + CACHE_SUFFIX;
  std::ofstream dst(cachePath.string());
  msgpack<int>::write(dst, BRIEF_SCHEMA_VERSION);  // brief schema version
  msgpack<std::vector<std::string>>::write(dst, _flavors);  // configured flavors
  msgpack<Repository>::write(dst, repo_);
  dst.close();
}

void Builder::loadCachedDesc(const fs::path &_repodesc) {
  if (!fs::is_regular_file(_repodesc) || !fs::exists(_repodesc))
    throw std::runtime_error("Repo description must be a file.");

  const fs::path cachePath = _repodesc.string() + CACHE_SUFFIX;

  if (!fs::exists(cachePath))
    throw std::runtime_error("Cache not present, probably unconfigured repo, use: \n"
                             "\tbrief configure <flavors/experimentals to enable, prefixed by \"<task name>:\">.");

  std::ifstream src(cachePath.string());
  int version;  // brief schema version
  msgpack<int>::read(src, version);
  std::vector<std::string> flavors;  // configured flavors
  msgpack<std::vector<std::string>>::read(src, flavors);

  const bool obsolete = version != BRIEF_SCHEMA_VERSION;
  const bool outdated = (fs::last_write_time(_repodesc) - fs::last_write_time(cachePath)) > 0;
  if (outdated || obsolete) {
    BRIEF_V(ctx_.logger_, "Cache " << cachePath << " outdated or obsolete, re-configuring...");
    src.close();
    fs::remove(cachePath);

    buildCache(_repodesc, flavors);

    src = std::ifstream(cachePath.string());
    msgpack<int>::read(src, version);
    flavors.clear();
    msgpack<std::vector<std::string>>::read(src, flavors);
  }

  try {
    BRIEF_V(ctx_.logger_, "Cache " << cachePath << " present, using it.");
    msgpack<Repository>::read(src, repo_);
    src.close();
  } catch (const std::runtime_error &e) {
    fs::remove(cachePath);
    throw std::runtime_error(std::string("Can't read cache, removed it. Caused by: ") + e.what());
  }
}

void Builder::build(const std::string &_task, const std::vector<std::string> &_flavors) {
  BRIEF_I(ctx_.logger_, "Building task " << _task << " with flavors: " << _flavors);

  // Merge task with active flavors
  // FIXME Cache thus merges
  Task merged = repo_.getTask(_task);
  std::multimap<std::string, Task> source;
  std::swap(merged.flavors_, source);
  for (const std::string &flavor : _flavors) {
    auto it = source.find(flavor);
    if (it == source.end())
      throw std::out_of_range(std::string("No flavor known as ") + flavor);
    merged = merged.merge(it->second);
  }
  BRIEF_D(ctx_.logger_, "Task merged with flavors: " << merged);

  // FIXME Notify trunks of the dependencies, and ask for refresh (rebuild if needed)

  // auto toolchain = ctx_.getToolchain(merged.toolchain_);
  // toolchain->build(merged, _flavors);
}

}  // namespace brief
