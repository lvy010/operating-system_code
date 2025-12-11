#include <iostream>
#include <chrono>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <cstdlib>
#include <ctime>

// 心率监测（周期性定时器）
class HeartRateMonitor {
public:
    void start() {
        timer_ = std::thread([this]() {
            while (running_) {
                auto data = readBiosensor();
                onData(data);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

    void stop() { running_ = false; timer_.join(); }

private:
    int readBiosensor() { return 60 + rand() % 40; } // 模拟60-100随机心率
    void onData(int bpm) { std::cout << "[HR] " << bpm << " bpm\n"; }
    std::thread timer_;
    bool running_{true};
};

// 设备固件升级（单次定时+状态机）
class FirmwareUpdater {
    enum State { IDLE, DOWNLOADING, VERIFYING, DONE, FAILED };
public:
    void startUpgrade() {
        state_ = DOWNLOADING;
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(30));
            if (state_ != DONE) timeout();
        }).detach();
    }

    void onProgress(int percent) {
        if (percent >= 100) state_ = DONE;
        std::cout << "Upgrade: " << percent << "%\n";
    }

private:
    void timeout() { 
        state_ = FAILED;
        std::cout << "! Upgrade timeout\n";
    }
    State state_{IDLE};
};

// 室内定位系统（多定时器协同）
class LocationSystem {
public:
    void addDevice(int id) {
        devices_.emplace_back([id]() {
            while (true) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100 * (id + 1)));
                std::cout << "Device#" << id << " report position\n";
            }
        });
    }

private:
    std::vector<std::thread> devices_;
};

// 低功耗传感器（定时唤醒）
class LowPowerSensor {
public:
    void run() {
        while (true) {
            sleepMode(false);
            auto data = readSensor();
            processData(data);
            sleepMode(true);
            std::this_thread::sleep_for(std::chrono::minutes(5));
        }
    }

private:
    void sleepMode(bool on) { 
        std::cout << (on ? "Enter sleep" : "Wake up") << "\n"; 
    }
    int readSensor() { return rand() % 100; }
    void processData(int v) { std::cout << "Sensor value: " << v << "\n"; }
};

int main() {
    std::cout << "=== 定时器应用场景测试 ===" << std::endl;
    
    // 初始化随机数种子
    srand(time(nullptr));
    
    std::cout << "\n1. 心率监测测试（周期性定时器）" << std::endl;
    HeartRateMonitor hr;
    hr.start();
    std::this_thread::sleep_for(std::chrono::seconds(3));
    hr.stop();

    std::cout << "\n2. 固件升级测试（单次定时+状态机）" << std::endl;
    FirmwareUpdater fw;
    fw.startUpgrade();
    std::this_thread::sleep_for(std::chrono::seconds(15));
    fw.onProgress(50); // 模拟进度更新

    std::cout << "\n3. 多设备定位测试（多定时器协同）" << std::endl;
    LocationSystem loc;
    loc.addDevice(0);
    loc.addDevice(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(250));

    std::cout << "\n4. 低功耗传感器测试（定时唤醒）" << std::endl;
    LowPowerSensor sensor;
    std::thread([&sensor]{ sensor.run(); }).detach();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "\n所有测试完成！" << std::endl;
    return 0;
} 