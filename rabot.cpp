#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstring>

#define BITS(n) ((unsigned)(~(((char)0x80) >> (7 - n))))

using Bytes = std::vector<uint8_t>;

constexpr unsigned qnote = 360;

unsigned numbits(unsigned x)
{
	unsigned n = 0;
	while (x >> n) n++;
	return n;
}

void writeVarNum(Bytes& b, unsigned n)
{
	const unsigned nbits = numbits(n);
	const unsigned nextrabytes = nbits > 8u ? (nbits - 9u) / 7u + 1u : 0u;
	for (unsigned i = 0; i < nextrabytes; i++)
	{
		b.push_back(0x80u | ((n >> ((nextrabytes - i - 1u) * 7u + 8u)) & BITS(7)));
	}
	b.push_back(n & 0xffu);
}

template <typename T>
void writeBigEndNum(Bytes& b, T n)
{
	for (unsigned i = 0; i < sizeof(T); i++)
	{
		b.push_back((n >> (8 * (sizeof(T) - 1 - i))) & 0xffu);
	}
}

struct Track
{
	Bytes b;

	void setTempo(unsigned delta, unsigned tempo)
	{
		writeVarNum(b, delta);
		b.push_back(0xff);
		b.push_back(0x51);
		b.push_back(0x03);
		b.push_back((tempo >> (8 * 2)) & 0xff);
		b.push_back((tempo >> (8 * 1)) & 0xff);
		b.push_back( tempo             & 0xff);
	}

	void noteOn(unsigned delta, unsigned ch, unsigned note, unsigned vel)
	{
		writeVarNum(b, delta);
		b.push_back(0b1001'0000u | (ch & BITS(4)));
		b.push_back(note & BITS(7));
		b.push_back(vel & BITS(7));
	}

	void noteOff(unsigned delta, unsigned ch, unsigned note)
	{
		writeVarNum(b, delta);
		b.push_back(0b1000'0000u | (ch & BITS(4)));
		b.push_back(note & BITS(7));
		b.push_back(0);
	}

	void playNoteFor(unsigned time, unsigned ch, unsigned note, unsigned vel)
	{
		noteOn(0, ch, note, vel);
		noteOff(time, ch, note);
	}
};

void writeMidiFile(Bytes& b, const Track& track)
{
	b.push_back('M');
	b.push_back('T');
	b.push_back('h');
	b.push_back('d');
	writeBigEndNum<uint32_t>(b, 6);
	writeBigEndNum<uint16_t>(b, 0);
	writeBigEndNum<uint16_t>(b, 1);
	writeBigEndNum<uint16_t>(b, qnote);

	b.push_back('M');
	b.push_back('T');
	b.push_back('r');
	b.push_back('k');
	writeBigEndNum<uint32_t>(b, track.b.size());
	const auto oldSize = b.size();
	b.resize(oldSize + track.b.size());
	std::memcpy(&b[oldSize], track.b.data(), track.b.size());
}

unsigned raboter(unsigned x)
{
	if (x == 0u) return 0u;

	unsigned op = 0u;
	unsigned c = 1u << (8u * sizeof(unsigned) - 1u);
	while (((c & x) == 0u) && c) c >>= 1u;

	unsigned bit = 1u;
	c >>= 1u;

	while (c)
	{
		if ((c & x) && bit)
		{
			op = (op << 1u) | 1u;
		}
		else if (!(c & x) && !bit)
		{
			op = op << 1u;
		}

		bit = c & x;
		c >>= 1u;
	}
	return op;
}

int main(int argc, char* argv[])
{
	unsigned long n = 0;
	std::string opname = "rabot.mid";
	unsigned tempo = 500000;
	for (int i = 1; i < argc; i++)
	{
		auto nextArg = [&]() -> bool {
			return ++i >= argc;
		};
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
				case 'h': goto help;
				case 'o':
				{
					if (nextArg()) goto help;
					opname = argv[i];
				}
				break;
				case 't':
				{
					if (nextArg()) goto help;
					tempo = 60000000 / std::strtoul(argv[i], nullptr, 0);
				}
				break;
				default: goto help;
			}
		}
		else 
		{
			n = std::strtoul(argv[i], nullptr, 0);
		}
	}
	if (n == 0)
	{
	help:
		std::puts("Användning: rabot [flaggor] n [flaggor]");
		std::puts("Flaggor:");
		std::puts("    -h  Visa hjälp");
		std::puts("    -o  Filnamn");
		std::puts("    -t  Tempo");
		return 0;
	}

	Track t;
	t.setTempo(0, tempo);
	for (unsigned long i = 0; i < n; i++)
	{
		t.playNoteFor(qnote, 0, (raboter(i) % 88) + 21, 100);
	}

	Bytes out;
	writeMidiFile(out, t);

	std::ofstream f(opname, std::ios::out | std::ios::binary);
	f.write((const char*)out.data(), out.size());
}