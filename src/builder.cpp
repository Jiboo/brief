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

#include "brief/builder.hpp"

#include <fstream>
#include <iostream>

#include "brief/context.hpp"

namespace brief {

namespace fs = boost::filesystem;

Builder::Builder(Context &_ctx) {
  for (auto file : fs::directory_iterator(fs::current_path())) {
    if (file.path().extension() == ".json") {
      init(_ctx, file);
      break;
    }
  }
}

Builder::Builder(Context &_ctx, const fs::path &_repodesc) {
  init(_ctx, _repodesc);
}

void Builder::init(Context &_ctx, const fs::path &_repodesc) {
  if (!fs::is_regular_file(_repodesc))
    throw std::runtime_error("Repo description must be a file");

  const fs::path cachePath = _repodesc.string() + ".brief";
  if(!fs::exists(cachePath)) {
    std::cout << "caching" <<std::endl;
    Repository repo = buildCache(_repodesc);
    std::ofstream dst(cachePath.string());
    msgpack<Repository>::write(dst, repo);
    dst.close();
    init(repo);
  } else {
    std::cout << "using cache" << std::endl;
    std::ifstream src(cachePath.string());
    Repository cache;
    msgpack<Repository>::read(src, cache);
    src.close();
    init(cache);
  }
}

Repository Builder::buildCache(const boost::filesystem::path &_repodesc) {
  std::ifstream src(_repodesc.string());
  std::string buf;
  Repository result;

  src.seekg(0, std::ios::end);
  buf.reserve(static_cast<unsigned long>(src.tellg()));
  src.seekg(0, std::ios::beg);
  buf.assign((std::istreambuf_iterator<char>(src)), std::istreambuf_iterator<char>());
  src.close();

  Tokenizer tokenizer(std::begin(buf).base(), std::end(buf).base());

  return parse<Repository>(tokenizer);
}

void Builder::init(const Repository &_repo) {
  std::cout << _repo.name_ <<std::endl;
}

}  // namespace brief
