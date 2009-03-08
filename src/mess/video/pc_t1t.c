/***************************************************************************

	IBM PC junior
	Tandy 1000 Graphics Adapter (T1T) section

	Note that in the IBM PC Junior world, the term 'vga' is not the 'vga' that
	most people think of

***************************************************************************/

#include "driver.h"
#include "pc_t1t.h"
#include "video/mc6845.h"
#include "machine/pic8259.h"


/***************************************************************************

	Static declarations

***************************************************************************/

static PALETTE_INIT( pcjr );
static VIDEO_START( pc_t1t );
static VIDEO_START( pc_pcjr );
static VIDEO_UPDATE( mc6845_t1000 );
static MC6845_UPDATE_ROW( t1000_update_row );
static MC6845_ON_DE_CHANGED( t1000_de_changed );
static MC6845_ON_VSYNC_CHANGED( t1000_vsync_changed );
static MC6845_ON_VSYNC_CHANGED( pcjr_vsync_changed );


static const mc6845_interface mc6845_t1000_intf = {
	T1000_SCREEN_NAME,		/* screen number */
	8,						/* numbers of pixels per video memory address */
	NULL,					/* begin_update */
	t1000_update_row,		/* update_row */
	NULL,					/* end_update */
	t1000_de_changed,		/* on_de_chaged */
	NULL,					/* on_hsync_changed */
	t1000_vsync_changed		/* on_vsync_changed */
};


MACHINE_DRIVER_START( pcvideo_t1000 )
	MDRV_SCREEN_ADD(T1000_SCREEN_NAME, RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(XTAL_14_31818MHz,912,0,640,262,0,200)
	MDRV_PALETTE_LENGTH( 16 )
	MDRV_PALETTE_INIT(pcjr)

	MDRV_MC6845_ADD(T1000_MC6845_NAME, MC6845, XTAL_14_31818MHz/8, mc6845_t1000_intf)

	MDRV_VIDEO_START(pc_t1t)
	MDRV_VIDEO_UPDATE( mc6845_t1000 )
MACHINE_DRIVER_END


static const mc6845_interface mc6845_pcjr_intf = {
	T1000_SCREEN_NAME,		/* screen number */
	8,						/* numbers of pixels per video memory address */
	NULL,					/* begin_update */
	t1000_update_row,		/* update_row */
	NULL,					/* end_update */
	t1000_de_changed,		/* on_de_chaged */
	NULL,					/* on_hsync_changed */
	pcjr_vsync_changed		/* on_vsync_changed */
};


MACHINE_DRIVER_START( pcvideo_pcjr )
	MDRV_SCREEN_ADD(T1000_SCREEN_NAME, RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_RAW_PARAMS(XTAL_14_31818MHz,912,0,640,262,0,200)
	MDRV_PALETTE_LENGTH( 16 )
	MDRV_PALETTE_INIT(pcjr)

	MDRV_MC6845_ADD(T1000_MC6845_NAME, MC6845, XTAL_14_31818MHz/16, mc6845_pcjr_intf)

	MDRV_VIDEO_START(pc_pcjr)
	MDRV_VIDEO_UPDATE( mc6845_t1000 )
MACHINE_DRIVER_END


/***************************************************************************

	Methods

***************************************************************************/

/* Initialise the cga palette */
static PALETTE_INIT( pcjr )
{
	const static unsigned char tga_palette[16][3] =
	{
		{ 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xaa }, { 0x00, 0xaa, 0x00 }, { 0x00, 0xaa, 0xaa },
		{ 0xaa, 0x00, 0x00 }, { 0xaa, 0x00, 0xaa }, { 0xaa, 0x55, 0x00 }, { 0xaa, 0xaa, 0xaa },
		{ 0x55, 0x55, 0x55 }, { 0x55, 0x55, 0xff }, { 0x55, 0xff, 0x55 }, { 0x55, 0xff, 0xff },
		{ 0xff, 0x55, 0x55 }, { 0xff, 0x55, 0xff }, { 0xff, 0xff, 0x55 }, { 0xff, 0xff, 0xff }
	};
	int i;

	for(i = 0; i < 16; i++)
		palette_set_color_rgb(machine, i, tga_palette[i][0], tga_palette[i][1], tga_palette[i][2]);
}


static struct
{
	UINT8 mode_control, color_select;
	UINT8 status;

	// used in tandy1000hx; used in pcjr???
	struct {
		UINT8 index;
		UINT8 data[0x20];
		/* see vgadoc
		   0 mode control 1
		   1 palette mask
		   2 border color
		   3 mode control 2
		   4 reset
		   0x10-0x1f palette registers
		*/
	} reg;

	UINT8 bank;

	int pc_framecnt;

	UINT8 *displayram;

	UINT8 *chr_gen;
	UINT8	chr_size;

	UINT8	address_data_ff;

	mc6845_update_row_func	update_row;
	UINT8	display_enable;
	UINT8	vsync;
} pcjr = { 0 };


static VIDEO_START( pc_t1t )
{
	pcjr.chr_gen = memory_region(machine, "gfx1");
	pcjr.update_row = NULL;
	pcjr.bank = 0;
	pcjr.chr_size = 16;

	videoram_size = 0x8000;
}


static VIDEO_START( pc_pcjr )
{
	pcjr.chr_gen = memory_region(machine, "gfx1");
	pcjr.update_row = NULL;
	pcjr.bank = 0;
	pcjr.mode_control = 0x08;
	pcjr.chr_size = 8;

	videoram_size = 0x8000;
}

static VIDEO_UPDATE( mc6845_t1000 )
{
	const device_config	*devconf = devtag_get_device(screen->machine, T1000_MC6845_NAME);
	mc6845_update( devconf, bitmap, cliprect);
	return 0;
}


static MC6845_UPDATE_ROW( t1000_text_inten_update_row )
{
	UINT16  *p = BITMAP_ADDR16(bitmap, y, 0);
	int i;

	if ( y == 0 ) logerror("t1000_text_inten_update_row\n");
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = pcjr.displayram[ offset ];
		UINT8 attr = pcjr.displayram[ offset +1 ];
		UINT8 data = pcjr.chr_gen[ chr * pcjr.chr_size + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = ( attr >> 4 ) & 0x07;

		if ( i == cursor_x && ( pcjr.pc_framecnt & 0x08 ) )
		{
			data = 0xFF;
		}

		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x40 ) ? fg : bg; p++;
		*p = ( data & 0x20 ) ? fg : bg; p++;
		*p = ( data & 0x10 ) ? fg : bg; p++;
		*p = ( data & 0x08 ) ? fg : bg; p++;
		*p = ( data & 0x04 ) ? fg : bg; p++;
		*p = ( data & 0x02 ) ? fg : bg; p++;
		*p = ( data & 0x01 ) ? fg : bg; p++;
	}
}


static MC6845_UPDATE_ROW( t1000_text_blink_update_row )
{
	UINT16	*p = BITMAP_ADDR16(bitmap, y, 0);
	int i;

	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = pcjr.displayram[ offset ];
		UINT8 attr = pcjr.displayram[ offset +1 ];
		UINT8 data = pcjr.chr_gen[ chr * pcjr.chr_size + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = ( attr >> 4 ) & 0x07;

		if ( i == cursor_x )
		{
			if ( pcjr.pc_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}
		else
		{
			if ( ( attr & 0x80 ) && ( pcjr.pc_framecnt & 0x10 ) )
			{
				data = 0x00;
			}
		}

		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x40 ) ? fg : bg; p++;
		*p = ( data & 0x20 ) ? fg : bg; p++;
		*p = ( data & 0x10 ) ? fg : bg; p++;
		*p = ( data & 0x08 ) ? fg : bg; p++;
		*p = ( data & 0x04 ) ? fg : bg; p++;
		*p = ( data & 0x02 ) ? fg : bg; p++;
		*p = ( data & 0x01 ) ? fg : bg; p++;
	}
}


static MC6845_UPDATE_ROW( t1000_gfx_4bpp_update_row )
{
	UINT16	*p = BITMAP_ADDR16(bitmap, y, 0);
	UINT8	*vid = pcjr.displayram + ( ra << 13 );
	int i;

	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x1fff;
		UINT8 data = vid[ offset ];

		*p = pcjr.reg.data[0x10 + ( data >> 4 )]; p++;
		*p = pcjr.reg.data[0x10 + ( data >> 4 )]; p++;
		*p = pcjr.reg.data[0x10 + ( data & 0x0F )]; p++;
		*p = pcjr.reg.data[0x10 + ( data & 0x0F )]; p++;

		data = vid[ offset + 1 ];

		*p = pcjr.reg.data[0x10 + ( data >> 4 )]; p++;
		*p = pcjr.reg.data[0x10 + ( data >> 4 )]; p++;
		*p = pcjr.reg.data[0x10 + ( data & 0x0F )]; p++;
		*p = pcjr.reg.data[0x10 + ( data & 0x0F )]; p++;
	}
}


static MC6845_UPDATE_ROW( t1000_gfx_2bpp_update_row )
{
	UINT16  *p = BITMAP_ADDR16(bitmap, y, 0);
	UINT8	*vid = pcjr.displayram + ( ra << 13 );
	int i;

	if ( y == 0 ) logerror("t1000_gfx_2bpp_update_row\n");
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x1fff;
		UINT8 data = vid[ offset ];

		*p = pcjr.reg.data[ 0x10 + ( ( data >> 6 ) & 0x03 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( data >> 4 ) & 0x03 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( data >> 2 ) & 0x03 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + (   data        & 0x03 ) ]; p++;

		data = vid[ offset+1 ];

		*p = pcjr.reg.data[ 0x10 + ( ( data >> 6 ) & 0x03 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( data >> 4 ) & 0x03 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( data >> 2 ) & 0x03 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + (   data        & 0x03 ) ]; p++;
	}
}


static MC6845_UPDATE_ROW( t1000_gfx_2bpp_tga_update_row )
{
	UINT16	*p = BITMAP_ADDR16(bitmap, y, 0); 
	UINT8	*vid = pcjr.displayram + ( ra << 13 );
	int i; 

	if ( y == 0 ) logerror("t1000_gfx_2bpp_tga_update_row\n");
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x1fff;
		UINT8 data = vid[ offset ]; 
		UINT16 data2 = ( vid[ offset + 1 ] ) << 1;
        
		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x100 ) | ( data & 0x80 ) ) >> 7 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x80 ) | ( data & 0x40 ) ) >> 6 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x40 ) | ( data & 0x20 ) ) >> 5 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x20 ) | ( data & 0x10 ) ) >> 4 ) ]; p++;

		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x10 ) | ( data & 0x08 ) ) >> 3 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x08 ) | ( data & 0x04 ) ) >> 2 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + ( ( ( data2 & 0x04 ) | ( data & 0x02 ) ) >> 1 ) ]; p++;
		*p = pcjr.reg.data[ 0x10 + (   ( data2 & 0x02 ) | ( data & 0x01 )        ) ]; p++;
	}
}


static MC6845_UPDATE_ROW( t1000_gfx_1bpp_update_row )
{
	UINT16  *p = BITMAP_ADDR16(bitmap, y, 0);
	UINT8	*vid = pcjr.displayram + ( ra << 13 );
	UINT8	fg = pcjr.reg.data[0x11];
	UINT8	bg = pcjr.reg.data[0x10];
	int i;

	if ( y == 0 ) logerror("t1000_gfx_1bpp_update_row\n");
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x1fff;
		UINT8 data = vid[ offset ];

		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x40 ) ? fg : bg; p++;
		*p = ( data & 0x20 ) ? fg : bg; p++;
		*p = ( data & 0x10 ) ? fg : bg; p++;
		*p = ( data & 0x08 ) ? fg : bg; p++;
		*p = ( data & 0x04 ) ? fg : bg; p++;
		*p = ( data & 0x02 ) ? fg : bg; p++;
		*p = ( data & 0x01 ) ? fg : bg; p++;

		data = vid[ offset + 1 ];

		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
		*p = ( data & 0x80 ) ? fg : bg; p++;
	}
}

static MC6845_UPDATE_ROW( t1000_update_row )
{
	if ( pcjr.update_row )
	{
		pcjr.update_row( device, bitmap, cliprect, ma, ra, y, x_count, cursor_x, param );
	}
}

 READ8_HANDLER ( pc_t1t_videoram_r )
{
	int data = 0xff;
	if( videoram )
		data = videoram[offset];
	return data;
}


static void pc_t1t_mode_switch( void )
{
	switch( pcjr.mode_control & 0x3B )
	{
	case 0x08: case 0x09:
		pcjr.update_row = t1000_text_inten_update_row;
		break;
	case 0x28: case 0x29:
		pcjr.update_row = t1000_text_blink_update_row;
		break;
	case 0x0A: case 0x0B: case 0x2A: case 0x2B:
		switch( pcjr.bank & 0xc0 )
		{
		case 0x00:
		case 0x40:
			//logerror("t1t_gfx_2bpp - 1\n");
			pcjr.update_row = t1000_gfx_2bpp_update_row;
			if ( pcjr.color_select )
			{
				pcjr.reg.data[0x10] = 0x00;
				pcjr.reg.data[0x11] = 0x0B;
				pcjr.reg.data[0x12] = 0x0D;
				pcjr.reg.data[0x13] = 0x0F;
			}
			else
			{
				pcjr.reg.data[0x10] = 0x00;
				pcjr.reg.data[0x11] = 0x0A;
				pcjr.reg.data[0x12] = 0x0C;
				pcjr.reg.data[0x13] = 0x0E;
			}
			break;
		case 0x80:
		case 0xc0:
			//logerror("t1t_gfx_4bpp\n");
			pcjr.update_row = t1000_gfx_4bpp_update_row;
			break;
		}
		break;
	case 0x18: case 0x19: case 0x1A: case 0x1B:
	case 0x38: case 0x39: case 0x3A: case 0x3B:
		switch( pcjr.bank & 0xc0 )
		{
		case 0x00:
		case 0x40:
			//logerror("t1t_gfx_1bpp\n");
			pcjr.update_row = t1000_gfx_1bpp_update_row;
			break;
		case 0x80:
		case 0xc0:
			//logerror("t1t_gfx_2bpp - 2\n");
			pcjr.update_row = t1000_gfx_2bpp_tga_update_row;
			break;
		}
		break;
	default:
		pcjr.update_row = NULL;
		break;
	}
}


/* mode control 1 ( pcjr.reg.data[0] ) */
/* bit0 - 0 = low bandwidth, 1 = high bandwidth */
/* bit1 - 0 = alpha, 1 = graphics */
/* bit2 - 0 = color, 1 = b/w */
/* bit3 - 0 = video disable, 1 = video enable */
/* bit4 - 1 = 16 color graphics */
/* mode control 2 ( pcjr.reg.data[3] ) */
/* bit1 - 1 = enable blink */
/* bit3 - 1 = 2 color graphics */

static void pc_pcjr_mode_switch( void )
{
	switch( pcjr.reg.data[0] & 0x1F )
	{
	case 0x08: case 0x09:
	case 0x0C: case 0x0D:	/* B/W modes */
		if ( pcjr.reg.data[3] & 0x02 )
		{
			pcjr.update_row = t1000_text_blink_update_row;
		}
		else
		{
			pcjr.update_row = t1000_text_inten_update_row;
		}
		break;
	case 0x1A: case 0x1B:
		pcjr.update_row = t1000_gfx_4bpp_update_row;
		break;
	case 0x0A: case 0x0B:
		pcjr.update_row = t1000_gfx_2bpp_update_row;
		break;
	case 0x0E:
		if ( pcjr.reg.data[3] & 0x08 )
		{
			pcjr.update_row = t1000_gfx_1bpp_update_row;
		}
		else
		{
			pcjr.update_row = t1000_gfx_2bpp_update_row;	/* B/W mode */
		}
		break;
	default:
		pcjr.update_row = NULL;
		break;
	}
}


/*
 * 3d8 rW	T1T mode control register (see #P138)
 */
static void pc_t1t_mode_control_w(int data)
{
	pcjr.mode_control = data;

	pc_t1t_mode_switch();
}

static int pc_t1t_mode_control_r(void)
{
    int data = pcjr.mode_control;
    return data;
}

/*
 * 3d9 ?W	color select register on color adapter
 */
static void pc_t1t_color_select_w(int data)
{
	if (pcjr.color_select == data)
		return;
	pcjr.color_select = data;
}

static int pc_t1t_color_select_r(void)
{
	int data = pcjr.color_select;
    return data;
}

/*  Bitfields for T1T status register:
 *  Bit(s)  Description (Table P179)
 *  7-6 not used
 *  5-4 color EGA, color ET4000: diagnose video display feedback, select
 *      from color plane enable
 *  4   holds current dot being displayed
 *  3   in vertical retrace
 *  2   (CGA,color EGA) light pen switch is off
 *      (MCGA,color ET4000) reserved (0)
 *  1   (CGA,color EGA) positive edge from light pen has set trigger
 *      (MCGA,color ET4000) reserved (0)
 *  0   display enabled
 *      =0  do not use memory
 *      =1  memory access without interfering with display
 *      (Genoa SuperEGA) horizontal or vertical retrace
 */
static int pc_t1t_status_r(void)
{
    int data = pcjr.vsync | pcjr.status | pcjr.display_enable;
	/* HACK HACK HACK */
	data |= ( pcjr.display_enable ? 0x10 : 0x00 );
	/* end HACK */
    return data;
}

/*
 * 3db -W	light pen strobe reset (on any value)
 */
static void pc_t1t_lightpen_strobe_w(int data)
{
//	pc_port[0x3db] = data;
}


/*
 * 3da -W	(mono EGA/mono VGA) feature control register
 *			(see PORT 03DAh-W for details; VGA, see PORT 03CAh-R)
 */
static void pc_t1t_vga_index_w(int data)
{
	pcjr.reg.index = data;
}

static void pc_t1t_vga_data_w(int data)
{
    pcjr.reg.data[pcjr.reg.index] = data;

	switch (pcjr.reg.index)
	{
        case 0x00: /* mode control 1 */
            break;
        case 0x01: /* palette mask (bits 3-0) */
            break;
        case 0x02: /* border color (bits 3-0) */
            break;
        case 0x03: /* mode control 2 */
            break;
        case 0x04: /* reset register */
            break;
        /* palette array */
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1a: case 0x1b:
        case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			pcjr.reg.data[pcjr.reg.index] = data & 0x0F;
            break;
    }
}


static void pc_pcjr_vga_data_w(int data)
{
	pcjr.reg.data[pcjr.reg.index] = data;

	switch (pcjr.reg.index)
	{
		case 0x00:	/* mode control 1 */
					/* bit0 - 0 = low bandwidth, 1 = high bandwidth */
					/* bit1 - 0 = alpha, 1 = graphics */
					/* bit2 - 0 = color, 1 = b/w */
					/* bit3 - 0 = video disable, 1 = video enable */
					/* bit4 - 1 = 16 color graphics */
			pc_pcjr_mode_switch();
			break;
		case 0x01:	/* palette mask (bits 3-0) */
			break;
		case 0x02:	/* border color (bits 3-0) */
			break;
		case 0x03:	/* mode control 2 */
					/* bit1 - 1 = enable blink */
					/* bit3 - 1 = 2 color graphics */
			pc_pcjr_mode_switch();
			break;
		case 0x04:	/* reset register */
			break;
					/* palette array */
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			pcjr.reg.data[pcjr.reg.index] = data & 0x0F;
			break;
	}
}


static int pc_t1t_vga_data_r(void)
{
	int data = pcjr.reg.data[pcjr.reg.index];

	switch (pcjr.reg.index)
	{
        case 0x00: /* mode control 1 */
            break;
        case 0x01: /* palette mask (bits 3-0) */
            break;
        case 0x02: /* border color (bits 3-0) */
            break;
        case 0x03: /* mode control 2 */
            break;
        case 0x04: /* reset register */
            break;
        /* palette array */
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x15: case 0x16: case 0x17:
        case 0x18: case 0x19: case 0x1a: case 0x1b:
        case 0x1c: case 0x1d: case 0x1e: case 0x1f:
            break;
    }
	return data;
}

/*
 * 3df RW	display bank, access bank, mode
 * bit 0-2	Identifies the page of main memory being displayed in units of 16K.
 *			0: 0K, 1: 16K...7: 112K. In 32K modes (bits 6-7 = 2) only 0,2,4 and
 *			6 are valid, as the next page will also be used.
 *	   3-5	Identifies the page of main memory that can be read/written at B8000h
 *			in units of 16K. 0: 0K, 1: 16K...7: 112K. In 32K modes (bits 6-7 = 2)
 *			only 0,2,4 and 6 are valid, as the next page will also be used.
 *	   6-7	Display mode. 0: Text, 1: 16K graphics mode (4,5,6,8)
 *			2: 32K graphics mode (9,Ah)
 */
static void pc_t1t_bank_w(running_machine *machine, int data)
{
	if (pcjr.bank != data)
	{
		UINT8 *ram = memory_region(machine, "maincpu");
		int dram, vram;
		pcjr.bank = data;
	/* it seems the video ram is mapped to the last 128K of main memory */
#if 1
		if ((data&0xc0)==0xc0) /* needed for lemmings */
		{
			dram = 0x80000 + ((data & 0x06) << 14);
			vram = 0x80000 + ((data & 0x30) << (14-3));
		}
		else
		{
			dram = 0x80000 + ((data & 0x07) << 14);
			vram = 0x80000 + ((data & 0x38) << (14-3));
		}
#else
		dram = (data & 0x07) << 14;
		vram = (data & 0x38) << (14-3);
#endif
		videoram = &ram[vram];
		pcjr.displayram = &ram[dram];
	}
	pc_t1t_mode_switch();
}


static void pc_pcjr_bank_w(running_machine *machine, int data)
{
	if (pcjr.bank != data)
	{
		int dram, vram;
		pcjr.bank = data;
		/* it seems the video ram is mapped to the last 128K of main memory */
#if 1
		if ((data&0xc0)==0xc0) /* needed for lemmings */
		{
			dram = 0x80000 + ((data & 0x06) << 14);
			vram = 0x80000 + ((data & 0x30) << (14-3));
		}
		else
		{
			dram = 0x80000 + ((data & 0x07) << 14);
			vram = 0x80000 + ((data & 0x38) << (14-3));
		}
#else
		dram = (data & 0x07) << 14;
		vram = (data & 0x38) << (14-3);
#endif
		videoram = &memory_region(machine, "maincpu")[vram];
		pcjr.displayram = &memory_region(machine, "maincpu")[dram];
	}
	pc_pcjr_mode_switch();
}


static int pc_t1t_bank_r(void)
{
	return pcjr.bank;
}

/*************************************************************************
 *
 *		T1T
 *		Tandy 1000 / PCjr
 *
 *************************************************************************/

WRITE8_HANDLER ( pc_T1T_w )
{
	device_config	*devconf;

	switch( offset )
	{
		case 0: case 2: case 4: case 6:
			devconf = (device_config *) devtag_get_device(space->machine, T1000_MC6845_NAME);
			mc6845_address_w( devconf, offset, data );
			break;
		case 1: case 3: case 5: case 7:
			devconf = (device_config *) devtag_get_device(space->machine, T1000_MC6845_NAME);
			mc6845_register_w( devconf, offset, data );
			break;
		case 8:
			pc_t1t_mode_control_w(data);
			break;
		case 9:
			pc_t1t_color_select_w(data);
			break;
		case 10:
			pc_t1t_vga_index_w(data);
			break;
		case 11:
			pc_t1t_lightpen_strobe_w(data);
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			pc_t1t_vga_data_w(data);
			break;
		case 15:
			pc_t1t_bank_w(space->machine, data);
			break;
    }
}


WRITE8_HANDLER( pc_pcjr_w )
{
	const device_config	*devconf;

	switch( offset )
	{
		case 0: case 4: 
			devconf = devtag_get_device(space->machine, T1000_MC6845_NAME);
			mc6845_address_w( devconf, offset, data );
			break;
		case 1: case 5:
			devconf = devtag_get_device(space->machine, T1000_MC6845_NAME);
			mc6845_register_w( devconf, offset, data );
			break;
		case 10:
			if ( pcjr.address_data_ff & 0x01 )
			{
				pc_pcjr_vga_data_w( data );
			}
			else
			{
				pc_t1t_vga_index_w( data );
			}
			pcjr.address_data_ff ^= 0x01;
			break;
		case 11:
			pc_t1t_lightpen_strobe_w(data);
			break;
		case 12:
			break;
		case 15:
			pc_pcjr_bank_w(space->machine, data);
			break;

		default:
			break;
	}
}


 READ8_HANDLER ( pc_T1T_r )
{
	device_config	*devconf;
	int				data = 0xff;

	switch( offset )
	{
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;

		case 1: case 3: case 5: case 7:
			devconf = (device_config *) devtag_get_device(space->machine, T1000_MC6845_NAME);
			data = mc6845_register_r( devconf, offset );
			break;

		case 8:
			data = pc_t1t_mode_control_r();
			break;

		case 9:
			data = pc_t1t_color_select_r();
			break;

		case 10:
			pcjr.address_data_ff = 0;
			data = pc_t1t_status_r();
			break;

		case 11:
			/* -W lightpen strobe reset */
			break;

		case 12:
		case 13:
            break;

		case 14:
			data = pc_t1t_vga_data_r();
            break;

        case 15:
			data = pc_t1t_bank_r();
			break;
    }
	return data;
}


static MC6845_ON_DE_CHANGED( t1000_de_changed )
{
	pcjr.display_enable = display_enabled ? 1 : 0;
}


static MC6845_ON_VSYNC_CHANGED( t1000_vsync_changed )
{
	pcjr.vsync = vsync ? 8 : 0;
	if ( vsync )
	{
		pcjr.pc_framecnt++;
	}
}


static MC6845_ON_VSYNC_CHANGED( pcjr_vsync_changed )
{
	pcjr.vsync = vsync ? 8 : 0;
	if ( vsync )
	{
		pcjr.pc_framecnt++;
	}
	pic8259_set_irq_line(devtag_get_device(device->machine, "pic8259_master"), 5, vsync);
}

