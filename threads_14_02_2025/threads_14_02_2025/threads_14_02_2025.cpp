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

// ЛЕКЦИЯ 14.02.2025. OMP
	
// чтобы вкл/откл openmp пкм по решению -> свойства -> c/c++ -> язык
// команда компилятору
// запускает столько потоков, сколько ядер в системе

// ПРИМЕР 1
// #pragma omp parallel
//    {
//        printf("Hello World!\n");
//    }
//


// ПРИМЕР 2
// #pragma omp parallel
//	{
//		int n = omp_get_num_threads(); // всего тредов
//		int m = omp_get_thread_num(); // 0 .. n-1 номер треда
//
//		printf("Hello %i, %i\n", m, n);
//	}
//

// ПРИМЕР 3
// как цкил разделить на потоке
// #pragma omp parallel
//	{
//		int n = omp_get_num_threads();
//		int m = omp_get_thread_num();
//				
//		int a = 0;
//		int b = 24;
//
//		// такая реализация отпимальнее, если внутри цикла похожая логика, так как потоки кешируют
//		for (size_t i = a + (b - a) * m / n; i < a + (b - a) * (m + 1) / n; i++) {
//			printf("%i, p = %i\n", i, m);
//		}
//
//		//for (size_t i = 0 + m; i < 12; i += n) {
//		//	printf("%i\n", i);
//		//}
//	}

// ПРИМЕР 4
// #pragma omp parallel
//	{
//		int a = 0;
//		int b = 24;
//
//// static - то же самое, что и в примере 3 (1 решение)
//// static, 1 - то же самое, что и в примере 3 (2 решение)
//// dynamic - каждый тред приходит к "общему механизму планирования" и просит работу)
//// "общий механизм планирования" стоит дорого)
//// стоит использовать, если тяжелые вычисления внутри цикла
//// dynamic(k), k - по сколько индексов выдают потоку
// #pragma omp for schedule (dynamic)
//		for (int i = a; i < b; i++) {
//			printf("%i\n", i);
//		}
//	}
//
//	return 0;


// ПРИМЕР 5

//	int count = 0;
//	auto start = omp_get_wtime();
//	int n = 1000000;
//
//#pragma omp parallel
//	{
//		int local_count = 0;
//
//		// почему на внешний вешаем потоки
//		// создание тредов тяжелая операция, а внутренних циклов будет много
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

// #pragma omp ctritical - участок программы, который может исполнять не больше 1 треда 
// еще дороже, чем автомарная операция
// личный счетчик: ТАК ДЕЛАТЬ НАДО
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
	//	выполняется один раз
	// в отличие от critical (в critical все потоки исполнят один раз тот код)
	// 
	//}
	// перед сингл должен быть барьер, перед и после синг треды должны дождаться друг друга


// ЛЕКЦИЯ 28.02.2025. Стандартные средства c++

// std::thread - создает новый тред, здесь указываем функцию (может быть лямбдой), которую нужно запустить
// сколько создавать тредов
// если доступно 8 ядер, то создаем 7 тредов (так как стартовый тред уже есть)

// std::atomic 
// атомарность привязана к типу переменным
// std::atomic привязана к конкретной переменной

// std::latch - ожидание тредов (барьер, про который я писала выше)

// std::mutex - аналог критической секции (чтобы не больше 1 тред мог зайти в одну область, мютексом может владеть только один тред)
}