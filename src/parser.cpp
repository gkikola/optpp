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

/**
 * @file
 * @brief Source file for `parser` implementation.
 */

#include <optionpp/parser.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace optionpp {

  option& parser::add_option(const option& opt) {
    auto it = find_group("");
    if (it == m_groups.end()) {
      m_groups.emplace_back("");
      return m_groups.back().add_option(opt);
    } else {
      return it->add_option(opt);
    }
  }

  option& parser::add_option(const std::string& long_name,
                             char short_name,
                             const std::string& description,
                             const std::string& arg_name,
                             bool arg_required,
                             const std::string& group_name) {
    return group(group_name).add_option(long_name, short_name)
      .description(description).argument(arg_name, arg_required);
  }

  option_group& parser::group(const std::string& name) {
    // We'll use reverse iterators since the user is more likely to
    // access a recently-added group
    auto it = std::find_if(m_groups.rbegin(), m_groups.rend(),
                           [&](const option_group& g) {
                             return g.name() == name;
                           });
    if (it == m_groups.rend()) {
      m_groups.emplace_back(name);
      return m_groups.back();
    } else {
      return *it;
    }
  }

  void parser::set_custom_strings(const std::string& delims,
                                  const std::string& short_prefix,
                                  const std::string& long_prefix,
                                  const std::string& end_indicator,
                                  const std::string& equals) {
    if (!delims.empty())
      m_delims = delims;
    if (!short_prefix.empty())
      m_short_option_prefix = short_prefix;
    if (!long_prefix.empty())
      m_long_option_prefix = long_prefix;
    if (!end_indicator.empty())
      m_end_of_options = end_indicator;
    if (!equals.empty())
      m_equals = equals;
  }

  void parser::sort_groups() {
    std::sort(m_groups.begin(), m_groups.end(),
              [](const option_group& a, const option_group& b) {
                return a.name() < b.name();
              });
  }

  void parser::sort_options() {
    std::for_each(m_groups.begin(), m_groups.end(),
                  [](option_group& g) { g.sort(); });
  }

  option& parser::operator[](const std::string& long_name) {
    option* opt = find_option(long_name);
    if (opt)
      return *opt;
    else
      return add_option().long_name(long_name);
  }

  option& parser::operator[](char short_name) {
    option* opt = find_option(short_name);
    if (opt)
      return *opt;
    else
      return add_option().short_name(short_name);
  }

  std::ostream& parser::print_help(std::ostream& os,
                                   int max_line_length,
                                   int group_indent,
                                   int option_indent,
                                   int desc_first_line_indent,
                                   int desc_multiline_indent) const {
    bool first = true;

    for (const auto& group : m_groups) {
      if (group.empty())
        continue;

      // Add extra newlines between groups
      if (first)
        first = false;
      else
        os << "\n\n";

      // Print group name
      if (!group.name().empty()) {
        os << utility::wrap_text(group.name(), max_line_length, group_indent)
           << "\n";
      }

      // Print options
      bool first_opt = true;
      for (const auto& opt : group) {
        // Add newline between options
        if (first_opt)
          first_opt = false;
        else
          os << "\n";

        std::string usage(option_indent, ' ');

        // Short name
        if (opt.short_name() != '\0') {
          usage += m_short_option_prefix;
          usage += opt.short_name();

          if (!opt.long_name().empty())
            usage += ", ";
        } else {
          usage += std::string(m_short_option_prefix.size() + 3, ' ');
        }

        // Long name
        if (!opt.long_name().empty()) {
          usage += m_long_option_prefix;
          usage += opt.long_name();
        }

        // Argument
        if (!opt.argument_name().empty()) {
          if (opt.is_argument_required())
            usage += m_equals + opt.argument_name();
          else
            usage += "[" + m_equals + opt.argument_name() + "]";
        }

        // Description
        int spacing = desc_first_line_indent - usage.size();
        if (spacing <= 1) {
          os << utility::wrap_text(usage, max_line_length);
          if (!opt.description().empty()) {
            os << "\n" << utility::wrap_text(opt.description(),
                                             max_line_length,
                                             desc_multiline_indent,
                                             desc_first_line_indent);
          }
        } else {
          if (!opt.description().empty()) {
            usage += std::string(spacing, ' ');
            usage += opt.description();
          }
          os << utility::wrap_text(usage, max_line_length,
                                   desc_multiline_indent, 0);
        }
      }
    }
    return os;
  }

  auto parser::find_group(const std::string& name) -> group_iterator {
    return std::find_if(m_groups.begin(), m_groups.end(),
                        [&](const option_group& g) {
                          return g.name() == name;
                        });
  }

  auto parser::find_group(const std::string& name) const -> group_const_iterator {
    return std::find_if(m_groups.begin(), m_groups.end(),
                        [&](const option_group& g) {
                          return g.name() == name;
                        });
  }

  option* parser::find_option(const std::string& long_name) {
    for (auto& group : m_groups) {
      auto it = group.find(long_name);
      if (it != group.end())
        return &(*it);
    }

    return nullptr;
  }

  const option* parser::find_option(const std::string& long_name) const {
    for (const auto& group : m_groups) {
      auto it = group.find(long_name);
      if (it != group.end())
        return &(*it);
    }

    return nullptr;
  }

  option* parser::find_option(char short_name) {
    for (auto& group : m_groups) {
      auto it = group.find(short_name);
      if (it != group.end())
        return &(*it);
    }

    return nullptr;
  }

  const option* parser::find_option(char short_name) const {
    for (const auto& group : m_groups) {
      auto it = group.find(short_name);
      if (it != group.end())
        return &(*it);
    }

    return nullptr;
  }

  parser_result parser::parse(int argc, char* argv[], bool ignore_first) const {
    return parse(argv, argv + argc, ignore_first);
  }

  parser_result parser::parse(const std::string& cmd_line, bool ignore_first) const {
    std::vector<std::string> container;
    utility::split(cmd_line, std::back_inserter(container),
                   m_delims, "\"'", '\\');
    return parse(container.begin(), container.end(), ignore_first);
  }

  void parser::write_option_argument(const parsed_entry& entry) const {
    if (!entry.opt_info)
      return;

    const option& opt = *entry.opt_info;
    if (!opt.has_bound_argument_variable())
      return;

    std::string::size_type pos = 0;
    const std::string& arg = entry.argument;
    const std::string& opt_name = entry.original_without_argument;
    const std::string& fn_name = "optionpp::parser::write_option_argument";

    try {
      switch (opt.argument_type()) {
      case option::uint_arg: {
        long long value = std::stoll(entry.argument, &pos);
        if (pos != arg.size())
          throw std::invalid_argument{"invalid argument"};
        if (value < 0)
          throw parse_error{"argument for option '" + opt_name + "' must not be negative",
              fn_name, opt_name};
        else if (value > std::numeric_limits<unsigned>::max())
          throw std::out_of_range{"out of range"};
        opt.write_uint(static_cast<unsigned>(value));
        break;
      }
      case option::int_arg: {
        int value = std::stoi(entry.argument, &pos);
        if (pos != arg.size())
          throw std::invalid_argument{"invalid argument"};
        opt.write_int(value);
        break;
      }
      case option::double_arg: {
        double value = std::stod(entry.argument, &pos);
        if (pos != arg.size())
          throw std::invalid_argument{"invalid argument"};
        opt.write_double(value);
        break;
      }
      default:
      case option::string_arg:
        opt.write_string(arg);
        break;
      }
    } catch(const std::invalid_argument&) {
      switch (opt.argument_type()) {
      case option::uint_arg:
      case option::int_arg:
        throw parse_error{"argument for option '" + opt_name + "' must be an integer",
            fn_name, opt_name};
      case option::double_arg:
        throw parse_error{"argument for option '" + opt_name + "' must be a number",
            fn_name, opt_name};
      default:
        throw type_error{"type error in argument for option '" + opt_name + "'", fn_name};
      }
    } catch(const std::out_of_range&) {
      throw parse_error{"argument for option '" + opt_name + "' is out of range",
          fn_name, opt_name};
    }
  }

  void parser::parse_argument(const std::string& argument,
                              parser_result& result, cl_arg_type& type) const {
    // Check for end-of-option marker
    if (is_end_indicator(argument)) {
      type = cl_arg_type::end_indicator;
      return;
    }

    // Split string into components
    std::string option_specifier;
    std::string option_argument;
    bool assignment_found = false;
    auto pos = argument.find(m_equals);
    if (pos == std::string::npos)
      option_specifier = argument;
    else {
      assignment_found = true;
      option_specifier = argument.substr(0, pos);
      pos += m_equals.size();
      option_argument = argument.substr(pos);

      // Check for bad syntax like -= and --=
      if (option_specifier == m_short_option_prefix
          || option_specifier == m_long_option_prefix) {
        option_specifier += m_equals;
        throw parse_error{"invalid option: '" + option_specifier + "'",
            "optionpp::parser::parse_argument", option_specifier};
      }
    }

    // Check option type
    parsed_entry arg_info;
    if (is_long_option(option_specifier)) {
      // Extract option name
      std::string option_name = option_specifier.substr(m_long_option_prefix.size());

      // Look up option info
      const option* opt = find_option(option_name);
      if (!opt)
        throw parse_error{"invalid option: '" + option_specifier + "'",
            "optionpp::parser::parse_argument", option_specifier};
      arg_info.opt_info = &(*opt);

      // Does this option take an argument?
      if (!opt->argument_name().empty()) {
        if (!assignment_found) { // No arg was found, caller should look for it
          if (opt->is_argument_required())
            type = cl_arg_type::arg_required;
          else
            type = cl_arg_type::arg_optional;
        } else { // Found an argument
          type = cl_arg_type::no_arg; // Caller should not look for argument
          arg_info.argument = option_argument;
        }
      } else { // Does not take an argument
        if (assignment_found) // Found an argument where there should be none
          throw parse_error{"option '" + option_specifier + "' does not accept arguments",
              "optionpp::parser::parse_argument", option_specifier};
        type = cl_arg_type::no_arg;
      }
      arg_info.original_text = argument;
      arg_info.original_without_argument = option_specifier;
      arg_info.is_option = true;
      arg_info.long_name = option_name;
      arg_info.short_name = opt->short_name();
      if (assignment_found)
        write_option_argument(arg_info);
      opt->write_bool(true);
      result.push_back(std::move(arg_info));
    } else if (is_short_option_group(option_specifier)) { // Short options
      parse_short_option_group(option_specifier.substr(m_short_option_prefix.size()),
                               option_argument, assignment_found,
                               result, type);
    } else {
      // If we get here, this argument is not an option
      type = cl_arg_type::non_option;
      arg_info.original_text = argument;
      arg_info.is_option = false;
      result.push_back(std::move(arg_info));
    }
  }

  void parser::parse_short_option_group(const std::string& short_names,
                                        const std::string& argument, bool has_arg,
                                        parser_result& result, cl_arg_type& type) const {
    using sz_t = std::string::size_type;
    for (sz_t pos = 0; pos != short_names.size(); ++pos) {
      // Look up option info
      const option* opt = find_option(short_names[pos]);
      if (!opt) {
        auto opt_name = m_short_option_prefix;
        opt_name.push_back(short_names[pos]);
        throw parse_error{"invalid option: '" + opt_name + "'",
            "optionpp::parser::parse_short_option_group", opt_name};
      }

      parsed_entry arg_info;
      arg_info.original_text = m_short_option_prefix;
      arg_info.original_text.push_back(short_names[pos]);
      arg_info.original_without_argument = arg_info.original_text;
      arg_info.is_option = true;
      arg_info.long_name = opt->long_name();
      arg_info.short_name = short_names[pos];
      arg_info.opt_info = &(*opt);
      opt->write_bool(true);

      // Check if option takes an argument
      if (!opt->argument_name().empty()) {
        if (pos + 1 < short_names.size()) {
          // This isn't the last option, so the rest of the string is an argument
          arg_info.argument = short_names.substr(pos + 1);
          if (has_arg) {
            // The assignment symbol is actually part of the argument
            arg_info.argument += m_equals;
            arg_info.argument += argument;
          }
          arg_info.original_text += arg_info.argument;
          write_option_argument(arg_info);
          result.push_back(std::move(arg_info));
          type = cl_arg_type::no_arg;
          break;
        } else {
          // This is the last option and it needs an argument
          if (has_arg) {
            arg_info.original_text += m_equals;
            arg_info.original_text += argument;
            arg_info.argument = argument;
            write_option_argument(arg_info);
            type = cl_arg_type::no_arg;
          } else if (opt->is_argument_required()) {
            type = cl_arg_type::arg_required;
          } else {
            type = cl_arg_type::arg_optional;
          }
          result.push_back(std::move(arg_info));
          break;
        }
      }

      // If we make it here, then the current option does not take an argument
      if (pos + 1 == short_names.size() && has_arg) {
        auto opt_name = m_short_option_prefix;
        opt_name.push_back(short_names[pos]);
        throw parse_error{"option '" + opt_name + "' does not accept arguments",
            "optionpp::parser::parse_short_option_group", opt_name};
      }

      result.push_back(std::move(arg_info));
      type = cl_arg_type::no_arg;
      arg_info = parsed_entry{};
    } // End for loop
  }

  std::ostream& operator<<(std::ostream& os, const parser& opt_parser) {
    return opt_parser.print_help(os);
  }

} // End namespace
