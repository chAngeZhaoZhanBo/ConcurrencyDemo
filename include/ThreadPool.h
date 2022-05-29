#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>
#include <iostream>

namespace ThreadDemo {
class ThreadPool {
   public:
    using TaskType = std::function<void()>;

    size_t queued_tasks_num() const {
        std::scoped_lock lock(queue_mu_);
        return tasks_queue_.size();
    }

    size_t total_tasks_num() const { return tasks_unfinished_.load(); }

    size_t running_tasks_num() const {
        return total_tasks_num() - queued_tasks_num();
    }

    ThreadPool(size_t thread_count = std::thread::hardware_concurrency())
        : running_{true},
          thread_count_(thread_count ? thread_count
                                     : std::thread::hardware_concurrency()) {
        create_threads();
    }

    ~ThreadPool() {
        wait_for_all_tasks();
        running_.store(false);
    }

    template <typename F, typename... Args,
              typename R =
                  std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
    std::future<R> submit(F&& f, Args&&... args) {
        std::shared_ptr<std::promise<R>> task_promise{new std::promise<R>};
        std::future<R> fut = task_promise->get_future();

        auto task = [f = std::forward<F>(f),
                     ... args = std::forward<Args>(args),
                     task_promise = std::move(task_promise)] {
            task_promise->set_value(f(args...));
        };

        add_task(std::move(task));
        return fut;
    }

   private:
    std::atomic_bool running_;
    mutable std::mutex queue_mu_{};
    std::queue<TaskType> tasks_queue_{};
    const size_t thread_count_;
    std::vector<std::jthread> threads_{};
    std::atomic_size_t tasks_unfinished_{0};

    void create_threads() {
        for (size_t i = 0; i != thread_count_; ++i) {
            threads_.emplace_back(&ThreadPool::worker, this);
        }
    }

    void wait_for_all_tasks() {
        while (tasks_unfinished_.load() != 0) {
            std::this_thread::yield();
        }
    }

    template <typename F>
    void add_task(F&& task) {
        tasks_unfinished_++;
        {
            std::scoped_lock lock(queue_mu_);
            TaskType t{std::move(task)};
            tasks_queue_.push(std::move(t));
        }
    }

    bool get_task(TaskType& task) {
        std::scoped_lock lock(queue_mu_);
        if (tasks_queue_.empty()) {
            return false;
        }
        task = std::move(tasks_queue_.front());
        tasks_queue_.pop();
        return true;
    }

    void worker() {
        while (running_.load()) {
            TaskType task;
            if (get_task(task)) {
                task();
                tasks_unfinished_--;
            } else {
                std::this_thread::yield();
            }
        }
    }
};
}  // namespace ThreadDemo