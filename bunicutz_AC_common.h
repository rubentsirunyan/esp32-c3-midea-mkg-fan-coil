#include "esphome.h"



#define DF2_DI_PIN 13   //TX GPIO1
#define DF2_RO_PIN 13   //RX  GPIO3
#define DF2_SERIAL_COM_MASTER_SEND_TIME 140
#define DF2_SERIAL_COM_SLAVE_TIMEOUT_TIME 250
#include <esp32_midea_RS485.h>


#define DF2_SERIAL_COM_BUS &Serial1
#define DF2_SERIAL_COM_MASTER_ID 0
#define DF2_SERIAL_COM_SLAVE_ID 0

#define NOP() asm volatile ("nop")
//float DesiredTemp2=18;

class BunicutzACSensor2 : public PollingComponent, public Sensor {
 public:
  // constructor
  

  Sensor *ACT1Temp = new Sensor();
  Sensor *ACT2ATemp = new Sensor();
//   Sensor *ACT2BTemp = new Sensor();
//   Sensor *ACT3Temp = new Sensor();
  Sensor *ACNotResponding = new Sensor();
  std::string SetMode = "Unknown";
  std::string SetFanMode = "Unknown";
  uint8_t SetTemp = 18;
  //bool aux_heat = 0;
  //bool echo_sleep = 0;
  //bool vent = 0;
  bool swing = 0;
  std::string swingPos = "Auto";
  //bool swingPos = 0;
  //bool lock = 0;

  esphome::template_::TemplateSelect *_SetMode;
  esphome::template_::TemplateSelect *_SetFanMode;
  esphome::template_::TemplateNumber *_SetTemp;
  //esphome::template_::TemplateSwitch *_aux_heat;
  //esphome::template_::TemplateSwitch *_echo_sleep;
  //esphome::template_::TemplateSwitch *_vent;
  esphome::template_::TemplateSwitch *_swing;
  esphome::template_::TemplateSelect *_swingPos; 
  //esphome::template_::TemplateSwitch *_lock;

  uint8_t update_command=0;
  uint8_t update_internal=0;

  BunicutzACSensor2(\
  esphome::template_::TemplateSelect *&_SetMode_in,\
  esphome::template_::TemplateSelect *&_SetFanMode_in,\
  esphome::template_::TemplateNumber *&_SetTemp_in, \
  esphome::template_::TemplateSwitch *&_swing_in, \
  esphome::template_::TemplateSelect *&_swing_pos_in \
  ) : PollingComponent(20000)  {
  _SetMode=_SetMode_in;
  _SetFanMode = _SetFanMode_in;
  _SetTemp=_SetTemp_in;
  _swing = _swing_in;
  _swingPos = _swing_pos_in;

    _SetMode->add_on_state_callback([this](std::string value, size_t size)
                                        { if((1 != update_internal)&&(value != SetMode)){update_command =1; update2();} });
    _SetFanMode->add_on_state_callback([this](std::string value, size_t size)
                                        { if((1 != update_internal)&&(value != SetFanMode)){update_command =1;update2();}});
    _SetTemp->add_on_state_callback([this](float value)
                                        { if((1 != update_internal)&&(value != SetTemp)){update_command =1;update2();}});
    _swing->add_on_state_callback([this](bool value)
                                        { if((1 != update_internal)&&(value != swing)){update_command =1;update2();}});
    _swingPos->add_on_state_callback([this](std::string value, size_t size)
                                        { if((1 != update_internal)&&(value != swingPos)){update_command =1;update2();} });
  
  }

  float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }

  void setup() override {
    
    ESP32_Midea_RS485.begin(DF2_SERIAL_COM_BUS,DF2_RO_PIN,DF2_DI_PIN,DF2_SERIAL_COM_MASTER_ID,DF2_SERIAL_COM_SLAVE_ID,
    DF2_SERIAL_COM_MASTER_SEND_TIME,DF2_SERIAL_COM_SLAVE_TIMEOUT_TIME);

  }

  void update2()  {
    ESP_LOGD("custom","Force update ");
    update();
  }

  int64_t lasttime = 0;
  int64_t started = 0;
  
  void update() override {  
    struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t ctime = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));

    ESP_LOGD("custom","update time 2 : %u ",ctime);
    if (started == 0 || ctime-lasttime > 4000) {
        lasttime = ctime;
        started = 1;
        updateExecute();
    } else {    
        ESP_LOGD("custom","Delay execution");
        uint32_t m = micros();
        uint32_t e = (m + 4000);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
        ESP_LOGD("custom","Continue execution");
        update_command = 1;
        started = 1;
        updateExecute();
    }
    started = 0;
    
    }

  void updateExecute()  {
  float temp;
  uint8_t index;

  if(1==update_command)
  {
    SetMode = _SetMode->state.c_str();
    swing = _swing->state;
    SetFanMode = _SetFanMode->state.c_str();
    swingPos = _swingPos->state.c_str();

    if(SetMode == "Auto")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_AUTO);
        if (ESP32_Midea_RS485.State.OpMode != ESP32_Midea_RS485.DesiredState.OpMode){
            swing=false;
            SetFanMode="Auto";  
        }        
    }
    if(SetMode == "Auto_Heat")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_AUTO_HEAT);
    }
    if(SetMode == "Auto_Cool")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_AUTO_COOL);
    }
    if(SetMode == "Off")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_OFF);
        swing=false;
    }
    if(SetMode =="Cool")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_COOL);
        if (ESP32_Midea_RS485.State.OpMode != ESP32_Midea_RS485.DesiredState.OpMode){
            swing=false;
            SetFanMode="Auto";  
            swingPos="Auto";
        }        
    }
    if(SetMode == "Heat")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_HEAT);
    }
    if(SetMode == "Dry")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_DRY);
    }
    if(SetMode == "Fan")
    {
        ESP32_Midea_RS485.SetMode(MIDEA_AC_OPMODE_FAN);
    }



    if(SetFanMode=="Auto")
    {
        ESP32_Midea_RS485.SetFanMode(MIDEA_AC_FANMODE_AUTO);
    }
    if(SetFanMode=="High")
    {
        ESP32_Midea_RS485.SetFanMode(MIDEA_AC_FANMODE_HIGH);
    }
    if(SetFanMode=="Medium")
    {
        ESP32_Midea_RS485.SetFanMode(MIDEA_AC_FANMODE_MEDIUM);
    }
    if(SetFanMode=="Low")
    {
        ESP32_Midea_RS485.SetFanMode(MIDEA_AC_FANMODE_LOW);
    }
    
    SetTemp = (_SetTemp->state)/1;
    ESP32_Midea_RS485.SetTemp(SetTemp);

    // aux_heat = _aux_heat->state;
    // if(!aux_heat)
    // {
    //     ESP32_Midea_RS485.SetAuxHeat_Turbo(MIDEA_AC_ACTIVE);
    // }else
    //     {
    //         ESP32_Midea_RS485.SetAuxHeat_Turbo(MIDEA_AC_INACTIVE);
    //     }
    
    // echo_sleep = _echo_sleep->state;
    // if(!echo_sleep)
    // {
    //     ESP32_Midea_RS485.SetEcho_Sleep(MIDEA_AC_ACTIVE);
    // }else
    //     {
    //         ESP32_Midea_RS485.SetEcho_Sleep(MIDEA_AC_INACTIVE);
    //     }

    // vent = _vent->state;
    // if(!vent)
    // {
    //     ESP32_Midea_RS485.SetVent(MIDEA_AC_ACTIVE);
    // }else
    //     {
    //         ESP32_Midea_RS485.SetVent(MIDEA_AC_INACTIVE);
    //     }


    if(swing)
    {
        ESP32_Midea_RS485.SetSwing(MIDEA_SWING_ON);

  
  
    }else
    {
        ESP32_Midea_RS485.SetSwing(MIDEA_SWING_OFF);
    }


    //m1 = ESP32_Midea_RS485.State.OpMode;
    //DesiredState.OpMode
    if (/*ESP32_Midea_RS485.State.OpMode != MIDEA_AC_OPMODE_OFF &&
        SetMode != "Off" &&*/
        ESP32_Midea_RS485.State.OpMode == ESP32_Midea_RS485.DesiredState.OpMode
      ){


      if(swingPos == "Auto")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING_AUTO);
        }
        if(swingPos == "1")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING_1);
        }
        if(swingPos =="2")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING_2);
        }
        if(swingPos == "3")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING_3);
        }
        if(swingPos == "4")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING_4);
        }
        if(swingPos == "5")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING_5);
        }
        if(swingPos == "OFF")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING1_OFF);
        }
        if(swingPos == "OFF1")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING1_OFF1);
        }
        if(swingPos == "OFF2")
        {
            ESP32_Midea_RS485.SetSwingPos(MIDEA_SWING1_OFF2);
        }
    }





    update_command = 2;
  } 

  ESP32_Midea_RS485.Update();

  ESP_LOGD("custom","SentData: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x ",\
  ESP32_Midea_RS485.SentData[0],\
  ESP32_Midea_RS485.SentData[1],\
  ESP32_Midea_RS485.SentData[2],\
  ESP32_Midea_RS485.SentData[3],\
  ESP32_Midea_RS485.SentData[4],\
  ESP32_Midea_RS485.SentData[5],\
  ESP32_Midea_RS485.SentData[6],\
  ESP32_Midea_RS485.SentData[7],\
  ESP32_Midea_RS485.SentData[8],\
  ESP32_Midea_RS485.SentData[9],\
  ESP32_Midea_RS485.SentData[10],\
  ESP32_Midea_RS485.SentData[11],\
  ESP32_Midea_RS485.SentData[12],\
  ESP32_Midea_RS485.SentData[13],\
  ESP32_Midea_RS485.SentData[14],\
  ESP32_Midea_RS485.SentData[15]);
  ESP_LOGD("custom","ReceivedData: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",\
  ESP32_Midea_RS485.ReceivedData[0],\
  ESP32_Midea_RS485.ReceivedData[1],\
  ESP32_Midea_RS485.ReceivedData[2],\
  ESP32_Midea_RS485.ReceivedData[3],\
  ESP32_Midea_RS485.ReceivedData[4],\
  ESP32_Midea_RS485.ReceivedData[5],\
  ESP32_Midea_RS485.ReceivedData[6],\
  ESP32_Midea_RS485.ReceivedData[7],\
  ESP32_Midea_RS485.ReceivedData[8],\
  ESP32_Midea_RS485.ReceivedData[9],\
  ESP32_Midea_RS485.ReceivedData[10],\
  ESP32_Midea_RS485.ReceivedData[11],\
  ESP32_Midea_RS485.ReceivedData[12],\
  ESP32_Midea_RS485.ReceivedData[13],\
  ESP32_Midea_RS485.ReceivedData[14],\
  ESP32_Midea_RS485.ReceivedData[15],\
  ESP32_Midea_RS485.ReceivedData[16],\
  ESP32_Midea_RS485.ReceivedData[17],\
  ESP32_Midea_RS485.ReceivedData[18],\
  ESP32_Midea_RS485.ReceivedData[19],\
  ESP32_Midea_RS485.ReceivedData[20],\
  ESP32_Midea_RS485.ReceivedData[21],\
  ESP32_Midea_RS485.ReceivedData[22],\
  ESP32_Midea_RS485.ReceivedData[23],\
  ESP32_Midea_RS485.ReceivedData[24],\
  ESP32_Midea_RS485.ReceivedData[25],\
  ESP32_Midea_RS485.ReceivedData[26],\
  ESP32_Midea_RS485.ReceivedData[27],\
  ESP32_Midea_RS485.ReceivedData[28],\
  ESP32_Midea_RS485.ReceivedData[29],\
  ESP32_Midea_RS485.ReceivedData[30],\
  ESP32_Midea_RS485.ReceivedData[31],\
  ESP32_Midea_RS485.ReceivedData[32],\
  ESP32_Midea_RS485.ReceivedData[33],\
  ESP32_Midea_RS485.ReceivedData[34],\
  ESP32_Midea_RS485.ReceivedData[35],\
  ESP32_Midea_RS485.ReceivedData[36],\
  ESP32_Midea_RS485.ReceivedData[37],\
  ESP32_Midea_RS485.ReceivedData[38],\
  ESP32_Midea_RS485.ReceivedData[39]);
  
  for(index=0;index<40;index++)
  {
    ESP32_Midea_RS485.ReceivedData[index]=0;
  }
  
  if(ESP32_Midea_RS485.State.ACNotResponding == 0)
  {
    update_internal = 1;
    
    //ESP_LOGD("custom","---- TimerStart: %x ",\
       ESP32_Midea_RS485.State.TimerStart);
    //ESP_LOGD("custom","---- TimerStop: %x ",\
       ESP32_Midea_RS485.State.TimerStop);

    if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_AUTO)
    {
       _SetMode->publish_state("Auto");
    } else if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_AUTO_COOL)
    {
       _SetMode->publish_state("Auto_Cool");
    }  else if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_AUTO_HEAT)
    {
       _SetMode->publish_state("Auto_Heat");
    }   else if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_OFF)
    {
        _SetMode->publish_state("Off");
    }else if(ESP32_Midea_RS485.State.OpMode ==MIDEA_AC_OPMODE_COOL)
    {
        _SetMode->publish_state("Cool");
    }else if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_HEAT)
    {
        _SetMode->publish_state("Heat");
    }else if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_DRY)
    {
        _SetMode->publish_state("Dry");
    }else if(ESP32_Midea_RS485.State.OpMode == MIDEA_AC_OPMODE_FAN)
    {
        _SetMode->publish_state("Fan");
    }else
    {
        _SetMode->publish_state("Unknown");
    }

    if(ESP32_Midea_RS485.State.FanMode == MIDEA_AC_FANMODE_AUTO)
    {
       _SetFanMode->publish_state("Auto");
    }else if(ESP32_Midea_RS485.State.FanMode == MIDEA_AC_FANMODE_HIGH)
    {
        _SetFanMode->publish_state("High");
    }else if(ESP32_Midea_RS485.State.FanMode ==MIDEA_AC_FANMODE_MEDIUM)
    {
        _SetFanMode->publish_state("Medium");
    }else if(ESP32_Midea_RS485.State.FanMode == MIDEA_AC_FANMODE_LOW)
    {
        _SetFanMode->publish_state("Low");
    }else
    {
        _SetFanMode->publish_state("Unknown");
    }
    
    if(ESP32_Midea_RS485.State.SetTemp>0)
    {
        temp = ESP32_Midea_RS485.State.SetTemp * 1.0;
        _SetTemp->publish_state(temp);
    }else
        {
            _SetTemp->publish_state(18);
        }
    
    //_aux_heat->publish_state((ESP32_Midea_RS485.State.OperatingFlags&0x02)>0);
    //_echo_sleep->publish_state((ESP32_Midea_RS485.State.OperatingFlags&0x01)>0);
    //_vent->publish_state((ESP32_Midea_RS485.State.OperatingFlags&0x88)>0);
    //ESP_LOGD("custom","OperatingFlags: %x ",\
    ESP32_Midea_RS485.State.OperatingFlags);
    //_swing->publish_state((ESP32_Midea_RS485.State.OperatingFlags&0x05)==1);
    _swing->publish_state(ESP32_Midea_RS485.State.OperatingFlags==5);
    //_swingPos->publish_state((ESP32_Midea_RS485.State.OperatingFlags&0x04)>0);
    

    update_internal = 0;
  } 

  ACT1Temp->publish_state(ESP32_Midea_RS485.State.T1Temp);
  ACT2ATemp->publish_state(ESP32_Midea_RS485.State.T2ATemp);
  //ACT2BTemp->publish_state(ESP32_Midea_RS485.State.T2BTemp);
  //ACT3Temp->publish_state(ESP32_Midea_RS485.State.T3Temp);
  ACNotResponding->publish_state(ESP32_Midea_RS485.State.ACNotResponding);

  }
};
