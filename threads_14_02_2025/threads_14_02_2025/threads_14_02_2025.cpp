// threads_14_02_2025.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <iostream>
#include <omp.h>
#include <math.h>
#include <string>

bool isSimple(int i) {
	for (int j = 2; j <= sqrt(i); j++) {
		if (i % j == 0) {
			return false;
		}
	}

	return true;
}


int main()
{

// ������ 14.02.2025. OMP
	
// ����� ���/���� openmp ��� �� ������� -> �������� -> c/c++ -> ����
// ������� �����������
// ��������� ������� �������, ������� ���� � �������

// ������ 1
// #pragma omp parallel
//    {
//        printf("Hello World!\n");
//    }
//


// ������ 2
// #pragma omp parallel
//	{
//		int n = omp_get_num_threads(); // ����� ������
//		int m = omp_get_thread_num(); // 0 .. n-1 ����� �����
//
//		printf("Hello %i, %i\n", m, n);
//	}
//

// ������ 3
// ��� ���� ��������� �� ������
// #pragma omp parallel
//	{
//		int n = omp_get_num_threads();
//		int m = omp_get_thread_num();
//				
//		int a = 0;
//		int b = 24;
//
//		// ����� ���������� �����������, ���� ������ ����� ������� ������, ��� ��� ������ ��������
//		for (size_t i = a + (b - a) * m / n; i < a + (b - a) * (m + 1) / n; i++) {
//			printf("%i, p = %i\n", i, m);
//		}
//
//		//for (size_t i = 0 + m; i < 12; i += n) {
//		//	printf("%i\n", i);
//		//}
//	}

// ������ 4
// #pragma omp parallel
//	{
//		int a = 0;
//		int b = 24;
//
//// static - �� �� �����, ��� � � ������� 3 (1 �������)
//// static, 1 - �� �� �����, ��� � � ������� 3 (2 �������)
//// dynamic - ������ ���� �������� � "������ ��������� ������������" � ������ ������)
//// "����� �������� ������������" ����� ������)
//// ����� ������������, ���� ������� ���������� ������ �����
//// dynamic(k), k - �� ������� �������� ������ ������
// #pragma omp for schedule (dynamic)
//		for (int i = a; i < b; i++) {
//			printf("%i\n", i);
//		}
//	}
//
//	return 0;


// ������ 5

//	int count = 0;
//	auto start = omp_get_wtime();
//	int n = 1000000;
//
//#pragma omp parallel
//	{
//		int local_count = 0;
//
//		// ������ �� ������� ������ ������
//		// �������� ������ ������� ��������, � ���������� ������ ����� �����
//		// 
//#pragma omp for
//		for (int i = 2; i < n; i++) {
//			if (isSimple(i)) {
//				local_count++;
//			}
//		}
//#pragma omp atomic
//		count += local_count;
//	}
//
//	auto end = omp_get_wtime();
//	printf("%i\n", count);
//	printf("%f ms\n", (end - start) * 1000);

// #pragma omp ctritical - ������� ���������, ������� ����� ��������� �� ������ 1 ����� 
// ��� ������, ��� ���������� ��������
// ������ �������: ��� ������ ����
//	int k = 0;
// #pragma omp parallel
//	int k2 = 0;
//
// #pragma omp for
//	for () {
//		k2++;
//	}
//
// #pragma omp ctritical
//	k += k2;

// #pragma omp single
	//{
	//	����������� ���� ���
	// � ������� �� critical (� critical ��� ������ �������� ���� ��� ��� ���)
	// 
	//}
	// ����� ����� ������ ���� ������, ����� � ����� ���� ����� ������ ��������� ���� �����


// ������ 28.02.2025. ����������� �������� c++

// std::thread - ������� ����� ����, ����� ��������� ������� (����� ���� �������), ������� ����� ���������
// ������� ��������� ������
// ���� �������� 8 ����, �� ������� 7 ������ (��� ��� ��������� ���� ��� ����)

// std::atomic 
// ����������� ��������� � ���� ����������
// std::atomic ��������� � ���������� ����������

// std::latch - �������� ������ (������, ��� ������� � ������ ����)

// std::mutex - ������ ����������� ������ (����� �� ������ 1 ���� ��� ����� � ���� �������, �������� ����� ������� ������ ���� ����)
}