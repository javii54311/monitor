/**
 * @file main.c
 * @brief Entry point of the system
 */

#include "expose_metrics.h"
#include <stdbool.h>

/**
 * @def SLEEP_TIME
 * @brief Defines the amount of time the system sleeps between metric collections.
 *
 * The system will pause for SLEEP_TIME seconds before repeating metric collection operations.
 */
#define SLEEP_TIME 1

/**
 * @brief Main function of the application.
 *
 * This function initializes the HTTP server in a separate thread to expose system metrics
 * and enters an infinite loop to periodically update various system metrics such as CPU usage,
 * memory usage, network statistics, and disk I/O.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return Returns EXIT_SUCCESS if the program terminates successfully, or EXIT_FAILURE if an error occurs.
 */
int main(int argc, char* argv[])
{

    // Creamos un hilo para exponer las métricas vía HTTP
    init_metrics();
    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE;
    }

    // Bucle principal para actualizar las métricas cada segundo
    while (true)
    {
        update_context_switches_gauge();
        update_network_gauge();
        update_cpu_gauge();
        update_memory_gauge();
        update_disk_io_gauge();
        update_process_count_gauge();
        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}