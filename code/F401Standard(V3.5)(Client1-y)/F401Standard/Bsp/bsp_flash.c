#include "bsp_flash.h"
#include "main.h"
#include "string.h"
#include "appMenu.h"

//STM32F401CCU6 FALSH SIZE = 256K
//The last sector is sctor5 which address is from 0x08020000 to 0x0803FFFF(size: 128k)


/****^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^�÷�ʾ����^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#define USER_FLASH_ADDRESS  ADDR_FLASH_SECTOR_11
#define FLASH_DATA_LENGHT   12

uint8_t after_erase_data[FLASH_DATA_LENGHT];
uint8_t write_data[FLASH_DATA_LENGHT] = "RoboMaster\r\n";
uint8_t after_write_data[FLASH_DATA_LENGHT];

    //erase flash page
    //����flashҳ
    flash_erase_address(USER_FLASH_ADDRESS, 1);
    //read data from flash, before writing data
    //��д����֮ǰ,��flash��ȡ����
    flash_read(USER_FLASH_ADDRESS, (uint32_t *)after_erase_data, (FLASH_DATA_LENGHT + 3) / 4);
    //write data to flash
    //��flashд����
    flash_write_single_address(USER_FLASH_ADDRESS, (uint32_t *)write_data, (FLASH_DATA_LENGHT + 3) / 4);
    //read data from flash, after writing data
    //��д����֮��,��flash��ȡ����
    flash_read(USER_FLASH_ADDRESS, (uint32_t *)after_write_data, (FLASH_DATA_LENGHT + 3) / 4);
***^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
static uint32_t ger_sector(uint32_t address);
extern int8_t DATA_OK;
extern float Configs0[LINE_NUM];	//��������
 uint16_t after_erase_data[FLASH_DATA_LENGHT];
 uint16_t write_data[FLASH_DATA_LENGHT];
 uint16_t after_write_data[FLASH_DATA_LENGHT];

/**
  * @brief          erase flash
  * @param[in]      address: flash address
  * @param[in]      len: page num
  * @retval         none
  */
/**
  * @brief          ����flash
  * @param[in]      address: flash ��ַ
  * @param[in]      len: ҳ����
  * @retval         none
  */
void flash_erase_address(uint32_t address, uint16_t len)
{
    FLASH_EraseInitTypeDef flash_erase;
    uint32_t error;

    flash_erase.Sector = ger_sector(address);
    flash_erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    flash_erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    flash_erase.NbSectors = len;

    HAL_FLASH_Unlock();
    HAL_FLASHEx_Erase(&flash_erase, &error);
    HAL_FLASH_Lock();
}

/**
  * @brief          write data to one page of flash
  * @param[in]      start_address: flash address
  * @param[in]      buf: data point
  * @param[in]      len: data num
  * @retval         success 0, fail -1
  */
/**
  * @brief          ��һҳflashд����
  * @param[in]      start_address: flash ��ַ
  * @param[in]      buf: ����ָ��
  * @param[in]      len: ���ݳ���
  * @retval         success 0, fail -1
  */
int8_t flash_write_single_address(uint32_t start_address, uint32_t *buf, uint32_t len)
{
    static uint32_t uw_address;
    static uint32_t end_address;
    static uint32_t *data_buf;
    static uint32_t data_len;

    HAL_FLASH_Unlock();
    
    uw_address = start_address;
    end_address = get_next_flash_address(start_address);
    data_buf = buf;
    data_len = 0;

    while (uw_address <= end_address)
    {

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,uw_address, *data_buf) == HAL_OK)
        {
            uw_address += 4;
            data_buf++;
            data_len++;
            if (data_len == len)
            {
                break;
            }
        }
        else
        {
            HAL_FLASH_Lock();
            return -1;
        }
    }
    HAL_FLASH_Lock();
    return 0;
}

/**
  * @brief          write data to some pages of flash
  * @param[in]      start_address: flash start address
  * @param[in]      end_address: flash end address
  * @param[in]      buf: data point
  * @param[in]      len: data num
  * @retval         success 0, fail -1
  */
/**
  * @brief          ����ҳflashд����
  * @param[in]      start_address: flash ��ʼ��ַ
  * @param[in]      end_address: flash ������ַ
  * @param[in]      buf: ����ָ��
  * @param[in]      len: ���ݳ���
  * @retval         success 0, fail -1
  */
int8_t flash_write_muli_address(uint32_t start_address, uint32_t end_address, uint32_t *buf, uint32_t len)
{
    uint32_t uw_address = 0;
    uint32_t *data_buf;
    uint32_t data_len;

    HAL_FLASH_Unlock();

    uw_address = start_address;
    data_buf = buf;
    data_len = 0;
    while (uw_address <= end_address)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,uw_address, *data_buf) == HAL_OK)
        {
            uw_address += 4;
            data_buf++;
            data_len++;
            if (data_len == len)
            {
                break;
            }
        }
        else
        {
            HAL_FLASH_Lock();
            return -1;
        }
    }

    HAL_FLASH_Lock(); 
    return 0;
}


/**
  * @brief          read data for flash
  * @param[in]      address: flash address
  * @param[out]     buf: data point
  * @param[in]      len: data num
  * @retval         none
  */
/**
  * @brief          ��flash������
  * @param[in]      start_address: flash ��ַ
  * @param[out]     buf: ����ָ��
  * @param[in]      len: ���ݳ���
  * @retval         none
  */
void flash_read(uint32_t address, uint32_t *buf, uint32_t len)
{
    memcpy(buf, (void*)address, len *4);
}
//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead)   	
{
	uint16_t i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}
//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
	return *(__IO uint16_t*)faddr; 
}
/**
  * @brief          get the sector number of flash
  * @param[in]      address: flash address
  * @retval         sector number
  */
/**
  * @brief          ��ȡflash��sector��
  * @param[in]      address: flash ��ַ
  * @retval         sector��
  */
uint32_t ger_sector(uint32_t address)
{
    uint32_t sector = 0;
    if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_SECTOR_0;
    }
    else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_SECTOR_1;
    }
    else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_SECTOR_2;
    }
    else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_SECTOR_3;
    }
    else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_SECTOR_4;
    }
    else
    {
        sector = FLASH_SECTOR_5;
    }

//    else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6))
//    {
//        sector = FLASH_SECTOR_6;
//    }
//    else if ((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7))
//    {
//        sector = FLASH_SECTOR_7;
//    }
//    else if ((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8))
//    {
//        sector = FLASH_SECTOR_8;
//    }
//    else if ((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9))
//    {
//        sector = FLASH_SECTOR_9;
//    }
//    else if ((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10))
//    {
//        sector = FLASH_SECTOR_10;
//    }
//    else if ((address < ADDR_FLASH_SECTOR_12) && (address >= ADDR_FLASH_SECTOR_11))
//    {
//        sector = FLASH_SECTOR_11;
//    }
//    else
//    {
//        sector = FLASH_SECTOR_11;
//    }

    return sector;
}

/**
  * @brief          get the next page flash address
  * @param[in]      address: flash address
  * @retval         next page flash address
  */
/**
  * @brief          ��ȡ��һҳflash��ַ
  * @param[in]      address: flash ��ַ
  * @retval         ��һҳflash��ַ
  */
uint32_t get_next_flash_address(uint32_t address)
{
    uint32_t sector = 0;

    if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0))
    {
        sector = ADDR_FLASH_SECTOR_1;
    }
    else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1))
    {
        sector = ADDR_FLASH_SECTOR_2;
    }
    else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2))
    {
        sector = ADDR_FLASH_SECTOR_3;
    }
    else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3))
    {
        sector = ADDR_FLASH_SECTOR_4;
    }
    else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4))
    {
        sector = ADDR_FLASH_SECTOR_5;
    }
    else if ((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5))
    {
        sector = ADDR_FLASH_SECTOR_6;
    }
    else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6))
    {
        sector = ADDR_FLASH_SECTOR_7;
    }
    else if ((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7))
    {
        sector = ADDR_FLASH_SECTOR_8;
    }
    else if ((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8))
    {
        sector = ADDR_FLASH_SECTOR_9;
    }
    else if ((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9))
    {
        sector = ADDR_FLASH_SECTOR_10;
    }
    else if ((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10))
    {
        sector = ADDR_FLASH_SECTOR_11;
    }
    else /*(address < FLASH_END_ADDR) && (address >= ADDR_FLASH_SECTOR_23))*/
    {
        sector = FLASH_END_ADDR;
    }
    return sector;
}
