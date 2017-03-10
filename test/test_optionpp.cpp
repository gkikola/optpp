/* optionpp -- read command-line program options
   Copyright (C) 2017 Gregory Kikola.

   This file is part of option++.

   option++ is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   option++ is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with option++.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Written by Gregory Kikola <gkikola@gmail.com>. */

#include <cstring>
#include <iostream>
#include "gtest/gtest.h"
#include "../src/optionpp.hpp"

class OptionParserTest : public ::testing::Test {
public:
  OptionParserTest();
protected:
  void parse(OptionParser& parser,
             const std::vector<std::string>& args);
  OptionParser sm_parser;
  OptionParser md_parser;
  OptionParser lg_parser;
  OptionParser copy;
};

OptionParserTest::OptionParserTest():
  sm_parser({
      {'a', "all", "", "list all files"},
      {'s', "sort", "", "sort files in list"},
      {'v', "verbose", "", "verbose mode"},
      {0, "version", "", "display program version"},
      {'?', "help", "", "display help text"}
      })
{
  md_parser.add({'?', "help", "", "display help text"});
  md_parser.add({ {0, "version", "", "display program version"},
        {'v', "verbose", "", "verbose mode"},
          {'f', "force", "", "write file even if it exists"} });
  md_parser.add('a', "all", "", "list all files", 1);
  md_parser.add('A', "almost-all", "", "do not list . and ..");
  md_parser.add(0, "block-size", "SIZE", "scale sizes by SIZE");

  lg_parser.add('x', "nonexistent", "blank", "overwritten");
  lg_parser = md_parser;
  lg_parser.add({
      {'b', "buffer", "N", "buffer size for each file"},
      {'B', "auto-buffers", "", "buffers allocated automatically"},
      {'c', "clear-screen", "", "clear screen on each repaint"},
      {'d', "dumb", "", "suppress error message if terminal is dumb"},
      {0, "color", "COLOR", "set color of text displayed"},
      {'e', "quit-at-eof", "", "automatically exit when end-of-file "
          "is reached"},
      {'h', "max-back-scroll", "N", "maximum number of lines to scroll "
          "backward"},
      {'i', "ignore-case", "", "searches ignore case"},
      {'I', "IGNORE-CASE", "", "really really ignores case"},
      {'n', "line-numbers", "", "show line numbers"},
      {'p', "pattern", "PATTERN", "start at first occurrence of "
          "PATTERN"},
      {'P', "", "PROMPT", "use custom prompt", 0, true},
      {'q', "quiet", "", "quiet mode, do not ring terminal bell"},
      {'s', "", "", "squeeze consecutive blank lines into one"},
      {'S', "", "", "chop long lines"},
      {'t', "tag", "TAG", "edit file containing tag TAG", 0, true},
      {'u', "underline-special", "", "underline special characters"},
      {'z', "window", "N", "change default scrolling window to N lines"}
    });
}

void OptionParserTest::parse(OptionParser& parser,
                             const std::vector<std::string>& args)
{
  int argc = args.size();
  char** argv = new char*[argc + 1];
  for (int i = 0; i < argc; ++i) {
    argv[i] = new char[args[i].size() + 1];
    std::strcpy(argv[i], args[i].c_str());
  }
  argv[argc] = nullptr;

  parser.parse(argc, argv);

  for (int i = 0; i < argc; ++i)
    delete[] argv[i];
  delete[] argv;
}

TEST_F(OptionParserTest, BadLookup) {
  EXPECT_EQ(nullptr, md_parser.lookup('V'));
  EXPECT_EQ(nullptr, md_parser.lookup("unknown"));
  EXPECT_EQ(nullptr, sm_parser.lookup("vERSION"));
  EXPECT_EQ(nullptr, lg_parser.lookup('r'));
}

TEST_F(OptionParserTest, Lookup) {
  OptionDesc* od;
  od = sm_parser.lookup('?');
  EXPECT_EQ("help", od->long_name);
  EXPECT_EQ("", od->argument_name);

  od = md_parser.lookup("version");
  EXPECT_EQ(0, od->short_name);
  EXPECT_EQ("version", od->long_name);

  od = lg_parser.lookup('p');
  EXPECT_EQ("pattern", od->long_name);
  EXPECT_EQ("PATTERN", od->argument_name);

  od = lg_parser.lookup("line-numbers");
  EXPECT_EQ('n', od->short_name);
  EXPECT_EQ("line-numbers", od->long_name);
  EXPECT_EQ("show line numbers", od->description);
}

TEST_F(OptionParserTest, NoArgs) {
  parse(sm_parser, {"prog"});

  EXPECT_EQ(1, sm_parser.program_args().size());
  EXPECT_EQ(0, sm_parser.size());
  EXPECT_EQ(true, sm_parser.empty());
  EXPECT_EQ(sm_parser.end(), sm_parser.begin());
}

TEST_F(OptionParserTest, NoOptions) {
  parse(sm_parser, {"prog", "arg1", "arg2", "arg3", "arg4"});

  EXPECT_EQ(0, sm_parser.size());
  EXPECT_EQ(true, sm_parser.empty());
  EXPECT_EQ(sm_parser.end(), sm_parser.begin());

  EXPECT_EQ(5, sm_parser.program_args().size());
  auto it = sm_parser.program_args().begin();
  EXPECT_EQ("prog", *it++);
  EXPECT_EQ("arg1", *it++);
  EXPECT_EQ("arg2", *it++);
  EXPECT_EQ("arg3", *it++);
  EXPECT_EQ("arg4", *it++);
  EXPECT_EQ(sm_parser.program_args().end(), it);
}

TEST_F(OptionParserTest, NoOptionHyphen) {
  parse(lg_parser, {"prog", "-", "blank"});

  EXPECT_EQ(true, lg_parser.empty());
  EXPECT_EQ(3, lg_parser.program_args().size());
  auto it = lg_parser.program_args().begin();
  EXPECT_EQ("prog", *it++);
  EXPECT_EQ("-", *it++);
  EXPECT_EQ("blank", *it++);
  EXPECT_EQ(lg_parser.program_args().end(), it);
}

TEST_F(OptionParserTest, Options) {
  parse(lg_parser, {"prog", "-BiuqS", "--line-numbers", "clear-screen", "-I"});

  EXPECT_EQ(7, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  auto it = lg_parser.begin();
  EXPECT_EQ("auto-buffers", (it++)->long_name);
  EXPECT_EQ("ignore-case", (it++)->long_name);
  EXPECT_EQ("underline-special", (it++)->long_name);
  EXPECT_EQ("quiet", (it++)->long_name);
  EXPECT_EQ('S', (it++)->short_name);
  EXPECT_EQ("line-numbers", (it++)->long_name);
  EXPECT_EQ("IGNORE-CASE", (it++)->long_name);
  EXPECT_EQ(lg_parser.end(), it);

  auto arg = lg_parser.program_args().begin();
  EXPECT_EQ(2, lg_parser.program_args().size());
  EXPECT_EQ("prog", *arg++);
  EXPECT_EQ("clear-screen", *arg++);
  EXPECT_EQ(lg_parser.program_args().end(), arg);
}

TEST_F(OptionParserTest, OptionsWithArgsEq) {
  parse(lg_parser, {"prog", "--max-back-scroll=12", "-ep=42",
        "-P=custom prompt", "--buffer=10", "--color=red"});

  EXPECT_EQ(6, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  auto it = lg_parser.begin();
  EXPECT_EQ("max-back-scroll", it->long_name);
  EXPECT_EQ("12", it->argument);
  ++it;
  
  EXPECT_EQ("quit-at-eof", it->long_name);
  EXPECT_EQ("", it->argument);
  ++it;
  
  EXPECT_EQ("pattern", it->long_name);
  EXPECT_EQ("42", it->argument);
  ++it;
  
  EXPECT_EQ('P', it->short_name);
  EXPECT_EQ("custom prompt", it->argument);
  ++it;

  EXPECT_EQ("buffer", it->long_name);
  EXPECT_EQ("10", it->argument);
  ++it;

  EXPECT_EQ("color", it->long_name);
  EXPECT_EQ("red", it->argument);
  ++it;

  EXPECT_EQ(lg_parser.end(), it);

  it = lg_parser.find("buffer");
  EXPECT_EQ("buffer", it->long_name);
  EXPECT_EQ("10", it->argument);
  it = lg_parser.find("boffer");
  EXPECT_EQ(lg_parser.end(), it);

  it = lg_parser.find('e');
  EXPECT_EQ("quit-at-eof", it->long_name);
  it = lg_parser.find('i');
  EXPECT_EQ(lg_parser.end(), it);
}

TEST_F(OptionParserTest, OptionsWithArgsSep) {
  parse(lg_parser, {"prog", "--max-back-scroll", "12", "-ep", "42",
        "-P", "custom prompt", "--buffer=", "10", "--color", "red"});

  EXPECT_EQ(6, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  auto it = lg_parser.begin();
  EXPECT_EQ("max-back-scroll", it->long_name);
  EXPECT_EQ("12", it->argument);
  ++it;
  
  EXPECT_EQ("quit-at-eof", it->long_name);
  EXPECT_EQ("", it->argument);
  ++it;
  
  EXPECT_EQ("pattern", it->long_name);
  EXPECT_EQ("42", it->argument);
  ++it;
  
  EXPECT_EQ('P', it->short_name);
  EXPECT_EQ("custom prompt", it->argument);
  ++it;

  EXPECT_EQ("buffer", it->long_name);
  EXPECT_EQ("10", it->argument);
  ++it;

  EXPECT_EQ("color", it->long_name);
  EXPECT_EQ("red", it->argument);
  ++it;

  EXPECT_EQ(lg_parser.end(), it);
}

TEST_F(OptionParserTest, OptionsWithHyphenArg) {
  parse(lg_parser, {"prog", "-p", "-"});

  EXPECT_EQ(1, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  auto it = lg_parser.begin();
  EXPECT_EQ("pattern", it->long_name);
  EXPECT_EQ("-", it->argument);
  ++it;
  EXPECT_EQ(lg_parser.end(), it);

  parse(lg_parser, {"prog", "--pattern", "-", "-e"});

  EXPECT_EQ(2, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  it = lg_parser.begin();
  EXPECT_EQ("pattern", it->long_name);
  EXPECT_EQ("-", it->argument);
  ++it;
  EXPECT_EQ("quit-at-eof", it->long_name);
  ++it;
  EXPECT_EQ(lg_parser.end(), it);
}

TEST_F(OptionParserTest, OptionsWithOptionalArgs) {
  parse(lg_parser, {"prog", "--max-back-scroll", "12", "-ep", "42",
        "-P", "--buffer", "10", "--tag", "--color", "red"});

  EXPECT_EQ(7, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  auto it = lg_parser.begin();
  EXPECT_EQ("max-back-scroll", it->long_name);
  EXPECT_EQ("12", it->argument);
  ++it;
  
  EXPECT_EQ("quit-at-eof", it->long_name);
  EXPECT_EQ("", it->argument);
  ++it;
  
  EXPECT_EQ("pattern", it->long_name);
  EXPECT_EQ("42", it->argument);
  ++it;
  
  EXPECT_EQ('P', it->short_name);
  EXPECT_EQ("", it->argument);
  ++it;

  EXPECT_EQ("buffer", it->long_name);
  EXPECT_EQ("10", it->argument);
  ++it;

  EXPECT_EQ("tag", it->long_name);
  EXPECT_EQ("", it->argument);
  ++it;

  EXPECT_EQ("color", it->long_name);
  EXPECT_EQ("red", it->argument);
  ++it;

  EXPECT_EQ(lg_parser.end(), it);
}

TEST_F(OptionParserTest, EndOfOptions) {
  parse(lg_parser, {"prog", "--max-back-scroll", "12", "--", "-ep", "42",
        "-P", "--buffer", "10", "--tag", "--color", "red"});

  EXPECT_EQ(1, lg_parser.size());
  EXPECT_EQ(false, lg_parser.empty());
  auto opt = lg_parser.begin();
  EXPECT_EQ("max-back-scroll", opt->long_name);
  EXPECT_EQ("12", opt->argument);
  ++opt;
  EXPECT_EQ(opt, lg_parser.end());

  EXPECT_EQ(9, lg_parser.program_args().size());
  auto it = lg_parser.program_args().begin();
  EXPECT_EQ("prog", *it++);
  EXPECT_EQ("-ep", *it++);
  EXPECT_EQ("42", *it++);
  EXPECT_EQ("-P", *it++);
  EXPECT_EQ("--buffer", *it++);
  EXPECT_EQ("10", *it++);
  EXPECT_EQ("--tag", *it++);
  EXPECT_EQ("--color", *it++);
  EXPECT_EQ("red", *it++);
  EXPECT_EQ(it, lg_parser.program_args().end());
}

TEST_F(OptionParserTest, BadOptions) {
  EXPECT_THROW(parse(lg_parser, {"prog", "-Bcdjei"}), BadOption);
  EXPECT_THROW(parse(lg_parser, {"prog", "--error"}), BadOption);
  EXPECT_THROW(parse(lg_parser, {"prog", "-eid", "--color=red", "--throw",
          "--window=16"}), BadOption);
}

TEST_F(OptionParserTest, BadOptionArgs) {
  EXPECT_THROW(parse(lg_parser, {"prog", "-bcdei"}), BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "-c=red", "--pattern", "-q"}),
               BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "-c=red", "--pattern", "--",
          "three"}), BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "--pattern"}), BadOptionArgument);
  EXPECT_NO_THROW(parse(lg_parser, {"prog", "--tag"}));
  EXPECT_THROW(parse(lg_parser, {"prog", "-cinp"}), BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "-cinp", "--", "hello"}),
               BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "-pcin"}), BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "-cinp="}), BadOptionArgument);
  EXPECT_THROW(parse(lg_parser, {"prog", "--pattern="}), BadOptionArgument);
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
