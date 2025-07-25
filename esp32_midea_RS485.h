/*
    ESP32 library to control/monitor Midea air conditioner via XYE (RS485) bus
*/

#ifndef _esp32_midea_rs485_h
    #define _esp32_midea_rs485_h
    
    #include "Arduino.h"
    
    typedef enum{
      MIDEA_AC_OPMODE_OFF=0,
      MIDEA_AC_OPMODE_AUTO,
      MIDEA_AC_OPMODE_AUTO_HEAT,
      MIDEA_AC_OPMODE_AUTO_COOL,
      MIDEA_AC_OPMODE_COOL,
      MIDEA_AC_OPMODE_DRY,
      MIDEA_AC_OPMODE_HEAT,
      MIDEA_AC_OPMODE_FAN,
      MIDEA_AC_OPMODE_UNKOWN
    }MideaACOpModeType;

    typedef enum{
      MIDEA_AC_FANMODE_AUTO=0,
      MIDEA_AC_FANMODE_HIGH,
      MIDEA_AC_FANMODE_MEDIUM,
      MIDEA_AC_FANMODE_LOW,
      MIDEA_AC_FANMODE_UNKNOWN
    }MideaACFanModeType;
    
    typedef enum{
      MIDEA_AC_ACTIVE=0,
      MIDEA_AC_INACTIVE=1,
    }MideaACOpFeatureStateType;

    typedef enum{
      MIDEA_SWING_OFF=0,
      MIDEA_SWING_ON=71
    } MideaACOpFeatureSwingType;

    typedef enum{
      MIDEA_SWING1_OFF=0,
      MIDEA_SWING1_OFF1=64,
      MIDEA_SWING1_OFF2=70,
      MIDEA_SWING_1=65,
      MIDEA_SWING_2=66,
      MIDEA_SWING_3=67,
      MIDEA_SWING_4=68,
      MIDEA_SWING_5=69,
      MIDEA_SWING_AUTO=71
    }MideaACOpFeatureSwingPosType;

    typedef struct{
      uint8_t Unknown1;
      uint8_t Capabilities;
      MideaACOpModeType OpMode;
      MideaACFanModeType FanMode;
      uint8_t SetTemp;
      int T1Temp;
      int T2ATemp;
      //int T2BTemp;
      //int T3Temp;
      uint8_t Current;
      uint8_t Unknown2;
      uint32_t TimerStart;
      uint32_t TimerStop;
      uint8_t Unknown3;
      uint8_t ModeFlags;
      uint8_t OperatingFlags;
      uint16_t ErrorFlags;
      uint16_t ProtectFlags;
      uint8_t CCMComErrorFlags;
      uint8_t Unknown4;
      uint8_t Unknown5;    
      uint8_t ACNotResponding;    
    }MideaACPropertiesType;

    typedef struct{
      MideaACOpModeType OpMode;
      MideaACFanModeType FanMode;
      uint8_t SetTemp;
      MideaACOpFeatureStateType AuxHeat_Turbo;
      MideaACOpFeatureStateType Echo_Sleep;
      MideaACOpFeatureSwingType Swing;
      MideaACOpFeatureSwingPosType SwingPos;
      MideaACOpFeatureStateType Vent;
      uint32_t TimerStart;
      uint32_t TimerStop;
    }MideaACCommandType;

class ESP32_Midea_RS485Class {
  public:
    ESP32_Midea_RS485Class(HardwareSerial *hwSerial, uint8_t ro_pin, uint8_t di_pin, /*uint8_t re_de_pin,*/ uint8_t master_id, uint8_t slave_id, uint8_t command_time, uint8_t response_timeout);
    
    virtual void begin(HardwareSerial *hwSerial, uint8_t ro_pin, uint8_t di_pin, /*uint8_t re_de_pin,*/ uint8_t master_id, uint8_t slave_id, uint8_t command_time, uint8_t response_timeout);
    uint8_t SetMode(MideaACOpModeType mode);
    uint8_t SetFanMode(MideaACFanModeType fan_mode);
    uint8_t SetTemp(uint8_t temp);
    uint8_t SetAuxHeat_Turbo(MideaACOpFeatureStateType value);
    uint8_t SetEcho_Sleep(MideaACOpFeatureStateType value);
    uint8_t SetSwing(MideaACOpFeatureSwingType value);
    uint8_t SetSwingPos(MideaACOpFeatureSwingPosType value);
    uint8_t SetVent(MideaACOpFeatureStateType value);
    void Update();
    void Lock();
    void Unlock();
    MideaACPropertiesType State;
    uint8_t SentData[16];
    uint8_t ReceivedData[40];
    MideaACCommandType DesiredState;

  private:
    HardwareSerial *SerialBus;
    uint8_t datain_pin;
    uint8_t dataout_pin;
    uint8_t SlaveId;
    uint8_t MasterId;
    uint8_t Master_Send_Time;
    uint8_t Slave_Resp_Time;
    uint8_t UpdateNextCycle;
  protected:
    uint8_t CalculateCRC(uint8_t len);
    void ClearResponseBuffer();
    uint8_t ParseResponse();
    uint8_t CalculateSetTime(uint32_t time);
    uint32_t CalculateGetTime(uint8_t time);
}; 

extern ESP32_Midea_RS485Class ESP32_Midea_RS485;  

#endif
