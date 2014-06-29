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
#define MARGIN 0

int expression_check(char expr[]);
int solveForX(char expr[], double *resultvalue, double argument);
int allegro_initialization(int widht, int height);
int draw_empty_chart(int X_axis_coord, int Y_axis_coord, int winXsize, int winYsize, int Xscale, int Yscale);
char *Allowedstrings[] = {"\n", "+", "-", "*", "/", "%", "^", ")", "(", ".", "asin", "acos", "atan", "sinh", "cosh", "tanh", "sin", "cos", "tan", "exp", "log", "log10", "sqrt", "floor", "ceil", "abs", "deg", "rad", "x", "pi", "e", NULL};

class Coordinate_system
{
private:
	int Y_axis_coord;
	int X_axis_coord;
	int Xscale;	// unit bar every this amount of pixels
	int Yscale;
	double max_visible_value;
	double min_visible_value;
	double graph_height;

public:
	Coordinate_system(int Yac, int Xac, int Xs, int Ys, int graphheight)
	{
		Y_axis_coord = Yac;
		X_axis_coord = Xac;
		Xscale = Xs;
		Yscale = Ys;
		graph_height = graphheight;
		max_visible_value = (X_axis_coord/*-MARGIN*/)*1.0/Yscale;
		min_visible_value = (graph_height/*-MARGIN*/-X_axis_coord)*(-1.0/Yscale);
		printf("done\n");
	}

	int read_axis_coord(char which_axis)
	{
		if(which_axis == 'x')
			return X_axis_coord;
		else if(which_axis == 'y')
			return Y_axis_coord;
		else
			return -1;
	}

	int read_scale_of_axis(char which_axis)
	{
				if(which_axis == 'x')
			return Xscale;
		else if(which_axis == 'y')
			return Yscale;
		else
			return -1;
	}

	double calculate_diff_between_pixels_on_X_axis()
	{
		return 1.0/(double)Xscale;	//Xscale describes number of pixels between bars, 
	}

	//change scale()

	int draw_function_line(int x1, double x1_value, int x2, double x2_value)
	{
		if(x1_value >= MARGIN && x1_value <= graph_height - MARGIN
			&& x2_value >= MARGIN && x2_value <= graph_height - MARGIN)
			al_draw_pixel(x2, x2_value, al_map_rgb(255, 200, 200));
			//al_draw_line(x1, x1_value, x2, x2_value, al_map_rgb(255, 200, 200), 1);
		al_flip_display();
		return 0;
	}

};

int main()
{
	char expr[1024] = {NULL};
	//printf("Wpisz wzor do narysowania np. sin(x) z jedna niewiadoma x lub wpisz literke q by zakonczyc\n");
	printf("Enter expression to draw  eg. sin(x) with one variable x or type q to quit\n");
	printf("You can use: \n");
	for(int i=1; Allowedstrings[i] != NULL; i++)
		printf("%s ", Allowedstrings[i]);
	printf("\nUse * when multiplicating or it will not work. 2x is bad, 2*x is good.\n");

	do
	{
		printf("y = ");
		//fgets(expr, 1023, stdin);
		printf("\n");
		//strcat(expr, "2+2*sin(x)\n");
		strcat(expr, "x\n");
		
		if(expr[0] == 'q')
			return 0;
	} while(expression_check(expr));

	int graph_area_widht = 800;
	int graph_area_height = 600;
	Coordinate_system coord_system(graph_area_widht / 2, graph_area_height / 2, 150, 150, graph_area_height);

#define EXTRA_DATA_AREA_ON_RIGHT_SIDE 0
	allegro_initialization( graph_area_widht+EXTRA_DATA_AREA_ON_RIGHT_SIDE, graph_area_height );
	ALLEGRO_EVENT ev;

	draw_empty_chart(coord_system.read_axis_coord('x'), coord_system.read_axis_coord('y'), graph_area_widht, graph_area_height, coord_system.read_scale_of_axis('x'), coord_system.read_scale_of_axis('y'));

	//DRAWING OF MAIN FUNCTION - from orygin to the left, then to the right
	double last_value, current_value;
	solveForX(expr, &last_value, 0.0);
	double pixel_unit = coord_system.calculate_diff_between_pixels_on_X_axis();
	for(int arg_px=-1; arg_px>(-(coord_system.read_axis_coord('y'))); arg_px--)
	{
		solveForX(expr, &current_value, (double)arg_px*pixel_unit);
		coord_system.draw_function_line(coord_system.read_axis_coord('y')+arg_px-1, coord_system.read_axis_coord('x')-last_value*coord_system.read_scale_of_axis('y'), coord_system.read_axis_coord('y')+arg_px, coord_system.read_axis_coord('x')-current_value*coord_system.read_scale_of_axis('y'));
		last_value = current_value;
		al_flip_display();
	}
	solveForX(expr, &last_value, 0.0);
	for(int arg_px=1; arg_px < graph_area_widht - coord_system.read_axis_coord('y'); arg_px++)
	{
		solveForX(expr, &current_value, (double)arg_px*pixel_unit);
		coord_system.draw_function_line(coord_system.read_axis_coord('y')+arg_px-1, coord_system.read_axis_coord('x')-last_value*coord_system.read_scale_of_axis('y'), coord_system.read_axis_coord('y')+arg_px, coord_system.read_axis_coord('x')-current_value*coord_system.read_scale_of_axis('y'));
		last_value = current_value;
	//	al_flip_display();
	}
	al_flip_display();

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
	al_clear_to_color(al_map_rgb(0,0,50));

	//first long grey unit lines and bars are being drawn, later axes and arrowheads to let them be on top
	int freqX = (int)(Xscale);
	int freqY = (int)(Yscale);
	int pos = 0;
	int bar_number = 0;
#define AL_GREY al_map_rgb(50, 50, 50)
	for(pos = Y_axis_coord-(Y_axis_coord%freqX); pos >= MARGIN; pos--)	//bars - negative X axis
	{
		if(pos % freqX < 1)
	{
			al_draw_line(pos+(Y_axis_coord%freqX), 0, pos+(Y_axis_coord%freqX), winYsize, AL_GREY, 1);
			al_draw_line(pos+(Y_axis_coord%freqX), X_axis_coord+3, pos+(Y_axis_coord%freqX), X_axis_coord-3, al_map_rgb(255, 255, 255), 1);
	}
	}
	for(pos = Y_axis_coord-(Y_axis_coord%freqX); pos <= winXsize-MARGIN-10; pos++)	// bars - positive X axis
	{
		if(pos % freqX < 1)
		{
			al_draw_line(pos+(Y_axis_coord%freqX), 0, pos+(Y_axis_coord%freqX), winYsize, AL_GREY, 1); 
			al_draw_line(pos+(Y_axis_coord%freqX), X_axis_coord+3, pos+(Y_axis_coord%freqX), X_axis_coord-3, al_map_rgb(255, 255, 255), 1); 
			bar_number++;
			if(bar_number == 2)
				al_draw_text(font_scale, al_map_rgb(255, 255, 255), pos+(Y_axis_coord%freqX), X_axis_coord+5, 0, "1");
		}
	}
	bar_number = 0;
	for(pos = X_axis_coord-(X_axis_coord%freqY); pos >= MARGIN+10; pos--)	// bars - positive Y axis
	{
		if(pos % freqY < 1)
		{
			al_draw_line(0, pos+(X_axis_coord%freqY), winXsize, pos+(X_axis_coord%freqY), AL_GREY, 1); 
			al_draw_line(Y_axis_coord+3, pos+(X_axis_coord%freqY), Y_axis_coord-3, pos+(X_axis_coord%freqY), al_map_rgb(255, 255, 255), 1); 
			bar_number++;
			if(bar_number == 2)
			{
			al_draw_text(font_scale, al_map_rgb(255, 255, 255), Y_axis_coord+5, pos+(X_axis_coord%freqY), 0, "1");

			}
		}

	}
	for(pos = X_axis_coord-(X_axis_coord%freqY); pos <= winYsize-MARGIN; pos++)	// bars - negative Y axis
	{
		if(pos % freqY < 1)
	{
			al_draw_line(0, pos+(X_axis_coord%freqY), winXsize, pos+(X_axis_coord%freqY), AL_GREY, 1); 
			al_draw_line(Y_axis_coord+3, pos+(X_axis_coord%freqY), Y_axis_coord-3, pos+(X_axis_coord%freqY), al_map_rgb(255, 255, 255), 1); 
	}
	}

	// now axes, arrowheads are being drawn
	al_draw_line(Y_axis_coord, MARGIN, Y_axis_coord, winYsize-MARGIN, al_map_rgb(255, 255, 255), 1);	// Y axis
	al_draw_line(MARGIN, X_axis_coord, winXsize-MARGIN, X_axis_coord, al_map_rgb(255, 255, 255), 1);	// X axis
	al_draw_line(Y_axis_coord+Xscale, MARGIN, Y_axis_coord+Xscale, winYsize-MARGIN, al_map_rgb(255, 0, 0), 1);	// Y axis
	al_draw_line(MARGIN, X_axis_coord-Yscale, winXsize-MARGIN, X_axis_coord-Yscale, al_map_rgb(255, 0, 0), 1);	// X axis
	al_draw_filled_triangle(Y_axis_coord, MARGIN, Y_axis_coord-5, MARGIN+8, Y_axis_coord+5, MARGIN+8, al_map_rgb(255, 255, 255));	// arrowhead Y axis
	al_draw_filled_triangle(winXsize-MARGIN, X_axis_coord, winXsize-MARGIN-8, X_axis_coord-5, winXsize-MARGIN-8, X_axis_coord+5, al_map_rgb(255, 255, 255));	// arrowhead X axis
	//al_draw_text(font, al_map_rgb(255, 200, 200), 5, 5, 0, expr);
	al_flip_display();

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