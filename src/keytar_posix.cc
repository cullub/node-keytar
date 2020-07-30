#include "keytar.h"

#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <algorithm>

namespace keytar {

// https://stackoverflow.com/a/37454181/3437608
std::vector<std::string> split(const std::string& str, const std::string& delim) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

// https://www.jeremymorgan.com/tutorials/c-programming/how-to-capture-the-output-of-a-linux-command-in-c/
std::string GetStdoutFromCommand(std::string cmd) {
  std::string data;
  FILE * stream;
  const int max_buffer = 256;
  char buffer[max_buffer];
  cmd.append(" 2>&1");

  stream = popen(cmd.c_str(), "r");
  if (stream) {
    while (!feof(stream))
      if (fgets(buffer, max_buffer, stream) != NULL) data.append(buffer);
        pclose(stream);
    }
  return data;
}

// https://stackoverflow.com/a/24315631/3437608b 
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

KEYTAR_OP_RESULT SetPassword(const std::string& service,
                             const std::string& account,
                             const std::string& password,
                             std::string* errStr) {

  std::string execString = "echo '" + ReplaceAll(password, "'", "\\'") + "' | pass insert -e -f " + service + "/" + account;
  *errStr = GetStdoutFromCommand(execString);
  return SUCCESS;
}

KEYTAR_OP_RESULT GetPassword(const std::string& service,
                             const std::string& account,
                             std::string* password,
                             std::string* errStr) {
  GetStdoutFromCommand("pass");

  std::string cmd = "pass show " + service + "/" + account;
  std::string raw_password = GetStdoutFromCommand(cmd);
  rtrim(raw_password);  // remove trailing whitespace or newlines


  if (raw_password.find("not in the password store") != std::string::npos) {
    *errStr = raw_password;
    return FAIL_NONFATAL;
  }

  *password = split(raw_password, "\n").back();  // return just the last line, to account for instances where gpg outputs a warning

  return SUCCESS;
}

KEYTAR_OP_RESULT DeletePassword(const std::string& service,
                                const std::string& account,
                                std::string* errStr) {
  GetStdoutFromCommand("pass rm -f " + service + "/" + account);
  return SUCCESS;
}

// NOT IMPLEMENTED
KEYTAR_OP_RESULT FindPassword(const std::string& service,
                              std::string* password,
                              std::string* errStr) {
  return FAIL_ERROR;
}

// NOT IMPLEMENTED
KEYTAR_OP_RESULT FindCredentials(const std::string& service,
                                 std::vector<Credentials>* credentials,
                                 std::string* errStr) {
  return FAIL_ERROR;
}

}  // namespace keytar
