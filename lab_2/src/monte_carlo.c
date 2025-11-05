#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

typedef struct {
    long long points;
    double radius;
    long long inside;
} thread_data_t;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
long long total_inside = 0;

void* monte_carlo_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    long long local_inside = 0;

    unsigned int seed = (unsigned int)time(NULL) + (unsigned int)(uintptr_t)arg;

    for (long long i = 0; i < data->points; i++) {
        double x = ((double)rand_r(&seed) / RAND_MAX) * 2 * data->radius - data->radius;
        double y = ((double)rand_r(&seed) / RAND_MAX) * 2 * data->radius - data->radius;

        if (x*x + y*y <= data->radius * data->radius) {
            local_inside++;
        }
    }

    pthread_mutex_lock(&mutex);
    total_inside += local_inside;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Ввод: %s <радиус> <количество_точек> <макс_потоков>\n", argv[0]);
        return 1;
    }

    double radius = atof(argv[1]);
    long long total_points = atoll(argv[2]);
    int max_threads = atoi(argv[3]);

    if (radius <= 0 || total_points <= 0 || max_threads <= 0) {
        printf("Ошибка: все параметры должны быть положительными\n");
        return 1;
    }

    printf("\nПараметры:\n");
    printf("Радиус: %.2f\n", radius);
    printf("Всего точек: %lld\n", total_points);
    printf("Максимум потоков: %d\n", max_threads);

    clock_t start = clock();

    pthread_t threads[max_threads];
    thread_data_t thread_data[max_threads];

    long long points_per_thread = total_points / max_threads;

    for (int i = 0; i < max_threads; i++) {
        thread_data[i].points = points_per_thread;
        thread_data[i].radius = radius;
        thread_data[i].inside = 0;
        pthread_create(&threads[i], NULL, monte_carlo_thread, &thread_data[i]);
    }

    for (int i = 0; i < max_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    double side = 2 * radius;
    double square_area = side * side;
    double circle_area = ((double)total_inside / total_points) * square_area;
    double theoretical_area = M_PI * radius * radius;

    clock_t end = clock();
    double time_spent = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("\nРезультаты:\n");
    printf("Точек внутри круга: %lld\n", total_inside);
    printf("Вычисленная площадь: %.6f\n", circle_area);
    printf("Теоретическая площадь: %.6f\n", theoretical_area);
    printf("Погрешность: %.4f%%\n", fabs(circle_area - theoretical_area) / theoretical_area * 100);
    printf("Время выполнения: %.3f секунд\n", time_spent);

    return 0;
}