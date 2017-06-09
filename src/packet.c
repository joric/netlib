#ifndef __PACKET_H__
#define __PACKET_H__

typedef struct {
	char *buf;
	int ofs;
	int buf_size;
	int len;
} PACKET;

int packet_init(PACKET * m, char *buf, int buf_size)
{
	m->buf = buf;
	m->buf_size = buf_size;
	m->len = 0;
	m->ofs = 0;
}

int packet_write(PACKET * m, int v)
{
	if (m->ofs < m->buf_size - 1)
	{
		m->buf[m->ofs++] = v & 0xff;

		if (m->len < m->ofs)
			m->len++;

		return 1;
	}
	return 0;
}

unsigned long long packet_read(PACKET * m)
{
	return (m->ofs < m->buf_size - 1) ? m->buf[m->ofs++] & 0xff : 0;
}

#define w8(m, v) packet_write(m, v & 0xff)
#define w16(m, v) w8(m, v >> 8) + w8(m, v)
#define w32(m, v) w16(m, v >> 16) + w16(m, v)
#define w64(m, v) w32(m, v >> 32) + w32(m, v)
#define wBuf(m, buf, len) { int k = 0, i = 0; for (i = 0; i < len; i++) k += w8(m, buf[i]); }
#define r8(m) packet_read(m)
#define r16(m) (((r8(m) << 8) | r8(m)) & 0xffff)
#define r32(m) ((r16(m) << 16) | r16(m))
#define r64(m) ((r32(m) << 32) | r32(m))
#define rBuf(m, buf, len) { int i= 0; for (i = 0; i < len; i++) buf[i] = r8(m); }

#endif //__PACKET_H__
