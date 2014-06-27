#define _CRT_SECURE_NO_DEPRECATE 1

#include <stdlib.h>
#include <math.h>
#include <setjmp.h>
#include <string>

#ifdef VAX
#include <ssdef.h>
#include <descrip.h>
#endif

 
#include "evaluate.h"

#define ERR(n) {ERROR=n; ERPOS=(int)(expression-ERANC-1); strcpy(ERTOK,token); longjmp(jb,1);}

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_E
#define M_E     2.71828182845904523536
#endif

double deg( double x );
double rad( double x );

int   ERROR;                 /* The error number */
char  ERTOK[TOKLEN + 1];     /* The token that generated the error */
int   ERPOS;                 /* The offset from the start of the expression */
char* ERANC;                 /* Used to calculate ERPOS */

VARIABLE Consts[] =
{
   /* name, value */
   { "pi",      M_PI },
   { "e",       M_E },
   { 0 }
};

//double (*fifi)(double);
//int ghjg;
//int rrghr()
//{
//	int uiy;
//
//   fifi = sin;
//   uiy = 9+7;
//   return uiy;
//}


FUNCTION Funcs[] =
{   /* name, funtion to call */
   { "sin",     1,    sin },
   { "cos",     1,    cos },
   { "tan",     1,    tan },
   { "asin",    1,    asin },
   { "acos",    1,    acos },
   { "atan",    1,    atan },
   { "sinh",    1,    sinh },
   { "cosh",    1,    cosh },
   { "tanh",    1,    tanh },
   { "exp",     1,    exp },
   { "log",     1,    log },
   { "log10",   1,    log10 },
   { "sqrt",    1,    sqrt },
   { "floor",   1,    floor },
   { "ceil",    1,    ceil },
   { "abs",     1,    fabs },
 /*  { "hypot",   2,    hypot }, */

   { "deg",     1,    deg },
   { "rad",     1,    rad },

   { 0 }
};

VARIABLE        Vars[MAXVARS];       /* Array for user-defined variables */
char*			expression;          /* Pointer to the user's expression */
char			token[TOKLEN + 1];   /* Holds the current token */
int             type;                /* Type of the current token */
jmp_buf         jb;                  /* jmp_buf for errors */

double deg( double x )
{
   return( x * 180.0 / M_PI );
}

double rad( double x )
{
   return( x * M_PI / 180.0 );
}

int GetSymbol( char* s, TYPE* v )
{
   char* e;

   e = getenv( s );
   if( !e )
      return( 0 );
   *v = atof( e );
   return( 1 );
}

#ifdef VAX
SetSymbol( char* s, char* v )
{
   struct dsc$descriptor_s sym;
   struct dsc$descriptor_s val;
   long                    typ = 1;

   sym.dsc$w_length = strlen( s );
   sym.dsc$a_pointer = s;
   sym.dsc$b_class = DSC$K_CLASS_S;
   sym.dsc$b_dtype = DSC$K_DTYPE_T;
   val.dsc$w_length = strlen( v );
   val.dsc$a_pointer = v;
   val.dsc$b_class = DSC$K_CLASS_S;
   val.dsc$b_dtype = DSC$K_DTYPE_T;
   return( LIB$SET_SYMBOL( &sym, &val, &typ ) );
}
#endif

void ClearAllVars()
{
   int i;

   for( i = 0; i < MAXVARS; i++ )
   {
      *Vars[i].name = 0;
      Vars[i].value = 0;
   }
}

int ClearVar( char* name )
{
   int i;

   for( i = 0; i < MAXVARS; i++ )
      if( *Vars[i].name && ! strcmp( name, Vars[i].name ) )
      {
         *Vars[i].name = 0;
         Vars[i].value = 0;
         return( 1 );
      }
   return( 0 );
}

int GetValue( char* name, TYPE* value )
{
   int i;

   if( *name == '_' )
      return( GetSymbol( name + 1, value ) );
   for( i = 0; i < MAXVARS; i++ )
      if( *Vars[i].name && ! strcmp( name, Vars[i].name ) )
      {
         *value = Vars[i].value;
         return( 1 );
      }
   for( i = 0; *Consts[i].name; i++ )
      if( *Consts[i].name && ! strcmp( name, Consts[i].name ) )
      {
         *value = Consts[i].value;
         return( 1 );
      }
   return( 0 );
}

int SetValue( char* name, TYPE* value )
{
   int  i;

#ifdef VAX
   char b[30];
   if( *name == '_' )
   {
      sprintf( b, "%g", *value );
      if( SetSymbol( name + 1, b ) != SS$_NORMAL )
         return( 0 );
      return( 1 );
   }
#endif
   ClearVar( name );
   for( i = 0; i < MAXVARS; i++ )
      if( ! *Vars[i].name )
      {
         strcpy( Vars[i].name, name );
         Vars[i].name[VARLEN] = 0;
         Vars[i].value = *value;
         return( 1 );
      }
   return( 0 );
}


static void Parse (void)
{
   char* t;

   type = 0;
   t = (char*)token;
   while( iswhite( *expression ) )
      expression++;
   if( isdelim( *expression ) )
   {
      type = DEL;
      *t++ = *expression++;
   }
   else if( isnumer( *expression ) )
   {
      type = NUM;
      while( isnumer( *expression ) )
         *t++ = *expression++;
   }
   else if( isalphanum( *expression ) )
   {
      type = VAR;
      while( isalphanum( *expression ) )
        *t++ = *expression++;
      token[VARLEN] = 0;
   }
   else if( *expression )
   {
      *t++ = *expression++;
      *t = 0;
      ERR( E_SYNTAX );
   }
   *t = 0;
   while( iswhite( *expression ) )
      expression++;
}

static
void Level2( TYPE* r );
void Level3( TYPE* r );
void Level4( TYPE* r );

static int Level1( TYPE* r )
{
   char t[VARLEN + 1];

   if( type == VAR )
      if( *expression == '=' )
      {
         strcpy( t, token );
         Parse();
         Parse();
         if( !*token )
         {
            ClearVar( t );
            return(1);
         }
         Level2( r );
         if( ! SetValue( t, r ) )
            ERR( E_MAXVARS );
         return( 1 );
      }
   Level2( r );
   return( 0 );
}

static void Level2( TYPE* r )
{
   TYPE t = 0;
   char o;

   Level3( r );
   while( (o = *token) == '+' || o == '-' )
   {
      Parse();
      Level3( &t );
      if( o == '+' )
         *r = *r + t;
      else if( o == '-' )
         *r = *r - t;
   }
}

static void Level3( TYPE* r )
{
   TYPE t;
   char o;

   Level4( r );
   while( (o = *token) == '*' || o == '/' || o == '%' )
   {
      Parse();
      Level4( &t );
      if( o == '*' )
         *r = *r * t;
      else if( o == '/' )
      {
         if( t == 0 )
            ERR( E_DIVZERO );
         *r = *r / t;
      }
      else if( o == '%' )
      {
         if( t == 0 )
            ERR( E_DIVZERO );
         *r = fmod( *r, t );
      }
   }
}

static void Level6( TYPE* r );

static void Level5( TYPE* r )
{
   char o = 0;

   if( *token == '+' || *token == '-' )
   {
      o = *token;
      Parse();
   }
   Level6( r );
   if( o == '-' )
      *r = -*r;
}


static void Level4( TYPE* r )
{
   TYPE t;

   Level5( r );
   if( *token == '^' )
   {
      Parse();
      Level5( &t );
      *r = pow( *r, t );
   }
}

static void Level6( TYPE* r )
{
   int  i;
   int  n;
   TYPE a[3];

   if( *token == '(' )
   {
      Parse();
      if( *token == ')' )
         ERR( E_NOARG );
      Level1( r );
      if( *token != ')' )
         ERR( E_UNBALAN );
      Parse();
   }
   else
   {
      if( type == NUM )
      {
         *r = (TYPE) atof( token );
         Parse();
      }
      else if( type == VAR )
      {
         if( *expression == '(' )
         {
            for( i = 0; *Funcs[i].name; i++ )
               if( ! strcmp( token, Funcs[i].name ) )
               {
                  Parse();
                  n = 0;
                  do
                  {
                     Parse();
                     if( *token == ')' || *token == ',' )
                        ERR( E_NOARG );
                     a[n] = 0;
                     Level1( &a[n] );
                     n++;
                  } while( n < 4 && *token == ',' );
                  Parse();
                  if( n != Funcs[i].args )
                  {
                     strcpy( token, Funcs[i].name );
                     ERR( E_NUMARGS );
                  }
                  /* *r = Funcs[i].func( a[0], a[1], a[2] ); */
				  *r = Funcs[i].func( a[0] ); 
                  return;
               }
               if( ! *Funcs[i].name )
                  ERR( E_BADFUNC );
            }
            else if( ! GetValue( token, r ) )
               ERR( E_UNKNOWN );
         Parse();
      }
      else
         ERR( E_SYNTAX );
   }
}

/*************************************************************************/

int evaluate( char* e, TYPE* result, int* a )
{
   if( setjmp( jb ) )
      return( ERROR );
   expression = e;
   ERANC = e;
   _strlwr( expression );
   *result = 0;
   Parse();
   if( ! *token )
      ERR( E_EMPTY );
   *a = Level1( result );
   return( E_OK );
}

/*************************************************************************/

#include <stdio.h>

char* ErrMsgs[] =
{
   "Syntax error",
   "Unbalanced parenthesis",
   "Division by zero",
   "Unknown variable",
   "Maximum variables exceeded",
   "Unrecognised funtion",
   "Wrong number of arguments to funtion",
   "Missing an argument",
   "Empty expression"
};


void trimspaces(char *s) /* remove white spaces at the end */
{
  char *p; p=s; while (*p) p++; p--;
  while ((p>=s) && (*p==' ' || *p=='\n' || *p=='\t')) *p--=0;
}


int solveForX(char expr[], double *resultvalue, double argument)
{

	//char expr[1024];
	*resultvalue = 1.0;
    double result;
    int a, ec;

	//printf("Enter an expression: ");
	//fgets(expr,1024,stdin);
	//expr[1023]=0;
	trimspaces(expr);
	//translating X into argument value
	char exprtemp[1024] = {NULL};
	char *where_var_is_in_expr = NULL;
	while(where_var_is_in_expr = strstr(expr, "x"))
	{
		printf(":%s\n", expr);
		strcpy(exprtemp, expr);
		char *where_rest_of_expr_is = strstr(exprtemp, "x") + 1 ;
		sprintf(where_var_is_in_expr, "(%f)", argument);	// place double where X is.
		strcat(expr, where_rest_of_expr_is);				//append rest od expression from backup
	}
	while(where_var_is_in_expr = strstr(expr, "e"))
	{
		printf(":%s\n", expr);
		strcpy(exprtemp, expr);
		char *where_rest_of_expr_is = strstr(exprtemp, "e") + 1 ;
		sprintf(where_var_is_in_expr, "(%f)", 2.718282);	// switch e for its value
		strcat(expr, where_rest_of_expr_is);				//append rest od expression from backup
	}
	printf(":%s\n", expr);
	if (expr[0]==0) //expression is empty
		return E_EMPTY;

    ec = evaluate( expr, &result, &a );

	if (ec == E_OK)
	{
		*resultvalue = result;
		printf("Result = %f\n\n",result);
	}
	else
		if (ec == E_EMPTY)
		{
            //printf("Empty expression\n");
			return E_EMPTY;
		}
		else
		{
			return ERROR;
            //printf( "ERROR: %s - %s", ErrMsgs[ERROR - 1], ERTOK );
            //printf( "\n%s", ERANC );
            //printf( "\n%*s^\n", ERPOS, "" );
		}

		//printf("\nThe end of demonstration.\n\n");
	return -1;
}