/**
 * @file expose_metrics.h
 * @brief Programa para leer el uso de CPU, memoria y otros recursos del sistema,
 * y exponerlos como métricas para Prometheus.
 */

#include "metrics.h"
#include <errno.h>
#include <prom.h>
#include <promhttp.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para sleep

/**
 * @brief Tamaño del buffer utilizado en operaciones de lectura.
 */
#define BUFFER_SIZE 256

/**
 * @brief Actualiza la métrica de uso de CPU.
 *
 * Lee el uso de CPU y actualiza la métrica correspondiente en Prometheus.
 */
void update_cpu_gauge();

/**
 * @brief Actualiza la métrica de uso de memoria.
 *
 * Lee el uso de memoria y actualiza la métrica correspondiente en Prometheus.
 */
void update_memory_gauge();

/**
 * @brief Actualiza la métrica de los cambios de contexto.
 *
 * Obtiene el número de cambios de contexto desde el sistema y actualiza
 * la métrica correspondiente en Prometheus.
 */
void update_context_switches_gauge();

/**
 * @brief Actualiza las métricas de lecturas y escrituras del disco.
 *
 * Obtiene el número de sectores leídos y escritos en el disco y actualiza
 * las métricas correspondientes en Prometheus.
 */
void update_disk_io_gauge();

/**
 * @brief Actualiza las métricas de estadísticas de red.
 *
 * Obtiene el número de bytes recibidos y transmitidos, así como errores
 * y colisiones, y actualiza las métricas correspondientes en Prometheus.
 */
void update_network_gauge();

/**
 * @brief Actualiza la métrica del contador de procesos en ejecución.
 *
 * Obtiene el número de procesos en ejecución y actualiza la métrica
 * correspondiente en Prometheus.
 */
void update_process_count_gauge();

/**
 * @brief Función del hilo para exponer las métricas vía HTTP en el puerto 8000.
 *
 * Inicializa un servidor HTTP para servir las métricas recopiladas.
 *
 * @param arg Argumento no utilizado.
 * @return NULL
 */
void* expose_metrics(void* arg);

/**
 * @brief Inicializa el mutex y las métricas de Prometheus.
 *
 * Configura el mutex para la sincronización y crea las métricas que
 * serán expuestas a través de Prometheus.
 *
 * @return EXIT_SUCCESS en caso de éxito, o EXIT_FAILURE en caso de error.
 */
int init_metrics();

/**
 * @brief Destruye el mutex utilizado para la sincronización de hilos.
 *
 * Libera los recursos asociados al mutex.
 */
void destroy_mutex();