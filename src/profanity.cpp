#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <map>
#include <set>
#include <ctime>
#include <chrono>
#include <filesystem>

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.h>
#include <OpenCL/cl_ext.h> // Included to get topology to get an actual unique identifier per device
#else
#include <CL/cl.h>
#include <CL/cl_ext.h> // Included to get topology to get an actual unique identifier per device
#endif

#define CL_DEVICE_PCI_BUS_ID_NV 0x4008
#define CL_DEVICE_PCI_SLOT_ID_NV 0x4009

#include "Dispatcher.hpp"
#include "ArgParser.hpp"
#include "Mode.hpp"
#include "help.hpp"
#include "../kernel/kernel_profanity.hpp"
#include "../kernel/kernel_sha256.hpp"
#include "../kernel/kernel_keccak.hpp"

std::string readFile(const char *const szFilename)
{
	std::ifstream in(szFilename, std::ios::in | std::ios::binary);
	std::ostringstream contents;
	contents << in.rdbuf();
	return contents.str();
}

std::vector<cl_device_id> getAllDevices(cl_device_type deviceType = CL_DEVICE_TYPE_GPU)
{
	std::vector<cl_device_id> vDevices;

	cl_uint platformIdCount = 0;
	clGetPlatformIDs(0, NULL, &platformIdCount);

	std::vector<cl_platform_id> platformIds(platformIdCount);
	clGetPlatformIDs(platformIdCount, platformIds.data(), NULL);

	for (auto it = platformIds.cbegin(); it != platformIds.cend(); ++it)
	{
		cl_uint countDevice;
		clGetDeviceIDs(*it, deviceType, 0, NULL, &countDevice);

		std::vector<cl_device_id> deviceIds(countDevice);
		clGetDeviceIDs(*it, deviceType, countDevice, deviceIds.data(), &countDevice);

		std::copy(deviceIds.begin(), deviceIds.end(), std::back_inserter(vDevices));
	}

	return vDevices;
}

template <typename T, typename U, typename V, typename W>
T clGetWrapper(U function, V param, W param2)
{
	T t;
	function(param, param2, sizeof(t), &t, NULL);
	return t;
}

template <typename U, typename V, typename W>
std::string clGetWrapperString(U function, V param, W param2)
{
	size_t len;
	function(param, param2, 0, NULL, &len);
	char *const szString = new char[len];
	function(param, param2, len, szString, NULL);
	std::string r(szString);
	delete[] szString;
	return r;
}

template <typename T, typename U, typename V, typename W>
std::vector<T> clGetWrapperVector(U function, V param, W param2)
{
	size_t len;
	function(param, param2, 0, NULL, &len);
	len /= sizeof(T);
	std::vector<T> v;
	if (len > 0)
	{
		T *pArray = new T[len];
		function(param, param2, len * sizeof(T), pArray, NULL);
		for (size_t i = 0; i < len; ++i)
		{
			v.push_back(pArray[i]);
		}
		delete[] pArray;
	}
	return v;
}

std::vector<std::string> getBinaries(cl_program &clProgram)
{
	std::vector<std::string> vReturn;
	auto vSizes = clGetWrapperVector<size_t>(clGetProgramInfo, clProgram, CL_PROGRAM_BINARY_SIZES);
	if (!vSizes.empty())
	{
		unsigned char **pBuffers = new unsigned char *[vSizes.size()];
		for (size_t i = 0; i < vSizes.size(); ++i)
		{
			pBuffers[i] = new unsigned char[vSizes[i]];
		}

		clGetProgramInfo(clProgram, CL_PROGRAM_BINARIES, vSizes.size() * sizeof(unsigned char *), pBuffers, NULL);
		for (size_t i = 0; i < vSizes.size(); ++i)
		{
			std::string strData(reinterpret_cast<char *>(pBuffers[i]), vSizes[i]);
			vReturn.push_back(strData);
			delete[] pBuffers[i];
		}

		delete[] pBuffers;
	}

	return vReturn;
}

unsigned int getUniqueDeviceIdentifier(const cl_device_id &deviceId)
{
#if defined(CL_DEVICE_TOPOLOGY_AMD)
	auto topology = clGetWrapper<cl_device_topology_amd>(clGetDeviceInfo, deviceId, CL_DEVICE_TOPOLOGY_AMD);
	if (topology.raw.type == CL_DEVICE_TOPOLOGY_TYPE_PCIE_AMD)
	{
		return (topology.pcie.bus << 16) + (topology.pcie.device << 8) + topology.pcie.function;
	}
#endif
	cl_int bus_id = clGetWrapper<cl_int>(clGetDeviceInfo, deviceId, CL_DEVICE_PCI_BUS_ID_NV);
	cl_int slot_id = clGetWrapper<cl_int>(clGetDeviceInfo, deviceId, CL_DEVICE_PCI_SLOT_ID_NV);
	return (bus_id << 16) + slot_id;
}

template <typename T>
bool printResult(const T &t, const cl_int &err)
{
	std::cout << ((t == NULL) ? toString(err) : "Done") << std::endl;
	return t == NULL;
}

bool printResult(const cl_int err)
{
	std::cout << ((err != CL_SUCCESS) ? toString(err) : "Done") << std::endl;
	return err != CL_SUCCESS;
}

std::string getDeviceCacheFilename(cl_device_id &d, const size_t &inverseSize)
{
	const auto uniqueId = getUniqueDeviceIdentifier(d);
	return "cache-opencl." + toString(inverseSize) + "." + toString(uniqueId);
}

int main(int argc, char **argv)
{
	try
	{
		ArgParser argp(argc, argv);
		bool bHelp = false;

		std::string matchingInput;
		std::string outputFile;
		std::vector<size_t> vDeviceSkipIndex;
		size_t worksizeLocal = 128;
		size_t worksizeMax = 0;
		bool bNoCache = false;
		size_t inverseSize = 128;
		size_t inverseMultiple = 32768;
		size_t prefixCount = 0;
		size_t suffixCount = 6;
		size_t quitCount = 0;

		// Load config file: -c/--config from command line, or default profanity.conf
		std::string configFile = argp.findValue('c', "config");
		if (!configFile.empty()) {
			if (!argp.loadFromFile(configFile)) {
				std::cout << "error: Failed to open config file '" << configFile << "'" << std::endl;
				return 1;
			}
		} else {
			argp.loadFromFile("profanity.conf");
		}

		argp.addSwitch('h', "help", bHelp);
		argp.addSwitch('m', "matching", matchingInput);
		argp.addSwitch('w', "work", worksizeLocal);
		argp.addSwitch('W', "work-max", worksizeMax);
		argp.addSwitch('n', "no-cache", bNoCache);
		argp.addSwitch('o', "output", outputFile);
		argp.addSwitch('i', "inverse-size", inverseSize);
		argp.addSwitch('I', "inverse-multiple", inverseMultiple);
		argp.addSwitch('b', "prefix-count", prefixCount);
		argp.addSwitch('e', "suffix-count", suffixCount);
		argp.addSwitch('q', "quit-count", quitCount);
		argp.addMultiSwitch('s', "skip", vDeviceSkipIndex);

		if (!argp.parse())
		{
			std::cout << "error: bad arguments, try again :<" << std::endl;
			return 1;
		}

		if (bHelp)
		{
			std::cout << g_strHelp << std::endl;
			return 0;
		}

		// Expand {date} placeholder in output filename to YYYYMMDD_HHMM
		{
			auto pos = outputFile.find("{date}");
			if (pos != std::string::npos) {
				auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
				struct tm tm_buf;
#ifdef _WIN32
				localtime_s(&tm_buf, &now);
#else
				localtime_r(&now, &tm_buf);
#endif
				char dateBuf[16];
				std::strftime(dateBuf, sizeof(dateBuf), "%Y%m%d_%H%M", &tm_buf);
				outputFile.replace(pos, 6, dateBuf);
			}
		}

		// Auto-create output directory if it doesn't exist
		if (!outputFile.empty()) {
			std::filesystem::path outPath(outputFile);
			auto parentDir = outPath.parent_path();
			if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
				std::error_code ec;
				std::filesystem::create_directories(parentDir, ec);
				if (ec) {
					std::cout << "error: failed to create output directory '" << parentDir.string() << "': " << ec.message() << std::endl;
					return 1;
				}
				std::cout << "  Created output directory: " << parentDir.string() << std::endl;
			}
		}

		if (matchingInput.empty())
		{
			std::cout << "error: matching file must be specified :<" << std::endl;
			return 1;
		}

		if (prefixCount > 10)
		{
			std::cout << "error: prefix-count must be 0-10, got " << prefixCount << std::endl;
			return 1;
		}

		if (suffixCount > 10)
		{
			std::cout << "error: suffix-count must be 0-10, got " << suffixCount << std::endl;
			return 1;
		}

		Mode mode = Mode::matching(matchingInput);

		if (mode.matchingCount <= 0)
		{
			std::cout << "error: please check your matching file to make sure the path and format are correct :<" << std::endl;
			return 1;
		}

		mode.prefixCount = prefixCount;
		mode.suffixCount = suffixCount;

		// Startup summary
		std::cout << std::endl;
		std::cout << "Configuration:" << std::endl;
		std::cout << "  Matching:      " << matchingInput << " (" << mode.matchingCount << " pattern(s))" << std::endl;
		std::cout << "  Prefix count:  " << prefixCount << std::endl;
		std::cout << "  Suffix count:  " << suffixCount << std::endl;
		if (quitCount > 0) {
			std::cout << "  Quit count:    " << quitCount << std::endl;
		}
		if (!outputFile.empty()) {
			std::cout << "  Output file:   " << outputFile << std::endl;
		}
		std::cout << std::endl;

		std::vector<cl_device_id> vFoundDevices = getAllDevices();
		std::vector<cl_device_id> vDevices;
		std::map<cl_device_id, size_t> mDeviceIndex;

		std::vector<std::string> vDeviceBinary;
		std::vector<size_t> vDeviceBinarySize;
		cl_int errorCode;
		bool bUsedCache = false;

		std::cout << "Devices:" << std::endl;
		for (size_t i = 0; i < vFoundDevices.size(); ++i)
		{
			if (std::find(vDeviceSkipIndex.begin(), vDeviceSkipIndex.end(), i) != vDeviceSkipIndex.end())
			{
				continue;
			}
			cl_device_id &deviceId = vFoundDevices[i];
			const auto strName = clGetWrapperString(clGetDeviceInfo, deviceId, CL_DEVICE_NAME);
			const auto computeUnits = clGetWrapper<cl_uint>(clGetDeviceInfo, deviceId, CL_DEVICE_MAX_COMPUTE_UNITS);
			const auto globalMemSize = clGetWrapper<cl_ulong>(clGetDeviceInfo, deviceId, CL_DEVICE_GLOBAL_MEM_SIZE);
			bool precompiled = false;

			if (!bNoCache)
			{
				std::ifstream fileIn(getDeviceCacheFilename(deviceId, inverseSize), std::ios::binary);
				if (fileIn.is_open())
				{
					vDeviceBinary.push_back(std::string((std::istreambuf_iterator<char>(fileIn)), std::istreambuf_iterator<char>()));
					vDeviceBinarySize.push_back(vDeviceBinary.back().size());
					precompiled = true;
				}
			}

			std::cout << "  GPU-" << i << ": " << strName << ", " << globalMemSize << " bytes available, " << computeUnits << " compute units (precompiled = " << (precompiled ? "yes" : "no") << ")" << std::endl;
			vDevices.push_back(vFoundDevices[i]);
			mDeviceIndex[vFoundDevices[i]] = i;
		}

		if (vDevices.empty())
		{
			return 1;
		}

		// Group devices by platform - OpenCL requires all devices in a context to be from the same platform
		if (vDevices.size() > 1)
		{
			std::map<cl_platform_id, std::vector<cl_device_id>> platformDevices;
			for (auto &dev : vDevices)
			{
				cl_platform_id platId;
				clGetDeviceInfo(dev, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platId, NULL);
				platformDevices[platId].push_back(dev);
			}

			if (platformDevices.size() > 1)
			{
				// Find the platform with the most compute units (likely the discrete GPU)
				cl_platform_id bestPlatform = platformDevices.begin()->first;
				cl_uint bestComputeUnits = 0;
				for (auto &pair : platformDevices)
				{
					cl_uint totalUnits = 0;
					for (auto &dev : pair.second)
					{
						totalUnits += clGetWrapper<cl_uint>(clGetDeviceInfo, dev, CL_DEVICE_MAX_COMPUTE_UNITS);
					}
					if (totalUnits > bestComputeUnits)
					{
						bestComputeUnits = totalUnits;
						bestPlatform = pair.first;
					}
				}

				// Filter to only keep devices from the best platform
				std::vector<cl_device_id> vFilteredDevices;
				std::vector<std::string> vFilteredBinary;
				std::vector<size_t> vFilteredBinarySize;
				size_t binaryIdx = 0;
				for (size_t i = 0; i < vDevices.size(); ++i)
				{
					cl_platform_id platId;
					clGetDeviceInfo(vDevices[i], CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platId, NULL);
					if (platId == bestPlatform)
					{
						vFilteredDevices.push_back(vDevices[i]);
						if (binaryIdx < vDeviceBinary.size())
						{
							vFilteredBinary.push_back(vDeviceBinary[binaryIdx]);
							vFilteredBinarySize.push_back(vDeviceBinarySize[binaryIdx]);
						}
					}
					else
					{
						const auto skippedName = clGetWrapperString(clGetDeviceInfo, vDevices[i], CL_DEVICE_NAME);
						std::cout << std::endl << "  Note: Skipping GPU-" << mDeviceIndex[vDevices[i]] << " (" << skippedName << ") - different OpenCL platform, cannot mix vendors in one context." << std::endl;
						mDeviceIndex.erase(vDevices[i]);
					}
					if (i < vDeviceBinary.size()) binaryIdx++;
				}
				vDevices = vFilteredDevices;
				vDeviceBinary = vFilteredBinary;
				vDeviceBinarySize = vFilteredBinarySize;

				if (vDevices.empty())
				{
					return 1;
				}
			}
		}

		std::cout << std::endl;
		std::cout << "OpenCL:" << std::endl;
		std::cout << "  Context creating ..." << std::flush;
		auto clContext = clCreateContext(NULL, vDevices.size(), vDevices.data(), NULL, NULL, &errorCode);
		if (printResult(clContext, errorCode))
		{
			return 1;
		}

		cl_program clProgram;
		if (vDeviceBinary.size() == vDevices.size())
		{
			// Create program from binaries
			bUsedCache = true;

			std::cout << "  Binary kernel loading..." << std::flush;
			std::vector<const unsigned char *> vKernels(vDevices.size());
			for (size_t i = 0; i < vDeviceBinary.size(); ++i)
			{
				vKernels[i] = reinterpret_cast<const unsigned char *>(vDeviceBinary[i].data());
			}

			std::vector<cl_int> vStatus(vDevices.size());

			clProgram = clCreateProgramWithBinary(clContext, vDevices.size(), vDevices.data(), vDeviceBinarySize.data(), vKernels.data(), vStatus.data(), &errorCode);
			if (printResult(clProgram, errorCode))
			{
				clReleaseContext(clContext);
				return 1;
			}
		}
		else
		{
			std::cout << "  Kernel compiling ..." << std::flush;
			const char *szKernels[] = { kernel_keccak.c_str(), kernel_sha256.c_str(), kernel_profanity.c_str() };
			clProgram = clCreateProgramWithSource(clContext, sizeof(szKernels) / sizeof(char *), szKernels, NULL, &errorCode);
			if (printResult(clProgram, errorCode))
			{
				clReleaseContext(clContext);
				return 1;
			}
		}

		// Build the program
		std::cout << "  Program building ..." << std::flush;
		const std::string strBuildOptions = "-D PROFANITY_INVERSE_SIZE=" + toString(inverseSize) + " -D PROFANITY_MAX_SCORE=" + toString(PROFANITY_MAX_SCORE);
		if (printResult(clBuildProgram(clProgram, vDevices.size(), vDevices.data(), strBuildOptions.c_str(), NULL, NULL)))
		{
			// Print build log on failure
			for (size_t i = 0; i < vDevices.size(); ++i)
			{
				size_t logSize = 0;
				clGetProgramBuildInfo(clProgram, vDevices[i], CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
				if (logSize > 1)
				{
					std::string buildLog(logSize, '\0');
					clGetProgramBuildInfo(clProgram, vDevices[i], CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], NULL);
					std::cerr << "  Build log (GPU-" << i << "): " << buildLog << std::endl;
				}
			}
			clReleaseProgram(clProgram);
			clReleaseContext(clContext);
			return 1;
		}

		// Print build warnings if any (skip NVIDIA noinline info messages)
		for (size_t i = 0; i < vDevices.size(); ++i)
		{
			size_t logSize = 0;
			clGetProgramBuildInfo(clProgram, vDevices[i], CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
			if (logSize > 2)
			{
				std::string buildLog(logSize, '\0');
				clGetProgramBuildInfo(clProgram, vDevices[i], CL_PROGRAM_BUILD_LOG, logSize, &buildLog[0], NULL);
				// Trim trailing whitespace/nulls
				size_t end = buildLog.find_last_not_of(" \t\n\r\0");
				if (end != std::string::npos && end > 0)
				{
					std::string trimmed = buildLog.substr(0, end + 1);
					// Skip if log only contains NVIDIA noinline info messages
					if (trimmed.find("warning:") != std::string::npos || trimmed.find("error:") != std::string::npos)
					{
						std::cout << "  Build log (GPU-" << i << "): " << trimmed << std::endl;
					}
				}
			}
		}

		// Save binary to improve future start times
		if (!bUsedCache && !bNoCache)
		{
			std::cout << "  Program saving ..." << std::flush;
			auto binaries = getBinaries(clProgram);
			for (size_t i = 0; i < binaries.size(); ++i)
			{
				std::ofstream fileOut(getDeviceCacheFilename(vDevices[i], inverseSize), std::ios::binary);
				fileOut.write(binaries[i].data(), binaries[i].size());
			}
			std::cout << "Done" << std::endl;
		}

		std::cout << std::endl;

		Dispatcher d(clContext, clProgram, mode, worksizeMax == 0 ? inverseSize * inverseMultiple : worksizeMax, inverseSize, inverseMultiple, quitCount, outputFile);

		for (auto &i : vDevices)
		{
			d.addDevice(i, worksizeLocal, mDeviceIndex[i]);
		}

		d.run();
		clReleaseProgram(clProgram);
		clReleaseContext(clContext);
		return 0;
	}
	catch (std::runtime_error &e)
	{
		std::cout << "std::runtime_error - " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "unknown exception occured" << std::endl;
	}

	return 1;
}
