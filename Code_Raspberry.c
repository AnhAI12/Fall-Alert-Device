
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <stdio.h>
#include <stdint.h>
#include <math.h> 
#include <stdlib.h>
#include <wiringPiSPI.h>  
// Define input and output
#define channel 0
#define canhbao 40
#define safe    38
#define red     36
#define green   37
// define mpu 6000
#define Sample_rate     25
#define Config          26
#define Gyro_config     27
#define Acc_config      28
#define Interrupt       56
#define PWR_managment   107

float canhbao_gtoc=2.2;
int canhbao_goc=80;


void SendData(uint8_t address, uint8_t value);
int8_t mpu6050;
float x,y,z, alpha_x,alpha_y,alpha_z;
int flag=0;
void init_mpu6050()
{
    //setup tg lay mau sample, 100HZ
    wiringPiI2CWriteReg8(mpu6050,Sample_rate,7);
    //config: DLPF <= 44HZ
    wiringPiI2CWriteReg8(mpu6050,Config,3);
    //GYRO CONFIG +-250 O/S
    wiringPiI2CWriteReg8(mpu6050,Gyro_config,0X00);
    // ACC CONFI: +-2G
    wiringPiI2CWriteReg8(mpu6050,Acc_config,0x00);
    //INT config: tat interrupt
    wiringPiI2CWriteReg8(mpu6050,Interrupt,1);
    //power config: clock source gyro x
    wiringPiI2CWriteReg8(mpu6050,PWR_managment,0x01);
}

int16_t read2thanhghi(unsigned char address)
{
    int16_t temp1,temp2,temp_gt;
    temp1=wiringPiI2CReadReg8(mpu6050,address);
    temp2=wiringPiI2CReadReg8(mpu6050,address+1);
    temp_gt=temp1<<8|temp2;
    return temp_gt;
}

void gia_tri_movement()
{
    // Doc gia tri van toc goc 
    float vx=abs((float)read2thanhghi(67)/131.0);
    float vy=abs((float)read2thanhghi(69)/131.0);
    float vz=abs((float)read2thanhghi(71)/131.0);
    // Doc gia tri goc
    float ax=abs((float)read2thanhghi(59)/4095.0);
    float ay=abs((float)read2thanhghi(61)/4095.0);
    float az=abs((float)read2thanhghi(63)/4095.0);

    // Tinh toan gt vector and vantoc goc
   // float pitch=atan2(ax,sqrt(pow(ay,2)+pow(az,2)))*180/3.14;
    //float roll=atan2(ay,sqrt(pow(ax,2)+pow(az,2)))*180/3.14;
    float giatoc_vector= sqrt(pow(ax,2)+pow(ay,2)+pow(az,2));
    float vantoc_goc= sqrt(pow(vx,2)+pow(vy,2)+pow(vz,2));

    printf("gia toc= %3.2f m/s^2, van toc goc=%3.2f deg/s\n",giatoc_vector,vantoc_goc);

    ////   WANRING   /////
    if (giatoc_vector>4.5  && vantoc_goc>60)
    {
        printf("ngã \n");
        flag=1;
    }
    
    wiringPiI2CReadReg8(mpu6050,0x58);
    
};
void xuly_ngat(void)
{
    if(digitalRead(safe)==0)
    {
        flag=0;
        digitalWrite(canhbao,LOW);
        
    } 
}

////////////// MAIN /////////////////////////////////////////////////////
int main(void)
{
    wiringPiSetupPhys();
    pinMode(canhbao,OUTPUT);
    pinMode(safe,INPUT);
    pinMode(green,OUTPUT);
    pinMode(red,OUTPUT);
    wiringPiISR(safe, INT_EDGE_BOTH,&xuly_ngat);
    mpu6050 = wiringPiI2CSetup(0x68);
    ///cau hinh mpu6050
    init_mpu6050();
    digitalWrite(green,HIGH);
    digitalWrite(red,LOW);

    while (1)
    {
        
        gia_tri_movement();
        delay(100);
        
        if (flag==0)
        {
            digitalWrite(canhbao,LOW);
            digitalWrite(green,HIGH);
            digitalWrite(red,LOW);
        }
        while (flag==1)
        {
            digitalWrite(canhbao,HIGH);
            digitalWrite(green,LOW);
            digitalWrite(red,HIGH);
            delay(300);
            digitalWrite(canhbao,LOW);
            digitalWrite(red,LOW);
            delay(300);
        //     digitalWrite(red,HIGH);
        //     digitalWrite(green,LOW);
        }
        
    }
    
    
}