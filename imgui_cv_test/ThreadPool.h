#pragma once
#include "stdafx.h"

class ThreadPool {
    std::vector<std::thread> workers;            // 워커 스레드 리스트
    std::queue<std::function<void()>> tasks;     // 작업 큐
    std::mutex queueMutex;                       // 작업 큐 보호용 뮤텍스
    std::condition_variable condition;          // 작업 대기용 조건 변수
    std::atomic<bool> stop;                      // 스레드 풀 종료 플래그

public:
    // 생성자: 스레드 워커를 threads 개수만큼 생성
    ThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;

                    // 작업 큐에서 작업을 가져옴
                    {
                        std::unique_lock<std::mutex> lock(this->queueMutex);
                        // 작업 큐가 비어있고 종료 신호가 없는 경우 대기
                        this->condition.wait(lock, [this] {
                            return this->stop || !this->tasks.empty();
                        });

                        // 종료 신호와 작업 큐가 비었으면 스레드 종료
                        if (this->stop && this->tasks.empty())
                            return;

                        // 작업 큐에서 작업을 가져옴
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // 작업 실행
                    task();
                }
            });
        }
    }

    // 소멸자: 모든 스레드 종료
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true; // 종료 플래그 설정
        }
        condition.notify_all(); // 모든 스레드 깨우기

        for (std::thread& worker : workers) {
            worker.join(); // 스레드가 끝날 때까지 대기
        }
    }

    // 작업 추가 메서드
    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.push(std::move(task)); // 작업 큐에 추가
        }
        condition.notify_one(); // 대기 중인 워커 스레드 깨우기
    }
};

