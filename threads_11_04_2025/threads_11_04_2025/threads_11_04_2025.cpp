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

    // ��� ��� (�����) �����, ����� ����� deviceId
    // ����� ���������������� � ����
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

// clGetPlatformIDs - ��� ���������/������ 0 - ��� �����, �������� ���
// clGetPlatformIDs(buf_size, buf*, buf_count*) // ������ ������ � ���������, ��������� ���� �������� ���������, ��������� ���� ����� �������� �������� ���������� ���������������
// ������������ ���������� (0, NULL, &platforms_num)
// �� ��������� ���� (platforms_num, buf, NULL) - ����� �������� id ��������
// ��������� ���� vector<cl_platform_id> y
// (platforms_num, buf, NULL) - ���� ������ buf ����� y
// clGetDeviceIDS - ����������� ��� ������� �� ��������� � �� ������
// clGetPlatformInfo - ���������� � ����������
// ��� ���� �������� ����� ������� ������������ ���� ��������, ���������� ��������� ���� clGetDeviceIDS ������ 0
// clGetDeviceInfo - ����� �������� ����, ����� ������ ���������� ��� ��������������� ����������, ������ �����������
// clCreateContext - ������� ������ � ���������� ���, �����(�� NULL) ������� (NULL), �������� ��� ������ ����� �������� ��������� - ��������� �� ���� �������� ��� ������
// �������� �������, ������� ����� ��� ������� ���������� - ������� ������� - clReleaseContext (�� ������ ������� create ������ ���� release)
// clCreateContext ��������� ����� �����, �� ������� ������� ���������� �������� � ����� ������� (������� �������� ������������� ����������, ��������� ��������� �������������)
// clCreateProgramWithSource - �������� ����� ���������, ������� ����� ��������� �� �������
// ������� a + b:
// kernel void add(global const int *a, global const int *b, global int *c)
// {
// *c = *a + *b;
//}
// ����� � ���� ������������ ������� ������ �� ������ add.cl � �������� � clCreateProgramWithSource
// clCreateProgramWithSource ��������� ������ ���������� �� ������� (������������ ����� ��������� �� ���������� ������), � ����� ������ ��� ����� ���� ��������� �������� � ��������� 1 ������
// � ��������� �� ������
// ����� ������� ��������� ������ ������ - sizeof(�������� ���������)
// � ����� �������� ��������� �������������
// ��� ��������� ����� �������������� - clBuildProgram - ���������� � ��������� ������ ������, � �� null
/// ����� �������� � ������ �������� null, ����� ����� �������������� �� ����
// if(clBuildProgram != 0), �� ����� ������� clGetProgramBuildInfo - ����� ��� ����� ������� device Id � ����� ��������� BUILD_LOG - ����� �������� ������ ���������
// ����� �������� ���������� - clCreateKernel("add") - �������� ��� �������, ������� ���������� ��������
// ������ ����� ��� �������� ��������� �������
// ������ �� ����� � �� ���������� ������, ������� ������ �������� ������ ���������
// �������� ������ �� ����� � ����� �������� ������ �� ����������
// clCreateBuffer - �������� ��������, ������ � ������ � ��� ��������� � ���� ������ (readonly, writeonly, readwrite). � ����� ������� a � b ������ ����� ������ ������, � � c ������
// � opencl ���� ������ ���������������� ��������, ��������
// short - 2
// int - 4
// long - 8
// double �� ������������
// � ������ ����� �������� sizeof(cl_int)
// ��� ������������ ������ clReleaseMemObject - ������� �����
// ������ ����� �������� ������ � ����� �� ������
// cl_int da, db, dc;
// clCreateCommandQueue �������� PROFILING ������ ������
// clEnqueueWriteBuffer
// ������� �������� ������ ����� ���� � ���������� � ������������ ����(������ ������� �������� ��������� � �������, ��������� ������ ��������� � ������� ����� ����������) ����� ������������ ������������ �����
// ����������� � ���������� clEnqueueWriteBuffer ������ CL_FALSE
// ����� �������� ��������� clSetKernelArg - ��� ���� �������� ��� �������� ���� ���������� clSetKernelArg(����� ���������(������� � 0), ������ ������, ����� �������� ����������)
// ������: clSetKernelArg(0, sizeof(cl_mem), &da)
// ������ �������� ������
// clEnqueueNDRangeKernel - ������ ������� ������ ������������ ��������, ��� ������� ������ ������ � ������� �� ���������
// ������ ��������� ��������
// clEnqueueReadBuffer - ������ ����������
// ����� ������������� ���������
// �� ����� ������� ������� ������ - ������� � ��� ���������
// kernel void add(global const int *a, global const int *b, global int *c)
//{
//  int x = get_global_id(0);
//  c[x] = a[x] + b[x];
//}
