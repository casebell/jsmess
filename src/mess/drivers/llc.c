/***************************************************************************

        LLC driver by Miodrag Milanovic

        17/04/2009 Preliminary driver.

****************************************************************************/


#include "driver.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80pio.h"
#include "machine/z80ctc.h"
#include "includes/llc.h"

/* Address maps */
static ADDRESS_MAP_START(llc1_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x07ff) AM_ROM // Monitor ROM
	AM_RANGE(0x0800, 0x13ff) AM_ROM // BASIC ROM
	AM_RANGE(0x1400, 0x1bff) AM_RAM // RAM
	AM_RANGE(0x1c00, 0x1fff) AM_RAM AM_BASE(&llc_video_ram) // Video RAM	
	AM_RANGE(0x2000, 0xffff) AM_RAM // RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( llc1_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH		
	AM_RANGE(0xEC, 0xEF) AM_DEVREADWRITE("z80pio", z80pio_r, z80pio_w)
	//AM_RANGE(0xF4, 0xF7) AM_DEVREADWRITE("z80pio", z80pio_r, z80pio_w)
	AM_RANGE(0xF8, 0xFB) AM_DEVREADWRITE("z80ctc", z80ctc_r, z80ctc_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(llc2_mem, ADDRESS_SPACE_PROGRAM, 8)
	AM_RANGE(0x0000, 0x3fff) AM_RAMBANK(1)
	AM_RANGE(0x4000, 0x5fff) AM_RAMBANK(2)
	AM_RANGE(0x6000, 0xbfff) AM_RAMBANK(3)
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK(4)
ADDRESS_MAP_END

static ADDRESS_MAP_START( llc2_io, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH	
	AM_RANGE(0xE0, 0xE3) AM_WRITE(llc2_rom_disable_w)
	AM_RANGE(0xE8, 0xEB) AM_DEVREADWRITE("z80pio", z80pio_r, z80pio_w)
	AM_RANGE(0xEC, 0xEC) AM_WRITE(llc2_basic_enable_w)
	AM_RANGE(0xF8, 0xFB) AM_DEVREADWRITE("z80ctc", z80ctc_r, z80ctc_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( llc1 )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) // Enter
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED) 
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\"")PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("%") PORT_CODE(KEYCODE_TILDE)   
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("&") PORT_CODE(KEYCODE_TILDE)   
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(") PORT_CODE(KEYCODE_TILDE)   
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(")") PORT_CODE(KEYCODE_TILDE)   
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_TILDE)
	PORT_START("LINE8")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_START("LINE9")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_START("LINE10")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_START("LINE11")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('\\') PORT_CHAR('|')
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("~") PORT_CODE(KEYCODE_TILDE)	// this alone gives nothing and gives ` with Shift!
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_EQUALS)	// this alone acts like Enter and gives no result with Shift			
INPUT_PORTS_END

static INPUT_PORTS_START( k7659 )
	PORT_START("A1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_START("A2")		
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D)		
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V)
	PORT_START("A3")	
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_START("A4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA)
	PORT_START("A5")	
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("|") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS)
	PORT_START("A6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E11") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("}") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("{") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("<") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("#") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("HT") PORT_CODE(KEYCODE_TILDE)
	PORT_START("A7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E13") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CR") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("{") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("}") PORT_CODE(KEYCODE_TILDE)
	PORT_START("A8")		 		
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_RSHIFT)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_START("A9")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) 
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CLS") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DEL") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F01") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F02") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F03") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F04") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F06") PORT_CODE(KEYCODE_TILDE)
	PORT_START("A10")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F07") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F08") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F09") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F11") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F12") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F13") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F14") PORT_CODE(KEYCODE_TILDE)
	PORT_START("A11")	
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F05") PORT_CODE(KEYCODE_TILDE)		
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E95") PORT_CODE(KEYCODE_TILDE)
	PORT_START("A12")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NMI") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED) 
		PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
		PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

static const z80_daisy_chain llc1_daisy_chain[] =
{
	{ "z80pio" },
	{ "z80ctc" },
	{ NULL }
};

/* Machine driver */
static MACHINE_DRIVER_START( llc1 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_3MHz)
	MDRV_CPU_CONFIG(llc1_daisy_chain)
	MDRV_CPU_PROGRAM_MAP(llc1_mem, 0)
	MDRV_CPU_IO_MAP(llc1_io, 0)	
	
	MDRV_MACHINE_START( llc1 )
	MDRV_MACHINE_RESET( llc1 )

    /* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 16*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 16*8-1)
	MDRV_PALETTE_LENGTH(2)
	MDRV_PALETTE_INIT(black_and_white)

	MDRV_VIDEO_START(llc1)
    MDRV_VIDEO_UPDATE(llc1)    
    
	MDRV_Z80PIO_ADD( "z80pio", llc1_z80pio_intf )    
	MDRV_Z80CTC_ADD( "z80ctc", XTAL_3MHz, llc1_ctc_intf )
MACHINE_DRIVER_END

static MACHINE_DRIVER_START( llc2 )
	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, XTAL_3MHz)
	MDRV_CPU_PROGRAM_MAP(llc2_mem, 0)
	MDRV_CPU_IO_MAP(llc2_io, 0)	
	
	MDRV_MACHINE_RESET( llc2 )

    /* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(50)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)
	MDRV_PALETTE_LENGTH(2)
	MDRV_PALETTE_INIT(black_and_white)

	MDRV_VIDEO_START(llc2)
    MDRV_VIDEO_UPDATE(llc2)    
    
	MDRV_Z80PIO_ADD( "z80pio", llc2_z80pio_intf )    
	MDRV_Z80CTC_ADD( "z80ctc", XTAL_3MHz, llc2_ctc_intf )
MACHINE_DRIVER_END
/* ROM definition */

ROM_START( llc1 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
    ROM_LOAD( "llc1-monitor.rom", 0x0000, 0x0800, CRC(0E81378D) )        
    ROM_LOAD( "llc1_tb1.bin", 0x0800, 0x0400, CRC(0D9D4039) )
    ROM_LOAD( "llc1_tb2.bin", 0x0C00, 0x0400, CRC(28BFEA2A) )
    ROM_LOAD( "llc1_tb3.bin", 0x1000, 0x0400, CRC(FE5E3132) )
	ROM_REGION(0x0400, "gfx1",0)
	ROM_LOAD ("llc1_zg.bin", 0x0000, 0x0400, CRC(FA2CD659) )    
ROM_END

ROM_START( llc2 )
	ROM_REGION( 0x12000, "maincpu", ROMREGION_ERASEFF )
    ROM_LOAD( "scchmon_91.bin", 0x0000, 0x1000, CRC(218D8236) )    
    ROM_LOAD( "gsbasic.bin", 0x10000, 0x2000, CRC(78A5F388) )    
	ROM_REGION(0x0800, "gfx1",0)
	ROM_LOAD ("llc2font.bin", 0x0000, 0x0800, CRC(CE53E55D) )    
	ROM_REGION(0x1000, "k7659",0)
	ROM_LOAD ("k7659n.bin", 0x0000, 0x0800, CRC(7454BF0A) )    
ROM_END
 
/* Driver */

static SYSTEM_CONFIG_START(llc2)
	CONFIG_RAM_DEFAULT(64 * 1024)
SYSTEM_CONFIG_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE     INPUT       INIT     CONFIG COMPANY          FULLNAME   	FLAGS */
COMP( 1984, llc1,	0,		0,		llc1, 		llc1, 		llc1, 	 0, 	"",		 "LLC-1",	 	GAME_NOT_WORKING)
COMP( 1984, llc2,	llc1,	0,		llc2, 		k7659, 		llc2, 	 llc2, 	"",		 "LLC-2",	 	0)
