/* joiner - information about what fields in what tables
 * in what databases can fruitfully be related together
 * or joined.  Another way of looking at it is this
 * defines identifiers shared across tables.  This also
 * defines what tables depend on what other tables
 * through dependency attributes and statements.
 *
 * The main routines you'll want to use here are 
 *    joinerRead - to read in a joiner file
 *    joinerRelate - to get list of possible joins given a table.
 */

#ifndef JOINER_H
#define JOINER_H

struct joinerField
/* A field that can be joined on. */
    {
    struct joinerField *next;	/* Next in list. */
    int lineIx;			/* Line index of start for error reporting */
    struct slName *dbList;	/* List of possible databases. */
    char *table;		/* Associated table. */
    char *field;		/* Associated field. */
    struct slName *chopBefore;	/* Chop before strings */
    struct slName *chopAfter;	/* Chop after strings */
    char *separator;		/* Separators for lists or NULL if not list. */
    boolean indexOf;		/* True if id is index in this list. */
    boolean isPrimary;		/* True if is primary key. */
    double minCheck;		/* Minimum ratio that must hit primary key */
    boolean unique;		/* No more than one row per identifier. */
    boolean full;		/* All identifiers covered. */
    char *splitPrefix;		/* Prefix for splitting tables. */
    char *splitSuffix;		/* Suffix for splitting tables. */
    struct slName *exclude;	/* List of keys to exclude from verification */
    };

struct joinerSet
/* Information on a set of fields that can be joined together. */
    {
    struct joinerSet *next;		/* Next in list. */
    char *name;				/* Name of field set. */
    int lineIx;			/* Line index of start for error reporting */
    struct joinerSet *parent;		/* Parsed-out parent type if any. */
    struct slRef *children;		/* References to children if any. */
    char *typeOf;			/* Parent type name if any. */
    char *external;			/* External name if any. */
    char *description;			/* Short description. */
    struct joinerField *fieldList;	/* List of fields. */
    boolean isFuzzy;		/* True if no real primary key. */
    boolean expanded;		/* True if an expanded set. */
    boolean isDependency;	/* Primary key update forces full update? */
    };

struct joinerTable
/* A list of tables (that may be in multiple datbases). */
    {
    struct joinerTable *next;	/* Next in list. */
    struct slName *dbList;	/* List of databases. */
    char *table;		/* The table name. */
    };

struct joinerDependency
/* A list of table dependencies. */
    {
    struct joinerDependency *next;	/* Next in list. */
    struct joinerTable *table;		/* A table. */
    struct joinerTable *dependsOnList;	/* List of tables it depends on. */
    int lineIx;				/* Line of dependency. */
    };

struct joinerType
/* A list of table types. */
    {
    struct joinerType *next;		/* Next in list. */
    char *name;				/* Type name. */
    struct joinerTable *tableList;	/* List of tables - can include SQL wildcards. */
    };

struct joinerIgnore
/* A list of tables to ignore. */
    {
    struct joinerIgnore *next;	/* Next in list. */
    struct slName *dbList;	/* List of databases. */
    struct slName *tableList;	/* List of tables. */
    };

struct joiner
/* Manage joining identifier information across all databases. */
    {
    struct joiner *next;	/* Next in list. */
    char *fileName;		/* Associated file name */
    struct joinerSet *jsList;	/* List of identifiers. */
    struct hash *symHash;	/* Hash of symbols from file. */
    struct hash *exclusiveSets; /* List of hashes of exclusive databases. */
    struct hash *databasesChecked; /* List of databases to check. */
    struct hash *databasesIgnored; /* List of database to ignore. */
    struct joinerDependency *dependencyList; /* List of table dependencies. */
    struct joinerType *typeList;	/* List of explicit table types. */
    struct joinerIgnore *tablesIgnored;	/* List of tables to ignore. */
    };

void joinerFree(struct joiner **pJoiner);
/* Free up memory associated with joiner */

struct joiner *joinerRead(char *fileName);
/* Read in a .joiner file. */

boolean joinerExclusiveCheck(struct joiner *joiner, char *aDatabase, 
	char *bDatabase);
/* Check that aDatabase and bDatabase are not in the same
 * exclusivity hash.  Return TRUE if join can happen between
 * these two databases. */

struct joinerDtf
/* Just database, table, and field. */
    {
    struct joinerDtf *next;	/* Next in list. */
    char *database;		/* Database. */
    char *table;		/* Table. */
    char *field;		/* Field. */
    };

struct joinerDtf *joinerDtfNew(char *database, char *table, char *field);
/* Get new joinerDtf. */

struct joinerDtf *joinerDtfClone(struct joinerDtf *dtf);
/* Return duplicate (deep copy) of joinerDtf. */

void joinerDtfFree(struct joinerDtf **pDtf);
/* Free up memory associated with joinerDtf. */

void joinerDtfFreeList(struct joinerDtf **pList);
/* Free up memory associated with list of joinerDtfs. */

struct joinerDtf *joinerDtfFromDottedTriple(char *triple);
/* Get joinerDtf from something in db.table.field format. */

struct joinerPair
/* A pair of linked fields. */
    {
    struct joinerPair *next;	/* Next in list. */
    struct joinerDtf *a;	/* Typically contains field from input table */
    struct joinerDtf *b;	/* Field in another table */
    struct joinerSet *identifier;	/* Identifier this is based on,
                                         * not allocated here. */
    };

void joinerPairFree(struct joinerPair **pJp);
/* Free up memory associated with joiner pair. */

void joinerPairFreeList(struct joinerPair **pList);
/* Free up memory associated with list of joinerPairs. */

void joinerPairDump(struct joinerPair *jpList, FILE *out);
/* Write out joiner pair list to file mostly for debugging. */

struct joinerPair *joinerRelate(struct joiner *joiner, char *database, 
	char *table);
/* Get list of all ways to link table in given database to other tables,
 * possibly in other databases. */

struct slRef *joinerSetInheritanceChain(struct joinerSet *js);
/* Return list of self, children, and parents (but not siblings).
 * slFreeList result when done. */

boolean joinerDtfSameTable(struct joinerDtf *a, struct joinerDtf *b);
/* Return TRUE if they are in the same database and table. */

boolean joinerDtfAllSameTable(struct joinerDtf *fieldList);
/* Return TRUE if all joinerPairs refer to same table. */

struct joinerPair *joinerFindRoute(struct joiner *joiner, 
	struct joinerDtf *a,  struct joinerDtf *b);
/* Find route between a and b.  Note the field element of a and b
 * are unused. */

struct joinerPair *joinerFindRouteThroughAll(struct joiner *joiner, 
	struct joinerDtf *tableList);
/* Return route that gets to all tables in fieldList.  Note that
 * the field element of the items in tableList can be NULL. */

#endif /* JOINER_H */