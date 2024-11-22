#include "expose_metrics.h"

/** Mutex for thread sincronization */
pthread_mutex_t lock;

/** Métric from Prometheus for the CPU usage */
static prom_gauge_t* cpu_usage_metric;

/** Métrica from Prometheus for the memory usage */
static prom_gauge_t* memory_usage_metric;

/** Métric de Prometheus for the number of readed sectors*/
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

void update_context_switches_gauge()
{
    unsigned long long ctxt = get_ctxt(); // Retrieves the current count of context switches

    // Checks if the retrieved value is valid
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

void update_disk_io_gauge()
{
    unsigned long long reads = 0, writes = 0;
    get_disk_io(&reads, &writes); // Retrieves disk I/O statistics for reads and writes

    // Checks if the read and write values are valid
    if (reads >= 0 && writes >= 0)
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

void update_network_gauge()
{
    unsigned long long rx_bytes = 0, tx_bytes = 0;
    unsigned long long rx_errors = 0, tx_errors = 0;
    unsigned long long collisions = 0;

    // Retrieves network statistics
    get_network_stats(&rx_bytes, &tx_bytes, &rx_errors, &tx_errors, &collisions);

    // Checks if all retrieved values are valid
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

void update_process_count_gauge()
{
    int process_count = get_process(); // Retrieves the current count of running processes

    // Checks if the retrieved value is valid
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

void* expose_metrics(void* arg)
{
    (void)arg; // Unused argument

    // Ensures the HTTP handler is attached to the default registry
    promhttp_set_active_collector_registry(NULL);

    // Starts the HTTP server on port 8000
    struct MHD_Daemon* daemon = promhttp_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, NULL, NULL);
    if (daemon == NULL)
    {
        fprintf(stderr, "Error starting HTTP server\n"); // Logs an error if server startup fails
        return NULL;
    }

    // Keeps the server running
    while (1)
    {
        sleep(1);
    }

    // Should never reach this point
    MHD_stop_daemon(daemon);
    return NULL;
}

int init_metrics()
{
    // Initializes the mutex
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "Error initializing mutex\n"); // Logs an error if mutex initialization fails
        return EXIT_FAILURE;
    }

    // Initializes the Prometheus collector registry
    if (prom_collector_registry_default_init() != 0)
    {
        fprintf(stderr, "Error initializing Prometheus registry\n"); // Logs an error if registry initialization fails
        return EXIT_FAILURE;
    }

    // Creates the CPU usage metric
    cpu_usage_metric = prom_gauge_new("cpu_usage_percentage", "CPU usage percentage", 0, NULL);
    if (cpu_usage_metric == NULL)
    {
        fprintf(stderr, "Error creating CPU usage metric\n");
        return EXIT_FAILURE;
    }

    // Creates the memory usage metric
    memory_usage_metric = prom_gauge_new("memory_usage_percentage", "Memory usage percentage", 0, NULL);
    if (memory_usage_metric == NULL)
    {
        fprintf(stderr, "Error creating memory usage metric\n");
        return EXIT_FAILURE;
    }

    // Creates Disk I/O metrics
    disk_io_reads_metric = prom_gauge_new("disk_io_reads", "Number of disk read sectors", 0, NULL);
    if (disk_io_reads_metric == NULL)
    {
        fprintf(stderr, "Error creating disk read metric\n");
        return EXIT_FAILURE;
    }

    disk_io_writes_metric = prom_gauge_new("disk_io_writes", "Number of disk write sectors", 0, NULL);
    if (disk_io_writes_metric == NULL)
    {
        fprintf(stderr, "Error creating disk write metric\n");
        return EXIT_FAILURE;
    }

    // Creates network metrics
    rx_bytes_metric = prom_gauge_new("network_rx_bytes", "Bytes received over the network", 0, NULL);
    if (rx_bytes_metric == NULL)
    {
        fprintf(stderr, "Error creating received bytes metric\n");
        return EXIT_FAILURE;
    }

    tx_bytes_metric = prom_gauge_new("network_tx_bytes", "Bytes transmitted over the network", 0, NULL);
    if (tx_bytes_metric == NULL)
    {
        fprintf(stderr, "Error creating transmitted bytes metric\n");
        return EXIT_FAILURE;
    }

    rx_errors_metric = prom_gauge_new("network_rx_errors", "Network receive errors", 0, NULL);
    if (rx_errors_metric == NULL)
    {
        fprintf(stderr, "Error creating receive errors metric\n");
        return EXIT_FAILURE;
    }

    tx_errors_metric = prom_gauge_new("network_tx_errors", "Network transmit errors", 0, NULL);
    if (tx_errors_metric == NULL)
    {
        fprintf(stderr, "Error creating transmit errors metric\n");
        return EXIT_FAILURE;
    }

    collisions_metric = prom_gauge_new("network_collisions", "Network collisions", 0, NULL);
    if (collisions_metric == NULL)
    {
        fprintf(stderr, "Error creating collisions metric\n");
        return EXIT_FAILURE;
    }

    // Creates the process count metric
    process_count_metric = prom_gauge_new("process_count", "Number of running processes", 0, NULL);
    if (process_count_metric == NULL)
    {
        fprintf(stderr, "Error creating process count metric\n");
        return EXIT_FAILURE;
    }

    // Creates the context switches metric
    context_switches_metric = prom_gauge_new("context_switches", "Number of context switches", 0, NULL);
    if (context_switches_metric == NULL)
    {
        fprintf(stderr, "Error creating context switches metric\n");
        return EXIT_FAILURE;
    }

    // Registers all metrics in the default registry
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

void destroy_mutex()
{
    pthread_mutex_destroy(&lock); // Destroys the mutex
}
