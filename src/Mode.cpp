#include "Mode.hpp"
#include <stdexcept>
#include <cstring>

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <string>

Mode::Mode() : score(0), prefixCount(0), suffixCount(0), matchingCount(0) {

}

static std::string::size_type hexValueNoException(char c) {
	const std::string hex = "0123456789abcdef";
	const std::string::size_type ret = hex.find(tolower(c));
	return ret;
}

static const std::string BASE58_CHARS = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static bool isBase58(char c) {
	return BASE58_CHARS.find(c) != std::string::npos;
}

static std::string findInvalidBase58Chars(const std::string &s) {
	std::string invalid;
	for (const char &c : s) {
		if (!isBase58(c) && invalid.find(c) == std::string::npos) {
			invalid += c;
		}
	}
	return invalid;
}

Mode Mode::matching(std::string matchingInput) {
	Mode r;
	std::vector<std::string> matchingList;

	if(matchingInput.size() == 34 && matchingInput[0] == 'T') {
		std::stringstream ss;
		matchingInput.erase(10, 14);
		for (const char &item: matchingInput) {
			ss << std::hex << int(item);
		}
		matchingList.push_back(ss.str());
	} else {
		std::ifstream file(matchingInput);
		if (file.is_open()) {
			// First pass: read all lines and validate Base58 characters
			std::vector<std::pair<size_t, std::string>> allLines;  // lineNumber, content
			std::vector<std::pair<size_t, std::string>> invalidBase58Lines;  // lineNumber, content
			{
				std::string line;
				size_t lineNumber = 0;
				while (std::getline(file, line)) {
					++lineNumber;
					if (!line.empty() && line.back() == '\r') {
						line.pop_back();
					}
					if (line.empty()) {
						continue;
					}
					std::string invalidChars = findInvalidBase58Chars(line);
					if (!invalidChars.empty()) {
						invalidBase58Lines.push_back({lineNumber, line});
					} else {
						allLines.push_back({lineNumber, line});
					}
				}
			}

			// If there are invalid Base58 lines, prompt the user
			if (!invalidBase58Lines.empty()) {
				std::cout << std::endl;
				std::cout << "WARNING: Found " << invalidBase58Lines.size() << " line(s) with invalid Base58 characters:" << std::endl;
				std::cout << "  (Base58 does not include: 0 O I l)" << std::endl;
				std::cout << std::endl;
				for (const auto &p : invalidBase58Lines) {
					std::string invalidChars = findInvalidBase58Chars(p.second);
					std::cout << "  Line " << p.first << ": \"" << p.second << "\" -> invalid char(s): ";
					for (size_t i = 0; i < invalidChars.size(); ++i) {
						if (i > 0) std::cout << ", ";
						std::cout << "'" << invalidChars[i] << "'";
					}
					std::cout << std::endl;
				}
				std::cout << std::endl;

				if (allLines.empty()) {
					// All lines are invalid
					std::cout << "No valid patterns to process." << std::endl;
					std::cout << "Press Enter to exit..." << std::endl;
					std::cin.get();
					return r;
				}

				// Some lines are valid, prompt user
				std::cout << "Press Enter to skip all invalid lines and continue (" << allLines.size() << " valid pattern(s) remaining), or Ctrl+C to exit and fix..." << std::endl;
				std::cin.get();
			}

			// Second pass: process valid lines
			size_t skippedCount = 0;
			for (const auto &p : allLines) {
				std::string line = p.second;
				size_t lineNumber = p.first;
				std::stringstream ss;
				if(line.size() == 20 || line.size() == 34) {
					if(line.size() == 34) {
						line.erase(10, 14);
					}
					for (const char &item: line) {
						ss << std::hex << int(item);
					}
					matchingList.push_back(ss.str());
				} else {
					++skippedCount;
					std::cout << "  warning: line " << lineNumber << " skipped (length " << line.size() << ", expected 20 or 34): " << line << std::endl;
				}
			}

			// Final summary
			size_t totalLines = allLines.size() + invalidBase58Lines.size();
			size_t totalSkipped = invalidBase58Lines.size() + skippedCount;
			std::cout << "  Loaded " << matchingList.size() << "/" << totalLines << " pattern(s) from " << matchingInput;
			if (totalSkipped > 0) {
				std::cout << " (" << totalSkipped << " skipped)";
			}
			std::cout << std::endl;
		} else {
			std::cout << "error: Failed to open matching file '" << matchingInput << "': " << strerror(errno) << std::endl;
		}
	}
	
	if(matchingList.size() > 0) {
		r.matchingCount = matchingList.size();
		for( size_t j = 0; j < matchingList.size(); j += 1) {
			const std::string matchingItem = matchingList[j];
			for( size_t i = 0; i < matchingItem.size(); i += 2 ) {
				const size_t indexHi = hexValueNoException(matchingItem[i]);
				const size_t indexLo = (i + 1) < matchingItem.size() ? hexValueNoException(matchingItem[i+1]) : std::string::npos;
				const unsigned long valHi = (indexHi == std::string::npos) ? 0 : indexHi << 4;
				const unsigned long valLo = (indexLo == std::string::npos) ? 0 : indexLo;
				const int maskHi = (indexHi == std::string::npos) ? 0 : 0xF << 4;
				const int maskLo = (indexLo == std::string::npos) ? 0 : 0xF;
				r.data1.push_back(maskHi | maskLo);
				r.data2.push_back(valHi | valLo);
			}
		}
	}

	return r;
}
