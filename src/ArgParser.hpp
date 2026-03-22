#ifndef HPP_ARGPARSER
#define HPP_ARGPARSER

#include <type_traits>
#include <stdexcept>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include "lexical_cast.hpp"

class ArgParser {
	private:
		class IArgument {
			public:
				virtual ~IArgument() {}
				virtual void parse(const std::string & s) = 0;
		};

		template <typename T>
		class Argument : public IArgument {
			public:
				Argument(T & t) : m_t(t) {}
				~Argument() {}
	
				void parse(const std::string & s) {
					m_t = fromString<T>(s);
				}
	
			private:
				T & m_t;
		};

		template <typename T>
		class MultiArgument : public IArgument {
			public:
				MultiArgument(std::vector<T> & t) : m_t(t) {}
				MultiArgument() {}

				void parse(const std::string & s) {
					m_t.push_back(fromString<T>(s));
				}

			private:
				std::vector<T> & m_t;
		};

	public:
		ArgParser(int argc, char * * argv) {
			for (int i = 1; i < argc; ++i) {
				m_args.push_back(argv[i]);
			}
		}

		~ArgParser() {
			for (auto & i : m_mapArgs) {
				delete i.second.second; // :)
			}
		}

		template <typename T>
		void addSwitch(const char switchShort, const std::string switchLong, T & t) {
			const std::string strShort = std::string("-") + switchShort;
			const std::string strLong = std::string("--") + switchLong;

			// :)
			IArgument * const pArgShort = new Argument<T>(t);
			IArgument * const pArgLong = new Argument<T>(t);
			m_mapArgs[strShort] = std::pair<bool, IArgument *>(std::is_same<bool, T>::value, pArgShort);
			m_mapArgs[strLong] = std::pair<bool, IArgument *>(std::is_same<bool, T>::value, pArgLong);
		}

		template <typename T>
		void addMultiSwitch(const char switchShort, const std::string switchLong, std::vector<T> & t) {
			const std::string strShort = std::string("-") + switchShort;
			const std::string strLong = std::string("--") + switchLong;

			// :)
			IArgument * const pArgShort = new MultiArgument<T>(t);
			IArgument * const pArgLong = new MultiArgument<T>(t);
			m_mapArgs[strShort] = std::pair<bool, IArgument *>(false, pArgShort);
			m_mapArgs[strLong] = std::pair<bool, IArgument *>(false, pArgLong);
		}

		// Pre-scan for a specific argument value before full parsing
		std::string findValue(const char switchShort, const std::string & switchLong) const {
			const std::string strShort = std::string("-") + switchShort;
			const std::string strLong = std::string("--") + switchLong;
			for (size_t i = 0; i + 1 < m_args.size(); ++i) {
				if (m_args[i] == strShort || m_args[i] == strLong) {
					return m_args[i + 1];
				}
			}
			return "";
		}

		bool loadFromFile(const std::string & filename) {
			std::ifstream file(filename);
			if (!file.is_open()) {
				return false;
			}

			std::string line;
			std::vector<std::string> fileArgs;
			while (std::getline(file, line)) {
				// Remove trailing \r from Windows line endings
				if (!line.empty() && line.back() == '\r') {
					line.pop_back();
				}
				// Skip empty lines and comments
				if (line.empty() || line[0] == '#') {
					continue;
				}
				auto pos = line.find('=');
				if (pos == std::string::npos) {
					continue;
				}
				std::string key = line.substr(0, pos);
				std::string value = line.substr(pos + 1);
				fileArgs.push_back("--" + key);
				if (value != "true") {
					fileArgs.push_back(value);
				}
			}

			// Prepend file args so command line args take precedence
			fileArgs.insert(fileArgs.end(), m_args.begin(), m_args.end());
			m_args = fileArgs;

			std::cout << "  Config loaded from " << filename << std::endl;
			return true;
		}

		bool parse() const {
			try {
				std::vector<std::string>::size_type i = 0;

				while (i < m_args.size()) {
					auto p = m_mapArgs.at(m_args[i]);
					const std::string s = p.first ? "1" : m_args.at(i + 1);

					p.second->parse(s);
					i += (p.first ? 1 : 2);
				}
			}
			catch (std::out_of_range & e) {
				return false;
			}

			return true;
		}

	private:
		std::vector<std::string> m_args;
		std::map<std::string, std::pair<bool, IArgument *>> m_mapArgs;
};

#endif /* HPP_ARGPARSER */