//ステッピングモータープログラム本体
//関数群はL6480.h参照
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include "L6480.h"
#include <time.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>

int main(void)
{
    int i;
    long speed;
 
    printf("***** start Stepper motor program *****\n");
 
    // SPI channel 0 を 1MHz で開始。
    if (wiringPiSPISetup(L6480_SPI_CHANNEL, 1000000) < 0)
    {
        printf("SPI Setup failed:\n");
    }
 
    //GPIO初期化
    wiringPiSetupGpio();
    pinMode(GPIO_STBY, OUTPUT);
    //STBYイネーブル
    digitalWrite(GPIO_STBY, 1);

    int lcd;       // ファイルディスクリプタ。
    char *i2cFileName = "/dev/i2c-1"; // I2Cドライバファイル名。
    int lcdAddress = LCD_ADDRESS;  // I2C LCD のアドレス。
 
    printf("***** start i2c lcd test program *****\n");
 
    // I2CポートをRead/Write属性でオープン。
    if ((lcd = open(i2cFileName, O_RDWR)) < 0)
    {
        printf("Faild to open i2c port\n");
        exit(1);
    }
 
     // 通信先アドレスの設定。
    if (ioctl(lcd, I2C_SLAVE, lcdAddress) < 0)
    {
        printf("Unable to get bus access to talk to slave\n");
        exit(1);
    }
 
    delay(100);
    LCD_ON(lcd);
    LCD_clear(lcd);

    // L6480の初期化。
    L6480_init();

    delay(5000);
    

    LCD_setCursor(0, 0, lcd);
    LCD_puts("In Operation", lcd);
    
    L6480_run(1, 47000);
    delay(5000);
    L6480_softhiz();
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("Motor Stopped", lcd);
    delay(1000);

    for(i = 0; i < 16; i++)
    {
        LCD_clear(lcd);
        LCD_setCursor(0, 0, lcd);
        LCD_puts("In Operation", lcd);
   	    L6480_move(1, 90);
	    delay(500);
    }

    L6480_run(1, 30000);
    delay(5000);
    L6480_softhiz();
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("Motor Stopped", lcd);
    delay(1000);
    L6480_gohome();
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("In Operation", lcd);
    
    //座標原点に戻るまで待機
    while(L6480_getparam_abspos() != 0)
    {
        
    }
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("Motor Stopped", lcd);
    delay(1000);
    L6480_gotodia(1, 400);
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("In Operation", lcd);
    //スピードが0になるまで待機
    while(L6480_getparam_speed() != 0)
    {
        
    }
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("Motor Stopped", lcd);
    delay(1000);
    L6480_gohome();
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("In Operation", lcd);
    
    //座標原点に戻るまで待機
    while(L6480_getparam_abspos() != 0)
    {
        
    }
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("Motor Stopped", lcd);
    delay(1000);
    L6480_gotodia(1, 400);
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("In Operation", lcd);
    delay(2000);
    LCD_clear(lcd);
    LCD_setCursor(0, 0, lcd);
    LCD_puts("Motor Stopped", lcd);
    LCD_clear(lcd);

    //STBYディスエーブル
    digitalWrite(GPIO_STBY, 0); 

    return 0;
}
