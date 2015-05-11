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
#include <flatbuffers/flatbuffers.h>

#include "brief/context.hpp"

TEST(ContextTests, SimpleVars) {
  brief::Context ctx;
  ctx.registerVar("inj1", "injval1");
  ctx.registerVar("inj2", "injval2");

  flatbuffers::FlatBufferBuilder builder;

  flatbuffers::Offset<brief::Constant> symbols[2];
  symbols[0] = brief::CreateConstant(builder, builder.CreateString("sym1"), builder.CreateString("symval1"));
  symbols[1] = brief::CreateConstant(builder, builder.CreateString("sym2"), builder.CreateString("symval2"));
  auto symbols_offset = builder.CreateVectorOfSortedTables(symbols, 2);

  brief::TaskBuilder taskBuilder(builder);
  taskBuilder.add_symbols(symbols_offset);
  auto task_offset = taskBuilder.Finish();

  brief::TaskBuilder emptyTaskBuilder(builder);
  auto etask_offset = emptyTaskBuilder.Finish();

  flatbuffers::Offset<brief::NamedTask> tasks[2];
  tasks[0] = brief::CreateNamedTask(builder, builder.CreateString("task1"), task_offset);
  tasks[1] = brief::CreateNamedTask(builder, builder.CreateString("task2"), etask_offset);
  auto tasks_offset = builder.CreateVector(tasks, 2);

  flatbuffers::Offset<brief::Constant> constants[2];
  constants[0] = brief::CreateConstant(builder, builder.CreateString("con1"), builder.CreateString("conval1"));
  constants[1] = brief::CreateConstant(builder, builder.CreateString("con2"), builder.CreateString("conval2"));
  auto constants_offset = builder.CreateVectorOfSortedTables(constants, 2);

  brief::RepositoryBuilder repositoryBuilder(builder);
  repositoryBuilder.add_tasks(tasks_offset);
  repositoryBuilder.add_constants(constants_offset);

  brief::FinishRepositoryBuffer(builder, repositoryBuilder.Finish());

  const brief::Repository &repo = *brief::GetRepository(builder.GetBufferPointer());
  const brief::Task &task1 = *repo.tasks()->LookupByKey("task1")->task();
  const brief::Task &task2 = *repo.tasks()->LookupByKey("task2")->task();

  EXPECT_EQ("injval2", ctx.lookupVar(repo, task1, "inj2"));
  EXPECT_EQ("injval1", ctx.lookupVar(repo, task1, "inj1"));

  EXPECT_EQ("symval2", ctx.lookupVar(repo, task1, "sym2"));
  EXPECT_EQ("symval1", ctx.lookupVar(repo, task1, "sym1"));

  EXPECT_EQ("conval2", ctx.lookupVar(repo, task1, "con2"));
  EXPECT_EQ("conval1", ctx.lookupVar(repo, task1, "con1"));

  EXPECT_ANY_THROW(ctx.lookupVar(repo, task2, "con1"));
  EXPECT_ANY_THROW(ctx.lookupVar(repo, task1, "unknown"));
}
