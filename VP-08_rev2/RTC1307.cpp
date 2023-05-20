#include "RTC1307.h"

bool false_I2C;            // �� �������� ���� ������ I2C
struct DS1307 DS;          // ��������� ��������� � ������� ��� �����


void I2C_init (void)       // �������������� I2C
{
     TWBR=0x20;  // 12Khz
}

void I2C_StartCondition(void)       // �������� ���������� ����
{
     TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
     while(!(TWCR&(1<<TWINT)));//�������� ���� ����������� TWIN // ������ ����� �� ����� 
}

void I2C_StopCondition(void)        // �������� ���� ����
{
     TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);     // ��������� ���� � �������� ���, �������� ����, � ��������� I2C
}

void I2C_SendByte(uint8_t c)              // �������� �����
{
     TWDR = c;//������� ���� � ������� ������
     TWCR = (1<<TWINT)|(1<<TWEN);//������� �������� �����
     while (!(TWCR & (1<<TWINT)));//�������� ���� ����������� TWIN
}

void I2C_write_ds1338 (void)            // ��� ������ � ���� (����� ��������� ���)
{
     I2C_SendByte (0b11010000);
     if ((TWSR & 0xF8) != TW_MT_DATA_ASK) {false_I2C = true;}     // ��� ��� ����������� ������
}

void I2C_read_ds1338 (void)             // ��� ������ �� ����� (����� ��������� ���) ��� �� �������
{
     I2C_SendByte (0b11010001);
     if ((TWSR & 0xF8) != TW_MT_DATA_ASK) {false_I2C = true;}
}

uint8_t I2C_ReadByte(void)              // ��� ������ �� I2C
{
     TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);  // ��� ��������� ���������� ������������� ����� ��������� �����
     while (!(TWCR & (1<<TWINT)));//�������� ��������� ���� TWIN
     return TWDR;//������ ������� ������
}

uint8_t I2C_ReadLastByte(void)           // ��� ������ ���������� �����, 
{
     TWCR = (1<<TWINT)|(1<<TWEN);        // ��� �� ���������� ��� �������������
     while (!(TWCR & (1<<TWINT)));//�������� ��������� ���� TWIN
     return TWDR;//������ ������� ������
}

void DS_Write_time (void)                // ������� ������ ������� � �����
{
     I2C_StartCondition();               // �������� �����
     I2C_write_ds1338();                 // �������� ����� ����� � ��� ������
     I2C_SendByte (0x00);                // �������� ������� � �������� ������ ������
     I2C_SendByte (C_BinDec(DS.sec));    //������� + �������������� � �������� ��� �����
     I2C_SendByte (C_BinDec(DS.min));    //������
     I2C_SendByte (C_BinDec(DS.hour));   //����
     I2C_StopCondition();                // �������� ����
}

void DS_Write_date (void)                // ������� ������ ����
{
     I2C_StartCondition();               // �������� �����
     I2C_write_ds1338();                 // �������� ����� ����� � ��� ������
     I2C_SendByte (0x04);                // �������� ������� � �������� ����������
     I2C_SendByte (C_BinDec(DS.date));   //����
     I2C_SendByte (C_BinDec(DS.month));  //�����
     I2C_SendByte (C_BinDec(DS.year));   //���
     I2C_StopCondition();                // ����
}

void DS_Write_init_date (void)           // ��� ��������� ������� ����
{
     DS_Read ();
     if (DS.sec == 80) {                 // ���� ���� �������� ��������� �� ��� ������ �������, ������ �������� �������������
          I2C_StartCondition();          // �����
          I2C_write_ds1338();            // ����� ����� � ��� ������
          I2C_SendByte (0x00);           // ����� � �������� �������� ������
          I2C_SendByte (C_BinDec(10));   //�������
          I2C_SendByte (C_BinDec(10));   //������
          I2C_SendByte (C_BinDec(10));   //����
          I2C_SendByte (C_BinDec(1));    //���
          I2C_SendByte (C_BinDec(01));   //����
          I2C_SendByte (C_BinDec(01));   //�����
          I2C_SendByte (C_BinDec(90));   //��� 90 ��� �������� ������������
          I2C_StopCondition();           // ������� ����
     }
}

void DS_Read(void)                       // ������ �� ����� �����
{
     I2C_StartCondition();               // �������� �����
     I2C_write_ds1338();                 // �������� ����� ����� � ��� ������
     I2C_SendByte (0x00);                // �������� ����� � �������� ������
     I2C_StopCondition();                // �������� ����
     I2C_StartCondition();               // �������� �����    // ��� ����� ����� ���������� ��������� ����� ��
     I2C_read_ds1338();                  // �������� ����� ����� � ��� ������
     DS.sec = I2C_ReadByte(); DS.min = I2C_ReadByte(); DS.hour = I2C_ReadByte();    // ��������� ��� �����
     DS.day = I2C_ReadByte(); DS.date = I2C_ReadByte(); DS.month = I2C_ReadByte();
     DS.year = I2C_ReadLastByte();       // �� ������� ������ ��������� ����
     I2C_StopCondition();                // ����
     //����������� � ���������� ������
     DS.sec = C_Dec(DS.sec); DS.min = C_Dec(DS.min);
     DS.hour &= ~0xC0;
     DS.hour = C_Dec(DS.hour);
     DS.year = C_Dec(DS.year); DS.month = C_Dec(DS.month);
     DS.date = C_Dec(DS.date);
}

uint8_t C_Dec(uint8_t data)      // ������� ��� �������������� ������ �� ���������� � ���������� �������
{
     return((data>>4)*10+(0x0F&data));
}

uint8_t C_BinDec(uint8_t data)   // ������� ��� �������������� �� ���������� �������� � ������������������ ���������
{
     return((data/10)<<4)|(data%10);
}