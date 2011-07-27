/*
** MPC823 Video Controller
** =======================
** (C) 2000 by Paolo Scaffardi (arsenio@tin.it)
** AIRVENT SAM s.p.a - RIMINI(ITALY)
**
*/

#ifndef _VIDEO_H_
#define _VIDEO_H_

/* Video functions */

int	video_init	(void *videobase);
void	video_putc	(const char c);
void	video_puts	(const char *s);
void	video_printf	(const char *fmt, ...);
void	video_position_cursor (unsigned col, unsigned row);
int	video_display_bitmap (ulong, int, int);
int	video_get_screen_rows (void);
int	video_get_screen_columns (void);
int	video_get_pixel_width (void);
int	video_get_pixel_height(void);

#endif
