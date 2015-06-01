/***********************************************************
 *  ファイル名： main.c
 * 　作成日時： 2015/05/29
 * 　　　機能： I2C通信のサンプルです。dsPIC30F6014Aを使用。
 ***********************************************************/

/***********************************************************
 * ヘッダファイル
 ***********************************************************/
#include <xc.h>

/***********************************************************
 * コンフィギュレーション設定
 ***********************************************************/
/* OSC（クロック設定）*/
#pragma config FOSFPR = FRC             /* 8[MHz]の内部発信器を使用 */
#pragma config FCKSMEN = CSW_FSCM_OFF   /* クロック切り替えを無効 */

/* FWDT（ウォッチドッグ・タイマ設定）*/
#pragma config FWPSB = WDTPSB_16        /* プリスケーラBの設定（デフォルト） */
#pragma config FWPSA = WDTPSA_512       /* プリスケーラAの設定 （デフォルト）*/
#pragma config WDT = WDT_OFF            /* ウォッチドッグ・タイマを無効 */

/* FBORPOR（ブラウン・アウト・リセット、パワー・オン・リセット設定）*/
#pragma config FPWRT = PWRT_64          /* 64[ms]のパルス幅でリセット*/
#pragma config BODENV = BORV_27         /* 2.7[V]以下になったらリセット*/
#pragma config BOREN = PBOR_ON          /* 自動リセットを有効*/
#pragma config MCLRE = MCLR_EN          /* MCLRピンを有効*/

/* FBS（ブート設定？デフォルト値使用）*/
#pragma config BWRP = WR_PROTECT_BOOT_OFF// Boot Segment Program Memory Write Protect (Boot Segment Program Memory may be written)
#pragma config BSS = NO_BOOT_CODE       // Boot Segment Program Flash Memory Code Protection (No Boot Segment)
#pragma config EBS = NO_BOOT_EEPROM     // Boot Segment Data EEPROM Protection (No Boot EEPROM)
#pragma config RBS = NO_BOOT_RAM        // Boot Segment Data RAM Protection (No Boot RAM)

/* FSS（セキュリティ設定？デフォルト値使用）*/
#pragma config SWRP = WR_PROT_SEC_OFF   // Secure Segment Program Write Protect (Disabled)
#pragma config SSS = NO_SEC_CODE        // Secure Segment Program Flash Memory Code Protection (No Secure Segment)
#pragma config ESS = NO_SEC_EEPROM      // Secure Segment Data EEPROM Protection (No Segment Data EEPROM)
#pragma config RSS = NO_SEC_RAM         // Secure Segment Data RAM Protection (No Secure RAM)

/* FGS（コードプロテクト設定）*/
#pragma config GWRP = GWRP_OFF          /* コード書き込み保護無効 */
#pragma config GCP = GSS_OFF            /* コード保護無効 */

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

/***********************************************************
 * 関数プロトタイプ
 ***********************************************************/
void            I2C_init(void);
void            I2C_start(void);
void            I2C_stop(void);
void            I2C_send(unsigned char data);
unsigned char   I2C_read(void);
void            I2C_send_ACK(unsigned char ack);
void            I2C_send_char(unsigned char address, unsigned char data);
void            I2C_send_int(unsigned char address, unsigned int data);
unsigned char   I2C_read_char(unsigned char address);
unsigned int    I2C_read_int(unsigned char address);

/***********************************************************
 * メイン関数
 ***********************************************************/
int main(void){
    unsigned int i;
    //unsigned char c;
    I2C_init();
    I2CBRG = 0x01;
    I2CADD = 0x09;
    i = I2C_read_int(0x04);
    //c = I2C_read_char(0x04);
    //I2C_send_char(0x04, c);
    //I2C_send_int(0x04, 1234);
    while(1){
    }

    return 0;
}

void I2C_init(void){
    I2CCONbits.I2CEN = 1;           /* I2C通信を有効 */
    I2CCONbits.A10M = 0;            /* 7bitアドレスモード */

}

void I2C_start(void){
    I2CCONbits.SEN = 1;
    while (I2CCONbits.SEN);         /* スタートシーケンス終了まで待つ */
}

void I2C_stop(void){
    I2CCONbits.PEN = 1;
    while (I2CCONbits.PEN);         /* ストップシーケンス終了まで待つ */
}

void I2C_send(unsigned char data){
    I2CTRN = data;
    while (I2CSTATbits.TRSTAT);     /* 送信終了まで待つ */
    while (I2CSTATbits.ACKSTAT);    /* スレーブ側からのアクノリッジを確認 */
}

unsigned char I2C_read(void){
    unsigned char temp;
    I2CCONbits.RCEN = 1;            /* 受信有効化 */
    while (I2CCONbits.RCEN);        /* データ受信完了まで待つ */
    temp = I2CRCV;
    return temp;
}

void I2C_send_char(unsigned char address, unsigned char data){
    I2C_start();
    address = address << 1;         /* 0ビット目をR/Wビットにするためシフト*/
    address &= 0b11111110;          /* 0ビット目をWに */
    I2C_send(address);              /* スレーブアドレスを送信 */
    I2C_send(data);
    I2C_stop();
}

void I2C_send_int(unsigned char address, unsigned int data){
    I2C_start();
    address = address << 1;
    address &= 0b11111110;          /* 0ビット目をクリアしてWに */
    I2C_send(address);              /* スレーブアドレスを送信 */

    I2C_send((unsigned char)(data >> 8));    /* 上位8ビット送信 */
    I2C_send((unsigned char)(0xFF & data)); /* 下位8ビット送信 */
    I2C_stop();
}

unsigned char I2C_read_char(unsigned char address){
    unsigned char data;
    I2C_start();

    address = address << 1;         /* 0ビット目をR/Wビットにするためシフト*/
    address |= 0b00000001;          /* 0ビット目をRに */
    I2C_send(address);              /* スレーブアドレスを送信 */

    data = I2C_read();
    I2C_send_ACK(1);                /* NACK  */
    I2C_stop();
    return data;
}

unsigned int I2C_read_int(unsigned char address){
    unsigned char temp1, temp2;
    I2C_start();

    address = address << 1;         /* 0ビット目をR/Wビットにするためシフト*/
    address |= 0b00000001;          /* 0ビット目をRに */
    I2C_send(address);              /* スレーブアドレスを送信 */

    temp1 = I2C_read();
    I2C_send_ACK(0);                     /* ACK */

    temp2 = I2C_read();
    I2C_send_ACK(1);                     /* NACK */

    I2C_stop();
    return (temp1 << 8) | (0xFF & temp2);
}

void I2C_send_ACK(unsigned char ack){
    I2CCONbits.ACKDT = ack;
    I2CCONbits.ACKEN = 1;           /* 応答シーケンス開始 */
    while (I2CCONbits.ACKEN);       /* 応答シーケンス終了まで待つ*/
}
