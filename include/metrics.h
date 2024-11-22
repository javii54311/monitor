/**
 * @file metrics.h
 * @brief Funciones para obtener estadísticas del sistema desde el sistema de archivos /proc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
/**
 * @brief buffer
 */
#define BUFFER_SIZE 256

/**
 * @brief Obtiene el porcentaje de uso de memoria desde /proc/meminfo.
 *
 * Lee los valores de memoria total y disponible desde /proc/meminfo y calcula
 * el porcentaje de uso de memoria.
 *
 * @return Uso de memoria como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_memory_usage();

/**
 * @brief Obtiene el porcentaje de uso de CPU desde /proc/stat.
 *
 * Lee los tiempos de CPU desde /proc/stat y calcula el porcentaje de uso de CPU
 * en un intervalo de tiempo.
 *
 * @return Uso de CPU como porcentaje (0.0 a 100.0), o -1.0 en caso de error.
 */
double get_cpu_usage();

/**
 * @brief Obtiene el número de cambios de contexto desde /proc/stat.
 *
 * Lee el valor correspondiente a los cambios de contexto (ctxt) desde /proc/stat,
 * que indica cuántas veces el sistema ha cambiado de un proceso a otro.
 *
 * @return Número de cambios de contexto, o 1 en caso de error al abrir el archivo.
 */
unsigned long long int get_ctxt();

/**
 * @brief Obtiene estadísticas de I/O de disco desde /proc/diskstats.
 *
 * Suma el total de sectores leídos y escritos por los dispositivos de disco.
 *
 * @param[out] reads Puntero donde se almacenará el total de sectores leídos.
 * @param[out] writes Puntero donde se almacenará el total de sectores escritos.
 */
void get_disk_io(unsigned long long* reads, unsigned long long* writes);

/**
 * @brief Obtiene estadísticas de red desde /proc/net/dev.
 *
 * Acumula los bytes recibidos y transmitidos, así como errores y colisiones en
 * las interfaces de red.
 *
 * @param[out] rx_bytes Puntero donde se almacenará el total de bytes recibidos.
 * @param[out] tx_bytes Puntero donde se almacenará el total de bytes transmitidos.
 * @param[out] rx_errors Puntero donde se almacenará el total de errores de recepción.
 * @param[out] tx_errors Puntero donde se almacenará el total de errores de transmisión.
 * @param[out] collisions Puntero donde se almacenará el total de colisiones.
 */
void get_network_stats(unsigned long long* rx_bytes, unsigned long long* tx_bytes, unsigned long long* rx_errors,
                       unsigned long long* tx_errors, unsigned long long* collisions);

/**
 * @brief Obtiene el número de procesos en ejecución desde /proc/stat.
 *
 * Lee el número de procesos que están en ejecución en el sistema.
 *
 * @return Número de procesos en ejecución, o -1 en caso de error al abrir el archivo.
 */
int get_process();
