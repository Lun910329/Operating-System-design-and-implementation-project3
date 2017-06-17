#include "syscl_fs.h"


int sortkey = BY_NAME;
int fts_options = 17;



int main(int argc, char **argv)
{
	static char dot[] = ".", *dotav[] = { dot, NULL };
	struct stat *sp;

	//
	//fts_options = 25;

	// sort by name
	sortfcn = namecmp;
	//DISPLAY d;
	FTSENT *cur;
	//printf("blocksize is %lu\n", blocksize);
	(void)getbsize(NULL, &blocksize);
	//printf("blocksize is %lu\n", blocksize);
	blocksize /= 512;

	if (argc)
	{
		DBG("arg is %d, fts_options is %d\n", argc, fts_options);
		traverse(argc, argv, fts_options);
	}
	else
	{
		DBG("arg is %d, fts_options is %d\n", argc, fts_options);
		traverse(1, dotav, fts_options);
	}
	//printf("blocksize is %lu\n", blocksize);
	return 0;
}

static int mastercmp(const FTSENT **a, const FTSENT **b)
{
	int a_info, b_info;

	a_info = (*a)->fts_info;
	if (a_info == FTS_ERR)
		return (0);
	b_info = (*b)->fts_info;
	if (b_info == FTS_ERR)
		return (0);

	if (a_info == FTS_NS || b_info == FTS_NS) {
		if (b_info != FTS_NS)
			return (1);
		else if (a_info != FTS_NS)
			return (-1);
		else
			return (namecmp(*a, *b));
	}

	if (a_info != b_info && !f_listdir &&
	    (*a)->fts_level == FTS_ROOTLEVEL) {
		if (a_info == FTS_D)
			return (1);
		else if (b_info == FTS_D)
			return (-1);
	}
	return (sortfcn(*a, *b));
}