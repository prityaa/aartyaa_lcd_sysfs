
obj-m = aartyaa_lcd.o
CFLAGS_aartyaa_lcd.o := -DDEBUG

CFLAGS_aartyaa.o := -DDEBUG
CROSS_COMPILE=arm-linux-gnueabihf-
RPI_COMPILE_OPTION = ARCH=arm CROSS_COMPILE=$(CROSS_COMPILE)
APP_SOURCE_CODE = aartyaa_lcd_app.c
COMPILER = gcc
APP_EXE = AARTYAA_LCD_APP
OUTPUT_FLAG = -o

KERNEL_DIR = /home/prityaa/documents/workspace/embeded/raspbery_pi/code_base/i2c_groove_lcd/codebase/aartyaa_lcd/aartyaa_lcd_sys_fs/aartyaa_lcd/linux

rpi:
	$(MAKE) $(RPI_COMPILE_OPTION) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) $(RPI_COMPILE_OPTION) -C $(KERNEL_DIR) M=$(PWD) clean


