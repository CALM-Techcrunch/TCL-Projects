#include <IRrecv.h>
#include "LS_config.h"

#define IR_RECEIVE_PIN 3  

IRrecv irrecv(IR_RECEIVE_PIN);
decode_results results;

void IR_init(void){
  irrecv.enableIRIn(); 
}

int getRemoteKey(uint64_t value){
  Serial.println(value);
   switch(value){
      case 0xE01:
      case 0x601:
        return 1;
        break;
      case 0xE02:
      case 0x602:
        return 2;
        break;
      case 0xE03:
      case 0x603:
        return 3;
        break;
      case 0xE04:
      case 0x604:
        return 4;
        break;
      case 0xE2D:
      case 0x63D:
        return 5;
        break;
      case 0xE06:
      case 0x606:
        return 6;
        break;
      default:
       return 99;     

   }
}

void handleRemote(){
  
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);  // Print received IR code
    int rem_key = getRemoteKey(results.value);
    Serial.println(rem_key);
    if(rem_key == 1){
      setRelay(1,true);
    }else if(rem_key == 2){
      setRelay(2,true);
    }else if(rem_key == 3){
      setRelay(3,true);
    } else if(rem_key == 4){
      setRelay(1,false);
    } else if(rem_key == 5){
      setRelay(2,false);
    } else if(rem_key == 6){
      setRelay(3,false);
    }
    irrecv.resume();  // Receive the next value
  }

}
