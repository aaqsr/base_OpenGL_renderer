#pragma once

#include "singleton.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

/**
 * @class Logger
 * @brief A thread-safe, asynchronous, singleton logger.
 * @ingroup util
 *
 * @details This class provides a global point of access for logging messages
 * throughout the application. It is designed to be highly performant by
 * offloading the slow I/O operations of writing log messages to a dedicated
 * background thread.
 *
 * @section Architecture
 * The logger is implemented using several key patterns:
 * - **Singleton:** It inherits from a `Singleton<Logger>` template, ensuring
 * that only one instance of the logger exists globally. This provides a
 * convenient, centralized point for all logging activities.
 * - **Asynchronous Processing:** Calls to the `log()` methods do not write
 * directly to the console. Instead, they push the log message into a
 * thread-safe queue and return immediately. A dedicated background thread
 * (`loggingThread`) is responsible for consuming messages from this queue and
 * performing the actual I/O.
 *
 * @section Technicality
 * The core of the asynchronous mechanism is a `std::queue` protected by a
 * `std::mutex` and coordinated with a `std::condition_variable`. This provides
 * thread-safe access for multiple producers and a single consumer.
 *
 * @section Performance
 * This design significantly improves performance by decoupling the main
 * application logic from slow I/O operations. The main simulation loop is not
 * blocked waiting for logs to be written to disk or the console, which prevents
 * stuttering and maintains a smooth frame rate.
 *
 * @section Caveats
 * The destructor handles a graceful shutdown by signalling the thread to stop
 * and joining it, ensuring all pending messages are flushed before the program
 * exits.
 */
class Logger : public Singleton<Logger>
{
    friend class Singleton<Logger>;

    /**
     * @brief The queue used to buffer log messages between producer threads
     * and the consumer thread.
     */
    std::queue<std::string> messageQueue;

    /**
     * @brief Mutex to protect access to the message queue.
     */
    std::mutex queueMutex;

    /**
     * @brief Condition variable to notify the logging thread when new messages
     * are available.
     */
    std::condition_variable queueCondition;

    /**
     * @brief The dedicated background thread that processes and writes log
     * messages.
     */
    std::thread loggingThread;

    /** @brief Whether logging is enabled. */
    std::atomic<bool> loggingEnabled{true};

    /** @brief Flag to signal the logging thread to shutdown. */
    std::atomic<bool> shutdownRequested{false};

    /**
     * @brief Private constructor to enforce the singleton pattern. It spawns
     * the logging thread.
     */
    Logger();

    /**
     * @brief The main function executed by the `loggingThread`. It runs in a
     * loop, consuming messages from the message queue and writing them to
     * `std::cout`.
     */
    void processMessages();

    /** @brief Private implementation method. */
    void logImpl(std::string str, bool force);

  public:
    // Delete copy/move constructors and assignment operators to prevent
    // duplication.
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    /**
     * @brief Destructor that ensures a graceful shutdown of the logging thread.
     * @details It signals the thread to shut down, then joins the thread,
     * waiting for it to finish processing any remaining messages in the queue.
     */
    ~Logger() override;

    /**
     * @brief Enqueues a string message to be logged asynchronously.
     * @param str The message to log.
     * @param force Whether to log even if logging is disabled.
     */
    static void log(std::string str, bool force = false);

    /** @brief Enable log messages being registered. */
    static void enable();
    /** @brief Disable log messages being registered. */
    static void disable();
};
