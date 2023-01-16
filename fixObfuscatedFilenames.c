//
// Created by paul on 1/16/23.
//

#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <ftw.h>
#include <ctype.h>

#unset DEBUG

const char * myName;

int usage( void )
{
    fprintf( stderr, "### %s: requires at least one path to process.\n",
             myName );
    return 0;
}

/**
 * returns a pointer to ether the last period or end of a filename
 * @return
 */
const char * getExt( const char * filename )
{
    const char *end = strrchr(filename, '.');
    if (end == NULL) {
        end = filename;
        while (*end != '\0') end++;
    }
    return end;
}

int isFilenameObfusticated( const char * filename, const char * ext )
{
    int result = 1;

    int count = 0;
    int digit = 0;
    int xdigit = 0;
    int lower = 0;
    int upper = 0;

    for (const char *p = filename; p < ext; ++p) {
        count++;
        if (isxdigit(*p)) xdigit++;

        if (isdigit(*p)) digit++;
        else if (islower(*p)) lower++;
        else if (isupper(*p)) upper++;
        else {
            /* non-alphanum character - therefore it's not obfusticated */
            result = 0;
        }
    }

#ifdef DEBUG
    fprintf( stderr, "count:%d xdigit:%d digit:%d lower:%d upper:%d\n",
             count, xdigit, digit, lower, upper);
#endif

    /* if we haven't already determined it's not obfusticated, check */
    if (result == 1) {
        if (count == xdigit) {
            /* all the characters are valid hex digits */
#ifdef DEBUG
            fprintf(stderr, ">>> all hex digits ");
#endif
        } else if (digit > 0 && lower > 0 && upper > 0) {
            /* a mix of digits, upper and lower case characters */
#ifdef DEBUG
            fprintf(stderr, ">>> all mixed alphanum ");
#endif
        } else result = 0;
    }

    return result;
}

int nftwEntry( const char * path, const struct stat * sb, int typeFlag, struct FTW * ftwbuf )
{
    int result = FTW_CONTINUE;
    const char * leaf = "";
    if (ftwbuf != NULL) {
        leaf = &path[ ftwbuf->base ];
    }
    switch ( typeFlag )
    {
        case FTW_F: // passed a file
            if ( leaf[0] == '.' )
            {
                fprintf( stderr, "### file: \'%s\' ignored\n", leaf );
            } else {
                const char * ext = getExt(leaf);
                if ( isFilenameObfusticated( leaf, ext ) ) {
                    fprintf( stderr, " obfusticated: \'%s\'\n", leaf );
                    const char * parent = path;
                    const char * parend = path;
                    for ( const char * p = path; *p != '\0'; ++p )
                    {
                        if ( *p == '/' ) {
                            parent = parend;
                            parend = p;
                        }
                    }
                    ++parent; /* skip over the slash */
                    int pLen = parend - path + 1;
                    int bLen = parend - parent;
                    int eLen = strlen(ext );
                    char * newPath = malloc(pLen + bLen + eLen + 1 );

                    memcpy(newPath, path, pLen );
                    memcpy(&newPath[pLen], parent, bLen );
                    strncpy(&newPath[pLen + bLen], ext, eLen );

                    if ( rename( path, newPath ) == -1 ) {
                        fprintf( stderr, "### %s: rename failed (%d: %s)\n",
                                 myName, errno, strerror(errno) );
                    }

                    fprintf(stderr, "\"%s\" -> \"%s\"\n", path, newPath );

                }
#ifdef DEBUG
                else {
                    fprintf( stderr, "### file: \'%s\'\n", leaf );
                }
#endif
            }
            break;

        case FTW_D: // passed a directory
            if ( leaf[0] == '.' )
            {
#ifdef DEBUG
                fprintf( stderr, "### directory: \'%s\' ignored\n", leaf );
#endif
                result = FTW_SKIP_SUBTREE;
            }
#ifdef DEBUG
            else {
                fprintf(stderr, "### directory: \'%s\'\n", leaf);
            }
#endif
            break;

        default:
#ifdef DEBUG
            fprintf( stderr, "### other: \'%s\'\n", leaf );
#endif
            break;
    }
    return result;
}

int scanHierarchy( const char * path )
{
#ifdef DEBUG
    fprintf( stderr, "%s: path=\'%s\'\n", myName, path );
#endif

    nftw( path, nftwEntry, 10, FTW_ACTIONRETVAL );
    return 0;
}

int main( int argc, char * argv[] )
{
    int exitcode = 0;

    myName = strrchr( argv[0], '/' );
    if ( myName++ == NULL ) myName = argv[0];

    if ( argc < 2 ) {
        exitcode = usage();
    }
    else for (int i = 1; exitcode == 0 && i < argc; ++i )
    {
        char * fullPath = realpath(argv[i], NULL );
        if ( fullPath == NULL )
        {
            fprintf( stderr, "### %s: unable to convert \'%s\' to a full path (%d: %s)\n",
                     myName, argv[i], errno, strerror(errno) );
            exitcode = -errno;
        } else {
            exitcode = scanHierarchy( fullPath );
            free( fullPath );
        }
    }
    return exitcode;
}
