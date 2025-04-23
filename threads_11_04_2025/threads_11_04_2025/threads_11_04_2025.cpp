//#define CL_TARGET_OPENCL_VERSION 120
//#ifdef __APPLE__
//#include <OpenCL/cl.h>
//#else
//#include <CL/cl.h>
//#endif
//#include <stdio.h>
//#include<vector>
//
//int main(void)
//{
//
//    // это все (почти) нужно, чтобы найти deviceId
//    // можно переиспользовать в лабе
//	cl_uint platforms_num;
//	if (clGetPlatformIDs(0, NULL, &platforms_num))
//		printf("No OpenCL platforms found\n");
//	else {
//		std::vector<cl_platform_id> platforms(platforms_num);
//		clGetPlatformIDs(platforms_num, platforms.data(), NULL);
//		printf("%u OpenCL platforms available\n", platforms_num);
//
//        for (cl_uint i = 0; i < platforms_num; ++i) {
//            char platform_name[128];
//            clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
//            printf("Platform %u: %s\n", i, platform_name);
//
//            cl_uint device_count;
//            cl_int status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
//            if (status != CL_SUCCESS || device_count == 0) {
//                printf("  No devices found on this platform.\n\n");
//                continue;
//            }
//
//            std::vector<cl_device_id> devices(device_count);
//            clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_count, devices.data(), NULL);
//
//            for (cl_uint j = 0; j < device_count; ++j) {
//                char device_name[128];
//                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
//                printf("  Device %u: %s\n", j, device_name);
//            }
//
//            printf("\n");
//        }
//	}
//
//	return 0;
//}

// clGetPlatformIDs - код успшности/ошибки 0 - все плохо, платформ нет
// clGetPlatformIDs(buf_size, buf*, buf_count*) // размер буфера в элементах, указатель куда записать результат, указатель куда будет записано реальное количество идентификаторов
// универсально передается (0, NULL, &platforms_num)
// на следующем шаге (platforms_num, buf, NULL) - чтобы получить id платформ
// сохраняем сюда vector<cl_platform_id> y
// (platforms_num, buf, NULL) - сюда вместо buf пишем y
// clGetDeviceIDS - перечислить все девайсы на платформе с их типами
// clGetPlatformInfo - информация о платформах
// Для всех платформ нужно вызвать перечисление всех девайсов, игнорируем платформу если clGetDeviceIDS вернул 0
// clGetDeviceInfo - много полезной инфы, можно понять дискретная или интегрированная видеокарта, разные ограничения
// clCreateContext - создает объект и возвращает его, успех(не NULL) неуспех (NULL), получить код ошибки через аргумент последний - указатель на куда положить код ошибки
// выделяет ресурсы, поэтому нужно эти ресурсы освободить - вызвать функцию - clReleaseContext (на каждую функцию create должна быть release)
// clCreateContext принимает много всего, но главное указать количество девайсов и номер девайса (ГЛАВНОЕ передать идентификатор устройства, остальные параметры необязательны)
// clCreateProgramWithSource - передать текст программы, который нужно запустить на девайсе
// Функция a + b:
// kernel void add(global const int *a, global const int *b, global int *c)
// {
// *c = *a + *b;
//}
// чтобы в лабе использовать сделать чтение из файлик add.cl и передать в clCreateProgramWithSource
// clCreateProgramWithSource принимает массив указателей на строчку (потенциально может состоятть из нескольких файлов), в нашем случае это будет один указатель передаем в аргументы 1 строка
// и указатель на строку
// также функция принимает размер текста - sizeof(байтовый массивчик)
// в отчет получаем некоторый идентификатор
// эту программу нужно скомпилировать - clBuildProgram - передавать в аргументы пустую строку, а не null
/// можно передать в список девайсов null, тогда будет скомпилирована на всех
// if(clBuildProgram != 0), то можно вызвать clGetProgramBuildInfo - здесь уже нужно указать device Id и можно попросить BUILD_LOG - здесь смотреть ошибки комиляции
// после успешной компиляции - clCreateKernel("add") - передаем имя кернела, который собираемся вызывать
// дальше будет про передачу аргумента кернелу
// память на хосте и на устройстве разные, поэтому нельзя передать просто указатель
// выделяем память на хосте и потом копируем данные на устройство
// clCreateBuffer - передаем контекст, размер в байтах и тип обращения к этой памяти (readonly, writeonly, readwrite). В нашем примере a и b кернел будет только читать, а в c писать
// в opencl типы данных гараинтированных размеров, например
// short - 2
// int - 4
// long - 8
// double НЕ ИСПОЛЬЗОВАТЬ
// В байтах можно передать sizeof(cl_int)
// для освобождения буфера clReleaseMemObject - очищает буфер
// теперь нужно передать данные с хоста на девайс
// cl_int da, db, dc;
// clCreateCommandQueue включаем PROFILING больше ничего
// clEnqueueWriteBuffer
// функции передачи данных могут быть в синхронном и ассинхронном виде(ставит задание передать параменты в очередь, позволяет быстро поставить в очередь много параметров) лучше использовать АССИНХРОННЫЙ режим
// указывается в аргументах clEnqueueWriteBuffer флагом CL_FALSE
// чтобы передать аргументы clSetKernelArg - три раза вызываем для передачи всех параметров clSetKernelArg(номер аргумента(начиная с 0), размер данных, адрес объъекта переменной)
// пример: clSetKernelArg(0, sizeof(cl_mem), &da)
// теперь вызываем кернел
// clEnqueueNDRangeKernel - запуск кернела всегда ассинхронная операция, эта функция ставит кернел в очередь на обработку
// теперь результат вызываем
// clEnqueueReadBuffer - чтение результата
// смысл использования видеокарт
// мы хотим создать столько тредов - сколько у нас элементов
// kernel void add(global const int *a, global const int *b, global int *c)
//{
//  int x = get_global_id(0);
//  c[x] = a[x] + b[x];
//}

// СЛОЖЕНИЕ ДВУХ ЧИСЕЛ
// 
//#define CL_TARGET_OPENCL_VERSION 120
//#ifdef __APPLE__
//#include <OpenCL/opencl.h>
//#else
//#include <CL/cl.h>
//#endif
//
//#include <iostream>
//#include <vector>
//#include <fstream>
//#include <sstream>
//
//int main() {
//    cl_int err;
//
//    // Получаем платформы
//    cl_uint platforms_num;
//    err = clGetPlatformIDs(0, NULL, &platforms_num);
//    if (err != CL_SUCCESS || platforms_num == 0) {
//        std::cerr << "No OpenCL platforms found\n";
//        return 1;
//    }
//
//    std::vector<cl_platform_id> platforms(platforms_num);
//    clGetPlatformIDs(platforms_num, platforms.data(), NULL);
//
//    // Получаем устройства с первой платформы
//    cl_uint device_count;
//    err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
//    if (err != CL_SUCCESS || device_count == 0) {
//        std::cerr << "No OpenCL devices found on platform 0\n";
//        return 1;
//    }
//
//    std::vector<cl_device_id> devices(device_count);
//    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, device_count, devices.data(), NULL);
//
//    cl_device_id device = devices[0];
//
//    // Создаем контекст
//    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
//    if (!context) {
//        std::cerr << "Failed to create OpenCL context\n";
//        return 1;
//    }
//
//    // Создаем очередь
//    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
//
//    // Читаем исходник программы
//    std::ifstream sourceFile("add.cl");
//    if (!sourceFile.is_open()) {
//        std::cerr << "Failed to open add.cl\n";
//        return 1;
//    }
//
//    std::stringstream ss;
//    ss << sourceFile.rdbuf();
//    std::string sourceStr = ss.str();
//    const char* source = sourceStr.c_str();
//
//    // Создаем и компилируем программу
//    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
//    if (clBuildProgram(program, 1, &device, "", NULL, NULL) != CL_SUCCESS) {
//        char log[2048];
//        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
//        std::cerr << "Build error:\n" << log << "\n";
//        return 1;
//    }
//
//    // Создаем кернел
//    cl_kernel kernel = clCreateKernel(program, "add", &err);
//
//    // Данные
//    cl_int a = 3;
//    cl_int b = 7;
//    cl_int c = 0;
//
//    // Буферы
//    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &a, &err);
//    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int), &b, &err);
//    cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int), NULL, &err);
//
//    // Установка аргументов кернела
//    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
//    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
//    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
//
//    // Запуск кернела
//    size_t global_work_size = 1;
//    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
//
//    // Чтение результата
//    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(cl_int), &c, 0, NULL, NULL);
//
//    std::cout << "Result: " << c << std::endl;
//
//    // Очистка
//    clReleaseMemObject(bufA);
//    clReleaseMemObject(bufB);
//    clReleaseMemObject(bufC);
//    clReleaseKernel(kernel);
//    clReleaseProgram(program);
//    clReleaseCommandQueue(queue);
//    clReleaseContext(context);
//
//    return 0;
//}



// 18.04.2025
// в opencl может быть многмерная нумерация тредов - двумерная, трехмерная.. 
// задается при запуске программы clEnqueueNDRangeKernel, в аргументе work_dim
// на хосте вместо int переменных будут массивы
// один тред - один элемент массива, то есть количество элементов будет равно количеству тредов
// global_work_size - количество тредов, которое нужно создать
// opencl не гарантирует, что на видеокарте будет запущено столько тредов, сколько мы ему указываем

//СЛОЖЕНИЕ ДВУХ МАССИВОВ

//#define CL_TARGET_OPENCL_VERSION 120
//#ifdef __APPLE__
//#include <OpenCL/opencl.h>
//#else
//#include <CL/cl.h>
//#endif
//
//#include <iostream>
//#include <vector>
//#include <fstream>
//#include <sstream>
//
//struct Context {
//    cl_context context;
//    cl_command_queue queue;
//};
//
//void process(Context& context) {
//
//}
//
//void release(Context& context) {
//    context.queue = clCreateCommandQueue();
//
//
//}
//
//int main() {
//    cl_int err;
//
//    const int N = 8;
//    std::vector<cl_int> A = { 1, 2, 3, 4, 5, 6, 7, 8 };
//    std::vector<cl_int> B = { 8, 7, 6, 5, 4, 3, 2, 1 };
//    std::vector<cl_int> C(N);
//
//    cl_uint platforms_num;
//    clGetPlatformIDs(0, NULL, &platforms_num);
//    std::vector<cl_platform_id> platforms(platforms_num);
//    clGetPlatformIDs(platforms_num, platforms.data(), NULL);
//
//    cl_uint device_count;
//    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
//    std::vector<cl_device_id> devices(device_count);
//    clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, device_count, devices.data(), NULL);
//
//    cl_device_id device = devices[0];
//
//    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
//    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
//
//    std::ifstream sourceFile("add.cl");
//    std::stringstream ss;
//    ss << sourceFile.rdbuf();
//    std::string sourceStr = ss.str();
//    const char* source = sourceStr.c_str();
//
//    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
//    err = clBuildProgram(program, 1, &device, "", NULL, NULL);
//    if (err != CL_SUCCESS) {
//        char log[2048];
//        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
//        std::cerr << "Build error:\n" << log << "\n";
//        return 1;
//    }
//
//    cl_kernel kernel = clCreateKernel(program, "add", &err);
//
//    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int) * N, A.data(), &err);
//    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_int) * N, B.data(), &err);
//    cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_int) * N, NULL, &err);
//
//    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
//    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
//    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
//
//    size_t global_work_size = N;
//    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
//
//    clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(cl_int) * N, C.data(), 0, NULL, NULL);
//
//    for (int i = 0; i < N; ++i)
//        std::cout << C[i] << " ";
//    std::cout << std::endl;
//
//    clReleaseMemObject(bufA);
//    clReleaseMemObject(bufB);
//    clReleaseMemObject(bufC);
//    clReleaseKernel(kernel);
//    clReleaseProgram(program);
//    clReleaseCommandQueue(queue);
//    clReleaseContext(context);
//
//    return 0;
//}

// как разумно управлять ресурсами
// на create вызываем release
// как сделать правильно:
// завести структуру:
//
//struct Context {
//    cl_command_queue queue;
//    // здесь также указать остальные ресурсы
//};
//создаем экземпляр структурки и инициализируем ноликом
// Context context = {};
// внутри констекста перечислить все ресурсы
// создать функцию process (создание всех ресурсов) и release (очищение всех ресурсов)
// process(&context){
// 
// context->queue = clCreateCommandQueue();
// if(context->queue == )
//}


// Перемножение матриц
// двумерный массив представляем одномерным
// float a[M (y)][K (x)]; - проблема: функции может не хватить размера стека
// лучше использовать динамические массивы:
// a[y * k + x] y - количество строк допустим, x - количество солбцов
// МАССИВ МАССИВОВ не передаем на девайс работать не будет

// Для умножения матриц - нужен тройной цикл по y, x, k
// Из-за того что слишком много обращений к памяти и мало вычислений на процессоре может работать быстрее чем на видеокарте

// for(..x)
//      for(..y)
//          float sum;
//          for(..k)
//              sum =
//              с[] = sum;
// мы хотим распараллелить самый внешний цикл
// SIMD - какая-то прикольная штука)
// Если работаем на видеокарте, то превращаем в треды фор х и y
// Здесь будет удобно использовать двумерную индексацию
// Вот эту штуку можно перенести в кернел a[y * k + x]
// y и x - будут номерами треда
// сравнение хостового результата и кернела по модулю разницы, который меньше чего-то)


#define CL_TARGET_OPENCL_VERSION 120
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

//struct Context {
//    cl_context context;
//    cl_command_queue queue;
//};
//
//void process(Context& context) {
//
//}
//
//void release(Context& context) {
//    context.queue = clCreateCommandQueue();
//
//
//}

int main() {
	cl_int err;

	cl_uint M = 4;
	cl_uint N = 2;
	cl_uint K = 4;
	std::vector<float> A = {
		1, 2, 3, 4,
		5, 6, 7, 8,
		9, 10, 11, 12,
		13, 14, 15, 16
	};

	std::vector<float> B = {
		1, 2,
		3, 4,
		5, 6,
		7, 8
	};

	std::vector<float> C(M * N);

	// реализация на хосте
	for (size_t i = 0; i < M; ++i) {
		for (size_t j = 0; j < N; ++j) {
			C[i * N + j] = 0.0f;
			for (size_t k = 0; k < K; ++k) {
				C[i * N + j] += A[i * K + k] * B[k * N + j];
			}
		}
	}

	for (size_t i = 0; i < C.size(); i++) {
		printf("%f\n", C[i]);
	}
	printf("-------------------\n");
	cl_uint platforms_num;
	clGetPlatformIDs(0, NULL, &platforms_num);
	std::vector<cl_platform_id> platforms(platforms_num);
	clGetPlatformIDs(platforms_num, platforms.data(), NULL);

	cl_uint device_count;
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
	std::vector<cl_device_id> devices(device_count);
	clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, device_count, devices.data(), NULL);

	cl_device_id device = devices[0];

	cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

	std::ifstream sourceFile("muptiply.cl");

	if (!sourceFile.is_open()) {
		std::cerr << "Failed to open multiply.cl\n";
		return 1;
	}

	std::stringstream ss;
	ss << sourceFile.rdbuf();
	std::string sourceStr = ss.str();
	const char* source = sourceStr.c_str();

	cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
	
	if (program == NULL) {
		std::cerr << "clCreateProgramWithSource failed";
		return 1;
	}

	err = clBuildProgram(program, 1, &device, "", NULL, NULL);

	if (err != 0) {
		char log[204800];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
		std::cerr << log << "\n";
		return 1;
	}

	cl_kernel kernel = clCreateKernel(program, "multiply", &err);

	if (kernel == NULL) {
		std::cerr << "clCreateKernel failed";
		return 1;
	}


	cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * A.size(), A.data(), &err);
	cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * B.size(), B.data(), &err);
	cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * C.size(), NULL, &err);

	clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
	clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
	clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
	clSetKernelArg(kernel, 3, sizeof(cl_uint), &M);
	clSetKernelArg(kernel, 4, sizeof(cl_uint), &N);
	clSetKernelArg(kernel, 5, sizeof(cl_uint), &K);

	size_t global_work_size[2] = { N, M };
	//size_t global_work_size = N * M;
	err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, NULL);

	if (err != 0) {
		std::cerr << "clEnqueueNDRangeKernel failed";
		return 1;
	}

	err = clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeof(cl_int) * C.size(), C.data(), 0, NULL, NULL);

	if (err != 0) {
		std::cerr << "clEnqueueReadBuffer failed";
		return 1;
	}


	for (size_t i = 0; i < C.size(); i++) {
		printf("%f\n", C[i]);
	}

	clReleaseMemObject(bufA);
	clReleaseMemObject(bufB);
	clReleaseMemObject(bufC);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);

	return 0;
}


// вычисление времени
// при вызове кернела
// передавать переменную типа cl_event и передать в функцию clEnqueueNDRangeKernel
// в этой перменной будет лежать идентификатор
// после clEnqueueReadBuffer можно clEventProfilingInfo - тут можно узнать начало и окончание времени выполнения на девайсе,
// в которую передается тип информации, которую хотим получить, там вроде НАНОСЕКУНДУ (нужно сконвертировать значения в мс)
// 