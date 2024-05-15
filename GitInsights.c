/*----------------------------------------------------------------------------
	Program : GitInsights.c
	Author  : Tom Stevelt
	Date    : Apr 2024
	Synopsis: Track github clones, forks, watchers and stars

	Who		Date		Modification
	---------------------------------------------------------------------
	tms		05/03/2024	Option to not over-write previous version

----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
	Copyright © 2024 Tom Stevelt
	Tom Stevelt <tstevelt@silverhammersoftware.com>
	This software is free software; you can redistribute it and/or modify
	it under the terms of the MIT license. See LICENSE for details.
---------------------------------------------------------------------------*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>

#include	"shslib.h"

#define		NAMELEN		30
#define		MAXREPO		50
#define		MAXTOKS		(MAXREPO*10)

typedef struct
{
	char	RepoName[NAMELEN];
	int		Clones[2];
	int		Forks[2];
	int		Watchers[2];
	int		Stars[2];
} RECORD;

static	RECORD	Array[MAXREPO];
static	RECORD	Key, *Ptr;
static	int		Count;

static int cmprec ( RECORD *a, RECORD *b )
{
	return ( strcmp ( a->RepoName, b->RepoName ) );
}


static RECORD *AddRecord ( char *RepoName )
{
	RECORD	key, *ptr;

	if ( Count >= MAXREPO )
	{
		printf ( "Exceeds MAXREPO\n" );
		exit ( 1 );
	}
	sprintf ( Array[Count].RepoName, "%s", RepoName );
	Count++;

	if ( Count == 1 )
	{
		ptr = &Array[0];
	}
	else
	{
		qsort ( Array, Count, sizeof(RECORD), (int(*)()) cmprec );
		sprintf ( key.RepoName, "%s", RepoName );
		ptr = bsearch ( &key, Array, Count, sizeof(RECORD), (int(*)()) cmprec );
	}
	return ( ptr );
}

int main ( int argc, char *argv[] )
{
	char	*Owner;
	char	DataFile[256];
	FILE	*fp;
	FILE	*tfp;
	char	buffer[10240];
	char	cmdline[1024];
	char	*tokens[MAXTOKS];
	int		tokcnt, xt, ndx, xl;
	int		Debug = 0;
	int		WriteNewFile = 1;

	if ( argc < 3 )
	{
		printf ( "GitInsights owner {txt|html} [-nowrite]\n" );
		exit ( 1 );
	}

	Owner = argv[1];
	int Format = 0;
	if ( strcmp ( argv[2], "txt" ) == 0 )
	{
		Format = 'T';
	}
	else if ( strcmp ( argv[2], "html" ) == 0 )
	{
		Format = 'H';
	}
	else 
	{
		printf ( "GitInsights owner {txt|html} [-nowrite]\n" );
		exit ( 1 );
	}

	if ( argc == 4 )
	{
		if ( strcmp ( argv[3], "-nowrite" ) == 0 )
		{
			WriteNewFile = 0;
		}
		else
		{
			printf ( "Unknown argument\n" );
			exit ( 1 );
		}
	}


	sprintf ( DataFile, "/var/local/GitInsights/%s.CSV", Owner );

	/*----------------------------------------------------------
		load file
	----------------------------------------------------------*/
	if (( fp = fopen ( DataFile, "r" )) != NULL )
	{
		while ( fgets ( buffer, sizeof(buffer), fp ) != NULL )
		{
			if (( tokcnt = GetTokensD ( buffer, ",\n", tokens, MAXTOKS )) < 5 )
			{
				continue;
			}

			sprintf ( Key.RepoName, "%s", tokens[0] );
			if (( Ptr = bsearch ( &Key, Array, Count, sizeof(RECORD), (int(*)()) cmprec )) == NULL )
			{
				Ptr = AddRecord ( Key.RepoName );
			}
			Ptr->Clones[0] = atoi ( tokens[1] );
			Ptr->Forks[0] = atoi ( tokens[2] );
			Ptr->Watchers[0] = atoi ( tokens[3] );
			Ptr->Stars[0] = atoi ( tokens[4] );
		}

		fclose ( fp );

		if ( Debug )
		{
			printf ( "Loaded %d records\n", Count );
		}
	}

	/*----------------------------------------------------------
		get repo names from github, look up in array, add if 
		necessary, set status
	----------------------------------------------------------*/
	sprintf ( cmdline, "gh repo list --json name > x1.json" );
	system ( cmdline );
	if (( tfp = fopen ( "x1.json", "r" )) == NULL )
	{
		printf ( "fopen x1.json failed\n" );
		exit ( 1 );
	}

	while ( fgets ( buffer, sizeof(buffer), tfp ) != NULL )
	{
		if ( Debug )
		{
			printf ( "%s\n", buffer );
		}
		if (( tokcnt = GetTokensD ( buffer, "[{]}:\",", tokens, MAXTOKS )) < 2 )
		{
			continue;
		}
		for ( xt = 0; xt < tokcnt; xt++ )
		{
			if ( strcmp ( tokens[xt], "name" ) == 0 )
			{
				if ( Debug )
				{
					printf ( "%s\n", tokens[xt+1] );
				}
				sprintf ( Key.RepoName, "%s", tokens[xt+1] );
				if (( Ptr = bsearch ( &Key, Array, Count, sizeof(RECORD), (int(*)()) cmprec )) == NULL )
				{
					Ptr = AddRecord ( Key.RepoName );
				}
			}
		}
	}

	fclose ( tfp );

	/*----------------------------------------------------------
		get clone info, put in array			
	----------------------------------------------------------*/
	for ( ndx = 0; ndx < Count; ndx++ )
	{
		sprintf ( cmdline, 
"gh api -H 'Accept: application/vnd.github+json' -H 'X-GitHub-Api-Version: 2022-11-28' /repos/%s/%s/traffic/clones > x1.json",
				Owner, Array[ndx].RepoName );

		system ( cmdline );

		if (( tfp = fopen ( "x1.json", "r" )) == NULL )
		{
			printf ( "fopen x1.json failed\n" );
			exit ( 1 );
		}
		fgets ( buffer, sizeof(buffer), tfp );
		fclose ( tfp );

		if (( tokcnt = GetTokensD ( buffer, "[{]}:\",", tokens, MAXTOKS )) < 2 )
		{
			continue;
		}
		if ( Debug )
		{
			printf ( "%s %s\n", Array[ndx].RepoName, tokens[1] );
		}

		Array[ndx].Clones[1] = atoi(tokens[1]);

		nap ( 100 );
	}

	/*----------------------------------------------------------
		get forks, watchers and stars info, put in array			
	----------------------------------------------------------*/
	sprintf ( cmdline, "gh repo list --json name,forkCount,watchers,stargazerCount > x1.json" );

	system ( cmdline );

	if (( tfp = fopen ( "x1.json", "r" )) == NULL )
	{
		printf ( "fopen x1.json failed\n" );
		exit ( 1 );
	}
	fgets ( buffer, sizeof(buffer), tfp );
	fclose ( tfp );

	if ( Debug )
	{
		printf ( "%s\n", buffer );
	}

	tokcnt = GetTokensD ( buffer, "[{]}:\",", tokens, MAXTOKS );

	for ( xt = 0; xt < tokcnt; xt++ )
	{
		if ( Debug && strcmp ( tokens[xt+3], "GenMealPlan" ) == 0 )
		{
			printf ( "kilroy was here\n" );
		}

		if ( strcmp ( tokens[xt], "forkCount" ) == 0 )
		{
			if ( Debug )
			{
				printf ( "%s %s %s %s %s\n", 
					tokens[xt+3], tokens[xt+1], tokens[xt+8], tokens[xt+5], Format == 'H' ? "<br>" : "" );
			}

			sprintf ( Key.RepoName, "%s", tokens[xt+3] );	
			if (( Ptr = bsearch ( &Key, Array, Count, sizeof(RECORD), (int(*)()) cmprec )) == NULL )
			{
				printf ( "Get forks...: Could not find %s\n", Key.RepoName );
			}
			else
			{
				Ptr->Forks[1] = atoi(tokens[xt+1]);
				Ptr->Watchers[1] = atoi(tokens[xt+8]);
				Ptr->Stars[1] = atoi(tokens[xt+5]);
			}
		}
	}

	if ( WriteNewFile )
	{
		/*----------------------------------------------------------
			write file and close
		----------------------------------------------------------*/
		if (( fp = fopen ( DataFile, "w" )) == NULL )
		{
			printf ( "fopen write %s failed\n", DataFile );
			exit ( 1 );
		}

		xl = 0;
		for ( ndx = 0; ndx < Count; ndx++ )
		{
			if ( xl < strlen(Array[ndx].RepoName) )
			{
				xl = strlen(Array[ndx].RepoName);
			}

			fprintf ( fp, "%s,%d,%d,%d,%d\n",
				Array[ndx].RepoName,
				Array[ndx].Clones[1],
				Array[ndx].Forks[1],
				Array[ndx].Watchers[1],
				Array[ndx].Stars[1] );
		}

		fclose ( fp );
	}
	else
	{
		xl = 0;
		for ( ndx = 0; ndx < Count; ndx++ )
		{
			if ( xl < strlen(Array[ndx].RepoName) )
			{
				xl = strlen(Array[ndx].RepoName);
			}
		}
	}


	/*----------------------------------------------------------
		print report
		clones are tracked by github for 14 days, so as they
		fall off, the clone count will go down
	----------------------------------------------------------*/
	if ( Format == 'H' )
	{
		printf ( "<table>\n" );
		printf ( "<tr>\n" );
		printf ( "<td>Repository</td>\n" );
		printf ( "<td>Clones</td>\n" );
		printf ( "<td>Forks</td>\n" );
		printf ( "<td>Watchers</td>\n" );
		printf ( "<td>Stars</td>\n" );
		printf ( "</tr>\n" );
	}
	for ( ndx = 0; ndx < Count; ndx++ )
	{
		if ( Format == 'H' )
		{
			printf ( "<tr>\n" );
			printf ( "<td>%-*.*s </td>", xl, xl, Array[ndx].RepoName );

// printf ( "<td>%2d,%2d%c </td>", Array[ndx].Clones[0], Array[ndx].Clones[1], Array[ndx].Clones[0] != Array[ndx].Clones[1] ? '*' : ' ' );
			printf ( "<td>%2d,%2d", Array[ndx].Clones[0], Array[ndx].Clones[1] );
			if ( Array[ndx].Clones[0] < Array[ndx].Clones[1] )
			{
				printf ( "+ " );
			}
			else if ( Array[ndx].Clones[0] > Array[ndx].Clones[1] )
			{
				printf ( "- " );
			}
			printf ( "</td>" );
	
			printf ( "<td>%2d%c </td>", Array[ndx].Forks[1], Array[ndx].Forks [0] != Array[ndx].Forks [1] ? '*' : ' ' );
			printf ( "<td>%2d%c </td>", Array[ndx].Watchers[1], Array[ndx].Watchers[0] != Array[ndx].Watchers[1] ? '*' : ' ' );
			printf ( "<td>%2d%c</td>", Array[ndx].Stars[1], Array[ndx].Stars [0] != Array[ndx].Stars [1] ? '*' : ' ' );
			printf ( "</tr>\n" );
		}
		else
		{
			char	FootNote = ' ';
			if ( Array[ndx].Clones[0] < Array[ndx].Clones[1] )
			{
				FootNote = '+';
			}
			else if ( Array[ndx].Clones[0] > Array[ndx].Clones[1] )
			{
				FootNote = '-';
			}

			printf ( "%-*.*s %2d,%2d%c %2d%c %2d%c %2d%c\n",
				xl, xl, Array[ndx].RepoName,
				Array[ndx].Clones[0], Array[ndx].Clones[1], FootNote,
				Array[ndx].Forks[1], Array[ndx].Forks [0] != Array[ndx].Forks [1] ? '*' : ' ',
				Array[ndx].Watchers[1], Array[ndx].Watchers[0] != Array[ndx].Watchers[1] ? '*' : ' ',
				Array[ndx].Stars[1], Array[ndx].Stars [0] != Array[ndx].Stars [1] ? '*' : ' ' );
		}
	}

	if ( Format == 'H' )
	{
		printf ( "</table>\n" );
	}

	unlink ( "x1.json" );

	return ( 0 );
}
