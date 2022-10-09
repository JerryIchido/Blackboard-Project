//接线： 棕色的10，红色的11！
#include <SoftwareSerial.h>
#define max_diff 20 // The higher the value, the lower the probability the sensor will regard sth as change
                    //However it should not be too low to avoid wrong actions of the fiilter.
#define sensitivity 3  //The higher the value, the slower the sensor will react to a light change.
                       //However the value should not go below 3 to aviod wrong sensations
int width=200;  //The width of the board set in openmv

//Next 5 lines are motor control!!
int IN1 = 2;   // IN1 connected to pin 1
int IN2 = 4;   
int ENA = 5;  
unsigned long time = 150;  //delay time
int value = 200;   // the duty cycle
int delay_sum = 1100; 

SoftwareSerial softSerial(10, 11); // RX, TX
typedef struct
{
  int data[50][2] = {{0,0}};
  int len = 0;
}List;
List list;

void setup() {
  // put your setup code here, to run once:
  softSerial.begin(9600);
  Serial.begin(9600);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

//We are putting digitalWrite in the loop!!!
//1 HI and 2 LO is clockwise!!!
//1 LO and 2 HI is counterclockwise!!!
//Both HI or both LO is stop!!!
//顺时针转，黑板向左偏
//逆时针转，黑板向右偏
//光照在右边，逆时针转
//光照在左边，顺时针转！！！

char filt_count = 0;
int receive_flag=0;
int position_set[8]={-1};
int mean_position = (position_set[0]+position_set[1]+position_set[2]+position_set[3]+position_set[4]+position_set[5]+position_set[6]+position_set[7])/8;
void loop() {
  receive_flag=0;

  if(softSerial.available())
  {
    
    int k = 0; // for FOR LOOP ONLY
    for(k = 0;k < 8;k++){
      
      getList();
    
    
      //Serial.println(list.data[0][0]);
      //Serial.print('\t');
      //Serial.println(list.data[i][1]);
      int light_in_x = list.data[0][0];
      if(light_in_x>0)  {Serial.println("aertygf");}
      Serial.println(list.data[0][0]);
      //Serial.println(light_in_x);
      //IDEA: if arduino is just setting up, previous x will be set to -1 to indicate.
      //If arduino has completed at least one loop, take the previous one's mean as the previous x. 
      if (k == 0){
        position_set[k] = filter(mean_position, light_in_x);
        }
      else{
        position_set[k]= filter(position_set[k-1],light_in_x);
        //Serial.println(position_set[k]);
        }
      
      clearList();
      delay(10);
   }
    //By now Position set should contain ALL 8 X positions!
    //Serial.println(position_set[0]);
    //Serial.println(position_set[1]);
    //Serial.println(position_set[2]);
    int position_previous = mean_position; //The last mean position, to be compared later
    Serial.print("Previous position is:");
    Serial.println(position_previous);
    
    mean_position = (position_set[0]+position_set[1]+position_set[2]+position_set[3]+position_set[4]+position_set[5]+position_set[6]+position_set[7])/8;
    Serial.print("The mean position for this time is:");
    Serial.println(mean_position);
  
    //NEXT comes the code controlling the motor!!!!! 只有光在不靠近边缘与中心的位置，motor才会工作。
    int grace_low = width*1/20;  // If the light is present too close to the edge, motor won't work
    int grace_high = width*19/20;
    int grace_middle_left = width*8/20;//If the light is present too close to the middle, it won't work.
    int grace_middle_right = width*12/20;
    int work_status = decide_if_work(position_previous, mean_position, grace_low, grace_high, grace_middle_left, grace_middle_right);
    if(work_status){ //1代表光在左，2代表光在右
      if(work_status == 1){ //光照在左，顺时针转！！！
        clockwise_rotate();
        
        delay(time);
        Serial.println("Motor rotate clockwise!");
        }
       else if(work_status == 2){
        counter_rotate();
        
        delay(time);
        
        Serial.println("Motor rotate counterclockwise!");
        }
       
      
      
      }
      else{
        motor_stop(); 
        Serial.println("Motor stops!");
        }
    
    
  
  }
  else{
    motor_stop();
    Serial.println("Motor stops!");
    }
  motor_stop();

}

int filter(int previous_x, int new_x){
  
    if(new_x==0){
      //Serial.println("Return previous x");
      return previous_x;
      }
    else if ((previous_x-new_x) > 20 || (new_x-previous_x) > 20){  //max diff is defined as 20 above
      filt_count++;
      if(filt_count >= sensitivity){
        //Serial.println("Return new x");
        filt_count = 0;
        return new_x;
        }  //If the filter has been working constantly, assume the initial value as wrong.
        
      //Serial.println("Return previous x");
      return previous_x;
      }
      
      
      
    else{
      if(filt_count>0)
        filt_count--;
      //Serial.println("Filter not working");
      return new_x;
      
      }
    
  }


int decide_if_work(int previous_pos, int current_pos, int grace_low, int grace_high, int middle_left, int middle_right){
  //Serial.println(previous_pos);
  //Serial.println(current_pos);
  //Serial.println(grace_low);
  //Serial.println(grace_high);
  //Serial.println(middle_left);
  //Serial.println(middle_right);
  if( (previous_pos < middle_left)  && (current_pos < middle_left))
    return 1; // 1代表光在左边
  else if( (previous_pos<grace_high) && (current_pos<grace_high)&& (previous_pos > middle_right) &&  (current_pos > middle_right))
    return 2; // 2代表光在右边
  else{
    Serial.println("Motor does not work.");
    return 0;
    }
  
  }

void clockwise_rotate(){
  
  analogWrite(ENA, value);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  }

void counter_rotate(){
  analogWrite(ENA, value);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  }

void motor_stop(){
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  }

String detectString()
{
  while(softSerial.read() != '{');
  return(softSerial.readStringUntil('}'));
}
void clearList()
{
  memset(list.data, sizeof(list.data),0);
  list.len = 0;
}
void getList()
{
  String s = detectString();
  String numStr = "";
  for(int i = 0; i<s.length(); i++)
  {
    if(s[i]=='('){
      numStr = "";
    }
    else if(s[i] == ','){
      list.data[list.len][0] = numStr.toInt();
      numStr = "";
    }
    else if(s[i]==')'){
      list.data[list.len][1] = numStr.toInt();
      numStr = "";
      list.len++;
    }
    else{
      numStr += s[i];
    }
  }
}
