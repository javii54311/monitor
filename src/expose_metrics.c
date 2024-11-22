#include "expose_metrics.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/** Mutex for thread synchronization */
pthread_mutex_t lock;

/** Métric from Prometheus for the CPU usage */
static prom_gauge_t* cpu_usage_metric;

/** Métrica from Prometheus for the memory usage */
static prom_gauge_t* memory_usage_metric;

/** Métric de Prometheus for the number of read sectors */
static prom_gauge_t* disk_io_reads_metric;

/** Métric from Prometheus for the number of written sectors */
static prom_gauge_t* disk_io_writes_metric;

/** Métric de Prometheus for the received bytes */
static prom_gauge_t* rx_bytes_metric;

/** Métric from Prometheus for the transmitted */
static prom_gauge_t* tx_bytes_metric;

/** Métric from Prometheus for the reception errors */
static prom_gauge_t* rx_errors_metric;

/** Métric from Prometheus for the transmission errors */
static prom_gauge_t* tx_errors_metric;

/** Métric from Prometheus for the collisions */
static prom_gauge_t* collisions_metric;

/** Métric from Prometheus for the number of running processes */
static prom_gauge_t* process_count_metric;

/** Métric from Prometheus for the number of context switches */
static prom_gauge_t* context_switches_metric;

/**
 * @brief Updates the context switch metric
 * 
 * This function retrieves the current context switch count and updates
 * the corresponding metric in the Prometheus registry.
 */
void update_context_switches_gauge()
{
    unsigned long long ctxt = get_ctxt(); // Retrieves the current count of context switches

    if (ctxt != (unsigned long long)-1) // Ensures no error occurred during retrieval
    {
        pthread_mutex_lock(&lock);                           // Locks the mutex for thread-safe access
        prom_gauge_set(context_switches_metric, ctxt, NULL); // Updates the context switch metric with the new count
        pthread_mutex_unlock(&lock);                         // Unlocks the mutex after updating
    }
    else
    {
        fprintf(stderr, "Error retrieving context switch count\n"); // Logs an error if retrieval failed
    }
}

/**
 * @brief Updates the CPU usage metric
 * 
 * This function retrieves the current CPU usage percentage and updates
 * the corresponding metric in the Prometheus registry.
 */
void update_cpu_gauge()
{
    double usage = get_cpu_usage(); // Retrieves the current CPU usage

    if (usage >= 0) // Checks if the retrieved CPU usage is valid
    {
        pthread_mutex_lock(&lock);                     // Locks the mutex for thread-safe access
        prom_gauge_set(cpu_usage_metric, usage, NULL); // Updates the CPU usage metric with the retrieved value
        pthread_mutex_unlock(&lock);                   // Unlocks the mutex after updating
    }
    else
    {
        fprintf(stderr, "Error retrieving CPU usage\n"); // Logs an error if retrieval failed
    }
}

/**
 * @brief Updates the memory usage metric
 * 
 * This function retrieves the current memory usage percentage and updates
 * the corresponding metric in the Prometheus registry.
 */
void update_memory_gauge()
{
    double usage = get_memory_usage(); // Retrieves the current memory usage

    if (usage >= 0) // Checks if the retrieved memory usage is valid
    {
        pthread_mutex_lock(&lock);                        // Locks the mutex for thread-safe access
        prom_gauge_set(memory_usage_metric, usage, NULL); // Updates the memory usage metric with the retrieved value
        pthread_mutex_unlock(&lock);                      // Unlocks the mutex after updating
    }
    else
    {
        fprintf(stderr, "Error retrieving memory usage\n"); // Logs an error if retrieval failed
    }
}

/**
 * @brief Updates the disk I/O metrics
 * 
 * This function retrieves the current disk read and write statistics and updates
 * the corresponding metrics in the Prometheus registry.
 */
void update_disk_io_gauge()
{
    unsigned long long reads = 0, writes = 0;
    get_disk_io(&reads, &writes); // Retrieves disk I/O statistics for reads and writes

    if (reads >= 0 && writes >= 0) // Checks if the read and write values are valid
    {
        pthread_mutex_lock(&lock);                           // Locks the mutex for thread-safe access
        prom_gauge_set(disk_io_reads_metric, reads, NULL);   // Updates the metric for disk read operations
        prom_gauge_set(disk_io_writes_metric, writes, NULL); // Updates the metric for disk write operations
        pthread_mutex_unlock(&lock);                         // Unlocks the mutex after updating
    }
    else
    {
        fprintf(stderr, "Error retrieving disk I/O statistics\n"); // Logs an error if retrieval failed
    }
}

/**
 * @brief Updates the network metrics
 * 
 * This function retrieves the current network statistics (received bytes, transmitted bytes,
 * receive errors, transmit errors, collisions) and updates the corresponding metrics
 * in the Prometheus registry.
 */
void update_network_gauge()
{
    unsigned long long rx_bytes = 0, tx_bytes = 0;
    unsigned long long rx_errors = 0, tx_errors = 0;
    unsigned long long collisions = 0;

    get_network_stats(&rx_bytes, &tx_bytes, &rx_errors, &tx_errors, &collisions);

    if (rx_bytes >= 0 && tx_bytes >= 0 && rx_errors >= 0 && tx_errors >= 0 && collisions >= 0)
    {
        pthread_mutex_lock(&lock);                           // Locks the mutex for thread-safe access
        prom_gauge_set(rx_bytes_metric, rx_bytes, NULL);     // Updates the metric for received bytes
        prom_gauge_set(tx_bytes_metric, tx_bytes, NULL);     // Updates the metric for transmitted bytes
        prom_gauge_set(rx_errors_metric, rx_errors, NULL);   // Updates the metric for receive errors
        prom_gauge_set(tx_errors_metric, tx_errors, NULL);   // Updates the metric for transmit errors
        prom_gauge_set(collisions_metric, collisions, NULL); // Updates the metric for network collisions
        pthread_mutex_unlock(&lock);                         // Unlocks the mutex after updating
    }
    else
    {
        fprintf(stderr, "Error retrieving network statistics\n"); // Logs an error if retrieval failed
    }
}

/**
 * @brief Updates the process count metric
 * 
 * This function retrieves the current count of running processes and updates
 * the corresponding metric in the Prometheus registry.
 */
void update_process_count_gauge()
{
    int process_count = get_process(); // Retrieves the current count of running processes

    if (process_count >= 0)
    {
        pthread_mutex_lock(&lock);                                 // Locks the mutex for thread-safe access
        prom_gauge_set(process_count_metric, process_count, NULL); // Updates the process count metric
        pthread_mutex_unlock(&lock);                               // Unlocks the mutex after updating
    }
    else
    {
        fprintf(stderr, "Error retrieving process count\n"); // Logs an error if retrieval failed
    }
}

/**
 * @brief Updates the context switches metric
 * 
 * This function retrieves the current context switch count and updates
 * the corresponding metric in the Prometheus registry.
 */
void update_context_switches_gauge()
{
    unsigned long long ctxt = get_ctxt();
    if (ctxt != (unsigned long long)-1)
    {
        pthread_mutex_lock(&lock);
        prom_gauge_set(context_switches_metric, ctxt, NULL);
        pthread_mutex_unlock(&lock);
    }
    else
    {
        fprintf(stderr, "Error retrieving context switch count\n");
    }
}

/**
 * @brief Function to expose metrics through an HTTP server
 * 
 * This function starts an HTTP server that exposes the system's metrics
 * to be scraped by Prometheus or other monitoring tools.
 */
void* expose_metrics(void* arg)
{
    (void)arg; // Unused argument
    promhttp_set_active_collector_registry(NULL);

    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error starting HTTP server\n");
        return NULL;
    }

    while (1)
    {
        sleep(1); // Keep the server running
    }

    MHD_stop_daemon(daemon); // Stop the server when done
    return NULL;
}

/**
 * @brief Initializes the Prometheus metrics
 * 
 * This function initializes all the Prometheus metrics and registers them
 * to the default collector registry.
 */
int init_metrics()
{
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error initializing mutex\n");
        return EXIT_FAILURE;
    }

    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error initializing Prometheus registry\n");
        return EXIT_FAILURE;
    }

    // Create metrics
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "CPU usage percentage", 0, NULL);
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Memory usage percentage", 0, NULL);
    disk_io_reads_metric = prom_gauge_new("disk_io_reads", "Number of disk read sectors", 0, NULL);
    disk_io_writes_metric = prom_gauge_new("disk_io_writes", "Number of disk write sectors", 0, NULL);
    rx_bytes_metric = prom_gauge_new("network_rx_bytes", "Bytes received over the network", 0, NULL);
    tx_bytes_metric = prom_gauge_new("network_tx_bytes", "Bytes transmitted over the network", 0, NULL);
    rx_errors_metric = prom_gauge_new("network_rx_errors", "Network receive errors", 0, NULL);
    tx_errors_metric = prom_gauge_new("network_tx_errors", "Network transmit errors", 0, NULL);
    collisions_metric = prom_gauge_new("network_collisions", "Network collisions", 0, NULL);
    process_count_metric = prom_gauge_new("process_count", "Number of running processes", 0, NULL);
    context_switches_metric = prom_gauge_new("context_switches", "Number of context switches", 0, NULL);

    // Register metrics
    prom_collector_registry_must_register_metric(cpu_usage_metric);
    prom_collector_registry_must_register_metric(memory_usage_metric);
    prom_collector_registry_must_register_metric(disk_io_reads_metric);
    prom_collector_registry_must_register_metric(disk_io_writes_metric);
    prom_collector_registry_must_register_metric(rx_bytes_metric);
    prom_collector_registry_must_register_metric(tx_bytes_metric);
    prom_collector_registry_must_register_metric(rx_errors_metric);
    prom_collector_registry_must_register_metric(tx_errors_metric);
    prom_collector_registry_must_register_metric(collisions_metric);
    prom_collector_registry_must_register_metric(process_count_metric);
    prom_collector_registry_must_register_metric(context_switches_metric);

    return EXIT_SUCCESS;
}

/**
 * @brief Destroys the mutex
 * 
 * This function cleans up and destroys the mutex used for synchronization.
 */
void destroy_mutex()
{
    pthread_mutex_destroy(&lock); // Destroy the mutex
}
