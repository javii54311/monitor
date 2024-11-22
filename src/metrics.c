#include "../include/metrics.h"

unsigned long long int get_ctxt()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long ctxt;

    // Abrir el archivo /proc/stat
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/stat"); // Registra un error si no se puede abrir el archivo
        return 1;
    }

    // Leer el valor total de cambios de contexto desde el archivo
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "ctxt %llu", &ctxt) == 1) // Extrae el valor "ctxt" del buffer
        {
            break;
        }
    }

    fclose(fp);

    // Verifica si el valor de los cambios de contexto fue recuperado con éxito
    if (ctxt == 0)
    {
        fprintf(stderr,
                "Error reading context switch information from /proc/stat\n"); // Registra un error si no se encuentra el valor
        return 1;
    }

    return ctxt; // Devuelve el número de cambios de contexto recuperado
}

double get_memory_usage()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Abrir el archivo /proc/meminfo
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/meminfo"); // Registra un error si no se puede abrir el archivo
        return -1.0;
    }

    // Leer los valores de memoria total y disponible
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal encontrado, continuar leyendo para MemAvailable
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable encontrado, detener la lectura
        }
    }

    fclose(fp);

    // Verifica si ambos valores fueron recuperados con éxito
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error reading memory information from /proc/meminfo\n"); // Registra un error si los valores faltan
        return -1.0;
    }

    // Calcular el porcentaje de memoria utilizada
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent; // Devuelve el porcentaje de memoria utilizada
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Abrir el archivo /proc/stat
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/stat"); // Registra un error si no se puede abrir el archivo
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error reading /proc/stat"); // Registra un error si no se puede leer el archivo
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Parsear los valores del tiempo de CPU del archivo
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error parsing /proc/stat\n"); // Registra un error si el análisis falla
        return -1.0;
    }

    // Calcular las diferencias entre las lecturas actuales y anteriores
    unsigned long long prev_idle_total = prev_idle + prev_iowait;
    unsigned long long idle_total = idle + iowait;

    unsigned long long prev_non_idle = prev_user + prev_nice + prev_system + prev_irq + prev_softirq + prev_steal;
    unsigned long long non_idle = user + nice + system + irq + softirq + steal;

    unsigned long long prev_total = prev_idle_total + prev_non_idle;
    unsigned long long total = idle_total + non_idle;

    totald = total - prev_total;
    idled = idle_total - prev_idle_total;

    if (totald == 0)
    {
        fprintf(stderr, "Totald is zero, cannot calculate CPU usage!\n"); // Registra un error si no hay diferencia en el tiempo total
        return -1.0;
    }

    // Calcular el porcentaje de uso de la CPU
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Actualizar los valores previos para la próxima lectura
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent; // Devuelve el porcentaje de uso de la CPU
}

void get_disk_io(unsigned long long* reads, unsigned long long* writes)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    *reads = 0;
    *writes = 0;

    // Abrir el archivo /proc/diskstats para leer
    fp = fopen("/proc/diskstats", "r");
    if (fp == NULL)
    { // Verifica si ocurrió un error al abrir el archivo
        perror("Error opening /proc/diskstats");
        return;
    }

    // Leer cada línea del archivo hasta llegar al final
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        unsigned long long read_sectors = 0, write_sectors = 0;
        int major, minor;         // Números de dispositivo
        char device[BUFFER_SIZE]; // Nombre del dispositivo
        long long unsigned int read_completed, read_merged, reading_ms;
        long long unsigned int write_completed, write_merged;

        // Parsear la línea para extraer los datos relevantes
        sscanf(buffer, "%d %d %s %llu %llu %llu %llu %llu %llu %llu", &major, &minor, device, &read_completed,
               &read_merged, &read_sectors, &reading_ms, &write_completed, &write_merged, &write_sectors);

        // Sumar los sectores leídos y escritos a los totales
        *reads += read_sectors;
        *writes += write_sectors;
    }

    fclose(fp); // Cerrar el archivo para liberar recursos
}

void get_network_stats(unsigned long long* rx_bytes, unsigned long long* tx_bytes, unsigned long long* rx_errors,
                       unsigned long long* tx_errors, unsigned long long* collisions)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    *rx_bytes = 0;   // Total de bytes recibidos
    *tx_bytes = 0;   // Total de bytes transmitidos
    *rx_errors = 0;  // Total de errores de recepción
    *tx_errors = 0;  // Total de errores de transmisión
    *collisions = 0; // Total de colisiones

    // Abrir el archivo /proc/net/dev para leer
    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    { // Verifica si ocurrió un error al abrir el archivo
        perror("Error opening /proc/net/dev");
        return;
    }

    // Omitir las primeras dos líneas del encabezado
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);

    // Leer cada línea del archivo hasta llegar al final
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char interface[BUFFER_SIZE]; // Nombre de la interfaz
        unsigned long long r_bytes, r_packets, r_errors, r_drop, r_fifo, r_frame, r_compressed, r_multicast;
        unsigned long long t_bytes, t_packets, t_errors, t_drop, t_fifo, t_colls, t_carrier, t_compressed;

        // Parsear la línea para extraer los datos relevantes
        sscanf(buffer, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", interface,
               &r_bytes, &r_packets, &r_errors, &r_drop, &r_fifo, &r_frame, &r_compressed, &r_multicast, &t_bytes,
               &t_packets, &t_errors, &t_drop, &t_fifo, &t_colls, &t_carrier, &t_compressed);

        // Acumular bytes recibidos y transmitidos
        *rx_bytes += r_bytes;
        *tx_bytes += t_bytes;
        *rx_errors += r_errors;
        *tx_errors += t_errors;
        *collisions += t_colls;
    }

    fclose(fp); // Cerrar el archivo para liberar recursos
}

int get_process()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int process_count = 0; // Inicializar contador de procesos a cero

    // Abrir el archivo /proc/stat para leer
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    { // Verifica si ocurrió un error al abrir el archivo
        perror("Error opening /proc/stat");
        return -1; // Devuelve -1 en caso de error
    }

    // Leer cada línea del archivo hasta llegar al final
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // Intentar extraer el número de procesos en ejecución
        if (sscanf(buffer, "procs_running %d", &process_count) == 1)
        {
            break; // Salir del bucle si se encuentra la línea deseada
        }
    }

    fclose(fp);           // Cerrar el archivo para liberar recursos
    return process_count; // Devuelve el número de procesos en ejecución
}
