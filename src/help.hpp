#ifndef HPP_HELP
#define HPP_HELP

#include <string>

const std::string g_strHelp = R"(
Usage: ./profanity [OPTIONS]

  Help:
    --help              Show help information

  Config:
    --config            Path to config file (default: profanity.conf)

  Modes with arguments:
    --matching          Matching input, file or single address.

  Matching configuration:
    --prefix-count      Minimum number of prefix matches, default 0
    --suffix-count      Minimum number of suffix matches, default 6
    --quit-count        Exit the program when the generated number is greater than, default 0

  Device control:
    --skip              Skip device given by index (auto-detected if not set)

  Output control:
    --output            The file to output the results to

Examples:

  ./profanity --matching profanity.txt
  ./profanity --matching profanity.txt --skip 0    (skip GPU-0, use GPU-1)
  ./profanity --matching profanity.txt --prefix-count 1 --suffix-count 8
  ./profanity --matching profanity.txt --prefix-count 1 --suffix-count 10 --quit-count 1
  ./profanity --matching profanity.txt --output result.txt
  ./profanity --config my.conf
  ./profanity --matching TUqEg3dzVEJNQSVW2HY98z5X8SBdhmao8D --prefix-count 2 --suffix-count 4 --quit-count 1

Config file format (one option per line, # for comments):
  matching=profanity.txt
  prefix-count=1
  suffix-count=8
  output=result.txt

About:

  Tron Address Hunter is a GPU-accelerated vanity address generator for TRON.
  Based on ethereum profanity: https://github.com/johguse/profanity
  Project: https://github.com/027123/Tron-Address-Hunter

Note:

  Base58 encoding excludes 4 characters: 0 (zero), O (uppercase), I (uppercase), l (lowercase).
  Patterns containing these characters will never match.

Warning:

  Before using a generated vanity address, always verify that it matches the printed private key.
  And always multi-sign the address to ensure account security.
)";

#endif /* HPP_HELP */
