#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <omp.h>
#include <vector>
#include <cstring>


// ./threads-lab1.exe --input C:/Users/gnipk/source/repos/threads-lab1/x64/Release/imgs/225H_xc.ppm --output C:/Users/gnipk/source/repos/threads-lab1/x64/Release/imgs/225H_xc_new.ppm --omp-threads default --coef 0.00390625
struct PNMImage {
	char format[3];
	int width;
	int height;
	int maxVal;
	std::vector<unsigned char> data;
};

PNMImage loadPNM(std::string filename) {
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

void adjustContrastParallel(PNMImage* image, double coefficient, int numThreads) {
	int pixelCount = image->width * image->height;
	int channels = (strcmp(image->format, "P6") == 0) ? 3 : 1;

	std::vector<std::vector<int>> histograms(channels, std::vector<int>(256, 0));

#pragma omp parallel num_threads(numThreads)
	{
		std::vector<std::vector<int>> localHistograms(channels, std::vector<int>(256, 0));

#pragma omp for schedule(static)
		for (int i = 0; i < pixelCount; i++) {
			for (int c = 0; c < channels; c++) {
				localHistograms[c][image->data[i * channels + c]]++;
			}
		}

#pragma omp critical
		{
			for (int v = 0; v < 256; v++) {
				for (int c = 0; c < channels; c++) {
					histograms[c][v] += localHistograms[c][v];
				}
			}
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

#pragma omp parallel num_threads(numThreads)
	{
#pragma omp for schedule(static)
		for (int i = 0; i < pixelCount * channels; i++) {
			int newValue = (image->data[i] - minVal) * 255 / (maxVal - minVal);
			image->data[i] = static_cast<unsigned char>(std::min(255, std::max(0, newValue)));
		}
	}
}

int main(int argc, char* argv[])
{
	bool useOmp;
	int numThreads = 1;
	std::string input;
	std::string output;
	double coef = 0.0;


#pragma region parse arguments
	for (int i = 1; i < argc; ++i) {
		std::string arg = argv[i];
		if (arg == "--help") {
			printf("Usage: lab0.exe [--help] \n"
				"         < --input [fname] > \n"
				"         < --output [fname] > \n"
				"         < --no-omp | --omp-threads [num_threads | default] > \n");
			return 0;
		}
		else if (arg == "--input" && i + 1 < argc) {
			input = argv[++i];
		}
		else if (arg == "--output" && i + 1 < argc) {
			output = argv[++i];
		}
		else if (arg == "--no-omp") {
			useOmp = false;
		}
		else if (arg == "--omp-threads" && i + 1 < argc) {
			std::string threads = argv[++i];
			useOmp = true;

			if (threads == "default") {
				numThreads = omp_get_max_threads();

			}
			else {
				numThreads = std::stoi(threads);
				int maxThreads = omp_get_max_threads();
				if (numThreads <= 0) {
					fprintf(stderr, "Error: Invalid number of threads.\n");
					return 1;
				}
				else if(numThreads > maxThreads){
					numThreads = maxThreads;
				}
			}
		}
		else if (arg == "--coef" && i + 1 < argc) {
			coef = atof(argv[++i]);
			if (coef < 0.0 || coef >= 0.5) {
				fprintf(stderr, "Error: Invalid coefficient (should be in [0.0, 0.5)).\n");
				return 1;
			}
		}
		else {
			fprintf(stderr, "Error: Unknown argument: %s\n", arg.c_str());
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

	if (useOmp) {
		start = omp_get_wtime();
		adjustContrastParallel(&image, coef, numThreads);
		end = omp_get_wtime();
	}
	else {
		start = omp_get_wtime();
		adjustContrast(&image, coef);
		end = omp_get_wtime();
	}

	savePNM(&image, output);

	printf("%i threads ", numThreads);
	printf("%f ms\n", (end - start) * 1000);

	return 0;
}
