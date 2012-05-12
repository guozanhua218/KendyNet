#ifndef _BUFFER_H
#define _BUFFER_H
/*	
    Copyright (C) <2012>  <huangweilook@21cn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
* �����ü�����buffer
*/
typedef struct buffer
{
	long ref_count;
	unsigned long capacity;
	unsigned long size;
	struct buffer *next;
	char   buf[0];
}*buffer_t;


buffer_t buffer_create_and_acquire(buffer_t,unsigned long);
buffer_t buffer_acquire(buffer_t,buffer_t);
void     buffer_release(buffer_t*);

#endif