#include <string>
#include <NimBLEDevice.h>

class BLEScannerSubscriber {
  public:
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice) = 0;
};

class BLEScannerPublisher {
  public:
    virtual void subscribe(BLEScannerSubscriber* subscriber) = 0;
    virtual void unsubscribe(BLEScannerSubscriber* subscriber) = 0;
};

class BleScanner : public BLEScannerPublisher, BLEAdvertisedDeviceCallbacks {
  public:
    BleScanner(int reservedSubscribers = 10);
    ~BleScanner() = default;

    void initialize(const std::string& deviceName = "blescanner", const bool wantDuplicates = false, const uint16_t interval = 80, const uint16_t window = 40);
    void update();

    void subscribe(BLEScannerSubscriber* subscriber) override;
    void unsubscribe(BLEScannerSubscriber* subscriber) override;

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;

  private:
    BLEScan* bleScan = nullptr;
    std::vector<BLEScannerSubscriber*> subscribers;
};
