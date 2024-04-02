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
     * @brief Gets the current config from the lock, updates the latitude parameter and sends the new
     * config to the lock via BLE
     *
     * @param degrees the desired latitude
     */
    Nuki::CmdResult setLatitude(const float degrees);

    /**
     * @brief Gets the current config from the lock, updates the longitude parameter and sends the new
     * config to the lock via BLE
     *
     * @param degrees the desired longitude
     */
    Nuki::CmdResult setLongitude(const float degrees);

    /**
     * @brief Gets the current config from the lock, updates the given fob action parameter and sends the new
     * config to the lock via BLE
     *
     * @param fobActionNr the fob action to change (1 = single press, 2 = double press, 3 = triple press)
     * @param fobAction the desired fob action setting
     */
    Nuki::CmdResult setFobAction(const uint8_t fobActionNr, const uint8_t fobAction);

    /**
     * @brief Gets the current config from the lock, updates the operating mode parameter and sends the new
     * config to the lock via BLE
     *
     * @param opmode the desired operating mode
     */
    Nuki::CmdResult setOperatingMode(const uint8_t opmode);

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
     * @brief Gets the current config from the lock, updates the intercom id parameter and sends the
     * new config to the lock via BLE
     *
     * @param intercomID the desired database ID of the connected intercom
     */
    Nuki::CmdResult setIntercomID(const uint16_t intercomID);

    /**
     * @brief Gets the current config from the lock, updates the bus mode switch parameter and sends the
     * new config to the lock via BLE
     *
     * @param busModeSwitch true for analogue mode, false for data mode
     */
    Nuki::CmdResult setBusModeSwitch(const bool busModeSwitch);

    /**
     * @brief Gets the current config from the lock, updates the short circuit duration parameter and sends the
     * new config to the lock via BLE
     *
     * @param duration the desired duration of the short circuit for BUS mode switching in ms
     */
    Nuki::CmdResult setShortCircuitDuration(const uint16_t duration);

    /**
     * @brief Gets the current config from the lock, updates the electric strike delay parameter and sends the
     * new config to the lock via BLE
     *
     * @param delay the desired electric strike delay in ms in case of an electric strike actuation by RTO
     */
    Nuki::CmdResult setElectricStrikeDelay(const uint16_t delay);

    /**
     * @brief Gets the current config from the lock, updates the random electric strike delay parameter and sends the
     * new config to the lock via BLE
     *
     * @param enable true if random electric strike delay enabled
     */
    Nuki::CmdResult enableRandomElectricStrikeDelay(const bool enable);

    /**
     * @brief Gets the current config from the lock, updates the electric strike duration parameter and sends the
     * new config to the lock via BLE
     *
     * @param duration the desired duration in ms of electric strike actuation.
     */
    Nuki::CmdResult setElectricStrikeDuration(const uint16_t duration);

    /**
     * @brief Gets the current config from the lock, updates the disable rto after ring parameter and sends the
     * new config to the lock via BLE
     *
     * @param disable true if RTO should be disabled after ring
     */
    Nuki::CmdResult disableRtoAfterRing(const bool disable);

    /**
     * @brief Gets the current config from the lock, updates the rto timeout parameter and sends the
     * new config to the lock via BLE
     *
     * @param timeout the desired timeout for RTO in minutes
     */
    Nuki::CmdResult setRtoTimeout(const uint8_t timeout);

    /**
     * @brief Gets the current config from the lock, updates the doorbell suppression parameter and sends the
     * new config to the lock via BLE
     *
     * @param suppression the desired setting for doorbell suppression (0 = Off, 1 = CM, 2 = RTO, 3 = CM & RTO, 4 = Ring, 5 = CM & Ring, 6 = RTO & Ring, 7 = CM & RTO & Ring)
     */
    Nuki::CmdResult setDoorbellSuppression(const uint8_t suppression);

    /**
     * @brief Gets the current config from the lock, updates the doorbell suppression duration parameter and sends the
     * new config to the lock via BLE
     *
     * @param duration the duration in ms of doorbell suppression (only in Operating mode 0x02,0x03,0x04,0x05,0x07,0x08 digital Intercom)
     */
    Nuki::CmdResult setDoorbellSuppressionDuration(const uint16_t duration);

    /**
     * @brief Gets the current config from the lock, updates the sound ring parameter and sends the
     * new config to the lock via BLE
     *
     * @param sound the desired sound setting for ring (0 = No Sound, 1 = Sound1, 2 = Sound2, 3 = Sound3)
     */
    Nuki::CmdResult setSoundRing(const uint8_t sound);

    /**
     * @brief Gets the current config from the lock, updates the sound open parameter and sends the
     * new config to the lock via BLE
     *
     * @param sound the desired sound setting for open (0 = No Sound, 1 = Sound1, 2 = Sound2, 3 = Sound3)
     */
    Nuki::CmdResult setSoundOpen(const uint8_t sound);

    /**
     * @brief Gets the current config from the lock, updates the sound RTO parameter and sends the
     * new config to the lock via BLE
     *
     * @param sound the desired sound setting for RTO (0 = No Sound, 1 = Sound1, 2 = Sound2, 3 = Sound3)
     */
    Nuki::CmdResult setSoundRto(const uint8_t sound);

    /**
     * @brief Gets the current config from the lock, updates the sound continuous mode parameter and sends the
     * new config to the lock via BLE
     *
     * @param sound the desired sound setting for continuous mode (0 = No Sound, 1 = Sound1, 2 = Sound2, 3 = Sound3)
     */
    Nuki::CmdResult setSoundCm(const uint8_t sound);

    /**
     * @brief Gets the current config from the lock, updates the enable sound confirmation parameter and sends the
     * new config to the lock via BLE
     *
     * @param enable true if sound confirmation enabled
     */
    Nuki::CmdResult enableSoundConfirmation(const bool enable);

    /**
     * @brief Gets the current advanced config from the lock, updates the single button press action
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param action the desired action for a single button press
     */
    Nuki::CmdResult setSingleButtonPressAction(const ButtonPressAction action);

    /**
     * @brief Gets the current advanced config from the lock, updates the double button press action
     * parameter and sends the new advanced config to the lock via BLE
     *
     * @param action the desired action for a double button press
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