/*
 * patricia.h
 *
 * Patricia trie implementation.
 *
 * Functions for inserting nodes, removing nodes, and searching in
 * a Patricia trie designed for IP addresses and netmasks.  A
 * head node must be created with (key,mask) = (0,0).
 *
 * NOTE: The fact that we keep multiple masks per node makes this
 *       more complicated/computationally expensive then a standard
 *       trie.  This is because we need to do longest prefix matching,
 *       which is useful for computer networks, but not as useful
 *       elsewhere.
 *
 * Matthew Smart <mcsmart@eecs.umich.edu>
 *
 * Copyright (c) 2000
 * The Regents of the University of Michigan
 * All rights reserved
 *
 * $Id: patricia.h,v 1.1.1.1 2000/11/06 19:53:17 mguthaus Exp $
 */

#ifndef _PATRICIA_H_
#define _PATRICIA_H_


/*
 * Patricia tree mask.
 * Each node in the tree can contain multiple masks, so this
 * structure is where the mask and data are kept.
 */
struct ptree_mask {
	unsigned long pm_mask;
	void *pm_data;
};


/*
 * Patricia tree node.
 */
struct ptree {
	unsigned long p_key;		/* Node key		*/
	struct ptree_mask *p_m;		/* Node masks		*/
	unsigned char p_mlen;		/* Number of masks	*/
	char p_b;			/* Bit to check		*/
	struct ptree *p_left;		/* Left pointer		*/
	struct ptree *p_right;		/* Right pointer	*/
};


extern struct ptree *pat_insert(struct ptree *n, struct ptree *head);
extern int           pat_remove(struct ptree *n, struct ptree *head);
extern struct ptree *pat_search(unsigned long key, struct ptree *head);

/* Import definitions. */
extern void *nanvix_malloc(size_t size);
extern void nanvix_free(void *ptr);
extern void nanvix_qsort(void *, size_t, size_t, int (*)(const void *, const void *));
extern void ___nanvix_exit(int);


#define bcopy(src,dest,n) umemcpy(src,dest,n)
#define malloc(x) nanvix_malloc(x)
#define free(x) nanvix_free(x)
#define exit(x) ___nanvix_exit(x)
#define bzero(s,n) umemset(s, 0, n)
#define perror(s) uprintf(s)
#define fprintf(f,str) uprintf(str)
#define printf(x, ...) uprintf(x, __VA_ARGS__)

#endif /* _PATRICIA_H_ */
