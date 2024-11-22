#include "metrics.h"

unsigned long long int get_ctxt()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long ctxt;

    // Open the /proc/stat file
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/stat"); // Logs an error if file opening fails
        return 1;
    }

    // Read the total context switches value from the file
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "ctxt %llu", &ctxt) == 1) // Extracts the "ctxt" value from the buffer
        {
            break;
        }
    }

    fclose(fp);

    // Checks if the context switch value was successfully retrieved
    if (ctxt == 0)
    {
        fprintf(stderr,
                "Error reading context switch information from /proc/stat\n"); // Logs an error if no value found
        return 1;
    }

    return ctxt; // Returns the retrieved context switch count
}

double get_memory_usage()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    unsigned long long total_mem = 0, free_mem = 0;

    // Open the /proc/meminfo file
    fp = fopen("/proc/meminfo", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/meminfo"); // Logs an error if file opening fails
        return -1.0;
    }

    // Read the total and available memory values
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        if (sscanf(buffer, "MemTotal: %llu kB", &total_mem) == 1)
        {
            continue; // MemTotal found, continue reading for MemAvailable
        }
        if (sscanf(buffer, "MemAvailable: %llu kB", &free_mem) == 1)
        {
            break; // MemAvailable found, stop reading
        }
    }

    fclose(fp);

    // Check if both values were successfully retrieved
    if (total_mem == 0 || free_mem == 0)
    {
        fprintf(stderr, "Error reading memory information from /proc/meminfo\n"); // Logs an error if values are missing
        return -1.0;
    }

    // Calculate the memory usage percentage
    double used_mem = total_mem - free_mem;
    double mem_usage_percent = (used_mem / total_mem) * 100.0;

    return mem_usage_percent; // Returns the calculated memory usage percentage
}

double get_cpu_usage()
{
    static unsigned long long prev_user = 0, prev_nice = 0, prev_system = 0, prev_idle = 0, prev_iowait = 0,
                              prev_irq = 0, prev_softirq = 0, prev_steal = 0;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long totald, idled;
    double cpu_usage_percent;

    // Open the /proc/stat file
    FILE* fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    {
        perror("Error opening /proc/stat"); // Logs error if file opening fails
        return -1.0;
    }

    char buffer[BUFFER_SIZE * 4];
    if (fgets(buffer, sizeof(buffer), fp) == NULL)
    {
        perror("Error reading /proc/stat"); // Logs error if file reading fails
        fclose(fp);
        return -1.0;
    }
    fclose(fp);

    // Parse the CPU time values from the file
    int ret = sscanf(buffer, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu", &user, &nice, &system, &idle, &iowait,
                     &irq, &softirq, &steal);
    if (ret < 8)
    {
        fprintf(stderr, "Error parsing /proc/stat\n"); // Logs error if parsing fails
        return -1.0;
    }

    // Calculate the differences between current and previous readings
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
        fprintf(stderr, "Totald is zero, cannot calculate CPU usage!\n"); // Logs error if no difference in total time
        return -1.0;
    }

    // Calculate the CPU usage percentage
    cpu_usage_percent = ((double)(totald - idled) / totald) * 100.0;

    // Update previous values for the next read
    prev_user = user;
    prev_nice = nice;
    prev_system = system;
    prev_idle = idle;
    prev_iowait = iowait;
    prev_irq = irq;
    prev_softirq = softirq;
    prev_steal = steal;

    return cpu_usage_percent; // Returns the calculated CPU usage percentage
}

void get_disk_io(unsigned long long* reads, unsigned long long* writes)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    *reads = 0;
    *writes = 0;

    // Open the /proc/diskstats file for reading
    fp = fopen("/proc/diskstats", "r");
    if (fp == NULL)
    { // Check for error opening the file
        perror("Error opening /proc/diskstats");
        return;
    }

    // Read each line of the file until end of file
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        unsigned long long read_sectors = 0, write_sectors = 0;
        int major, minor;         // Device numbers
        char device[BUFFER_SIZE]; // Device name
        long long unsigned int read_completed, read_merged, reading_ms;
        long long unsigned int write_completed, write_merged;

        // Parse the line to extract relevant data
        sscanf(buffer, "%d %d %s %llu %llu %llu %llu %llu %llu %llu", &major, &minor, device, &read_completed,
               &read_merged, &read_sectors, &reading_ms, &write_completed, &write_merged, &write_sectors);

        // Add read and write sectors to the totals
        *reads += read_sectors;
        *writes += write_sectors;
    }

    fclose(fp); // Close the file to free resources
}

void get_network_stats(unsigned long long* rx_bytes, unsigned long long* tx_bytes, unsigned long long* rx_errors,
                       unsigned long long* tx_errors, unsigned long long* collisions)
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    *rx_bytes = 0;   // Total number of received bytes
    *tx_bytes = 0;   // Total number of transmitted bytes
    *rx_errors = 0;  // Total number of receive errors
    *tx_errors = 0;  // Total number of transmit errors
    *collisions = 0; // Total number of collisions

    // Open the /proc/net/dev file for reading
    fp = fopen("/proc/net/dev", "r");
    if (fp == NULL)
    { // Check for error opening the file
        perror("Error opening /proc/net/dev");
        return;
    }

    // Skip the first two header lines
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);

    // Read each line of the file until end of file
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char interface[BUFFER_SIZE]; // Interface name
        unsigned long long r_bytes, r_packets, r_errors, r_drop, r_fifo, r_frame, r_compressed, r_multicast;
        unsigned long long t_bytes, t_packets, t_errors, t_drop, t_fifo, t_colls, t_carrier, t_compressed;

        // Parse the line to extract relevant data
        sscanf(buffer, "%s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", interface,
               &r_bytes, &r_packets, &r_errors, &r_drop, &r_fifo, &r_frame, &r_compressed, &r_multicast, &t_bytes,
               &t_packets, &t_errors, &t_drop, &t_fifo, &t_colls, &t_carrier, &t_compressed);

        // Accumulate received and transmitted bytes
        *rx_bytes += r_bytes;   // Accumulate received bytes
        *tx_bytes += t_bytes;   // Accumulate transmitted bytes
        *rx_errors += r_errors; // Accumulate receive errors
        *tx_errors += t_errors; // Accumulate transmit errors
        *collisions += t_colls; // Accumulate collisions
    }

    fclose(fp); // Close the file to free resources
}

int get_process()
{
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int process_count = 0; // Initialize process counter to zero

    // Open the /proc/stat file for reading
    fp = fopen("/proc/stat", "r");
    if (fp == NULL)
    { // Check for an error opening the file
        perror("Error opening /proc/stat");
        return -1; // Return -1 on error
    }

    // Read each line of the file until end of file
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        // Try to extract the number of running processes
        if (sscanf(buffer, "procs_running %d", &process_count) == 1)
        {
            break; // Exit the loop if the desired line is found
        }
    }

    fclose(fp);           // Close the file to free resources
    return process_count; // Return the number of running processes
}
