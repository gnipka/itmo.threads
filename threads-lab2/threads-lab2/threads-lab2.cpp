#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <functional>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <omp.h>

enum class ScheduleType { Static, Dynamic };

struct PNMImage {
	char format[3];
	int width;
	int height;
	int maxVal;
	std::vector<unsigned char> data;
};

PNMImage loadPNM(const std::string& filename) {
	FILE* file = fopen(filename.c_str(), "rb");
	if (!file) {
		throw std::runtime_error("Error: Cannot open file " + filename);
	}

	PNMImage image;
	if (fscanf(file, "%2s\n", image.format) != 1 || (strcmp(image.format, "P5") && strcmp(image.format, "P6"))) {
		fclose(file);
		throw std::runtime_error("Error: Unsupported PNM format");
	}

	char c;

	// %c - считает один символ перевода строки после чтения хедера
	if (fscanf(file, "%d %d\n%d\n%с", &image.width, &image.height, &image.maxVal, &c) != 3) {
		fclose(file);
		throw std::runtime_error("Error: Invalid PNM header");
	}

	int dataSize = image.width * image.height * (strcmp(image.format, "P6") == 0 ? 3 : 1);
	image.data.resize(dataSize);

	if (fread(image.data.data(), 1, dataSize, file) != (size_t)dataSize) {
		fclose(file);
		throw std::runtime_error("Error: Failed to read image data");
	}

	fclose(file);
	return image;
}

void savePNM(const PNMImage* image, const std::string& filename) {
	FILE* file = fopen(filename.c_str(), "wb");
	if (!file) {
		throw std::runtime_error("Error: Cannot open file for writing " + filename);
	}

	fprintf(file, "%s\n%d %d\n%d\n", image->format, image->width, image->height, image->maxVal);
	fwrite(image->data.data(), 1, image->data.size(), file);
	fclose(file);
}

void scheduleStatic(int total, int chunkSize, int numThreads, const std::function<void(int, int)>& func) {
	std::vector<std::thread> threads;
	
	int chunks;

	if (chunkSize == 0) {
		chunks = total / numThreads;
	}
	else {
		chunks = (total + chunkSize - 1) / chunkSize;
	}

	for (int t = 0; t < numThreads; ++t) {
		threads.emplace_back([=, &func]() {
			for (int i = t; i < chunks; i += numThreads) {
				int start = i * chunkSize;
				int end = std::min(start + chunkSize, total);
				func(start, end);
			}
			});
	}
	for (auto& t : threads) t.join();
}

void scheduleDynamic(int total, int chunkSize, int numThreads, const std::function<void(int, int)>& func) {

	if (chunkSize == 0) {
		chunkSize = 1;
	}

	std::atomic<int> index(0);
	std::vector<std::thread> threads;

	for (int t = 0; t < numThreads; ++t) {
		threads.emplace_back([&]() {
			while (true) {
				int i = index.fetch_add(chunkSize);
				if (i >= total) break;
				int end = std::min(i + chunkSize, total);
				func(i, end);
			}
			});
	}
	for (auto& t : threads) t.join();
}

void adjustContrast(PNMImage* image, double coefficient) {
	int pixelCount = image->width * image->height;
	int channels = (strcmp(image->format, "P6") == 0) ? 3 : 1;

	std::vector<std::vector<int>> histograms(channels, std::vector<int>(256, 0));

	for (int i = 0; i < pixelCount; i++) {
		for (int c = 0; c < channels; c++) {
			histograms[c][image->data[i * channels + c]]++;
		}
	}

	int ignorePixels = static_cast<int>(pixelCount * coefficient);
	int minVal = 255, maxVal = 0;


	for (int c = 0; c < channels; c++) {
		int localMinVal = 0, localMaxVal = 255, sum = 0;
		for (int v = 0; v < 256; ++v) {
			sum += histograms[c][v];
			if (sum > ignorePixels) {
				localMinVal = v;
				break;
			}
		}


		sum = 0;
		for (int v = 255; v >= 0; --v) {
			sum += histograms[c][v];
			if (sum > ignorePixels) {
				localMaxVal = v;
				break;
			}
		}

		minVal = std::min(minVal, localMinVal);
		maxVal = std::max(maxVal, localMaxVal);
	}

	if (minVal == maxVal)
		return;

	for (int i = 0; i < pixelCount * channels; i++) {
		int newValue = (image->data[i] - minVal) * 255 / (maxVal - minVal);
		image->data[i] = static_cast<unsigned char>(std::min(255, std::max(0, newValue)));
	}
}

void adjustContrastParallel(PNMImage* image, double coefficient, int numThreads, ScheduleType scheduleType, int chunkSize) {
	int pixelCount = image->width * image->height;
	int channels = strcmp(image->format, "P6") == 0 ? 3 : 1;

	std::vector<std::vector<int>> histograms(channels, std::vector<int>(256, 0));
	std::vector<std::vector<std::vector<int>>> localHist(numThreads, std::vector<std::vector<int>>(channels, std::vector<int>(256, 0)));

	int index = 0;
	auto runScheduler = [&](const std::function<void(int, int, int)>& func) {
		if (scheduleType == ScheduleType::Static) {
			scheduleStatic(pixelCount, chunkSize, numThreads, [&](int start, int end) {
				int tid = index++ % numThreads;
				func(start, end, tid);
				});
		}
		else {
			std::atomic<int> threadCounter(0);
			scheduleDynamic(pixelCount, chunkSize, numThreads, [&](int start, int end) {
				int tid = threadCounter++ % numThreads;
				func(start, end, tid);
				});
		}
		};

	auto histFunc = [&](int start, int end, int threadId) {
		for (int i = start; i < end; ++i)
			for (int c = 0; c < channels; ++c)
				localHist[threadId][c][image->data[i * channels + c]]++;
		};

	runScheduler(histFunc);

	for (int c = 0; c < channels; ++c)
		for (int i = 0; i < 256; ++i)
			for (int t = 0; t < numThreads; ++t)
				histograms[c][i] += localHist[t][c][i];

	int ignorePixels = static_cast<int>(pixelCount * coefficient);
	int minVal = 255, maxVal = 0;

	for (int c = 0; c < channels; c++) {
		int localMinVal = 0, localMaxVal = 255, sum = 0;
		for (int v = 0; v < 256; ++v) {
			sum += histograms[c][v];
			if (sum > ignorePixels) {
				localMinVal = v;
				break;
			}
		}


		sum = 0;
		for (int v = 255; v >= 0; --v) {
			sum += histograms[c][v];
			if (sum > ignorePixels) {
				localMaxVal = v;
				break;
			}
		}

		minVal = std::min(minVal, localMinVal);
		maxVal = std::max(maxVal, localMaxVal);
	}

	if (minVal == maxVal)
		return;

	auto contrastFunc = [&](int start, int end, int) {
		for (int i = start * channels; i < end * channels; ++i) {
			int newValue = (image->data[i] - minVal) * 255 / (maxVal - minVal);
			image->data[i] = static_cast<unsigned char>(std::min(255, std::max(0, newValue)));
		}};

	runScheduler(contrastFunc);
}

int main(int argc, char* argv[]) {
	bool useCppThreads = false;
	int numThreads = 1;
	std::string input;
	std::string output;
	double coef = 0.0;
	ScheduleType scheduleType = ScheduleType::Static;
	int chunkSize = 0;

#pragma region parse arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--help") {
			printf("Usage: threads-lab2.exe [--help] \n"
				"         < --input [fname] > \n"
				"         < --output [fname] > \n"
				"         < --coef [0.0, 0.5) > \n"
				"		  < --no - cpp - threads | --cpp - threads[num_threads | default] > \n"
				"         < --schedule [static|dynamic] > \n"
				"         < --chunk-size [num_chunk_size] > \n");

			return 0;
		}
		else if (arg == "--input" && i + 1 < argc) {
			input = argv[++i];
		}
		else if (arg == "--output" && i + 1 < argc) {
			output = argv[++i];
		}
		else if (arg == "--no-cpp-threads") {
			useCppThreads = false;
		}
		else if (arg == "--cpp-threads" && i + 1 < argc) {
			std::string val = argv[++i];
			useCppThreads = true;
			numThreads = (val == "default") ? std::thread::hardware_concurrency() : std::stoi(val);
			if (numThreads <= 0) numThreads = 1;
		}
		else if (arg == "--coef" && i + 1 < argc) {
			coef = std::stod(argv[++i]);
			if (coef < 0.0 || coef >= 0.5) {
				std::cerr << "Invalid coef (must be in [0.0, 0.5))\n";
				return 1;
			}
		}
		else if (arg == "--schedule" && i + 1 < argc) {
			std::string type = argv[++i];
			scheduleType = (type == "dynamic") ? ScheduleType::Dynamic : ScheduleType::Static;
		}
		else if (arg == "--chunk-size" && i + 1 < argc) {
			chunkSize = std::stoi(argv[++i]);

			if (chunkSize < 1) {
				std::cerr << "Invalid chunk size (must be grater 1)\n";
				return 1;
			}
		}
		else {
			std::cerr << "Unknown argument: " << arg << "\n";
			return 1;
		}
	}
#pragma endregion

	PNMImage image;
	try {
		image = loadPNM(input);
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}

	double start = 0;
	double end = 0;

	if (!useCppThreads || numThreads == 1) {
		start = omp_get_wtime();
		adjustContrast(&image, coef);
		end = omp_get_wtime();
	}
	else {
		start = omp_get_wtime();
		adjustContrastParallel(&image, coef, numThreads, scheduleType, chunkSize);
		end = omp_get_wtime();
	}

	savePNM(&image, output);

	printf("%i threads ", numThreads);
	printf("%f ms\n", (end - start) * 1000);

	return 0;
}
