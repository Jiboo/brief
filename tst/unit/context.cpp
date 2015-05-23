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

#include <gtest/gtest.h>

#include <string>

#include "brief/context.hpp"

TEST(ContextTests, SimpleVars) {
  brief::Context ctx;
  ctx.registerVar("inj1", "injval1");
  ctx.registerVar("inj2", "injval2");
  ctx.registerVarPrefix("prefix1", [](const brief::Repository &, const brief::Task &, const std::string &var) {
    return var;
  });

  brief::Repository repo;
  repo.constants_.emplace("con1", "conval1");
  repo.constants_.emplace("con2", "conval2");

  brief::Task task1, task2;
  task1.symbols_.emplace("sym1", "symval1");
  task1.symbols_.emplace("sym2", "symval2");
  repo.tasks_.emplace("task1", task1);
  repo.tasks_.emplace("task2", task2);

  EXPECT_EQ("test1", ctx.lookupVar(repo, task1, "prefix1::test1"));
  EXPECT_EQ("injval2", ctx.lookupVar(repo, task1, "inj2"));
  EXPECT_EQ("injval1", ctx.lookupVar(repo, task1, "inj1"));

  EXPECT_EQ("symval2", ctx.lookupVar(repo, task1, "sym2"));
  EXPECT_EQ("symval1", ctx.lookupVar(repo, task1, "sym1"));

  EXPECT_EQ("conval2", ctx.lookupVar(repo, task1, "con2"));
  EXPECT_EQ("conval1", ctx.lookupVar(repo, task1, "con1"));

  EXPECT_ANY_THROW(ctx.lookupVar(repo, task2, "sym1"));
  EXPECT_ANY_THROW(ctx.lookupVar(repo, task1, "unknown"));
}

TEST(ContextTests, Builder) {
  brief::Context ctx;
  ctx.builder_.buildCache(boost::filesystem::path("tst") / "helloworld" / "helloworld.json", {});
  ctx.builder_.build("helloworld", {"withDesc"});
}
