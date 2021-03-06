/******************************************************************************
 *
 * File:        gridprm.c
 *
 * Created:     17/12/2014
 *
 * Author:      Pavel Sakov
 *              Bureau of Meteorology
 *
 * Description:
 *
 * Revisions:
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "definitions.h"
#include "grid.h"
#include "gridprm.h"
#include "utils.h"

#define MAPTYPE_DEF 'B'

typedef struct {
    char* vtype_tag;
    int vtype;
    char* levelvarnameentry;
} gridvtype_entry;

gridvtype_entry allgridvtypeentries[] = {
    {"Z", GRIDVTYPE_Z, "NUMLEVELSVARNAME"},
    {"SIGMA", GRIDVTYPE_SIGMA, "MASKVARNAME"},
    {"GENERAL", GRIDVTYPE_GEN, "NUMLEVELSVARNAME"}
};
int ngridvtypeentries = sizeof(allgridvtypeentries) / sizeof(gridvtype_entry);

/**
 */
void gridprm_create(char* fname, int* ngrid, gridprm** prm)
{
    gridprm* now = NULL;
    FILE* f = NULL;
    int ignore = 0;
    char buf[MAXSTRLEN];
    int line;
    int i;

    assert(*ngrid == 0 && *prm == NULL);

    f = enkf_fopen(fname, "r");
    line = 0;
    while (fgets(buf, MAXSTRLEN, f) != NULL) {
        char seps[] = " =\t\n";
        char* token;

        line++;
        if (buf[0] == '#')
            continue;
        if ((token = strtok(buf, seps)) == NULL)
            continue;
        if (strcasecmp(token, "NAME") == 0) {
            (*ngrid)++;
            *prm = realloc(*prm, sizeof(gridprm) * (*ngrid));
            now = &(*prm)[*ngrid - 1];
            memset(now, 0, sizeof(gridprm));
            now->sfactor = 1.0;
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: NAME not specified", fname, line);
            else
                now->name = strdup(token);
            ignore = 0;
            if ((token = strtok(NULL, seps)) != NULL) {
                if (strcasecmp("PREP", token) != 0 && strcasecmp("CALC", token) != 0)
                    enkf_quit("%s, l.%d: grid qualifier = \"%s\"; must be either \"PREP\" or \"CALC\"\n", fname, line, token);
#if defined(ENKF_PREP)
                if (strcasecmp("PREP", token) != 0) {
                    (*ngrid)--;
                    now = NULL;
                    ignore = 1;
                }
#elif defined(ENKF_CALC) || defined(ENKF_UPDATE)
                if (strcasecmp("CALC", token) != 0) {
                    (*ngrid)--;
                    now = NULL;
                    ignore = 1;
                }
#endif
            }
            continue;
        } else if (now == NULL) {
            if (ignore)
                continue;
            else
                enkf_quit("%s, l.%d: entry NAME is required to start grid initialisation", fname, line);
        }

        if (strcasecmp(token, "VTYPE") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: VTYPE not specified", fname, line);
            else if (now->vtype != NULL)
                enkf_quit("%s, l.%d: VTYPE specified twice", fname, line);
            else
                now->vtype = strdup(token);

            for (i = 0; i < ngridvtypeentries; ++i)
                if (strcasecmp(allgridvtypeentries[i].vtype_tag, token) == 0)
                    break;
            if (i == ngridvtypeentries)
                enkf_quit("%s, l %d: VTYPE \"%s\" is unknown", fname, line, token);
            if (allgridvtypeentries[i].levelvarnameentry != NULL)
                now->levelvarnameentry = strdup(allgridvtypeentries[i].levelvarnameentry);
#if !defined(NO_GRIDUTILS)
        } else if (strcasecmp(token, "MAPTYPE") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: MAPTYPE not specified", fname, line);
            if (token[0] == 'b' || token[0] == 'B' || token[0] == 'k' || token[0] == 'K')
                now->maptype = token[0];
            else
                enkf_quit("%s, l %d: MAPTYPE \"%s\" is unknown", fname, line, token);
#endif
        } else if (strcasecmp(token, "DATA") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: DATA not specified", fname, line);
            else if (now->fname != NULL)
                enkf_quit("%s, l.%d: DATA specified twice", fname, line);
            else
                now->fname = strdup(token);
        } else if (strcasecmp(token, "XDIMNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: XDIMNAME not specified", fname, line);
            else if (now->xdimname != NULL)
                enkf_quit("%s, l.%d: XDIMNAME specified twice", fname, line);
            else
                now->xdimname = strdup(token);
        } else if (strcasecmp(token, "YDIMNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: YDIMNAME not specified", fname, line);
            else if (now->ydimname != NULL)
                enkf_quit("%s, l.%d: YDIMNAME specified twice", fname, line);
            else
                now->ydimname = strdup(token);
        } else if (strcasecmp(token, "ZDIMNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: ZDIMNAME not specified", fname, line);
            else if (now->zdimname != NULL)
                enkf_quit("%s, l.%d: ZDIMNAME specified twice", fname, line);
            else
                now->zdimname = strdup(token);
        } else if (strcasecmp(token, "XVARNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: XVARNAME not specified", fname, line);
            else if (now->xvarname != NULL)
                enkf_quit("%s, l.%d: XVARNAME specified twice", fname, line);
            else
                now->xvarname = strdup(token);
        } else if (strcasecmp(token, "YVARNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: YVARNAME not specified", fname, line);
            else if (now->yvarname != NULL)
                enkf_quit("%s, l.%d: YVARNAME specified twice", fname, line);
            else
                now->yvarname = strdup(token);
        } else if (strcasecmp(token, "ZVARNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: ZVARNAME not specified", fname, line);
            else if (now->zvarname != NULL)
                enkf_quit("%s, l.%d: ZVARNAME specified twice", fname, line);
            else
                now->zvarname = strdup(token);
        } else if (strcasecmp(token, "DEPTHVARNAME") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: DEPTHVARNAME not specified", fname, line);
            else if (now->depthvarname != NULL)
                enkf_quit("%s, l.%d: DEPTHVARNAME specified twice", fname, line);
            else
                now->depthvarname = strdup(token);
        } else if (strcasecmp(token, "SFACTOR") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: SFACTOR not specified", fname, line);
            if (now->sfactor != 1.0)
                enkf_quit("%s, l.%d: SFACTOR specified twice", fname, line);
            if (!str2double(token, &now->sfactor))
                enkf_quit("%s, l.%d: could not convert \"%s\" to double", fname, line, token);
        } else if (strcasecmp(token, "STRIDE") == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_quit("%s, l.%d: STRIDE not specified", fname, line);
            if (now->stride != 0)
                enkf_quit("%s, l.%d: STRIDE specified twice", fname, line);
            if (!str2int(token, &now->stride))
                enkf_quit("%s, l.%d: could not convert \"%s\" to int", fname, line, token);
        } else if (now->levelvarnameentry != NULL && strcasecmp(token, now->levelvarnameentry) == 0) {
            if ((token = strtok(NULL, seps)) == NULL)
                enkf_printf("%s, l.%d: \"%s\" not specified", fname, line, now->levelvarnameentry);
            else if (now->levelvarname != NULL)
                enkf_quit("%s, l.%d: \"%s\" specified twice", fname, line, now->levelvarnameentry);
            else
                now->levelvarname = strdup(token);
        }
    }

    fclose(f);

    for (i = 0; i < *ngrid; ++i) {
        gridprm* now = &(*prm)[i];

        if (now->vtype == NULL)
            enkf_quit("%s: VTYPE not specified for grid \"%s\"", fname, now->name);
        if (now->fname == NULL)
            enkf_quit("%s: DATA not specified for grid \"%s\"", fname, now->name);
#if !defined(NO_GRIDUTILS)
        if (now->maptype == 0)
            now->maptype = MAPTYPE_DEF;
#endif
        if (now->xdimname == NULL)
            enkf_quit("%s: XDIMNAME not specified for grid \"%s\"", fname, now->name);
        if (now->ydimname == NULL)
            enkf_quit("%s: YDIMNAME not specified for grid \"%s\"", fname, now->name);
        if (now->zdimname == NULL)
            enkf_quit("%s: ZDIMNAME not specified for grid \"%s\"", fname, now->name);
        if (now->xvarname == NULL)
            enkf_quit("%s: XVARNAME not specified for grid \"%s\"", fname, now->name);
        if (now->yvarname == NULL)
            enkf_quit("%s: YVARNAME not specified for grid \"%s\"", fname, now->name);
        if (now->zvarname == NULL)
            enkf_quit("%s: ZVARNAME not specified for grid \"%s\"", fname, now->name);
        if (!isfinite(now->sfactor) || now->sfactor <= 0.0)
            enkf_quit("%s: SFACTOR = %.3g\n", now->sfactor);
    }
}

/**
 */
void gridprm_destroy(int ngrid, gridprm prm[])
{
    int i;

    for (i = 0; i < ngrid; ++i) {
        gridprm* now = &prm[i];

        free(now->name);
        free(now->fname);
        free(now->xdimname);
        free(now->ydimname);
        free(now->zdimname);
        free(now->xvarname);
        free(now->yvarname);
        free(now->zvarname);
        free(now->depthvarname);
        if (now->levelvarnameentry != NULL) {
            free(now->levelvarname);
            free(now->levelvarnameentry);
        }
    }
    free(prm);
}

/**
 */
void gridprm_print(gridprm* prm, char offset[])
{
    enkf_printf("%sgrid prm info:\n", offset);
    enkf_printf("%s  NAME = \"%s\"\n", offset, prm->name);
    enkf_printf("%s  DATA = \"%s\"\n", offset, prm->fname);
#if !defined(NO_GRIDUTILS)
    enkf_printf("%s  MAPTYPE = \"%c\"\n", offset, prm->maptype);
#endif
    enkf_printf("%s  VTYPE = \"%s\"\n", offset, prm->vtype);
    enkf_printf("%s  XDIMNAME = \"%s\"\n", offset, prm->xdimname);
    enkf_printf("%s  YDIMNAME = \"%s\"\n", offset, prm->ydimname);
    enkf_printf("%s  ZDIMNAME = \"%s\"\n", offset, prm->zdimname);
    enkf_printf("%s  XVARNAME = \"%s\"\n", offset, prm->xvarname);
    enkf_printf("%s  YVARNAME = \"%s\"\n", offset, prm->yvarname);
    enkf_printf("%s  ZVARNAME = \"%s\"\n", offset, prm->zvarname);
    enkf_printf("%s  DEPTHVARNAME = \"%s\"\n", offset, prm->depthvarname);
    if (prm->levelvarnameentry != NULL && prm->levelvarname != NULL)
        enkf_printf("%s  %s = \"%s\"\n", offset, prm->levelvarnameentry, prm->levelvarname);
    if (prm->stride != 0)
        enkf_printf("%s  STRIDE = %d\n", offset, prm->stride);
    if (prm->sfactor != 1.0)
        enkf_printf("%s  SFACTOR = \"%.f\"\n", offset, prm->sfactor);
}

/**
 */
int gridprm_getvtype(gridprm* prm)
{
    int i;

    for (i = 0; i < ngridvtypeentries; ++i) {
        if (strcasecmp(prm->vtype, allgridvtypeentries[i].vtype_tag) == 0)
            return allgridvtypeentries[i].vtype;
    }

    return GRIDVTYPE_NONE;
}
