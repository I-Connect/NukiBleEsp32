#include <string>
#include <NimBLEDevice.h>

class BLEScannerSubscriber {
  public:
    virtual void onResult(NimBLEAdvertisedDevice* advertisedDevice) = 0;
};

class BleScanner : public BLEAdvertisedDeviceCallbacks {
  public:
    BleScanner(int reservedSubscribers = 10);
    ~BleScanner() = default;

    void initialize(const std::string& deviceName = "blescanner", bool wantDuplicates = false, uint16_t interval = 80, uint16_t window = 40);
    void update();

    void subscribe(BLEScannerSubscriber* subscriber);
    void unsubscribe(BLEScannerSubscriber* subscriber);

    void onResult(NimBLEAdvertisedDevice* advertisedDevice) override;

  private:
    BLEScan* bleScan = nullptr;
    std::vector<BLEScannerSubscriber*> subscribers;
};
