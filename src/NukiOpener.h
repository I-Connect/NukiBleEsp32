#pragma once

#include "NukiBle.h"
#include "NukiOpenerConstants.h"

namespace NukiOpener {

class NukiOpener : public Nuki::NukiBle {
  public:
    NukiOpener(const std::string& deviceName, const uint32_t deviceId);

    /**
     * @brief Sends lock action cmd via BLE to the lock
     *
     * @param lockAction
     * @param nukiAppId 0 = App, 1 = Bridge, 2 = Fob, 3 = Keypad
     * @param flags optional
     * @param nameSuffix optional
     * @param nameSuffixLen len of nameSuffix if used
     * @return Nuki::CmdResult
     */
    Nuki::CmdResult lockAction(const LockAction lockAction, const uint32_t nukiAppId = 1, const uint8_t flags = 0,
                               const char* nameSuffix = nullptr, const uint8_t nameSuffixLen = 0);


    /**
     * @brief Requests keyturner state from Lock via BLE
     *
     * @param state Nuki api based datatype to store the retrieved keyturnerstate
     */
    Nuki::CmdResult requestOpenerState(OpenerState* state);

    /**
     * @brief Gets the last keyturner state stored on the esp
     *
     * @param openerState Nuki api based datatype to store the retrieved keyturnerstate
     */
    void retrieveOpenerState(OpenerState* openerState);


    /**
     * @brief Requests battery status from Lock via BLE
     *
     * @param retrievedBatteryReport Nuki api based datatype to store the retrieved battery status
     */
    Nuki::CmdResult requestBatteryReport(BatteryReport* retrievedBatteryReport);


    /**
     * @brief Gets the current config from the lock, updates the name parameter and sends the
     * new config to the lock via BLE
     *
     * @param name max 32 character name
     */
    Nuki::CmdResult setName(const std::string& name);

    /**
     * @brief Gets the current config from the lock, updates the dst parameter and sends the new
     * config to the lock via BLE
     *
     * @param enable The desired daylight saving time mode. false disabled, true european
     */
    Nuki::CmdResult enableDst(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the timezone offset parameter and
     * sends the new config to the lock via BLE
     *
     * @param minutes The timezone offset (UTC) in minutes
     */
    Nuki::CmdResult setTimeZoneOffset(const int16_t minutes);

    /**
     * @brief Gets the current config from the lock, updates the timezone id parameter and sends the
     * new config to the lock via BLE
     *
     * @param timeZoneId 	The id of the current timezone or 0xFFFF if timezones are not supported
     */
    Nuki::CmdResult setTimeZoneId(const TimeZoneId timeZoneId);

    /**
     * @brief Gets the current config from the lock, updates the enable button parameter and sends the
     * new config to the lock via BLE
     *
     * @param enable true if button enabled
     */
    Nuki::CmdResult enableButton(const bool enable);

    /**
     * @brief Gets the current advanced config from the lock, updates the single button press action
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param action the deired action for a single button press
     */
    Nuki::CmdResult setSingleButtonPressAction(const ButtonPressAction action);

    /**
     * @brief Gets the current advanced config from the lock, updates the double button press action
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param action the deired action for a double button press
     */
    Nuki::CmdResult setDoubleButtonPressAction(const ButtonPressAction action);

    /**
     * @brief Gets the current advanced config from the lock, updates the battery type parameter and
     * sends the new advanced config to the lock via BLE
     *
     * @param type 	The type of the batteries present in the smart lock.
     */
    Nuki::CmdResult setBatteryType(const BatteryType type);

    /**
     * @brief Gets the current advanced config from the lock, updates the enable battery type
     * detection parameter and sends the new advanced config to the lock via BLE
     *
     * @param enable true if the automatic detection of the battery type is enabled
     */
    Nuki::CmdResult enableAutoBatteryTypeDetection(const bool enable);


    /**
     * @brief Sets the lock ability to pair with other devices (can be used to prevent unauthorized pairing)
     * Gets the current config from the lock, updates the pairing parameter and sends the new config to the lock via BLE
     * (CAUTION: if pairing is set to false and credentials are deleted a factory reset of the lock needs to be performed
     * before pairing is possible again)
     *
     * @param enable true if allowed to pair with other devices
     */
    Nuki::CmdResult enablePairing(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the whether or not the flashing
     * LED should be enabled to signal an unlocked door. And sends the new config to the lock via BLE
     *
     * @param enable true if led enabled
     */
    CmdResult enableLedFlash(const bool enable);

    /**
     * @brief Gets the current config from the lock, and updates the sound level.
     *
     * @param enable true if led enabled
     */
    CmdResult setSoundLevel(const uint8_t value);

    /**
     * @brief Gets the current config from the lock, updates the advertising frequency parameter
     * and sends the new config to the lock via BLE
     *
     * @param mode 0x00 Automatic, 0x01 Normal, 0x02 Slow, 0x03 Slowest (~400ms till ~1s)
     */
    Nuki::CmdResult setAdvertisingMode(const AdvertisingMode mode);


    /**
     * @brief Sends a new time(d) control entry via BLE to the lock.
     * This entry is independant of keypad or authorization entries, it will execute the
     * defined action at the defined time in the newTimeControlEntry
     *
     * @param newTimecontrolEntry Nuki api based datatype to send
     */
    Nuki::CmdResult addTimeControlEntry(NewTimeControlEntry newTimecontrolEntry);

    /**
     * @brief Sends an updated time(d) control entry via BLE to the lock.
     * (see addTimeControlEntry())
     *
     * @param TimeControlEntry Nuki api based datatype to send.
     * The ID can be retrieved via retrieveTimeControlEntries()
     */
    Nuki::CmdResult updateTimeControlEntry(TimeControlEntry TimeControlEntry);

    /**
     * @brief Deletes a time(d) control entry via BLE to the lock.
     * (see addTimeControlEntry())
     *
     * @param entryId The ID to be deleted, can be retrieved via retrieveTimeControlEntries()
     */
    Nuki::CmdResult removeTimeControlEntry(uint8_t entryId);

    /**
     * @brief Request the lock via BLE to send the existing time control entries
     *
     */
    Nuki::CmdResult retrieveTimeControlEntries();

    /**
     * @brief Get the time control entries stored on the esp (after executing retrieveTimeControlEntries())
     *
     * @param timeControlEntries list to store the returned time control entries
     */
    void getTimeControlEntries(std::list<TimeControlEntry>* timeControlEntries);

    /**
     * @brief Get the Log Entries stored on the esp. Only available after executing retreiveLogEntries.
     *
     * @param requestedLogEntries list to store the returned log entries
     */
    void getLogEntries(std::list<LogEntry>* requestedLogEntries);

    /**
    * @brief Request the lock via BLE to send the log entries
    *
    * @param startIndex Startindex of first log msg to be send
    * @param count The number of log entries to be read, starting at the specified start index.
    * @param sortOrder The desired sort order
    * @param totalCount true if a Log Entry Count is requested from the lock
    */
    Nuki::CmdResult retrieveLogEntries(const uint32_t startIndex, const uint16_t count, const uint8_t sortOrder,
                                       const bool totalCount);

    /**
     * @brief Requests config from Lock via BLE
     *
     * @param retrievedConfig Nuki api based datatype to store the retrieved config
     */
    Nuki::CmdResult requestConfig(Config* retrievedConfig);

    /**
     * @brief Requests advanced config from Lock via BLE
     *
     * @param retrievedAdvancedConfig Nuki api based datatype to store the retrieved advanced config
     */
    Nuki::CmdResult requestAdvancedConfig(AdvancedConfig* retrievedAdvancedConfig);


    /**
     * @brief Returns battery critical state parsed from the battery state byte (battery critical byte)
     *
     * Note that `retrieveOpenerState()` needs to be called first to retrieve the needed data
     *
     * @return true if critical
     */
    bool isBatteryCritical();

    /**
     * @brief Get the Last Error code received from the lock
     */
    const ErrorCode getLastError() const;

    virtual void logErrorCode(uint8_t errorCode) override;

  protected:
    void handleReturnMessage(Command returnCode, unsigned char* data, uint16_t dataLen) override;


  private:
    Nuki::CmdResult setConfig(NewConfig newConfig);
    Nuki::CmdResult setFromConfig(const Config config);
    Nuki::CmdResult setAdvancedConfig(NewAdvancedConfig newAdvancedConfig);
    void createNewConfig(const Config* oldConfig, NewConfig* newConfig);
    void createNewAdvancedConfig(const AdvancedConfig* oldConfig, NewAdvancedConfig* newConfig);
    Nuki::CmdResult setFromAdvancedConfig(const AdvancedConfig config);

    OpenerState openerState;
    BatteryReport batteryReport;
    std::list<TimeControlEntry> listOfTimeControlEntries;
    std::list<LogEntry> listOfLogEntries;

    Config config;
    AdvancedConfig advancedConfig;

};

}