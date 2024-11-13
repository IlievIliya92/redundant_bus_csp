#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <condition_variable>

class ThreadPool {
public:
    // Constructor to create a thread pool with a given number of threads
    ThreadPool(size_t numThreads) {
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    // Lock the queue and wait for tasks
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(lock, [this] {
                          return !tasks.empty() || stop;
                        });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    // Execute the task
                    task();
                }
            });
        }
    }

    // Destructor to join all threads and clean up
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();

        for (std::thread &worker : workers) {
            worker.join();
        }
    }

    // Enqueue a task into the thread pool
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using returnType = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<returnType> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stop) {
                throw std::runtime_error("ThreadPool is stopped");
            }

            tasks.emplace([task]() { (*task)(); });
        }

        condition.notify_one();
        return result;
    }

private:
    // Vector of worker threads
    std::vector<std::thread> workers;

    // Task queue
    std::queue<std::function<void()>> tasks;

    // Synchronization primitives
    std::mutex queueMutex;
    std::condition_variable condition;

    // Flag to indicate if the thread pool should stop
    bool stop = false;
};
