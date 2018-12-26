/* Option++ -- read command-line program options
   Copyright (C) 2017-2018 Greg Kikola.

   This file is part of Option++.

   Option++ is free software: you can redistribute it and/or modify
   it under the terms of the Boost Software License version 1.0.

   Option++ is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   Boost Software License for more details.

   You should have received a copy of the Boost Software License
   along with Option++.  If not, see
   <https://www.boost.org/LICENSE_1_0.txt>.
 */

/* Written by Greg Kikola <gkikola@gmail.com>. */

#ifndef OPTIONPP_HPP
#define OPTIONPP_HPP

#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace optionpp {

  struct OptionDesc {
    char short_name;
    std::string long_name;
    std::string argument_name;
    std::string description;
    int group = 0;
    bool arg_optional = false;
  };

  struct Option {
    char short_name = 0;
    std::string long_name;
    std::string argument;
    OptionDesc* desc = nullptr;

    int arg_to_int() const;
    unsigned arg_to_unsigned() const;
    long arg_to_long() const;
    double arg_to_double() const;

  private:
    std::string option_name() const;
    void validate_arg(std::size_t end_pos) const;
  };

  constexpr unsigned def_term_width = 80;
  constexpr unsigned def_tab_stop = 30;

  class OptionParser {
  public:
    typedef std::vector<OptionDesc> desc_set;
    typedef std::vector<Option> option_seq;
    typedef std::vector<std::string> arg_seq;

    typedef option_seq::size_type size_type;
    typedef option_seq::iterator iterator;
    typedef option_seq::const_iterator const_iterator;
    typedef option_seq::reverse_iterator reverse_iterator;
    typedef option_seq::const_reverse_iterator const_reverse_iterator;

    OptionParser() = default;
    OptionParser(std::initializer_list<OptionDesc> opts):
      m_opts(opts.begin(), opts.end()) { }

    void allow_bad_opts(bool allow = true) { m_allow_bad_opts = allow; }
    void allow_bad_args(bool allow = true) { m_allow_bad_args = allow; }

    void add(const OptionDesc& desc) { m_opts.push_back(desc); }
    void add(char sname, const std::string& lname,
             const std::string& aname, const std::string& desc,
             int group = 0);
    void add(std::initializer_list<OptionDesc> opts);

    OptionDesc* lookup(char short_name);
    const OptionDesc* lookup(char short_name) const;
    OptionDesc* lookup(const std::string& long_name);
    const OptionDesc* lookup(const std::string& long_name) const;

    void parse(int argc, char* argv[]);

    size_type size() const { return m_opts_read.size(); }
    bool empty() const { return size() == 0; }

    iterator begin() { return m_opts_read.begin(); }
    const_iterator begin() const { return m_opts_read.begin(); }
    iterator end() { return m_opts_read.end(); }
    const_iterator end() const { return m_opts_read.end(); }

    reverse_iterator rbegin() { return m_opts_read.rbegin(); }
    const_reverse_iterator rbegin() const { return m_opts_read.rbegin(); }
    reverse_iterator rend() { return m_opts_read.rend(); }
    const_reverse_iterator rend() const { return m_opts_read.rend(); }

    const_iterator cbegin() const { return m_opts_read.cbegin(); }
    const_iterator cend() const { return m_opts_read.cend(); }

    const_reverse_iterator crbegin() const { return m_opts_read.crbegin(); }
    const_reverse_iterator crend() const { return m_opts_read.crend(); }

    iterator find(char short_name);
    const_iterator find(char short_name) const;
    iterator find(const std::string& long_name);
    const_iterator find(const std::string& long_name) const;

    std::string program_cmd() const { return m_prog_cmd; }

    arg_seq& program_args() { return m_prog_args; }
    const arg_seq& program_args() const { return m_prog_args; }

    std::ostream& print_usage(std::ostream& out,
                              unsigned tab_stop = def_tab_stop,
                              unsigned term_width = def_term_width) const;
    void usage(std::string& opt_usage_str,
               unsigned tab_stop = def_tab_stop,
               unsigned term_width = def_term_width) const;
  private:
    //return true if argument is to be read next
    bool read_short_opts(const std::string& argstr);
    bool read_long_opt(const std::string& argstr);

    bool m_allow_bad_opts = false;
    bool m_allow_bad_args = false;

    desc_set m_opts;
    option_seq m_opts_read;
    arg_seq m_prog_args;
    std::string m_prog_cmd;
    std::string m_last_option_read; //used for error reporting
  };

  class BadOption : public std::logic_error {
  public:
    explicit BadOption(const std::string& what_arg): logic_error(what_arg) { }
    explicit BadOption(const char* what_arg): logic_error(what_arg) { }
  };

  class BadOptionArgument : public std::logic_error {
  public:
    explicit BadOptionArgument(const std::string& what_arg):
      logic_error(what_arg) { }
    explicit BadOptionArgument(const char* what_arg):
      logic_error(what_arg) { }
  };

  bool operator<(const OptionDesc& o1,
                 const OptionDesc& o2);
  bool operator<=(const OptionDesc& o1,
                  const OptionDesc& o2);
  bool operator>(const OptionDesc& o1,
                 const OptionDesc& o2);
  bool operator>=(const OptionDesc& o1,
                  const OptionDesc& o2);
  bool operator==(const OptionDesc& o1,
                  const OptionDesc& o2);
  bool operator!=(const OptionDesc& o1,
                  const OptionDesc& o2);

} // End namespace optionpp

#endif