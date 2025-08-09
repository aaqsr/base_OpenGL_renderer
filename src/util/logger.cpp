#include "util/logger.hpp"

#include <iostream>
#include <string>
#include <utility>

Logger::Logger()
  : loggingThread{[this]() {
        this->processMessages();
    }}
{
}

void Logger::processMessages()
{
    while (true) {
        std::unique_lock<std::mutex> lock(queueMutex);

        // Wait for messages or shutdown signal
        queueCondition.wait(lock, [this] {
            return !messageQueue.empty() || shutdownRequested.load();
        });

        // Process all available messages
        while (!messageQueue.empty()) {
            std::string message = std::move(messageQueue.front());
            messageQueue.pop();

            // Unlock while doing I/O to allow other threads to queue messages
            lock.unlock();
            std::cout << message << "\n";
            lock.lock();
        }

        // Only exit after processing all messages and shutdown is requested
        if (shutdownRequested.load() && messageQueue.empty()) {
            break;
        }
    }
}

Logger::~Logger()
{
    shutdownRequested.store(true);
    queueCondition.notify_all();

    if (loggingThread.joinable()) {
        loggingThread.join();
    }
}

void Logger::logImpl(std::string str, bool force)
{
    if (!force && !loggingEnabled.load()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push(std::move(str));
    }
    queueCondition.notify_one();
}

void Logger::log(std::string str, bool force)
{
    GetInstance().logImpl(std::move(str), force);
}

void Logger::enable()
{
    GetInstance().loggingEnabled.store(true);
}

void Logger::disable()
{
    GetInstance().loggingEnabled.store(false);
}
