/*
******************************************************************************
File:     tiny_scanf.c
Info:     Based on Atollic TrueSTUDIO 9.0.0's tiny_printf.c   2018-03-12

Abstract: Minimal iscanf/siscanf.
          Provides aliased declarations for scanf/sscanf
          pointing to *iscanf variants.

          The argument contains a format string that may include
          conversion specifications. Each conversion specification
          is introduced by the character %, and ends with a
          conversion specifier.

          The following conversion specifiers are supported
          cdi[1...2^32-1]suxX%

		  The following regex are also supported
		  *[c]

          Usage:
          c    			character
          d,i  			signed integer (-sign added, + sign not supported)
          [0...2^32-1]	character string lenght restriction
		  s    			character string
          u    			unsigned integer as decimal
          x,X  			unsigned integer as hexadecimal (uppercase letter)
          %    			% is written (conversion specification is '%%')
		  *[c]			ignores character until another is read.

          Note:
          Character padding is not supported

The MIT License (MIT)
Copyright (c) 2018 STMicroelectronics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************
*/

/* Includes */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* Defines */
#undef DEBUG

/* Create aliases for *scanf to integer variant *iscanf */
__attribute__ ((alias("iscanf"))) int scanf(const char *fmt, ...);
__attribute__ ((alias("siscanf"))) int sscanf(const char *str, const char *fmt, ...);

/* External function prototypes (defined in syscalls.c) */
extern int _read(int fd, char **str, int len);

/* Private function prototypes */
unsigned int ts_atoi(const char **buf, int base);
int ts_format_va_list(const char *buf, const char *fmt, va_list va);

/* Private functions */

/**
**---------------------------------------------------------------------------
**  Abstract: Convert ascii to integer
**  Returns:  unsigned int
**---------------------------------------------------------------------------
*/
unsigned int ts_atoi(const char **buf, int base)
{
	int mul = 1;
	unsigned int d = 0;

	while(mul != 0)
	{
		if((**buf <= '9') && (**buf >= '0'))
		{
			d = d*mul;
			d += (*((*buf)++) - '0');
			mul = base;
		}
		else if((base == 16) && (((**buf <= 'F') && (**buf >= 'A')) || ((**buf >= 'a') && (**buf <= 'f'))))
		{
			d = d*mul;
			if(**buf >= 'a')
				d += (*((*buf)++) - ('a'-10));
			else
				d += (*((*buf)++) - ('A'-10));
			mul = base;
		}else
		{
			mul = 0;
		}
	}

	return d;
}

/**
**---------------------------------------------------------------------------
**  Abstract: Writes buffer to arguments va list according to format fmt
**  Returns:  Number of variables read from buffer
**---------------------------------------------------------------------------
*/
int ts_format_va_list(const char *buf, const char *fmt, va_list va)
{
	int read_val = 0;
	uint32_t lenght = 0;
	while(*fmt)
	{
		/* Character needs formating? */
		if (*fmt == '%')
		{
			switch (*(++fmt))
			{
			case 'c':
			  	{
			  		int *c_val = va_arg(va, int*);
					*c_val = *buf++;
        		}
				break;
			case 'd':
			case 'i':
				{
					signed int *i_val = va_arg(va, signed int*);
					signed int i_valn = 1;
					if (*buf == '-')
					{
						i_valn *= -1;
						buf++;
					}
					if(*buf != '\0')
						*i_val = ts_atoi(&buf, 10)*i_valn;
				}
				break;
			#ifdef __GNUC__
			case '0' ... '9': /* Exclusive to the GNU C compiler */
			#else
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			#endif
				lenght = ts_atoi(&fmt, 10) - 1;
			case 's':
				{
					char *arg = va_arg(va, char *);
					char s_val = '%';
					uint32_t s_pos = 0;
					if(*(fmt + 1) != '\0')
						s_val = *(fmt + 1);
					if(s_val == '%')
					{
						while (*buf && ((lenght == 0) || (s_pos <= lenght)))
							arg[s_pos++] = *buf++;
					}else
					{
						while ((*buf) && (*buf != s_val) && ((lenght == 0) || (s_pos <= lenght)))
							arg[s_pos++] = *buf++;
						while(*buf != s_val)
							*buf++;
					}
					arg[s_pos] = '\0';
				}
				break;
			case 'u':
				{
					unsigned int *u_val = va_arg(va, unsigned int*);
					if(*buf != '\0')
						*u_val = ts_atoi(&buf, 10);
				}	
				break;
			case 'x':
			case 'X':
			  	{
					unsigned int *x_val = va_arg(va, unsigned int*);
					if(*buf != '\0')	
						*x_val = ts_atoi(&buf, 16);
			  	}
				break;
			case '*':
				{
					*fmt++;
					if(*fmt++ == '[')
					{
						while(*buf == *fmt)
							*buf++;
						*fmt++;
					}
					read_val--;
				}
			case '%':
				break;
			}
			fmt++;
			read_val++;
		}
		/* Else just traverse the buffer */
		else
		{
			buf++;
			fmt++;
		}
	}
	return (int)(read_val);
}

/**
**===========================================================================
**  Abstract: Loads data from the given character string and writes them to the
**            given variables according to the format parameter.
**  Returns:  Number of variables written to the list
**===========================================================================
*/
int siscanf(const char *str, const char *fmt, ...)
{
	int read_variables;
	va_list va;
	va_start(va, fmt);
	read_variables = ts_format_va_list(str, fmt, va);
	va_end(va);
	return read_variables;
}

/**
**===========================================================================
**  Abstract: Loads data from the standard input and writes it's data to the
**            given variables according to the format parameter.
**  Returns:  Number of variables written to the list
**
**===========================================================================
*/
int iscanf(const char *fmt, ...)
{
	int read_variables = 0;
	char *buf;
	va_list va;
	va_start(va, fmt);
	_read(1, &buf, 1);
	read_variables = ts_format_va_list(buf, fmt, va);
	va_end(va);
	return read_variables;
}