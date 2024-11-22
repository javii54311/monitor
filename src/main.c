/**
 * @file main.c
 * @brief Entry point of the system
 */

#include "expose_metrics.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "../include/monitor.h"  // Para la integración con el TP2
#include <cjson/cJSON.h>  // Include cJSON library

/**
 * @def SLEEP_TIME
 * @brief Defines the amount of time the system sleeps between metric collections.
 *
 * The system will pause for SLEEP_TIME seconds before repeating metric collection operations.
 */
#define SLEEP_TIME 1

/**
 * @brief Señal para recargar la configuración.
 */
volatile sig_atomic_t reload_config = 0;

/**
 * @brief Señal para detener el programa.
 */
volatile sig_atomic_t stop_program = 0;

/**
 * @brief Variables booleanas para controlar qué métricas se deben mostrar.
 */
bool show_cpu_usage = true;
bool show_memory_usage = true;
bool show_disk_io = true;
bool show_network_stats = true;
bool show_process_count = true;
bool show_context_switches = true;

/**
 * @brief Intervalo de tiempo entre actualizaciones de métricas.
 */
int interval = 5;

/**
 * @brief Manejador de señales para recargar la configuración o detener el programa.
 *
 * @param signal Señal recibida.
 */
void handle_signal(int signal)
{
    if (signal == SIGUSR1)
    {
        reload_config = 1;
    }
    else if (signal == SIGINT)
    {
        stop_program = 1;
    }
}

/**
 * @brief Lee la configuración desde un archivo JSON.
 *
 * @param config_filename Nombre del archivo de configuración.
 */
void read_config(const char* config_filename)
{
    FILE* file = fopen(config_filename, "r");
    if (file == NULL)
    {
        perror("Error al abrir el archivo de configuración, usando métricas por defecto");
        // Usar métricas por defecto
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* data = (char*)malloc(length + 1);
    fread(data, 1, length, file);
    fclose(file);
    data[length] = '\0';

    cJSON* json = cJSON_Parse(data);
    if (json == NULL)
    {
        perror("Error al parsear el archivo JSON, usando métricas por defecto");
        free(data);
        return;
    }

    cJSON* metrics_json = cJSON_GetObjectItemCaseSensitive(json, "metrics");
    cJSON* interval_json = cJSON_GetObjectItemCaseSensitive(json, "interval");

    if (!cJSON_IsObject(metrics_json) || !cJSON_IsNumber(interval_json))
    {
        perror("Formato de archivo JSON inválido, usando métricas por defecto");
        cJSON_Delete(json);
        free(data);
        return;
    }

    show_cpu_usage = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "cpu"));
    show_memory_usage = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "memory"));
    show_disk_io = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "disk_io"));
    show_network_stats = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "network_stats"));
    show_process_count = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "process_count"));
    show_context_switches = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(metrics_json, "context_switches"));

    interval = interval_json->valueint;

    cJSON_Delete(json);
    free(data);
}

/**
 * @brief Punto de entrada del programa.
 *
 * @param argc Número de argumentos.
 * @param argv Argumentos del programa.
 * @return int Código de salida.
 */
int main(int argc, char* argv[])
{
    signal(SIGUSR1, handle_signal);  // Configurar el manejador de señales
    signal(SIGINT, handle_signal);

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ruta_al_archivo_config.json>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* config_filename = argv[1];

    // Leer la configuración inicial
    read_config(config_filename);

    // Creamos un hilo para exponer las métricas vía HTTP
    pthread_t tid;
    if (pthread_create(&tid, NULL, expose_metrics, NULL) != 0)
    {
        fprintf(stderr, "Error al crear el hilo del servidor HTTP\n");
        return EXIT_FAILURE;
    }

    init_metrics();

    // Bucle principal para actualizar las métricas según el intervalo especificado
    while (!stop_program)
    {
        if (reload_config)
        {
            // Volver a leer la configuración
            read_config(config_filename);
            reload_config = 0;
        }

        // Actualizar las métricas según las configuraciones
        if (show_cpu_usage)
        {
            update_cpu_gauge();
        }
        if (show_memory_usage)
        {
            update_memory_gauge();
            update_memory_gauge2();
        }
        if (show_disk_io)
        {
            update_disk_io_gauge();
        }
        if (show_network_stats)
        {
            update_network_gauge();
        }
        if (show_process_count)
        {
            update_process_count_gauge();
        }
        if (show_context_switches)
        {
            update_context_switches_gauge();
        }

        sleep(interval);  // Esperar el tiempo especificado en el intervalo
    }

    return EXIT_SUCCESS;
}
