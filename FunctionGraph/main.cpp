﻿#include <stdio.h>
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

#define TEXT_COLOR 255, 255, 255
#define TEXT_SIZE 17

int expression_check(char expr[]);
int solveForX(char expr[], double *resultvalue);
int allegro_initialization(int widht, int height);
int draw_empty_chart(int X_axis_place, int Y_axis_place, int winXsize, int winYsize);

char *Allowedstrings[] = {"\n", "+", "-", "*", "/", "%", ")", "(", "x", "asin", "acos", "atan", "sinh", "cosh", "tanh", "sin", "cos", "tan", "exp", "log", "log10", "sqrt", "floor", "ceil", "abs", "deg", "rad", NULL};

int main()
{
	char expr[1024] = {NULL};
	char end = 0;
	printf("Wpisz wzor do narysowania np. sin(x) z jedna niewiadoma x lub literke q by zakonczyc\n");
	printf("Enter expression to draw ex. sin(x) with one variable x or letter q to quit\n");
	printf("You can use: \n");
	for(int i=1; Allowedstrings[i] != NULL; i++)
		printf("%s ", Allowedstrings[i]);
	printf("\n");

	while(1)
	{
		printf("y = ");
		fgets(expr, 1023, stdin);
		printf("\n");
		//strcat(expr, "2+2*sin(x)\n");

		if(expr[0] == 'q')
			end = 1;
		//expression check
		if(expression_check(expr))
			continue;
		else
			break;
	}
	int window_widht = 800; //REAL SIZE OF WINDOW IS GREATER by the constant value given in allegro_initialization() call
	int window_height = 600;
	allegro_initialization(window_widht+250, window_height);
	ALLEGRO_EVENT ev;
	draw_empty_chart(window_widht/2, window_height/2, window_widht, window_height);

	//int Xscale = 10;
	//int Yscale = 10;

	while(1)
	{
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && (ev.keyboard.keycode == ALLEGRO_KEY_Q || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)))
			return 0;  //exit program
	}
	return 0;
}



int draw_empty_chart(int X_axis_place, int Y_axis_place, int winXsize, int winYsize)
{
#define MARGIN 20
	al_clear_to_color(al_map_rgb(0,0,20));
	al_draw_filled_rectangle(MARGIN, MARGIN, winXsize-MARGIN, winYsize-MARGIN, al_map_rgb(0,0,20));
	al_draw_line(X_axis_place, MARGIN, X_axis_place, winYsize-MARGIN, al_map_rgb(255, 255, 255), 1);
	al_draw_line(MARGIN, Y_axis_place, winXsize-MARGIN, Y_axis_place, al_map_rgb(255, 255, 255), 1);
	al_draw_filled_triangle(X_axis_place, MARGIN, X_axis_place-5, MARGIN+8, X_axis_place+5, MARGIN+8, al_map_rgb(255, 255, 255));
	al_draw_filled_triangle(winXsize-MARGIN, Y_axis_place, winXsize-MARGIN-8, Y_axis_place-5, winXsize-MARGIN-8, Y_axis_place+5, al_map_rgb(255, 255, 255));
	al_flip_display();
	return 0;
}

int expression_check(char expr[])
{
	char *exprdup = strdup(expr); //create duplicate of expression string
	int possibly_incorrect_chars_counter = strlen(exprdup);
	for(int i=0; exprdup[i] != NULL; i++)
		if(exprdup[i] >= '0' && exprdup[i] <= '9') // subtract number of possible incorrect characters
		{
			exprdup[i] = 32;
			possibly_incorrect_chars_counter--;
		}
	for(int i=0; Allowedstrings[i] != NULL; i++) // for all allowed epressions
	{
		char *strpos = NULL;
		while(strpos = strstr(exprdup, Allowedstrings[i])) //if they are present in expression
		{
			for(int j = 0; j < strlen(Allowedstrings[i]); j++)
			{
				strpos[0+j] = 32;
				possibly_incorrect_chars_counter--;
			}
			strpos = NULL;
		}	
	}	
	if(possibly_incorrect_chars_counter)
		printf("There is %d incorrect characters in this expression\n", possibly_incorrect_chars_counter);
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
	if(!font)
	{
		al_destroy_event_queue(event_queue);
		al_show_native_message_box(display, "Error", "Error", "Failed to initialize font!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		al_destroy_display(display);
		return -1;
	}
	else
		printf("Font installed\n");

	al_install_keyboard();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	ALLEGRO_EVENT ev;
 
	al_init_primitives_addon();
	//al_clear_to_color(al_map_rgb(0,0,20));
	//al_flip_display();
	return 0;
}
