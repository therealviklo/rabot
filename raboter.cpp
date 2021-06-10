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