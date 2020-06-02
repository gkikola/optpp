/* Option++ -- read command-line program options
 * Copyright (C) 2017-2020 Greg Kikola.
 *
 * This file is part of Option++.
 *
 * Option++ is free software: you can redistribute it and/or modify
 * it under the terms of the Boost Software License version 1.0.
 *
 * Option++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * Boost Software License for more details.
 *
 * You should have received a copy of the Boost Software License
 * along with Option++.  If not, see
 * <https://www.boost.org/LICENSE_1_0.txt>.
 */
/* Written by Greg Kikola <gkikola@gmail.com>. */

#include <stdexcept>
#include <vector>
#include <catch2/catch.hpp>
#include "../src/parser_result.hpp"

using namespace optionpp;

TEST_CASE("parser_result") {
  parser_result result;

  parser_result::item version { "--version", true, "version", '\0', "" };
  parser_result::item help { "-?", true, "help", '?', "" };
  parser_result::item non_option { "command", false, "", '\0', "" };
  parser_result::item file { "-f myfile.txt", true, "file", 'f', "myfile.txt" };

  SECTION("constructors, push_back, size, and empty") {
    result = parser_result{};
    REQUIRE(result.empty());
    REQUIRE(result.size() == 0);

    result.push_back(version);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.size() == 1);
    result.push_back(help);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.size() == 2);
    result.push_back(non_option);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.size() == 3);
    result.push_back(file);
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.size() == 4);

    std::vector<parser_result::item> items;
    items.push_back(version);
    items.push_back(help);
    items.push_back(non_option);
    items.push_back(file);
    parser_result result2{items.begin(), items.end()};
    REQUIRE_FALSE(result2.empty());
    REQUIRE(result2.size() == 4);

    parser_result result3{version, help, file};
    REQUIRE_FALSE(result3.empty());
    REQUIRE(result3.size() == 3);

    parser_result::item non_opt2{"another command", false, "", '\0', ""};
    result3.push_back(std::move(non_opt2));
    REQUIRE_FALSE(result3.empty());
    REQUIRE(result3.size() == 4);
  }

  SECTION("iteration") {
    result = parser_result{version, help, non_option, file};

    // Plain iterator
    auto it = result.begin();
    REQUIRE(it->original_text == "--version");
    ++it;
    REQUIRE(it->original_text == "-?");
    ++it;
    REQUIRE(it->original_text == "command");
    ++it;
    REQUIRE(it->original_text == "-f myfile.txt");
    ++it;
    REQUIRE(it == result.end());

    // const_iterator
    const parser_result cresult = result;
    auto cit = cresult.begin();
    REQUIRE(cit->original_text == "--version");
    ++cit;
    REQUIRE(cit->original_text == "-?");
    ++cit;
    REQUIRE(cit->original_text == "command");
    ++cit;
    REQUIRE(cit->original_text == "-f myfile.txt");
    ++cit;
    REQUIRE(cit == cresult.end());

    // reverse_iterator
    auto rit = result.rbegin();
    REQUIRE(rit->original_text == "-f myfile.txt");
    ++rit;
    REQUIRE(rit->original_text == "command");
    ++rit;
    REQUIRE(rit->original_text == "-?");
    ++rit;
    REQUIRE(rit->original_text == "--version");
    ++rit;
    REQUIRE(rit == result.rend());

    // const_reverse_iterator
    auto crit = cresult.rbegin();
    REQUIRE(crit->original_text == "-f myfile.txt");
    ++crit;
    REQUIRE(crit->original_text == "command");
    ++crit;
    REQUIRE(crit->original_text == "-?");
    ++crit;
    REQUIRE(crit->original_text == "--version");
    ++crit;
    REQUIRE(crit == cresult.rend());
  }

  SECTION("clear") {
    result = parser_result{help, version, non_option, file};

    REQUIRE(result.size() == 4);
    REQUIRE_FALSE(result.empty());

    result.clear();
    REQUIRE(result.size() == 0);
    REQUIRE(result.empty());
    REQUIRE(result.begin() == result.end());

    result.push_back(help);
    REQUIRE(result.size() == 1);
    REQUIRE_FALSE(result.empty());
  }

  SECTION("operator[] and at") {
    result = parser_result{version, help, non_option, file};

    REQUIRE(result[0].original_text == "--version");
    REQUIRE(result[1].original_text == "-?");
    REQUIRE(result[2].original_text == "command");
    REQUIRE(result[3].original_text == "-f myfile.txt");

    REQUIRE(result.at(0).original_text == "--version");
    REQUIRE(result.at(1).original_text == "-?");
    REQUIRE(result.at(2).original_text == "command");
    REQUIRE(result.at(3).original_text == "-f myfile.txt");

    REQUIRE_THROWS_AS(result.at(4), std::out_of_range);
    REQUIRE_THROWS_AS(result.at(5), std::out_of_range);
    REQUIRE_THROWS_AS(result.at(10), std::out_of_range);

    const parser_result cresult = result;
    REQUIRE(cresult.at(0).original_text == "--version");
    REQUIRE(cresult.at(1).original_text == "-?");
    REQUIRE(cresult.at(2).original_text == "command");
    REQUIRE(cresult.at(3).original_text == "-f myfile.txt");

    REQUIRE_THROWS_AS(cresult.at(4), std::out_of_range);
    REQUIRE_THROWS_AS(cresult.at(5), std::out_of_range);
    REQUIRE_THROWS_AS(cresult.at(10), std::out_of_range);
  }
}
