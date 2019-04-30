
//================================================
// ACT8937 PMIC/BatteryCharger Driver
// Written by Evan Tan <hongfei.tang@samsung.com>
// 2010-12-13
//================================================

#include <common.h>
#include <regs.h>
#include <asm/io.h>

#include "bitmap.c"

#if  (CONFIG_T34_H)
/*T34 H 
   GPJ3_2:PMIC SCL, 
   GPJ3_3 PMIC_SDA
*/
#define s5p_iic0_scl_set(val) \
	writel((readl(ELFIN_GPIO_BASE+GPJ3DAT_OFFSET)&(~(1<<2)))|((val?1:0)<<2), ELFIN_GPIO_BASE+GPJ3DAT_OFFSET)

#define s5p_iic0_sda_set(val) \
	writel((readl(ELFIN_GPIO_BASE+GPJ3DAT_OFFSET)&(~(1<<3)))|((val?1:0)<<3), ELFIN_GPIO_BASE+GPJ3DAT_OFFSET)

#define s5p_iic0_sda_get(val) \
	((readl(ELFIN_GPIO_BASE+GPJ3DAT_OFFSET)&0x8) >> 3)

typedef enum {
	INPUT = 0,
	OUTPUT
} io_dir;

static void s5p_iic0_sda_as(io_dir d)
{
	writel((readl(ELFIN_GPIO_BASE+GPJ3CON_OFFSET)&(~0xF000))|((d<<12)&(0x1<<12)), ELFIN_GPIO_BASE+GPJ3CON_OFFSET);
}

static void s5p_iic0_init()
{
	writel((readl(ELFIN_GPIO_BASE+GPJ3CON_OFFSET)&(~0xFF00))|0x1100, ELFIN_GPIO_BASE+GPJ3CON_OFFSET);
	//writel((readl(ELFIN_GPIO_BASE+GPJ0PUD_OFFSET)&(~0xF0))|0xA0, ELFIN_GPIO_BASE+GPJ0PUD_OFFSET); 
	writel((readl(ELFIN_GPIO_BASE+GPJ3PUD_OFFSET)&(~0xF0)), ELFIN_GPIO_BASE+GPJ3PUD_OFFSET); 
	writel((readl(ELFIN_GPIO_BASE+GPJ3DRV_SR_OFFSET)|0xF0), ELFIN_GPIO_BASE+GPJ3DRV_SR_OFFSET); 
	writel((readl(ELFIN_GPIO_BASE+GPJ3CONPDN_OFFSET)&(~0xF0))|0x50, ELFIN_GPIO_BASE+GPJ3CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPJ3PUDPDN_OFFSET)&(~0xF0), ELFIN_GPIO_BASE+GPJ3PUDPDN_OFFSET);
	s5p_iic0_sda_set(1);
	s5p_iic0_scl_set(1);
	udelay(5);
}

#elif (CONFIG_T34_PVT)
/*T34 V3.0 
   GPJ0_0:PMIC SCL, 
   GPJ0_1 PMIC_SDA
*/
#define s5p_iic0_scl_set(val) \
	writel((readl(ELFIN_GPIO_BASE+GPJ0DAT_OFFSET)&(~(1<<0)))|((val?1:0)<<0), ELFIN_GPIO_BASE+GPJ0DAT_OFFSET)

#define s5p_iic0_sda_set(val) \
	writel((readl(ELFIN_GPIO_BASE+GPJ0DAT_OFFSET)&(~(1<<1)))|((val?1:0)<<1), ELFIN_GPIO_BASE+GPJ0DAT_OFFSET)

#define s5p_iic0_sda_get(val) \
	((readl(ELFIN_GPIO_BASE+GPJ0DAT_OFFSET)&0x2) >> 1)

typedef enum {
	INPUT = 0,
	OUTPUT
} io_dir;

static void s5p_iic0_sda_as(io_dir d)
{
	writel((readl(ELFIN_GPIO_BASE+GPJ0CON_OFFSET)&(~0xF0))|((d<<4)&(0x1<<4)), ELFIN_GPIO_BASE+GPJ0CON_OFFSET);
}

static void s5p_iic0_init()
{
	writel((readl(ELFIN_GPIO_BASE+GPJ0CON_OFFSET)&(~0xFF))|0x11, ELFIN_GPIO_BASE+GPJ0CON_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPJ0PUD_OFFSET)&(~0xF))|0xA, ELFIN_GPIO_BASE+GPJ0PUD_OFFSET); // pull up since pull resiter is absent
	writel((readl(ELFIN_GPIO_BASE+GPJ0DRV_SR_OFFSET)|0xF), ELFIN_GPIO_BASE+GPJ0DRV_SR_OFFSET); //add drive str
	writel((readl(ELFIN_GPIO_BASE+GPJ0CONPDN_OFFSET)&(~0xF))|0x5, ELFIN_GPIO_BASE+GPJ0CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPJ0PUDPDN_OFFSET)&(~0xF), ELFIN_GPIO_BASE+GPJ0PUDPDN_OFFSET);
	s5p_iic0_sda_set(1);
	s5p_iic0_scl_set(1);
	udelay(5);
}
#else
#define s5p_iic0_scl_set(val) \
	writel((readl(ELFIN_GPIO_BASE+GPD1DAT_OFFSET)&(~(1<<1)))|((val?1:0)<<1), ELFIN_GPIO_BASE+GPD1DAT_OFFSET)

#define s5p_iic0_sda_set(val) \
	writel((readl(ELFIN_GPIO_BASE+GPD1DAT_OFFSET)&(~(1<<0)))|((val?1:0)<<0), ELFIN_GPIO_BASE+GPD1DAT_OFFSET)

#define s5p_iic0_sda_get(val) \
	(readl(ELFIN_GPIO_BASE+GPD1DAT_OFFSET)&0x1)

typedef enum {
	INPUT = 0,
	OUTPUT
} io_dir;

static void s5p_iic0_sda_as(io_dir d)
{
	writel((readl(ELFIN_GPIO_BASE+GPD1CON_OFFSET)&(~0xF))|(d&(0x1)), ELFIN_GPIO_BASE+GPD1CON_OFFSET);
}

static void s5p_iic0_init()
{
	writel((readl(ELFIN_GPIO_BASE+GPD1CON_OFFSET)&(~0xFF))|0x11, ELFIN_GPIO_BASE+GPD1CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPD1PUD_OFFSET)&(~0xF), ELFIN_GPIO_BASE+GPD1PUD_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPD1CONPDN_OFFSET)&(~0xF))|0x5, ELFIN_GPIO_BASE+GPD1CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPD1PUDPDN_OFFSET)&(~0xF), ELFIN_GPIO_BASE+GPD1PUDPDN_OFFSET);
	s5p_iic0_sda_set(1);
	s5p_iic0_scl_set(1);
	udelay(5);
}
#endif

static void s5p_iic0_start()
{
	s5p_iic0_sda_set(1);
	s5p_iic0_scl_set(1);
	udelay(5);
	s5p_iic0_sda_set(0);
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
}

static void s5p_iic0_stop()
{
	s5p_iic0_scl_set(0);
	s5p_iic0_sda_set(0);
	udelay(5);
	s5p_iic0_scl_set(1);
	udelay(10);
	s5p_iic0_sda_set(1);
	udelay(10);
}

static void s5p_iic0_send_ack()
{
	s5p_iic0_sda_set(0);
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
	s5p_iic0_scl_set(1);
	udelay(30);
	s5p_iic0_scl_set(0);
	udelay(20);
}

static int s5p_iic0_rcv_ack()
{
	int i, ret=0;

	s5p_iic0_sda_as(INPUT);
	s5p_iic0_scl_set(0);
	udelay(20);
	s5p_iic0_scl_set(1);
	udelay(20);
	for (i=0; i<5; i++)
	{
		if(!s5p_iic0_sda_get())
		{
			ret = 1;
			break;
		}
	}
	udelay(10);
	s5p_iic0_scl_set(0);
	udelay(10);
	s5p_iic0_sda_set(1);
	s5p_iic0_sda_as(OUTPUT);

	return ret;
}

static int s5p_iic0_send_byte(unsigned char val)
{
	int i;

	for (i=0; i<8; i++)
	{
		s5p_iic0_scl_set(0);
		udelay(10);
		s5p_iic0_sda_set((val&(0x80>>i))?1:0);
		udelay(10);
		s5p_iic0_scl_set(1);
		udelay(30);
		s5p_iic0_scl_set(0);
		udelay(10);
	}

	return s5p_iic0_rcv_ack();
}

static unsigned char s5p_iic0_rcv_byte()
{
	int i;
	unsigned char rdata = 0;

	s5p_iic0_sda_as(INPUT);
	for(i=0; i<8; i++)
	{
		s5p_iic0_scl_set(0);
		udelay(10);
		s5p_iic0_scl_set(1);
		udelay(20);
		rdata |= (s5p_iic0_sda_get()<<(7-i));
		udelay(10);
		s5p_iic0_scl_set(0);
		udelay(20);
	}
	s5p_iic0_sda_as(OUTPUT);

	return rdata;
}

static int s5p_iic0_write(unsigned char chip, unsigned char reg, unsigned char val)
{
	s5p_iic0_start();
	if(!s5p_iic0_send_byte(chip))
		return -1;
	if(!s5p_iic0_send_byte(reg))
		return -1;
	if(!s5p_iic0_send_byte(val))
		return -1;
	s5p_iic0_stop();

	return 0;
}

static int s5p_iic0_read(unsigned char chip, unsigned char reg)
{
	unsigned char val;

	s5p_iic0_start();
	if(!s5p_iic0_send_byte(chip)) return -1;
	if(!s5p_iic0_send_byte(reg)) return -1;
	s5p_iic0_stop();
	s5p_iic0_start();
	if(!s5p_iic0_send_byte(chip+1)) return -1;
	val =s5p_iic0_rcv_byte();
	s5p_iic0_send_ack();
	s5p_iic0_stop();

	return (int)val;
}

static void s5pv210_platform_enter_io_sleep()
{
	writel(0xAAAA, GPA0CONPDN);
	writel(0x555, GPA0PUDPDN);

	writel(0xAA, GPA1CONPDN);
	writel(0x55, GPA1PUDPDN);
	
	writel(0xAAAA, GPBCONPDN);
	writel(0x5550, GPBPUDPDN);

	writel(0xABAA, GPC0CONPDN); // GPC0_4 keeps previous state
	writel(0x5440, GPC0PUDPDN);

	writel(0x2AB, GPC1CONPDN); // GPC1_0 keeps previous state
	writel(0x154, GPC1PUDPDN);

	writel(0xAA, GPD0CONPDN);
	writel(0x55, GPD0PUDPDN);

	writel(0xAAAA, GPD1CONPDN);
	writel(0x5000, GPD1PUDPDN);

	writel(0xAAAA, GPE0CONPDN);
	writel(0x5555, GPE0PUDPDN);

	writel(0x2AA, GPE1CONPDN);
	writel(0x155, GPE1PUDPDN);

	writel(0xAAAA, GPF0CONPDN);
	writel(0x5555, GPF0PUDPDN);

	writel(0xAAAA, GPF1CONPDN);
	writel(0x5555, GPF1PUDPDN);

	writel(0xAAAA, GPF2CONPDN);
	writel(0x5555, GPF2PUDPDN);

	writel(0xAAA, GPF3CONPDN);
	writel(0x555, GPF3PUDPDN);

	writel(0x2AA6, GPG0CONPDN);
	writel(0x1551, GPG0PUDPDN);

	writel(0x2A0A, GPG1CONPDN);
	writel(0x1505, GPG1PUDPDN);

	writel(0x2AAA, GPG2CONPDN);
	writel(0x1541, GPG2PUDPDN);

	writel(0x2AAA, GPG3CONPDN);
	writel(0x1551, GPG3PUDPDN);

	writel(0x2AAA, GPIPUDPDN-0x4 /*GPICONPDN*/);
	writel(0x1555, GPIPUDPDN);

	writel(0xAAAA, GPJ0CONPDN);
	writel(0x5555, GPJ0PUDPDN);

	writel(0xAAA, GPJ1CONPDN);
	writel(0x555, GPJ1PUDPDN);

	writel(0xAAAA, GPJ2CONPDN);
	writel(0x5555, GPJ2PUDPDN);

	writel(0xAAAA, GPJ3CONPDN);
	writel(0x5555, GPJ3PUDPDN);
	
	writel(0x2AA, GPJ4CONPDN);
	writel(0x155, GPJ4PUDPDN);

	writel(0xAAAA, MP01CONPDN);
	writel(0x5555, MP01PUDPDN);

	writel(0xAAAA, MP02CONPDN);
	writel(0x5555, MP02PUDPDN);

	writel(0xAAAA, MP03CONPDN);
	writel(0x5555, MP03PUDPDN);

	writel(0xAAAA, MP04CONPDN);
	writel(0x5555, MP04PUDPDN);

	writel(0xAAAA, MP05CONPDN);
	writel(0x5555, MP05PUDPDN);

	writel(0xAAAA, MP06CONPDN);
	writel(0x5555, MP06PUDPDN);

	writel(0xAAAA, MP07CONPDN);
	writel(0x5555, MP07PUDPDN);

	writel(0xAAAA, MP10CONPDN);
	writel(0x5555, MP10PUDPDN);

	writel(0xAAAA, MP11CONPDN);
	writel(0x5555, MP11PUDPDN);
	
	writel(0xAAAA, MP12CONPDN);
	writel(0x5555, MP12PUDPDN);
	
	writel(0xAAAA, MP13CONPDN);
	writel(0x5555, MP13PUDPDN);
	
	writel(0xAAAA, MP14CONPDN);
	writel(0x5555, MP14PUDPDN);

	writel(0xAAAA, MP15CONPDN);
	writel(0x5555, MP15PUDPDN);

	writel(0xAAAA, MP16CONPDN);
	writel(0x5555, MP16PUDPDN);

	writel(0xAAAA, MP17CONPDN);
	writel(0x5555, MP17PUDPDN);

	writel(0xAAAA, MP20CONPDN);
	writel(0x5555, MP20PUDPDN);

	writel(0xAAAA, MP21CONPDN);
	writel(0x5555, MP21PUDPDN);
	
	writel(0xAAAA, MP22CONPDN);
	writel(0x5555, MP22PUDPDN);
	
	writel(0xAAAA, MP23CONPDN);
	writel(0x5555, MP23PUDPDN);
	
	writel(0xAAAA, MP24CONPDN);
	writel(0x5555, MP24PUDPDN);

	writel(0xAAAA, MP25CONPDN);
	writel(0x5555, MP25PUDPDN);

	writel(0xAAAA, MP26CONPDN);
	writel(0x5555, MP26PUDPDN);

	writel(0xAAAA, MP27CONPDN);
	writel(0x5555, MP27PUDPDN);
	
	writel(0xAAAA, MP28CONPDN);
	writel(0x5555, MP28PUDPDN);
}

static void zzz(unsigned long val)
{
	printf("z...zZ...ZZZ...\n");
	if (val&0xFF)
	{
		writel((readl(ELFIN_GPIO_BASE+GPD0CON_OFFSET)&(~0xFF0))|0x110, ELFIN_GPIO_BASE+GPD0CON_OFFSET);
		writel(readl(ELFIN_GPIO_BASE+GPD0PUD_OFFSET)&(~0x3C), ELFIN_GPIO_BASE+GPD0PUD_OFFSET);
		writel(readl(ELFIN_GPIO_BASE+GPD0DAT_OFFSET)&(~0x6), ELFIN_GPIO_BASE+GPD0DAT_OFFSET);
		udelay(100000);
		/*still keeps acin and chglev original status when entering sleep mode*/
		//writel(readl(ELFIN_GPIO_BASE+GPC0DAT_OFFSET)&(~(1<<4)), ELFIN_GPIO_BASE+GPC0DAT_OFFSET);
		//writel(readl(ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET);
		//writel(readl(ELFIN_GPIO_BASE+GPC1DAT_OFFSET)&(~(1<<0)), ELFIN_GPIO_BASE+GPC1DAT_OFFSET);
		//writel(readl(ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET);
	}
	s5pv210_platform_enter_io_sleep();
	s5p_iic0_write(0xB6, 0x61, s5p_iic0_read(0xB6, 0x61)&(~0x80));
	s5p_iic0_write(0xB6, 0x65, s5p_iic0_read(0xB6, 0x65)&(~0x80));
#if (CONFIG_T34_PVT) || (CONFIG_T34_H)
	writel((readl(ELFIN_GPIO_BASE+GPJ3CONPDN_OFFSET)&(~0xF0))|0x5, ELFIN_GPIO_BASE+GPJ3CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPJ3PUDPDN_OFFSET)&(~0xF0), ELFIN_GPIO_BASE+GPJ3PUDPDN_OFFSET);
#else
	writel((readl(ELFIN_GPIO_BASE+GPD1CONPDN_OFFSET)&(~0xF))|0x5, ELFIN_GPIO_BASE+GPD1CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPD1PUDPDN_OFFSET)&(~0xF), ELFIN_GPIO_BASE+GPD1PUDPDN_OFFSET);
#endif
	writel(0xFFAADDEE, INF_REG_BASE+INF_REG0_OFFSET);
	writel(val, INF_REG_BASE+INF_REG1_OFFSET);
	writel(readl(SLEEP_CFG)&(~0x03), SLEEP_CFG);
	writel((readl(PWR_CFG)&(~(3 << 8)))|(3 << 8), PWR_CFG);
	writel(readl(OTHERS)|(0x01), OTHERS);
	writel(0xFFFFFFFF, ELFIN_VIC0_BASE_ADDR+0x14);
	writel(0xFFFFFFFF, ELFIN_VIC1_BASE_ADDR+0x14);
	writel(0xFFFFFFFF, ELFIN_VIC2_BASE_ADDR+0x14);
	writel(0xFFFFFFFF, ELFIN_VIC3_BASE_ADDR+0x14);
	writel(0xFFFFFFFF, ELFIN_VIC0_BASE_ADDR+0x1C);
	writel(0xFFFFFFFF, ELFIN_VIC1_BASE_ADDR+0x1C);
	writel(0xFFFFFFFF, ELFIN_VIC2_BASE_ADDR+0x1C);
	writel(0xFFFFFFFF, ELFIN_VIC3_BASE_ADDR+0x1C);
	writel(0xFFFFFFFF, ELFIN_CLOCK_POWER_BASE+EINT_WAKEUP_MASK_OFFSET);
	writel(0xFFFFFFFF, ELFIN_CLOCK_POWER_BASE+WAKEUP_MASK_OFFSET);
	writel(readl(WAKEUP_STAT), WAKEUP_STAT);
	writel(((0x5<<12)|(0x1<<9)|(0x1<<8)|(0x1<<0)), ELFIN_CLOCK_POWER_BASE+PS_HOLD_CONTROL_OFFSET);

	writel((readl(ELFIN_GPIO_BASE+GPH2CON_OFFSET)&(~0xFFF))|0x111, ELFIN_GPIO_BASE+GPH2CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH2PUD_OFFSET)&(~0x3F), ELFIN_GPIO_BASE+GPH2PUD_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH2DAT_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPH2DAT_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPH3CON_OFFSET)&(~0xFFFF))|0xFFFF, ELFIN_GPIO_BASE+GPH3CON_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPH3PUD_OFFSET)&(~0xFF))|0xAA, ELFIN_GPIO_BASE+GPH3PUD_OFFSET);
	writel((readl(0xE0200E0C)&(~0xFFFF))|0x2222, 0xE0200E0C);
#if 0 // the port definition has been changed	
	writel(readl(ELFIN_CLOCK_POWER_BASE+EINT_WAKEUP_MASK_OFFSET)&(~((1<<24)|(1<<25)|(1<<26)|(1<<27))), 
		   ELFIN_CLOCK_POWER_BASE+EINT_WAKEUP_MASK_OFFSET);
#endif

	writel((readl(ELFIN_GPIO_BASE+GPH0CON_OFFSET)&(~0xFF000))|0xFF000, ELFIN_GPIO_BASE+GPH0CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH0PUD_OFFSET)&(~0x3C0), ELFIN_GPIO_BASE+GPH0PUD_OFFSET);
	writel((readl(0xE0200E00)&(~0xF0000))|0x22000, 0xE0200E00);
	writel(readl(ELFIN_CLOCK_POWER_BASE+EINT_WAKEUP_MASK_OFFSET)&(~((1<<4)|(1<<3))), 
		   ELFIN_CLOCK_POWER_BASE+EINT_WAKEUP_MASK_OFFSET);
	writel(readl(ELFIN_CLOCK_POWER_BASE+WAKEUP_MASK_OFFSET)&(~(1<<1)), 
		   ELFIN_CLOCK_POWER_BASE+WAKEUP_MASK_OFFSET);

	asm("b 1f\n\t"
	    ".align 5\n\t"
	    "1:\n\t"
	    "mov r5, #0\n\t"
	    "mcr p15, 0, r5, c7, c10, 5\n\t"
	    "mcr p15, 0, r5, c7, c10, 4\n\t"
	    ".word 0xe320f003");
}

void s5p_lcd_draw_bitmap(const unsigned char * buf)
{
	unsigned long i, j, sx, sy;
	unsigned long* pBuffer = (unsigned long*)CFG_LCD_FBUFFER;
	unsigned short* pBitmap = (unsigned short*)buf;
	unsigned long iBitmapData;

	sx = (1024-BAT_CHG_BITMAP_WIDTH)>>1;
	sy = (600-BAT_CHG_BITMAP_HEIGHT)>>1;
	sy -= 20;

	for (i=sy; i<sy+BAT_CHG_BITMAP_HEIGHT; i++)
	{
		for (j=sx; j<sx+BAT_CHG_BITMAP_WIDTH; j++)
		{
			iBitmapData = 0xFF<<24;
			iBitmapData |= ((*pBitmap>>8)&0xF8)<<16;
			iBitmapData |= ((*pBitmap>>5)&0x3F)<<10;
			iBitmapData |= ((*pBitmap<<3)&0xF8);
			*(pBuffer+(i*1024)+j) = iBitmapData;
			pBitmap++;
		}
	}
}

void act8937_dump_ldo_register()
{
	printf("0x00 is 0x%x\n", s5p_iic0_read(0xB6, 0x00));
	printf("0x01 is 0x%x\n", s5p_iic0_read(0xB6, 0x01));
	printf("0x20 is 0x%x\n", s5p_iic0_read(0xB6, 0x20));
	printf("0x21 is 0x%x\n", s5p_iic0_read(0xB6, 0x21));
	printf("0x22 is 0x%x\n", s5p_iic0_read(0xB6, 0x22));
	printf("0x30 is 0x%x\n", s5p_iic0_read(0xB6, 0x30));
	printf("0x31 is 0x%x\n", s5p_iic0_read(0xB6, 0x31));
	printf("0x32 is 0x%x\n", s5p_iic0_read(0xB6, 0x32));
	printf("0x40 is 0x%x\n", s5p_iic0_read(0xB6, 0x40));
	printf("0x41 is 0x%x\n", s5p_iic0_read(0xB6, 0x41));
	printf("0x42 is 0x%x\n", s5p_iic0_read(0xB6, 0x42));
	printf("0x50 is 0x%x\n", s5p_iic0_read(0xB6, 0x50));
	printf("0x51 is 0x%x\n", s5p_iic0_read(0xB6, 0x51));
	printf("0x54 is 0x%x\n", s5p_iic0_read(0xB6, 0x54));
	printf("0x55 is 0x%x\n", s5p_iic0_read(0xB6, 0x55));
	printf("0x60 is 0x%x\n", s5p_iic0_read(0xB6, 0x60));
	printf("0x61 is 0x%x\n", s5p_iic0_read(0xB6, 0x61));
	printf("0x64 is 0x%x\n", s5p_iic0_read(0xB6, 0x64));
	printf("0x65 is 0x%x\n", s5p_iic0_read(0xB6, 0x65));
	printf("0x70 is 0x%x\n", s5p_iic0_read(0xB6, 0x70));
	printf("0x71 is 0x%x\n", s5p_iic0_read(0xB6, 0x71));
	printf("0x78 is 0x%x\n", s5p_iic0_read(0xB6, 0x78));
	printf("0x79 is 0x%x\n", s5p_iic0_read(0xB6, 0x79));
	printf("0x7A is 0x%x\n", s5p_iic0_read(0xB6, 0x7A));
}

void act8937_charging_loop()
{
	// --------------------------------------------------------------------
	//  NOTE: USB Charger is NOT supported because of T34 H/W schmetatics!
	// --------------------------------------------------------------------

	int i, iTmp, iDelayCounter, iEocDone, iStSlp=0xFF, iFrameNr, iPKNr, bCharged = 0;

	s5p_iic0_init();
	writel(readl(ELFIN_GPIO_BASE+GPH0CON_OFFSET)&(~0xF000), ELFIN_GPIO_BASE+GPH0CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH0PUD_OFFSET)&(~0xC0), ELFIN_GPIO_BASE+GPH0PUD_OFFSET);

	if (readl(INF_REG_BASE+INF_REG0_OFFSET)==0xFFAADDEE)
		goto CHARGING;

	//temp, add one iic read process here to avoid iic operation failure
	s5p_iic0_read(0xB6, 0x71);
	
	iTmp = s5p_iic0_read(0xB6, 0x78);
	if (iTmp<0)
	{
		printf("ERROR: failed to read data from ACT8937 via I2C bus check acin presentation.\n");
		iTmp = s5p_iic0_read(0xB6, 0x78);
		if(iTmp<0)
			printf("failed again....\n");
		memset((void*)CFG_LCD_FBUFFER, 0x00, 1366*500*4);
		return;
	}
	if (!(iTmp&0x2))
	{
		return;	// goto DOWN;
	}

	iTmp = 1;
	iDelayCounter = 2;
	printf("**********************************\n");
	printf(" WILL ENTER BATTERY CHARGING LOOP\n");
	printf("**********************************\n");
	printf("Hit any key to skip to normal mode: %d", iDelayCounter);
	for (; iDelayCounter>0; )
	{
		iDelayCounter--;
		for (i=0; i<50; i++)
		{
			if (tstc())
			{
				iTmp = 0;
				iDelayCounter = 0;
				getc();
				break;
			}
			udelay(20000);
		}
		printf("\b%d", iDelayCounter);
		if (!iTmp)
		{
			printf("\n");
			return;
		}
	}

CHARGING:

	printf("\nEnter Battery Charging Loop......\n");

#if 1
	/*checking whether it is usb charger or acin charger*/
	extern int s3c_check_usb_connect_status();
	int usb;
	usb = s3c_check_usb_connect_status();
	if(usb == 1)
	{
		/*usb_mode: limit charging current to 450mA, gpc0_4[ACIN] to 0*/
		printf("usb charger!\n");
		writel(readl(ELFIN_GPIO_BASE+GPC0PUD_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUD_OFFSET);
		writel((readl(ELFIN_GPIO_BASE+GPC0CON_OFFSET)&(~0xF0000))|0x10000, ELFIN_GPIO_BASE+GPC0CON_OFFSET);
		writel(readl(ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET);

		writel(readl(ELFIN_GPIO_BASE+GPC0DAT_OFFSET)&(~1<<4), ELFIN_GPIO_BASE+GPC0DAT_OFFSET);
		writel((readl(ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET)&(~0x300))|0x100, ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET);
	}
	else
	{
		/*ac_in: no limit to charger current, gpc1_0[ACIN] to 1*/
		printf("acin charger!\n");
		writel(readl(ELFIN_GPIO_BASE+GPC0PUD_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUD_OFFSET);
		writel((readl(ELFIN_GPIO_BASE+GPC0CON_OFFSET)&(~0xF0000))|0x10000, ELFIN_GPIO_BASE+GPC0CON_OFFSET);
		writel(readl(ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET);

		writel(readl(ELFIN_GPIO_BASE+GPC0DAT_OFFSET)|(1<<4), ELFIN_GPIO_BASE+GPC0DAT_OFFSET);
		writel((readl(ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET)&(~0x300))|0x100, ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET);
	}
#endif

	if (readl(INF_REG_BASE+INF_REG0_OFFSET)!=0xFFAADDEE)
	{
		if(usb == 1)
		{
			s5p_iic0_write(0xB6, 0x78, 0x20); //disable eoc in and out interrupt, otherwise in usb mode, it will reboot repeatedly
		}
		else
		{
			s5p_iic0_write(0xB6, 0x78, 0x20); //acin still keeps eoc irq, when plug out the acin, make it display charged fully
		}
		
		s5p_iic0_write(0xB6, 0x79, 0x32);
		s5p_iic0_write(0xB6, 0x71, 0x28); // prolong the charging time-out time
	}
	else if ((readl(INF_REG_BASE+INF_REG1_OFFSET)&0xFF99EE00)==0xFF99EE00)
	{
		iStSlp = readl(INF_REG_BASE+INF_REG1_OFFSET)&0xFF;
	}

	//writel(readl(ELFIN_GPIO_BASE+GPC0PUD_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUD_OFFSET);
	//writel((readl(ELFIN_GPIO_BASE+GPC0CON_OFFSET)&(~0xF0000))|0x10000, ELFIN_GPIO_BASE+GPC0CON_OFFSET);
	//writel(readl(ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1PUD_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1PUD_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPC1CON_OFFSET)&(~0xF))|0x1, ELFIN_GPIO_BASE+GPC1CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1PUDPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1PUDPDN_OFFSET);
	//writel(readl(ELFIN_GPIO_BASE+GPC0DAT_OFFSET)|(1<<4), ELFIN_GPIO_BASE+GPC0DAT_OFFSET);
	//writel((readl(ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET)&(~0x300))|0x100, ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1DAT_OFFSET)|(1<<0), ELFIN_GPIO_BASE+GPC1DAT_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET)&(~0x3))|0x1, ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET);

	writel((readl(ELFIN_GPIO_BASE+GPH2CON_OFFSET)&(~0xFFF))|0x111, ELFIN_GPIO_BASE+GPH2CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH2PUD_OFFSET)&(~0x3F), ELFIN_GPIO_BASE+GPH2PUD_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH2DAT_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPH2DAT_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPH3CON_OFFSET)&(~0xFFFF), ELFIN_GPIO_BASE+GPH3CON_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPH3PUD_OFFSET)&(~0xFF))|0xAA, ELFIN_GPIO_BASE+GPH3PUD_OFFSET);

	if (readl(INF_REG_BASE+INF_REG0_OFFSET)==0xFFAADDEE)
	{
	    /*return from uboot sleep, release gpj retention first*/
	    writel(readl(OTHERS) | (0x01 << 30), OTHERS);
	    s5p_iic0_init();
	}

	/*here we get the charging status before turn on the LCD, since the process LCD sometimes will restart the charging*/
	bCharged = s5p_iic0_read(0xB6, 0x78) & 0x01;
	
	extern void LCD_turnon();
	memset((void*)CFG_LCD_FBUFFER, 0x00, 1366*500*4);
	LCD_turnon(0);

	iEocDone = 0;
	iFrameNr = 0;
	iPKNr = 0;
	iDelayCounter = 0;

	udelay(120000);

	for (;;)
	{
		iTmp = s5p_iic0_read(0xB6, 0x78);
		i = s5p_iic0_read(0xB6, 0x7A);
		if ((iTmp<0)||(i<0))
		{
			printf("ERROR: failed to read data from ACT8937 via I2C bus.\n");
			memset((void*)CFG_LCD_FBUFFER, 0x00, 1366*500*4);
			return;
		}
		if ((readl(ELFIN_GPIO_BASE+GPH3DAT_OFFSET)&0xF)!=0xF)
		{
			printf("s5p_iic0_read(0xB6, 0x78) = 0x%X\n", iTmp);
			printf("s5p_iic0_read(0xB6, 0x7A) = 0x%X\n", i);
		}

		if((!(iTmp&0x1)) && (iTmp&0x2))
		{
			if (iEocDone)
			{
				writel(readl(ELFIN_GPIO_BASE+GPC1DAT_OFFSET)|(1<<0), ELFIN_GPIO_BASE+GPC1DAT_OFFSET);
				writel((readl(ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET)&(~0x3))|0x1, ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET);
				iEocDone = 0;
			}

			if(bCharged)
			{
				/*still display the system is charged when the system indicates charged before LCD is on*/
				s5p_lcd_draw_bitmap(bat_charged);
			}
			else
			{
			s5p_lcd_draw_bitmap(bat_charging[iFrameNr]);
			}
			iStSlp = 0xFF;
		}
		if (iTmp&0x1)
		{
			if (!iEocDone)
			{
				printf("INFO: The battery has been fully charged(EOC).\n");
				s5p_lcd_draw_bitmap(bat_charged);
				iEocDone = 1;
				iFrameNr = 0;
				iStSlp = 0xFF;
			}
		}
		if (!(iTmp&0x2))
		{
			if (readl(ELFIN_GPIO_BASE+GPH0DAT_OFFSET)&0x8)
			{
				switch (iStSlp)
				{
				case 1:
					s5p_lcd_draw_bitmap(bat_charged);
					break;
				case 0:
					if(bCharged)
					{
						/*still display the system is charged when the system indicates charged before LCD is on*/
						s5p_lcd_draw_bitmap(bat_charged);
					}
					else
					{
					s5p_lcd_draw_bitmap(bat_charging[1]);
					}
					
					break;
				}
				udelay(3000000);
				goto DOWN;
			}
		}
		for (i=0; i<25; i++)
		{
			if (!(readl(ELFIN_GPIO_BASE+GPH0DAT_OFFSET)&0x8))
			{
				iPKNr++;
				iDelayCounter = 0;
			}
			else
				iPKNr = 0;
			if (iPKNr>75)
			{
				printf("Booting the system...\n");
				writel(readl(ELFIN_GPIO_BASE+GPC1DAT_OFFSET)&(~(1<<0)), ELFIN_GPIO_BASE+GPC1DAT_OFFSET);
				writel(readl(ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET);
				memset((void*)CFG_LCD_FBUFFER, 0x00, 1366*500*4);
				return;
			}
			if ((readl(ELFIN_GPIO_BASE+GPH3DAT_OFFSET)&0xF)!=0xF)
				iDelayCounter = 0;
			if (iDelayCounter++ > 500)
			{
				iDelayCounter = 0;
				zzz(0xFF99EE00|iEocDone);
			}
			udelay(20000);
		}
		iFrameNr++;
		if (iFrameNr>2) iFrameNr = 0;
	}

DOWN:
	printf("Power off the device...\n");
#if 0	/*ps hold pin exists one pull up resiter, use iic to turn off vout1 method for powering off process*/
	writel(readl(ELFIN_GPIO_BASE+GPC0DAT_OFFSET)&(~(1<<4)), ELFIN_GPIO_BASE+GPC0DAT_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1DAT_OFFSET)&(~(1<<0)), ELFIN_GPIO_BASE+GPC1DAT_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC0PUD_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUD_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPC0CON_OFFSET)&(~0xF0000))|0x10000, ELFIN_GPIO_BASE+GPC0CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1PUD_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1PUD_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPC1CON_OFFSET)&(~0xF))|0x1, ELFIN_GPIO_BASE+GPC1CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1PUDPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1PUDPDN_OFFSET);
	s5p_iic0_write(0xB6, 0x61, s5p_iic0_read(0xB6, 0x61)&(~0x80));
	s5p_iic0_write(0xB6, 0x65, s5p_iic0_read(0xB6, 0x65)&(~0x80));
	writel(0x201, ELFIN_CLOCK_POWER_BASE+PS_HOLD_CONTROL_OFFSET);
#else
	writel(readl(ELFIN_GPIO_BASE+GPC0DAT_OFFSET)&(~(1<<4)), ELFIN_GPIO_BASE+GPC0DAT_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1DAT_OFFSET)&(~(1<<0)), ELFIN_GPIO_BASE+GPC1DAT_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1CONPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC0PUD_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUD_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPC0CON_OFFSET)&(~0xF0000))|0x10000, ELFIN_GPIO_BASE+GPC0CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET)&(~0x300), ELFIN_GPIO_BASE+GPC0PUDPDN_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1PUD_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1PUD_OFFSET);
	writel((readl(ELFIN_GPIO_BASE+GPC1CON_OFFSET)&(~0xF))|0x1, ELFIN_GPIO_BASE+GPC1CON_OFFSET);
	writel(readl(ELFIN_GPIO_BASE+GPC1PUDPDN_OFFSET)&(~0x3), ELFIN_GPIO_BASE+GPC1PUDPDN_OFFSET);

	//printf("before power off process...\n");
	//act8937_dump_ldo_register();	
	
	/*set the enable bit to 1 first, otherwise it is unable to disable output*/
	s5p_iic0_write(0xB6, 0x22, s5p_iic0_read(0xB6, 0x22) | (0x01 << 7));
	
	printf("after setting on bit...\n");
	act8937_dump_ldo_register();
	
	s5p_iic0_write(0xB6, 0x22, s5p_iic0_read(0xB6, 0x22)&(~(0x01 << 7))); //vout1	

	printf("power off finished\n");
#endif	
	for (;;) ;
}

