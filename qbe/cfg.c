#include "all.h"

Blk *
qbe_newblk()
{
	static Blk z;
	Blk *b;

	b = qbe_alloc(sizeof *b);
	*b = z;
	b->ins = qbe_vnew(0, sizeof b->ins[0], PFn);
	b->pred = qbe_vnew(0, sizeof b->pred[0], PFn);
	return b;
}

static void
fixphis(Fn *f)
{
	Blk *b;
	Phi *p;
	uint n, n0;

	for (b=f->start; b; b=b->link) {
		assert(b->id < f->nblk);
		for (p=b->phi; p; p=p->link) {
			for (n=n0=0; n<p->narg; n++)
				if (p->blk[n]->id != -1u) {
					p->blk[n0] = p->blk[n];
					p->arg[n0] = p->arg[n];
					n0++;
				}
			assert(n0 > 0);
			p->narg = n0;
		}
	}
}

static void
addpred(Blk *bp, Blk *b)
{
	qbe_vgrow(&b->pred, ++b->npred);
	b->pred[b->npred-1] = bp;
}

void
qbe_fillpreds(Fn *f)
{
	Blk *b;

	for (b=f->start; b; b=b->link)
		b->npred = 0;
	for (b=f->start; b; b=b->link) {
		if (b->s1)
			addpred(b, b->s1);
		if (b->s2 && b->s2 != b->s1)
			addpred(b, b->s2);
	}
}

static void
porec(Blk *b, uint *npo)
{
	Blk *s1, *s2;

	if (!b || b->id != -1u)
		return;
	b->id = 0; /* marker */
	s1 = b->s1;
	s2 = b->s2;
	if (s1 && s2 && s1->loop > s2->loop) {
		s1 = b->s2;
		s2 = b->s1;
	}
	porec(s1, npo);
	porec(s2, npo);
	b->id = (*npo)++;
}

static void
fillrpo(Fn *f)
{
	Blk *b, **p;

	for (b=f->start; b; b=b->link)
		b->id = -1u;
	f->nblk = 0;
	porec(f->start, &f->nblk);
	qbe_vgrow(&f->rpo, f->nblk);
	for (p=&f->start; (b=*p);) {
		if (b->id == -1u) {
			*p = b->link;
		} else {
			b->id = f->nblk-b->id-1;
			f->rpo[b->id] = b;
			p = &b->link;
		}
	}
}

/* fill rpo, preds; prune dead blks */
void
qbe_fillcfg(Fn *f)
{
	fillrpo(f);
	qbe_fillpreds(f);
	fixphis(f);
}

/* for dominators computation, read
 * "A Simple, Fast Dominance Algorithm"
 * by K. Cooper, T. Harvey, and K. Kennedy.
 */

static Blk *
inter(Blk *b1, Blk *b2)
{
	Blk *bt;

	if (b1 == 0)
		return b2;
	while (b1 != b2) {
		if (b1->id < b2->id) {
			bt = b1;
			b1 = b2;
			b2 = bt;
		}
		while (b1->id > b2->id) {
			b1 = b1->idom;
			assert(b1);
		}
	}
	return b1;
}

void
qbe_filldom(Fn *fn)
{
	Blk *b, *d;
	int ch;
	uint n, p;

	for (b=fn->start; b; b=b->link) {
		b->idom = 0;
		b->dom = 0;
		b->dlink = 0;
	}
	do {
		ch = 0;
		for (n=1; n<fn->nblk; n++) {
			b = fn->rpo[n];
			d = 0;
			for (p=0; p<b->npred; p++)
				if (b->pred[p]->idom
				||  b->pred[p] == fn->start)
					d = inter(d, b->pred[p]);
			if (d != b->idom) {
				ch++;
				b->idom = d;
			}
		}
	} while (ch);
	for (b=fn->start; b; b=b->link)
		if ((d=b->idom)) {
			assert(d != b);
			b->dlink = d->dom;
			d->dom = b;
		}
}

int
qbe_sdom(Blk *b1, Blk *b2)
{
	assert(b1 && b2);
	if (b1 == b2)
		return 0;
	while (b2->id > b1->id)
		b2 = b2->idom;
	return b1 == b2;
}

int
qbe_dom(Blk *b1, Blk *b2)
{
	return b1 == b2 || qbe_sdom(b1, b2);
}

static void
addfron(Blk *a, Blk *b)
{
	uint n;

	for (n=0; n<a->nfron; n++)
		if (a->fron[n] == b)
			return;
	if (!a->nfron)
		a->fron = qbe_vnew(++a->nfron, sizeof a->fron[0], PFn);
	else
		qbe_vgrow(&a->fron, ++a->nfron);
	a->fron[a->nfron-1] = b;
}

/* fill the dominance frontier */
void
qbe_fillfron(Fn *fn)
{
	Blk *a, *b;

	for (b=fn->start; b; b=b->link)
		b->nfron = 0;
	for (b=fn->start; b; b=b->link) {
		if (b->s1)
			for (a=b; !qbe_sdom(a, b->s1); a=a->idom)
				addfron(a, b->s1);
		if (b->s2)
			for (a=b; !qbe_sdom(a, b->s2); a=a->idom)
				addfron(a, b->s2);
	}
}

static void
loopmark(Blk *hd, Blk *b, void f(Blk *, Blk *))
{
	uint p;

	if (b->id < hd->id || b->visit == hd->id)
		return;
	b->visit = hd->id;
	f(hd, b);
	for (p=0; p<b->npred; ++p)
		loopmark(hd, b->pred[p], f);
}

void
qbe_loopiter(Fn *fn, void f(Blk *, Blk *))
{
	uint n, p;
	Blk *b;

	for (b=fn->start; b; b=b->link)
		b->visit = -1u;
	for (n=0; n<fn->nblk; ++n) {
		b = fn->rpo[n];
		for (p=0; p<b->npred; ++p)
			if (b->pred[p]->id >= n)
				loopmark(b, b->pred[p], f);
	}
}

/* dominator tree depth */
void
qbe_filldepth(Fn *fn)
{
	Blk *b, *d;
	int depth;

	for (b=fn->start; b; b=b->link)
		b->depth = -1;

	fn->start->depth = 0;

	for (b=fn->start; b; b=b->link) {
		if (b->depth != -1)
			continue;
		depth = 1;
		for (d=b->idom; d->depth==-1; d=d->idom)
			depth++;
		depth += d->depth;
		b->depth = depth;
		for (d=b->idom; d->depth==-1; d=d->idom)
			d->depth = --depth;
	}
}

/* least common ancestor in dom tree */
Blk *
qbe_lca(Blk *b1, Blk *b2)
{
	if (!b1)
		return b2;
	if (!b2)
		return b1;
	while (b1->depth > b2->depth)
		b1 = b1->idom;
	while (b2->depth > b1->depth)
		b2 = b2->idom;
	while (b1 != b2) {
		b1 = b1->idom;
		b2 = b2->idom;
	}
	return b1;
}

void
multloop(Blk *hd, Blk *b)
{
	(void)hd;
	b->loop *= 10;
}

void
qbe_fillloop(Fn *fn)
{
	Blk *b;

	for (b=fn->start; b; b=b->link)
		b->loop = 1;
	qbe_loopiter(fn, multloop);
}

static void
uffind(Blk **pb, Blk **uf)
{
	Blk **pb1;

	pb1 = &uf[(*pb)->id];
	if (*pb1) {
		uffind(pb1, uf);
		*pb = *pb1;
	}
}

/* requires rpo and no phis, breaks cfg */
void
qbe_simpljmp(Fn *fn)
{

	Blk **uf; /* union-find */
	Blk **p, *b, *ret;

	ret = qbe_newblk();
	ret->id = fn->nblk++;
	ret->jmp.type = Jret0;
	uf = qbe_emalloc(fn->nblk * sizeof uf[0]);
	for (b=fn->start; b; b=b->link) {
		assert(!b->phi);
		if (b->jmp.type == Jret0) {
			b->jmp.type = Jjmp;
			b->s1 = ret;
		}
		if (b->nins == 0)
		if (b->jmp.type == Jjmp) {
			uffind(&b->s1, uf);
			if (b->s1 != b)
				uf[b->id] = b->s1;
		}
	}
	for (p=&fn->start; (b=*p); p=&b->link) {
		if (b->s1)
			uffind(&b->s1, uf);
		if (b->s2)
			uffind(&b->s2, uf);
		if (b->s1 && b->s1 == b->s2) {
			b->jmp.type = Jjmp;
			b->s2 = 0;
		}
	}
	*p = ret;
	free(uf);
}

static int
reachrec(Blk *b, Blk *to)
{
	if (b == to)
		return 1;
	if (!b || b->visit)
		return 0;

	b->visit = 1;
	if (reachrec(b->s1, to))
		return 1;
	if (reachrec(b->s2, to))
		return 1;

	return 0;
}

/* Blk.visit needs to be clear at entry */
int
qbe_reaches(Fn *fn, Blk *b, Blk *to)
{
	int r;

	assert(to);
	r = reachrec(b, to);
	for (b=fn->start; b; b=b->link)
		b->visit = 0;
	return r;
}

/* can b reach 'to' not through excl
 * Blk.visit needs to be clear at entry */
int
qbe_reachesnotvia(Fn *fn, Blk *b, Blk *to, Blk *excl)
{
	excl->visit = 1;
	return qbe_reaches(fn, b, to);
}
