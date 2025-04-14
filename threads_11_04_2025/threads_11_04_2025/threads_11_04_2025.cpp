#define CL_TARGET_OPENCL_VERSION 120
#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#include <stdio.h>
#include<vector>

int main(void)
{

    // это все (почти) нужно, чтобы найти deviceId
    // можно переиспользовать в лабе
	cl_uint platforms_num;
	if (clGetPlatformIDs(0, NULL, &platforms_num))
		printf("No OpenCL platforms found\n");
	else {
		std::vector<cl_platform_id> platforms(platforms_num);
		clGetPlatformIDs(platforms_num, platforms.data(), NULL);
		printf("%u OpenCL platforms available\n", platforms_num);

        for (cl_uint i = 0; i < platforms_num; ++i) {
            char platform_name[128];
            clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
            printf("Platform %u: %s\n", i, platform_name);

            cl_uint device_count;
            cl_int status = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &device_count);
            if (status != CL_SUCCESS || device_count == 0) {
                printf("  No devices found on this platform.\n\n");
                continue;
            }

            std::vector<cl_device_id> devices(device_count);
            clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, device_count, devices.data(), NULL);

            for (cl_uint j = 0; j < device_count; ++j) {
                char device_name[128];
                clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
                printf("  Device %u: %s\n", j, device_name);
            }

            printf("\n");
        }
	}

	return 0;
}

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
