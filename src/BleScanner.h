#pragma once
#include <string>
#include <NimBLEDevice.h>
#include "BleInterfaces.h"

class BleScanner : public BLEScannerPublisher, BLEAdvertisedDeviceCallbacks {
  public:
    BleScanner(int reservedSubscribers = 10);
    ~BleScanner() = default;

    void initialize(const std::string& deviceName = "blescanner", const bool wantDuplicates = false, const uint16_t interval = 40, const uint16_t window = 40);
    void update();

    void subscribe(BLEScannerSubscriber* subscriber) override;
    void unsubscribe(BLEScannerSubscriber* subscriber) override;

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;

  private:
    BLEScan* bleScan = nullptr;
    std::vector<BLEScannerSubscriber*> subscribers;
};
