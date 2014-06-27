#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "evaluate.h"
#include <allegro5\allegro5.h>
#include <allegro5\allegro_native_dialog.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_FONT *font_scale = NULL;

#define TEXT_COLOR 255, 255, 255
#define TEXT_SIZE 15

int expression_check(char expr[]);
int solveForX(char expr[], double *resultvalue, double argument);
int allegro_initialization(int widht, int height);
int draw_empty_chart(int X_axis_coord, int Y_axis_coord, int winXsize, int winYsize, int Xscale, int Yscale);
char *Allowedstrings[] = {"\n", "+", "-", "*", "/", "%", "^", ")", "(", ".", "asin", "acos", "atan", "sinh", "cosh", "tanh", "sin", "cos", "tan", "exp", "log", "log10", "sqrt", "floor", "ceil", "abs", "deg", "rad", "x", "pi", "e", NULL};

int main()
{
	char expr[1024] = {NULL};
	//printf("Wpisz wzor do narysowania np. sin(x) z jedna niewiadoma x lub wpisz literke q by zakonczyc\n");
	printf("Enter expression to draw  eg. sin(x) with one variable x or type q to quit\n");
	printf("You can use: \n");
	for(int i=1; Allowedstrings[i] != NULL; i++)
		printf("%s ", Allowedstrings[i]);
	printf("\nUse * when multiplicating or it will not work. 2(x) is bad, 2*(x) is good.\n");

	do
	{
		printf("y = ");
		fgets(expr, 1023, stdin);
		printf("\n");
		//strcat(expr, "2+2*sin(x)\n");

		if(expr[0] == 'q')
			return 0;
	}
	while(expression_check(expr));
	double bbbb = 0;
	solveForX(expr, &bbbb, 1.234);
	return 0;

	int graph_area_widht = 800;
	int graph_area_height = 600;
#define EXTRA_DATA_AREA_ON_RIGHT_SIDE 250
	allegro_initialization( graph_area_widht+EXTRA_DATA_AREA_ON_RIGHT_SIDE, graph_area_height );
	ALLEGRO_EVENT ev;

	int Y_axis_coord = graph_area_widht / 2;
	int X_axis_coord = graph_area_height / 2;
	int Xscale = 30;	// unit bar every this amount of pixels
	int Yscale = 30;
	draw_empty_chart(X_axis_coord, Y_axis_coord, graph_area_widht, graph_area_height, Xscale, Yscale);

	while(1)
	{
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && (ev.keyboard.keycode == ALLEGRO_KEY_Q || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)))
			return 0;  //exit program
	}
	return 0;
}

int draw_empty_chart(int X_axis_coord, int Y_axis_coord, int winXsize, int winYsize, int Xscale, int Yscale)
{
#define MARGIN 20
	al_clear_to_color(al_map_rgb(0,0,20));
	al_draw_filled_rectangle(0, 0, winXsize, winYsize, al_map_rgb(0,0,30));		// graph background color
	al_draw_line(Y_axis_coord, MARGIN, Y_axis_coord, winYsize-MARGIN, al_map_rgb(255, 255, 255), 1);	// Y axis
	al_draw_line(MARGIN, X_axis_coord, winXsize-MARGIN, X_axis_coord, al_map_rgb(255, 255, 255), 1);	// X axis
	al_draw_filled_triangle(Y_axis_coord, MARGIN, Y_axis_coord-5, MARGIN+8, Y_axis_coord+5, MARGIN+8, al_map_rgb(255, 255, 255));	// arrowhead Y axis
	al_draw_filled_triangle(winXsize-MARGIN, X_axis_coord, winXsize-MARGIN-8, X_axis_coord-5, winXsize-MARGIN-8, X_axis_coord+5, al_map_rgb(255, 255, 255));	// arrowhead X axis
	al_flip_display();

#define MARGIN 30	// to avoid drawing bars on arrowheads
	int freq = (int)(/*winXsize / */(Xscale));
	int pos = 0;
	long bar_number = 0;
	for(pos = Y_axis_coord-(Y_axis_coord%freq); pos >= MARGIN; pos--)	//bars - negative X axis
	{
		if(pos % freq < 1)
			al_draw_line(pos+(Y_axis_coord%freq), X_axis_coord+3, pos+(Y_axis_coord%freq), X_axis_coord-3, al_map_rgb(255, 255, 255), 1); 
	}
	for(pos = Y_axis_coord-(Y_axis_coord%freq); pos <= winXsize-MARGIN; pos++)	// bars - positive X axis
	{
		if(pos % freq < 1)
		{
			al_draw_line(pos+(Y_axis_coord%freq), X_axis_coord+3, pos+(Y_axis_coord%freq), X_axis_coord-3, al_map_rgb(255, 255, 255), 1); 
			bar_number++;
			if(bar_number == 2)
			{
				al_draw_text(font_scale, al_map_rgb(255, 255, 255), pos+(Y_axis_coord%freq), X_axis_coord+5, 0, "1");

			}
		}

	}
	bar_number = 0;
	for(pos = X_axis_coord-(X_axis_coord%freq); pos >= MARGIN; pos--)	// bars - positive Y axis
	{
		if(pos % freq < 1)
		{
			al_draw_line(Y_axis_coord+3, pos+(X_axis_coord%freq), Y_axis_coord-3, pos+(X_axis_coord%freq), al_map_rgb(255, 255, 255), 1); 
			bar_number++;
			if(bar_number == 2)
			{
			al_draw_text(font_scale, al_map_rgb(255, 255, 255), Y_axis_coord+5, pos+(X_axis_coord%freq), 0, "1");

			}
		}

	}
	for(pos = X_axis_coord-(X_axis_coord%freq); pos <= winYsize-MARGIN; pos++)	// bars - negative Y axis
	{
		if(pos % freq < 1)
			al_draw_line(Y_axis_coord+3, pos+(X_axis_coord%freq), Y_axis_coord-3, pos+(X_axis_coord%freq), al_map_rgb(255, 255, 255), 1); 
	}

	al_flip_display();
	return 0;
}

int expression_check(char expr[])
{
	char *exprdup = strdup(expr); //create duplicate of expression string
	int possibly_incorrect_chars_counter = strlen(exprdup);
	for(int i=0; exprdup[i] != NULL; i++)
		if(exprdup[i] >= '0' && exprdup[i] <= '9') // if element is a digit - subtract number of possible incorrect characters
		{
			exprdup[i] = 32;
			possibly_incorrect_chars_counter--;

		}
	for(int i=0; Allowedstrings[i] != NULL; i++) // for all allowed expressions
	{
		char *strpos = NULL;
		while(strpos = strstr(exprdup, Allowedstrings[i])) //is currently searched string in expression?
		{
			for(int j = 0; j < strlen(Allowedstrings[i]); j++) //turn all letters of allowed string to spaces
			{
				strpos[j] = 32;
				possibly_incorrect_chars_counter--;
				printf("%s\n", exprdup);
			}
			strpos = NULL;
		}	
	}	
	if(possibly_incorrect_chars_counter == 1)
		printf("There is %d incorrect character in this expression\n%s\n", possibly_incorrect_chars_counter, exprdup);
	else if(possibly_incorrect_chars_counter > 1)
		printf("There are %d incorrect characters in this expression\n%s\n", possibly_incorrect_chars_counter, exprdup);
	else
		printf("Expression is correct\n");

	return possibly_incorrect_chars_counter;
}

int allegro_initialization(int widht, int height)
{
	if(!al_init()) 
	{
		al_show_native_message_box(display, "Error", "Error", "Failed to initialize allegro!", 
                                 NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	else
		printf("Allegro initialized\n");

	display = al_create_display(widht, height);
	if(!display) 
	{
		al_show_native_message_box(display, "Error", "Error", "failed to create display!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	else
		printf("Display created\n");

	event_queue = al_create_event_queue();
	if(!event_queue) 
	{
		al_show_native_message_box(display, "Error", "Error", "failed to create event_queue!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		al_destroy_display(display);
		return -1;
	}
	else
		printf("Event queue created\n");
	al_flush_event_queue(event_queue);

	al_init_font_addon();
	al_init_ttf_addon();
	font = al_load_ttf_font("C:\\Windows\\Fonts\\cour.ttf", TEXT_SIZE, 0);
	font_scale = al_load_ttf_font("C:\\Windows\\Fonts\\cour.ttf", TEXT_SIZE, 0);
	if(!font || !font_scale)
	{
		al_destroy_event_queue(event_queue);
		al_show_native_message_box(display, "Error", "Error", "Failed to initialize font!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		al_destroy_display(display);
		return -1;
	}
	else
		printf("Fonts installed\n");

	al_install_keyboard();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	ALLEGRO_EVENT ev;
 
	al_init_primitives_addon();
	//al_clear_to_color(al_map_rgb(0,0,20));
	//al_flip_display();
	return 0;
}
