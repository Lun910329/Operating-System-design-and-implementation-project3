
#ifndef __SYSCL_FS_H__
#define __SYSCL_FS_H__


#include <sys/cdefs.h>
#include <sys/types.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for strcmp

#include <sys/stat.h> // for struct stat

#include        <limits.h> // for wcrtomb

#include <wchar.h>   // for wcrtomb
#include <wctype.h>  // for iswprint


#ifndef NULL
#define NULL 0
#endif 

#define	BY_NAME 0

//
// define kDEBUG, 1 = turn on
//              , 0 = turn off(default) *
//
#define kDEBUG 0

#if kDEBUG
#define DEBUG_PREFIX "Debug: "
#define DBG(arg...) do { printf(DEBUG_PREFIX arg); } while(0)
#else
#define DBG(arg...) do {} while(0)
#endif

#define kRETSUCCESS 0
#define kRETFAILURE 1

#define kTRUE 		1
#define kFALSE 		0


#ifndef howmany
#define	howmany(x, y)	(((x)+((y)-1))/(y))
#endif

int termwidth = 80;		/* default terminal width */

// function constant declare here
/* flags */
int f_accesstime;		/* use time of last access */
int f_column;			/* columnated format */
int f_columnacross;		/* columnated format, sorted across */
int f_flags;			/* show flags associated with a file */
int f_grouponly;		/* long listing without owner */
int f_humanize;			/* humanize the size field */
int f_commas;           /* separate size field with comma */
int f_inode = 1;			/* print inode */
int f_listdir;			/* list actual directory, not contents */
int f_listdot;			/* list files beginning with . */
int f_longform;			/* long listing format */
int f_nonprint;			/* show unprintables as ? */
int f_nosort;			/* don't sort output */
int f_numericonly;		/* don't convert uid/gid to name */
int f_octal;			/* print octal escapes for nongraphic characters */
int f_octal_escape;		/* like f_octal but use C escapes if possible */
int f_recursive = 1;		/* ls subdirectories also */
int f_reversesort;		/* reverse whatever sort is used */
int f_sectime;			/* print the real time for all files */
int f_singlecol;		/* use single column output */
int f_size;			/* list size in short listing */
int f_statustime;		/* use time of last mode change */
int f_stream;			/* stream format */
int f_type;			/* add type character for non-regular files */
int f_typedir;			/* add type character for directories */
int f_whiteout;			/* show whiteout entries */

// end of constant declare

#define NO_PRINT	1

typedef struct {
	FTSENT *list;
	u_int64_t btotal;
	u_int64_t stotal;
	int entries;
	unsigned int maxlen;
	int s_block;
	int s_flags;
	int s_group;
	int s_inode;
	int s_nlink;
	int s_size;
	int s_user;
	int s_major;
	int s_minor;
} DISPLAY;

typedef struct {
	char *user;
	char *group;
	char *flags;
	char data[1];
} NAMES;

#define	IS_NOPRINT(p)	((p)->fts_number == NO_PRINT)

int namecmp(const FTSENT *a, const FTSENT *b)
{
	DBG("namecmp() ===>\n");
	return (strcmp(a->fts_name, b->fts_name));
	DBG("namecmp() <===\n");
}

typedef __mbstate_t mbstate_t;

static int printwc(wchar_t wc, mbstate_t *pst)
{
	size_t size;
	char buf[MB_LEN_MAX];

	size = wcrtomb(buf, wc, pst);
	if (size == (size_t)-1) /* This shouldn't happen, but for sure */
		return 0;
	if (wc == L'\0') {
		/* The following condition must be always true, but for sure */
		if (size > 0 && buf[size - 1] == '\0')
			--size;
	}
	if (size > 0)
		fwrite(buf, 1, size, stdout);
	return wc == L'\0' ? 0 : wcwidth(wc);
}


int printescaped(const char *src)
{
	int n = 0;
	mbstate_t src_state, stdout_state;
	/* The following +1 is to pass '\0' at the end of src to mbrtowc(). */
	const char *endptr = src + strlen(src) + 1;

	/*
	 * We have to reset src_state each time in this function, because
	 * the codeset of src pathname may not match with current locale.
	 * Note that if we pass NULL instead of src_state to mbrtowc(),
	 * there is no way to reset the state.
	 */
	memset(&src_state, 0, sizeof(src_state));
	memset(&stdout_state, 0, sizeof(stdout_state));
	while (src < endptr) {
		wchar_t wc;
		size_t rv, span = endptr - src;

		rv = mbrtowc(&wc, src, span, &src_state);
		if (rv == 0) { /* assert(wc == L'\0'); */
			/* The following may output a shift sequence. */
			n += printwc(wc, &stdout_state);
			break;
		}
		if (rv == (size_t)-1) { /* probably errno == EILSEQ */
			n += printwc(L'?', &stdout_state);
			/* try to skip 1byte, because there is no better way */
			src++;
			memset(&src_state, 0, sizeof(src_state));
		} else if (rv == (size_t)-2) {
			if (span < MB_CUR_MAX) { /* incomplete char */
				n += printwc(L'?', &stdout_state);
				break;
			}
			src += span; /* a redundant shift sequence? */
		} else {
			n += printwc(iswprint(wc) ? wc : L'?', &stdout_state);
			src += rv;
		}
	}
	return n;
}

static int mastercmp(const FTSENT **a, const FTSENT **b);
static void display(FTSENT *p, FTSENT *list);

static int printaname(FTSENT *p, int inodefield, int sizefield)
{
	struct stat *sp;
	int chcnt;
	char szbuf[5];

	sp = p->fts_statp;
	chcnt = 0;
	if (f_inode)
	{
		DBG("f_inode ===>\n");
		DBG("inodefield = %*lu\n", inodefield, (unsigned long)sp->st_ino);
		chcnt += printf("%*lu ", inodefield, (unsigned long)sp->st_ino);
	}

	/*
	if (f_size) {
		if (f_humanize) {
			if ((humanize_number(szbuf, sizeof(szbuf), sp->st_size,
			    "", HN_AUTOSCALE,
			    (HN_DECIMAL | HN_B | HN_NOSPACE))) == -1)
				err(1, "humanize_number");
			chcnt += printf("%*s ", sizefield, szbuf);
		} else {
			chcnt += printf(f_commas ? "%'*llu " : "%*llu ",
			    sizefield, (unsigned long long)
			    howmany(sp->st_blocks, blocksize));
		}
	}
	*/
	//if (f_octal || f_octal_escape)
		//chcnt += safe_print(p->fts_name);
	if (f_nonprint)
		chcnt += printescaped(p->fts_name);
	else
		chcnt += printf("%s", p->fts_name);
	//if (f_type || (f_typedir && S_ISDIR(sp->st_mode)))
		//chcnt += printtype(sp->st_mode);
	return (chcnt);
}

void printscol(DISPLAY *dp)
{
	DBG("printscol() ===>\n");
	FTSENT *p;

	for (p = dp->list; p; p = p->fts_link) 
	{
		if (IS_NOPRINT(p))
		{
			continue;
		}
		DBG("dp->s_inode = %d\n", dp->s_inode);
		(void)printaname(p, dp->s_inode, dp->s_block);
		(void)putchar('\n');
	}

	DBG("printscol() <===\n");
}

static int output;			/* If anything output. */

static void traverse(int argc, char *argv[], int options)
{
	DBG("traverse() ===>\n");
	FTS *ftsp;
	FTSENT *p, *chp;
	int ch_options, error;

	if ((ftsp =
	    fts_open(argv, options, f_nosort ? NULL : mastercmp)) == NULL)
		//err(EXIT_FAILURE, NULL);

	display(NULL, fts_children(ftsp, 0));
	if (f_listdir) {
		(void)fts_close(ftsp);
		return;
	}

	/*
	 * If not recursing down this tree and don't need stat info, just get
	 * the names.
	 */
	ch_options = !f_recursive && options & FTS_NOSTAT ? FTS_NAMEONLY : 0;

	while ((p = fts_read(ftsp)) != NULL)
		switch (p->fts_info) {
		case FTS_DC:
			//warnx("%s: directory causes a cycle", p->fts_name);
			break;
		case FTS_DNR:
		case FTS_ERR:
			//warnx("%s: %s", p->fts_name, strerror(p->fts_errno));
			//rval = EXIT_FAILURE;
			break;
		case FTS_D:
			if (p->fts_level != FTS_ROOTLEVEL &&
			    p->fts_name[0] == '.' && !f_listdot)
				break;

			/*
			 * If already output something, put out a newline as
			 * a separator.  If multiple arguments, precede each
			 * directory with its name.
			 */
			if (output)
				(void)printf("\n%s:\n", p->fts_path);
			else if (argc > 1) {
				(void)printf("%s:\n", p->fts_path);
				output = 1;
			}

			chp = fts_children(ftsp, ch_options);
			display(p, chp);

			if (!f_recursive && chp != NULL)
				(void)fts_set(ftsp, p, FTS_SKIP);
			break;
		}
	//error = errno;
	(void)fts_close(ftsp);
	//errno = error;
	//if (errno)
	//	err(EXIT_FAILURE, "fts_read");
	DBG("traverse() <===\n");
}


// function called by pointer
static void (*printfcn)(DISPLAY *);
static int (*sortfcn)(const FTSENT *, const FTSENT *);
void printcol(DISPLAY *dp);




long blocksize;			/* block size units */



static void display(FTSENT *p, FTSENT *list)
{
	DBG("display() ===>\n");
	struct stat *sp;
	DISPLAY d;
	FTSENT *cur;
	NAMES *np;
	u_int64_t btotal, stotal;
	off_t maxsize;
	blkcnt_t maxblock;
	ino_t maxinode;
	int maxmajor, maxminor;
	uint32_t maxnlink;
	int bcfile, entries, flen, glen, ulen, maxflags, maxgroup;
	unsigned int maxlen;
	int maxuser, needstats;
	const char *user, *group;
	char buf[21];		/* 64 bits == 20 digits, +1 for NUL */
	char nuser[12], ngroup[12];
	char *flags = NULL;

	if (list == NULL)
		return;

	needstats = 1;
	flen = 0;
	maxinode = maxnlink = 0;
	bcfile = 0;
	maxuser = maxgroup = maxflags = maxlen = 0;
	btotal = stotal = maxblock = maxsize = 0;
	maxmajor = maxminor = 0;

	for (cur = list, entries = 0; cur; cur = cur->fts_link) 
	{
		DBG("inside loop ===>\n");
		/*
		 * P is NULL if list is the argv list, to which different rules
		 * apply.
		 */
		if (p == NULL) 
		{
			DBG("p == NULl ===>\n");
			/* Directories will be displayed later. */
			if (cur->fts_info == FTS_D && !f_listdir) 
			{
				cur->fts_number = NO_PRINT;
				continue;
			}
			DBG("p == NULL <===\n");
		} 
		else 
		{
			DBG("p != NULL ===>\n");
			/* Only display dot file if -a/-A set. */
			if (cur->fts_name[0] == '.' && !f_listdot) 
			{
				cur->fts_number = NO_PRINT;
				continue;
			}
			DBG("p != NULL <===\n");
		}
		if (cur->fts_namelen > maxlen)
		{
			DBG("cur-fts_namelen = %d, maxlen = %d\n", cur->fts_namelen, maxlen);
			maxlen = cur->fts_namelen;
		}

		if (needstats) 
		{
			DBG("needstats ===>\n");
			sp = cur->fts_statp;
			DBG("sp = cur->fts_statp\n");
			
			if (sp->st_blocks > maxblock)
			{
				DBG("sp->st_blocks = %llu, maxblock = %llu\n", sp->st_blocks, maxblock);
				maxblock = sp->st_blocks;
			}
			
			if (sp->st_ino > maxinode)
			{
				DBG("maxinode is %d\n", maxinode);
				maxinode = sp->st_ino;
			}
			
			if (sp->st_nlink > maxnlink)
			{
				maxnlink = sp->st_nlink;
			}
			if (sp->st_size > maxsize)
			{
				maxsize = sp->st_size;
			}

			
			if (S_ISCHR(sp->st_mode) || S_ISBLK(sp->st_mode)) 
			{
				bcfile = 1;
				if (major(sp->st_rdev) > maxmajor)
					maxmajor = major(sp->st_rdev);
				if (minor(sp->st_rdev) > maxminor)
					maxminor = minor(sp->st_rdev);
			}

			btotal += sp->st_blocks;
			stotal += sp->st_size;
		}
		++entries;
	}

	DBG("number of entries %d\n", entries);

	if (!entries)
		return;

	d.list = list;
	d.entries = entries;
	d.maxlen = maxlen;
	if (needstats) 
	{
		DBG("needstats\n");
		d.btotal = btotal;
		d.stotal = stotal;
		if (f_humanize) {
			DBG("f_humanize\n");
			d.s_block = 4; /* min buf length for humanize_number */
		} else {
			DBG("snprintf...\n");
			(void)snprintf(buf, sizeof(buf), "%llu",
			    (long long)howmany(maxblock, blocksize));
			DBG("d.s_block = %d\n", strlen(buf));
			d.s_block = strlen(buf);
			if (f_commas) /* allow for commas before every third digit */
			{
				DBG("f_commas\n");
				d.s_block += (d.s_block - 1) / 3;
			}
		}
		d.s_flags = maxflags;
		d.s_group = maxgroup;
		DBG("d.s_flags = %d, d.s_group = %d\n", d.s_flags, d.s_group);
		DBG("maxinode is %d\n", maxinode);
		(void)snprintf(buf, sizeof(buf), "%llu",
		    (unsigned long long)maxinode);
		d.s_inode = strlen(buf);
		DBG("d.s_inode = %d\n", d.s_inode);
		(void)snprintf(buf, sizeof(buf), "%u", maxnlink);
		d.s_nlink = strlen(buf);
		if (f_humanize) {
			DBG("f_humanize2\n");
			d.s_size = 4; /* min buf length for humanize_number */
		} else {
			DBG("not f_humanize2\n");
			DBG("snprintf2...\n");
			(void)snprintf(buf, sizeof(buf), "%llu",
			    (long long)maxsize);
			d.s_size = strlen(buf);
			DBG("d.s_size = %d\n", d.s_size);
			if (f_commas) /* allow for commas before every third digit */
			{
				DBG("f_commas2\n");
				d.s_size += (d.s_size - 1) / 3;
			}
		}
		d.s_user = maxuser;
		DBG("d.s_user = %d\n", d.s_user);
		if (bcfile) {
			DBG("bcfile ===>\n");
			(void)snprintf(buf, sizeof(buf), "%u", maxmajor);
			d.s_major = strlen(buf);
			(void)snprintf(buf, sizeof(buf), "%u", maxminor);
			d.s_minor = strlen(buf);
			if (d.s_major + d.s_minor + 2 > d.s_size)
			{
				d.s_size = d.s_major + d.s_minor + 2;
			}
			else if (d.s_size - d.s_minor - 2 > d.s_major)
			{
				d.s_major = d.s_size - d.s_minor - 2;
			}
			DBG("bcfile <===\n");
		} else {
			DBG("not bcfile ===>\n");
			d.s_major = 0;
			d.s_minor = 0;
			DBG("not bcfile <===\n");
		}
	}



	//printfcn(&d);
	printcol(&d);
	output = 1;
	DBG("display() <===\n");
}

static void	printtotal(DISPLAY *dp);

void printcol(DISPLAY *dp)
{
	DBG("printcol() ===>\n");
	static FTSENT **array;
	static int lastentries = -1;
	FTSENT *p;
	int base, chcnt, col, colwidth, num;
	int numcols, numrows, row;

	colwidth = dp->maxlen;
	if (f_inode)
		colwidth += dp->s_inode + 1;
	if (f_size) {
		if (f_humanize)
			colwidth += dp->s_size + 1;
		else
			colwidth += dp->s_block + 1;
	}
	if (f_type || f_typedir)
		colwidth += 1;

	colwidth += 1;

	if (termwidth < 2 * colwidth) 
	{
		printscol(dp);
		return;
	}

	/*
	 * Have to do random access in the linked list -- build a table
	 * of pointers.
	 */
	if (dp->entries > lastentries) {
		FTSENT **newarray;

		newarray = realloc(array, dp->entries * sizeof(FTSENT *));
		if (newarray == NULL) {
			//warn(NULL);
			printscol(dp);
			return;
		}
		lastentries = dp->entries;
		array = newarray;
	}
	for (p = dp->list, num = 0; p; p = p->fts_link)
		if (p->fts_number != NO_PRINT)
			array[num++] = p;

	numcols = termwidth / colwidth;
	colwidth = termwidth / numcols;		/* spread out if possible */
	numrows = num / numcols;
	if (num % numcols)
		++numrows;

	printtotal(dp);				/* "total: %u\n" */

	DBG("for row = 0 ===>\n");
	for (row = 0; row < numrows; ++row) {
		for (base = row, chcnt = col = 0; col < numcols; ++col) 
		{
			DBG("dp->s_inode = %d\n", dp->s_inode);
			chcnt = printaname(array[base], dp->s_inode,
			    f_humanize ? dp->s_size : dp->s_block);
			if ((base += numrows) >= num)
				break;
			while (chcnt++ < colwidth)
				(void)putchar(' ');
		}
		(void)putchar('\n');
	}
	DBG("end for row = %d\n", row);
	DBG("printcol() <===\n");
	return;
}


static void printtotal(DISPLAY *dp)
{
	DBG("printtotal() ===>\n");
	char szbuf[5];

	if (dp->list->fts_level != FTS_ROOTLEVEL && (f_longform || f_size)) 
	{
		if (f_humanize) 
		{
			if ((humanize_number(szbuf, sizeof(szbuf), (int64_t)dp->stotal,
			    "", HN_AUTOSCALE,
			    (HN_DECIMAL | HN_B | HN_NOSPACE))) == -1)
			{
				puts("error\n");
			}
				//err(1, "humanize_number");
			(void)printf("total %s\n", szbuf);
		} else {
			(void)printf(f_commas ? "total %'llu\n" :
			    "total %llu\n", (unsigned long long)
			    howmany(dp->btotal, blocksize));
		}
	}
	DBG("printtotal() <===\n");
}

#endif
