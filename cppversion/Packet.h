namespace net
{
	struct Packet
	{
		unsigned char *buf;		
		int ofs;
		int size;
		int len;

		Packet(unsigned char *buf, int size)
		{
			this->buf = buf;
			this->size = size;
			len = 0;
			ofs = 0;
		}

		int write(int v)
		{
			if (ofs < size - 1)
			{
				buf[ofs++] = (unsigned char)(v & 0xff);

				if (len < ofs)
					len++;

				return 1;
			}
			return 0;
		}

		unsigned long long read()
		{
			return ofs < size - 1 ? (buf[ofs++] & 0xff) : 0;
		}

		int w8(unsigned char v)
		{
			return write(v & 0xff);
		}

		int w16(int v)
		{
			return w8(v >> 8) + w8(v);
		}

		int w32(int v)
		{
			return w16(v >> 16) + w16(v);
		}

		int w64(long long v)
		{
			return w32((int)(v >> 32)) + w32((int)v);
		}

		int wBuf(char *buf, int len)
		{
			int k = 0, i = 0;
			for (i = 0; i < len; i++)
				k += w8(buf[i]);
			return k;
		}

		unsigned char r8()
		{
			return (unsigned char) read();
		}

		int r16()
		{
			return (((r8() << 8) | r8()) & 0xffff);
		}

		int r32()
		{
			return ((r16() << 16) | r16());
		}

		long long r64()
		{
			return (((long long)(r32())) << 32) | r32();
		}

		int rBuf(char *buf, int len)
		{
			int i = 0;
			for (i = 0; i < len; i++)
				buf[i] = r8();
			return len;
		}
	};
}
