/*L6480 コントロールコマンド
 引数-----------------------
 dia   1:正転 0:逆転,
 spd  (20bit)(0.01490*spd[step/s])
 pos  (22bit)
 n_step (22bit)
 act   1:現在座標をマーク  0:現在座標をリセット
 mssec ミリ秒
 val 各レジスタに書き込む値
 ---------------------------*/


#define L6480_SPI_CHANNEL 0
#define GPIO_STBY 05
#define GPIO_LED 14
#define LCD_ADDRESS (0b111100)
#define LCD_CONTRAST 100
#define LCD_RS_CMD (0x00)
#define LCD_RS_DATA (0x40)
#define LCD_CMD_CLEAR (0x01)
#define LCD_CMD_HOME (0x02)

// 関数プロトタイプ宣言
void L6480_write(unsigned char data);
unsigned char L6480_read(unsigned char data);
void L6480_init(void);
void L6480_run(int dia, long spd);
void L6480_move(int dia, long n_step);
void L6480_goto(long pos);
void L6480_gotodia(int dia,int pos);
void L6480_gountil(int act,int dia,long spd);
void L6480_relesesw(int act,int dia);
void L6480_softstop(void);
void L6480_hardstop(void);
void L6480_softhiz(void);
void L6480_hardhiz(void);
void L6480_gohome(void);
void L6480_gomark(void);
void L6480_transfer(int add, int bytes, long val);
unsigned long L6480_getparam(unsigned char add, int bytes);
unsigned long L6480_getparam_abspos(void);
unsigned long L6480_getparam_elpos(void);
unsigned long L6480_getparam_speed(void);
void LCD_write(unsigned char rs, unsigned char data, int fd);
void LCD_clear(int fd);
void LCD_setCursor(unsigned char col, unsigned char row, int fd);
void LCD_putc(unsigned char c, int fd);
void LCD_puts(char *str, int fd);
void LCD_ON(int fd);
/*-----------------------
L6480_write(unsigned char data);SPIデータ送信
L6480_read(unsigned char data);getparam用データ受信
L6480_init(void);初期設定
L6480_run(dia,spd);指定方向に連続回転
L6480_move(dia,n_step);指定方向に指定数ステップする
L6480_goto(pos);指定座標に最短でいける回転方向で移動
L6480_gotodia(dia,pos);回転方向を指定して指定座標に移動
L6480_gountil(act,dia,spd);
指定した回転方向に指定した速度で回転しスイッチのONで急停止と座標処理
L6480_relesesw(act,dia);
スイッチがOFFに戻るまで最低速度で回転し停止と座標処理 
L6480_softstop();回転停止、保持トルクあり
L6480_hardstop();回転急停止、保持トルクあり
L6480_softhiz();回転停止、保持トルクなし
L6480_hardhiz();回転急停止、保持トルクなし
L6480_gohome();座標原点に移動
L6480_gomark();マーク座標に移動
L6480_resetpos();絶対座標リセット
-----------------------*/

void L6480_write(unsigned char data)
{
    wiringPiSPIDataRW(L6480_SPI_CHANNEL, &data, 1);
}

unsigned char L6480_read(unsigned char data)
{
    wiringPiSPIDataRW(L6480_SPI_CHANNEL, &data, 1);
    return data;
}



/*初期設定-----------------------
[R]:読み取り専用
[WR]:いつでも書き換え可
[WH]:書き込みは出力がハイインピーダンスの時のみ可
[WS]:書き換えはモータが停止している時のみ可
--------------------------------*/
void L6480_init(void)
{
    //abspos
    L6480_write(0x01);
    L6480_write(0x00);
    L6480_write(0x00);
    L6480_write(0x00);
    //[R, WS]
    //現在座標default 0x000000 (22bit)

    //elpos
    L6480_write(0x02);
    L6480_write(0x00);
    L6480_write(0x00);
    //[R, WS]
    //コイル励磁の電気的位置default 0x000 (2+7bit)

    //mark
    L6480_write(0x03);
    L6480_write(0x00);
    L6480_write(0x00);
    L6480_write(0x00);
    //[R, WR]
    //マーク座標default 0x000000 (22bit)

    //speed
    L6480_write(0x04);
    L6480_write(0x00);
    L6480_write(0x00);
    L6480_write(0x00);
    //[R]
    //現在速度read onry  (20bit)

    //acc
    L6480_write(0x05);
    L6480_write(0x00);
    L6480_write(0x50);
    //[R, WS]
    //加速度default 0x08A (12bit) (14.55*val+14.55[step/s^2])

    //dec
    L6480_write(0x06);
    L6480_write(0x00);
    L6480_write(0x50);
    //[R, WS]
    //減速度default 0x08A (12bit) (14.55*val+14.55[step/s^2])

    //maxspeed
    L6480_write(0x07);
    L6480_write(0x00);
    L6480_write(0x2D);
    //[R, WR]
    //最大速度default 0x041 (10bit) (15.25*val+15.25[step/s])

    //minspeed
    L6480_write(0x08);
    L6480_write(0x00);
    L6480_write(0x00);
    //[R, WS]
    //最小速度default 0x0000 (1+12bit) (0.238*val+[step/s])

    //fsspeed
    L6480_write(0x15);
    L6480_write(0x00);
    L6480_write(0x35);
    //[R, WR]
    //μステップからフルステップへの切替点速度default 0x027 (10bit) (15.25*val+7.63[step/s])

    //kvalhold
    L6480_write(0x09);
    L6480_write(0x55);
    //[R, WR]
    //停止時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)

    //kvalrun
    L6480_write(0x0A);
    L6480_write(0x55);
    //[R, WR]
    //定速回転時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)

    //kvalacc
    L6480_write(0x0B);
    L6480_write(0x55);
    //[R, WR]
    //加速時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)

    //kvaldec
    L6480_write(0x0C);
    L6480_write(0x55);
    //[R, WR]
    //減速時励磁電圧default 0x29 (8bit) (Vs[V]*val/256)

    //intspeed
    L6480_write(0x0D);
    L6480_write(0x07);
    L6480_write(0x00);
    //[R, WH]
    //逆起電力補償切替点速度default 0x0408 (14bit) (0.238*val[step/s])

    //stslp
    L6480_write(0x0E);
    L6480_write(0x35);
    //[R, WH]
    //逆起電力補償低速時勾配default 0x19 (8bit) (0.000015*val[% s/step])

    //fnslpacc
    L6480_write(0x0F);
    L6480_write(0x40);
    //[R, WH]
    //逆起電力補償高速時加速勾配default 0x29 (8bit) (0.000015*val[% s/step])

    //fnslpdec
    L6480_write(0x10);
    L6480_write(0x70);
    //[R, WH]
    //逆起電力補償高速時減速勾配default 0x29 (8bit) (0.000015*val[% s/step])

    //ktherm
    L6480_write(0x11);
    L6480_write(0x01);
    //[R, WR]
    //不明default 0x0 (4bit) (0.03125*val+1)

    //ocdth
    L6480_write(0x13);
    L6480_write(0x15);
    //[R, WR]
    //過電流しきい値default 0x8 (5bit) (31.25*val+31.25[mV])

    //stallth
    L6480_write(0x14);
    L6480_write(0x02);
    //[R, WR]
    //失速電流しきい値？default 0x10 (5bit) (31.25*val+31.25[mV])

    //stepmode
    L6480_write(0x16);
    L6480_write(0x00);
    //[R, WH]
    //ステップモードdefault 0x07 (8bit)L6480_setparam_alareen(val)

    //alarmen
    L6480_write(0x17);
    L6480_write(0xFF);
    //[R, WS]
    //有効アラームdefault 0xff (1+1+1+1+1+1+1+1bit)

    //gatecfg1
    L6480_write(0x18);
    L6480_write(0x00);
    L6480_write(0xF7);
    //[R, WH]
    //geta driver configuration default 0x000(1+3+3+5bit)

    //gatecfg2
    L6480_write(0x19);
    L6480_write(0x89);
    //[R, WH]
    //geta driver configuration default 0x00(3+5bit)

    //config
    L6480_write(0x1A);
    L6480_write(0x5E);
    L6480_write(0x81);
    //[R, WH]
    //各種設定default 0x2c88 (3+3+1+1+1+1+1+1+1+3bit)  
}

void L6480_run(int dia, long spd)
{
    if(dia == 1)
    {
        L6480_transfer(0x51, 3, spd);
    }
    else
    {
        L6480_transfer(0x50, 3, spd);
    }
}

void L6480_move(int dia, long n_step)
{
    if(dia == 1)
    {
        L6480_transfer(0x41, 3, n_step / 1.8);
    }  
    else
    {
        L6480_transfer(0x40, 3, n_step / 1.8);
    }
}

void L6480_goto(long pos)
{
    L6480_transfer(0x60, 3, pos);
}

void L6480_gotodia(int dia, int pos)
{
    if(dia == 1)
    {    
        L6480_transfer(0x69 ,3 ,pos);
    }
    else
    {    
        L6480_transfer(0x68, 3, pos);
    }
}

void L6480_gountil(int act, int dia, long spd)
{
    if(act == 1)
    {
        if(dia == 1)
        {
            L6480_transfer(0x8b, 3, spd);
        }
        else
        {
            L6480_transfer(0x8a, 3, spd);
        }
    }
    else
    {
        if(dia == 1)
        {
            L6480_transfer(0x83, 3, spd);
        }
        else
        {
            L6480_transfer(0x82, 3, spd);
        }
    }
}

void L6480_relesesw(int act, int dia)
{
    if(act == 1)
    {
        if(dia == 1)
        {
            L6480_transfer(0x9b, 0, 0);
        }
        else
        {
            L6480_transfer(0x9a, 0, 0);
        }
    }
    else
    {
        if(dia == 1)
        {
            L6480_transfer(0x93, 0, 0);
        }
        else
        {
            L6480_transfer(0x92, 0, 0);
        }
    }
}

void L6480_softstop(void)
{
    L6480_transfer(0xb0, 0, 0);
}

void L6480_hardstop(void)
{
    L6480_transfer(0xb8, 0, 0);
}

void L6480_softhiz(void)
{
    L6480_transfer(0xa0, 0, 0);
}

void L6480_hardhiz(void)
{
    L6480_transfer(0xa8, 0, 0);
}

void L6480_gohome(void)
{
    L6480_transfer(0x70, 0, 0);
}

void L6480_gomark(void)
{
    L6480_transfer(0x78, 0, 0);
}

void L6480_resetpos(void)
{
    L6480_transfer(0xd8, 0, 0);
}

void L6480_transfer(int add, int bytes, long val)
{
    int data[3];
    int i;
    L6480_write(add);
    for(i = 0; i <= bytes-1; i++)
    {
        data[i] = val & 0xff;  
        val = val >> 8;
    }
    if(bytes == 3)
    {
        L6480_write(data[2]);
    }
    if(bytes >= 2)
    {
        L6480_write(data[1]);
    }
    if(bytes >= 1)
    {
        L6480_write(data[0]);
    }  
}




unsigned long L6480_getparam(unsigned char add, int bytes)
{
    unsigned char data = 0x00;
    long val;
    int i;
    unsigned char send_add = add | 0x20;
    L6480_write(send_add);
    for(i = 0; i < bytes; i++)
    {
        val = val << 8;
        val = val | L6480_read(data);
    }
    
    return val;
}

unsigned long L6480_getparam_abspos(void)
{
    return L6480_getparam(0x01, 3);
}

unsigned long L6480_getparam_elpos(void)
{
    return L6480_getparam(0x02, 2);
}

unsigned long L6480_getparam_speed(void)
{
    return L6480_getparam(0x04, 3);
}

void LCD_write(unsigned char rs, unsigned char data, int fd)
{
    unsigned char buf[2];
 
    if (rs == LCD_RS_CMD || rs == LCD_RS_DATA)
    {
        // LCD_RS_CMD ならコマンドモード。LCD_RS_DATA ならデータモード。
        
  buf[0] = rs;
  buf[1] = data;
  if (write(fd, buf, 2) != 2)
  {
   printf("Error writeing to i2c slave1\n");
  }        
    }
    else
    {
        // rsの指定がLCD_RS_CMD,LCD_RS_DATA以外ならなにもしない。
    }
}

void LCD_clear(int fd)
{
    LCD_write(LCD_RS_CMD, LCD_CMD_CLEAR, fd);
    delay(2);
    LCD_write(LCD_RS_CMD, LCD_CMD_HOME, fd);
    delay(2);
}

void LCD_setCursor(unsigned char col, unsigned char row, int fd)
{
    unsigned char offset[] = {0x00, 0x20};
    
    if (row > 1)    row = 1;
    if (col > 16)    col = 16;
    
    LCD_write(LCD_RS_CMD, 0x80 | (col + offset[row]), fd);
}

void LCD_putc(unsigned char c, int fd)
{
    LCD_write(LCD_RS_DATA, c, fd);
}

void LCD_puts(char *str, int fd)
{
    int i;
    for (i = 0; i < 16; i++)
    {
        if (str[i] == 0x00)
        {
            break;
        }
        else
        {
            LCD_putc((unsigned int)str[i], fd);
        }
    }
}

void LCD_ON(int fd)
{
    LCD_write(LCD_RS_CMD, 0x0C, fd);
}