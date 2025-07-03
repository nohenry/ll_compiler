#include "all.h"
#include "config.h"
#include <ctype.h>
#include <getopt.h>

Target T;

char debug['Z'+1] = {
	['P'] = 0, /* parsing */
	['M'] = 0, /* memory optimization */
	['N'] = 0, /* ssa construction */
	['C'] = 0, /* copy elimination */
	['F'] = 0, /* constant folding */
	['A'] = 0, /* abi lowering */
	['I'] = 0, /* instruction selection */
	['L'] = 0, /* liveness */
	['S'] = 0, /* spilling */
	['R'] = 0, /* reg. allocation */
};

extern Target T_amd64_sysv;
extern Target T_amd64_apple;
extern Target T_arm64;
extern Target T_arm64_apple;
extern Target T_rv64;

static Target *tlist[] = {
	&T_amd64_sysv,
	&T_amd64_apple,
	&T_arm64,
	&T_arm64_apple,
	&T_rv64,
	0
};
static FILE *outf;
static int dbg;

static void
data(Dat *d)
{
	if (dbg)
		return;
	qbe_emitdat(d, outf);
	if (d->type == DEnd) {
		fputs("/* end data */\n\n", outf);
		qbe_freeall();
	}
}

static void
func(Fn *fn)
{
	uint n;

	if (dbg)
		fprintf(stderr, "**** Function %s ****", fn->name);
	if (debug['P']) {
		fprintf(stderr, "\n> After parsing:\n");
		qbe_printfn(fn, stderr);
	}
	T.abi0(fn);
	qbe_fillcfg(fn);
	qbe_filluse(fn);
	qbe_promote(fn);
	qbe_filluse(fn);
	qbe_ssa(fn);
	qbe_filluse(fn);
	qbe_ssacheck(fn);
	qbe_fillalias(fn);
	qbe_loadopt(fn);
	qbe_filluse(fn);
	qbe_fillalias(fn);
	qbe_coalesce(fn);
	qbe_filluse(fn);
	qbe_filldom(fn);
	qbe_ssacheck(fn);
	qbe_gvn(fn);
	qbe_fillcfg(fn);
	qbe_filluse(fn);
	qbe_filldom(fn);
	qbe_gcm(fn);
	qbe_filluse(fn);
	qbe_ssacheck(fn);
	T.abi1(fn);
	qbe_simpl(fn);
	qbe_fillcfg(fn);
	qbe_filluse(fn);
	T.isel(fn);
	qbe_fillcfg(fn);
	qbe_filllive(fn);
	qbe_fillloop(fn);
	qbe_fillcost(fn);
	qbe_spill(fn);
	qbe_rega(fn);
	qbe_fillcfg(fn);
	qbe_simpljmp(fn);
	qbe_fillcfg(fn);
	assert(fn->rpo[0] == fn->start);
	for (n=0;; n++)
		if (n == fn->nblk-1) {
			fn->rpo[n]->link = 0;
			break;
		} else
			fn->rpo[n]->link = fn->rpo[n+1];
	if (!dbg) {
		T.emitfn(fn, outf);
		fprintf(outf, "/* end function %s */\n\n", fn->name);
	} else
		fprintf(stderr, "\n");
	qbe_freeall();
}

static void
dbgfile(char *fn)
{
	qbe_emitdbgfile(fn, outf);
}

int
main(int ac, char *av[])
{
	Target **t;
	FILE *inf, *hf;
	char *f, *sep;
	int c;

	T = Deftgt;
	outf = stdout;
	while ((c = getopt(ac, av, "hd:o:t:")) != -1)
		switch (c) {
		case 'd':
			for (; *optarg; optarg++)
				if (isalpha(*optarg)) {
					debug[toupper(*optarg)] = 1;
					dbg = 1;
				}
			break;
		case 'o':
			if (strcmp(optarg, "-") != 0) {
				outf = fopen(optarg, "w");
				if (!outf) {
					fprintf(stderr, "cannot open '%s'\n", optarg);
					exit(1);
				}
			}
			break;
		case 't':
			if (strcmp(optarg, "?") == 0) {
				puts(T.name);
				exit(0);
			}
			for (t=tlist;; t++) {
				if (!*t) {
					fprintf(stderr, "unknown target '%s'\n", optarg);
					exit(1);
				}
				if (strcmp(optarg, (*t)->name) == 0) {
					T = **t;
					break;
				}
			}
			break;
		case 'h':
		default:
			hf = c != 'h' ? stderr : stdout;
			fprintf(hf, "%s [OPTIONS] {file.ssa, -}\n", av[0]);
			fprintf(hf, "\t%-11s prints this help\n", "-h");
			fprintf(hf, "\t%-11s output to file\n", "-o file");
			fprintf(hf, "\t%-11s generate for a target among:\n", "-t <target>");
			fprintf(hf, "\t%-11s ", "");
			for (t=tlist, sep=""; *t; t++, sep=", ") {
				fprintf(hf, "%s%s", sep, (*t)->name);
				if (*t == &Deftgt)
					fputs(" (default)", hf);
			}
			fprintf(hf, "\n");
			fprintf(hf, "\t%-11s dump debug information\n", "-d <flags>");
			exit(c != 'h');
		}

	do {
		f = av[optind];
		if (!f || strcmp(f, "-") == 0) {
			inf = stdin;
			f = "-";
		} else {
			inf = fopen(f, "r");
			if (!inf) {
				fprintf(stderr, "cannot open '%s'\n", f);
				exit(1);
			}
		}
		qbe_parse(inf, f, dbgfile, data, func);
		fclose(inf);
	} while (++optind < ac);

	if (!dbg)
		T.emitfin(outf);

	exit(0);
}
