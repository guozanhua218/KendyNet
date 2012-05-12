#include "rpacket.h"
#include "wpacket.h"
#include <stdlib.h>
#include <string.h>

rpacket_t rpacket_create(buffer_t b,unsigned long pos/*������b�е���ʼ�±�*/)
{
	rpacket_t r = calloc(sizeof(*r),1);
	r->binbuf = 0;
	r->binbufpos = 0;
	r->buf = buffer_acquire(0,b);
	r->readbuf = buffer_acquire(0,b);
	r->len = *(unsigned long*)(&(b->buf[pos]));
	r->data_remain = r->len;
	r->rpos = pos + sizeof(r->len);
	return r;
}

rpacket_t rpacket_create_by_wpacket(struct wpacket *w)
{
	rpacket_t r = calloc(sizeof(*r),1);
	r->binbuf = 0;
	r->binbufpos = 0;
	r->buf = buffer_acquire(0,w->buf);
	r->readbuf = buffer_acquire(0,w->buf);
	r->len = *(unsigned long*)(&(w->buf->buf[0]));
	r->data_remain = r->len;
	r->rpos = 0 + sizeof(r->len);
	return r;
}

void      rpacket_destroy(rpacket_t *r)
{
	//�ͷ����ж�buffer_t������
	buffer_release(&(*r)->buf);
	buffer_release(&(*r)->readbuf);
	buffer_release(&(*r)->binbuf);
}

unsigned long  rpacket_read_cmd(rpacket_t r)
{
	return r->cmd;
}

unsigned long  rpacket_len(rpacket_t r)
{
	return r->len;
}

unsigned long  rpacket_data_remain(rpacket_t r)
{
	return r->data_remain;
}

static int rpacket_read(rpacket_t r,char *out,unsigned long size)
{
	buffer_t _next = 0;
	if(r->data_remain < size)
		return -1;
	while(size>0)
	{
		unsigned long copy_size = r->readbuf->size - r->rpos;
		copy_size = copy_size >= size ? size:copy_size;
		memcpy(out,r->readbuf->buf + r->rpos,copy_size);
		size -= copy_size;
		r->rpos += copy_size;
		r->data_remain -= copy_size;
		out += copy_size;
		if(r->rpos >= r->readbuf->size && r->data_remain)
		{
			//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
			r->rpos = 0;
			r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
		}
	}
	return 0;
}

unsigned char  rpacket_read_char(rpacket_t r)
{
	unsigned char value = 0;
	rpacket_read(r,(char*)&value,sizeof(value));
	return value;
}

unsigned short rpacket_read_short(rpacket_t r)
{
	unsigned short value = 0;
	rpacket_read(r,(char*)&value,sizeof(value));
	return value;
}

unsigned long  rpacket_read_long(rpacket_t r)
{
	unsigned long value = 0;
	rpacket_read(r,(char*)&value,sizeof(value));
	return value;
}

double   rpacket_read_double(rpacket_t r)
{
	double value = 0;
	rpacket_read(r,(char*)&value,sizeof(value));
	return value;
}

const char* rpacket_read_string(rpacket_t r)
{
	unsigned long len = 0;
	return (const char *)rpacket_read_binary(r,&len);
}

const void* rpacket_read_binary(rpacket_t r,unsigned long *len)
{
	void *addr = 0;
	unsigned long size = rpacket_read_long(r);
	*len = size;
	if(r->data_remain < size)
		return addr;
	if(r->buf->size - r->rpos >= size)
	{
		addr = &r->buf[r->rpos];
		r->rpos += size;
		r->data_remain -= size;
		if(r->rpos >= r->readbuf->size && r->data_remain)
		{
			//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
			r->rpos = 0;
			r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
		}
	}
	else
	{
		//���ݿ�Խ��buffer�߽�,����binbuf,�����ݿ�����binbuf��
		if(!r->binbuf)
		{
			r->binbufpos = 0;
			r->binbuf = buffer_create_and_acquire(0,r->len);
		}
		addr = r->binbuf->buf + r->binbufpos;
		while(size)
		{
			unsigned long copy_size = r->readbuf->size - r->rpos;
			copy_size = copy_size >= size ? size:copy_size;
			memcpy(r->binbuf->buf + r->binbufpos,r->readbuf->buf + r->rpos,copy_size);
			size -= copy_size;
			r->rpos += copy_size;
			r->data_remain -= copy_size;
			r->binbufpos += copy_size;		
			if(r->rpos >= r->readbuf->size && r->data_remain)
			{
				//��ǰbuffer�����Ѿ�������,�л�����һ��buffer
				r->rpos = 0;
				r->readbuf = buffer_acquire(r->readbuf,r->readbuf->next);
			}
		}

	}
	return addr;
}

