#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>
#include <map>
#include "evaluate.h"
#include <allegro5\allegro5.h>
#include <allegro5\allegro_native_dialog.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>
//using namespace std;

#define TEXT_COLOR 255, 255, 255
#define TEXT_SIZE 15
#define MARGIN 0
#define GRAPH_WIDHT 800
#define GRAPH_HEIGHT 600
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_FONT *font = NULL;
ALLEGRO_FONT *font_scale = NULL;
long number_of_calculations = 0;
//double **function_table = new double*[5];
std::map<double, std::vector<double> > function_map;


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

	int draw_function_line(double x1, double x1_value, double x2, double x2_value, ALLEGRO_COLOR line_color)
	{
		if(x1_value >= min_visible_value && x1_value <= max_visible_value
			&& x2_value >= min_visible_value && x2_value <= max_visible_value)
		{
			x1 = Y_axis_coord + (x1 * (double)Xscale);
			x1_value = X_axis_coord - (x1_value * (double)Yscale);
			//printf("x=%f y=%f\n", x1, x1_value);
			al_draw_pixel(x1, x1_value, line_color);
			//al_draw_line(x1, x1_value, x2, x2_value, al_map_rgb(255, 200, 200), 1);
		if(!((int)x2 % 100))
			al_flip_display();
		}
		return 0;
	}
};
void recalculate_and_draw(Coordinate_system *cs, char expr[], char number_of_derivatives, float step);
int calc_function(Coordinate_system *cs, char expression[], char derivative_order, float step);
int draw_function(Coordinate_system *cs, char expression[], char der_num);
int expression_check(char expr[]);
int solveForX(char expr[], double *resultvalue, double argument);
int allegro_initialization(int widht, int height);
int draw_empty_chart(int X_axis_coord, int Y_axis_coord, int winXsize, int winYsize, int Xscale, int Yscale);
char *Allowedstrings[] = {"\n", "+", "-", "*", "/", "%", "^", ")", "(", ".", "asin", "acos", "atan", "sinh", "cosh", "tanh", "sin", "cos", "tan", "exp", "log", "log10", "sqrt", "floor", "ceil", "abs", "deg", "rad", "x", "pi", "e", NULL};


int main(int argc, char **argv)
{
	srand(time(NULL));
	char expr[1024] = {NULL};

	if(argc < 2)
	{
		//printf("Wpisz wzor do narysowania np. sin(x) z jedna niewiadoma x lub wpisz literke q by zakonczyc\n");
		printf("Enter expression to draw  eg. sin(x) with one variable x or type q to quit\n");
		printf("Type 'help' to see what you can build function of.\n");
		printf("You can use: \n");
		printf("\nUse * when multiplicating or it will not work. sin2x is bad, sin(2*x) is good.\n");

		do
		{
			printf("y = ");
			fgets(expr, 1023, stdin);
			//printf("\n");
			//strcat(expr, "2+2*sin(x)-e^x\n");
			//strcat(expr, "2*sinx+cos(4*x)+2*atanx\n");
			//printf("%s", expr);
		
			if(expr[0] == 'q')
				return 0;
			else if(strstr(expr, "help"))
			{
				for(int i=1; Allowedstrings[i] != NULL; i++)
					printf("%s ", Allowedstrings[i]);
				printf("\n Now press any key\n");
				continue;
			}

		} while(expression_check(expr));
	}
	else if(argc > 1)
	{
		strcpy(expr, argv[1]);
		printf("%s", expr);
		if(expression_check(expr))
			return 0;
	}


	Coordinate_system cs((GRAPH_WIDHT / 2), (GRAPH_HEIGHT / 2), 20, 20, GRAPH_HEIGHT);
	char number_of_derivatives = 0;
	calc_function(&cs, expr, number_of_derivatives, 0.001);

	allegro_initialization( GRAPH_WIDHT, GRAPH_HEIGHT );
	ALLEGRO_EVENT ev;

	draw_empty_chart(cs.read_axis_coord('x'), cs.read_axis_coord('y'), GRAPH_WIDHT, GRAPH_HEIGHT, cs.read_scale_of_axis('x'), cs.read_scale_of_axis('y'));
	//DRAWING OF MAIN FUNCTION - from left to the right
	draw_function(&cs, expr, number_of_derivatives);

	// HERE - WHAT U WANT TO DO NOW?
	// if again - use int recalculate_and_draw(Coordinate_system *cs, char expr[], char number_of_derivatives, float step)

	while(1)
	{
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE || (ev.type == ALLEGRO_EVENT_KEY_DOWN && (ev.keyboard.keycode == ALLEGRO_KEY_Q || ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE)))
			return 0;  //exit program
	}
	return 0;
}

void recalculate_and_draw(Coordinate_system *cs, char expr[], char number_of_derivatives, float step)
{
	calc_function(cs, expr, number_of_derivatives, 0.001);
	draw_empty_chart(cs->read_axis_coord('x'), cs->read_axis_coord('y'), GRAPH_WIDHT, GRAPH_HEIGHT, cs->read_scale_of_axis('x'), cs->read_scale_of_axis('y'));
	draw_function(cs, expr, number_of_derivatives);
}


int calc_function(Coordinate_system *cs, char expression[], char derivative_order, float step)
{
	printf("Calculating... \n");
	double min_arg = (-1) * (cs->read_axis_coord('y') - MARGIN ) * cs->calculate_diff_between_pixels_on_X_axis();
	double max_arg = ( GRAPH_WIDHT - 2*MARGIN - cs->read_axis_coord('y') ) * cs->calculate_diff_between_pixels_on_X_axis();
	double current_value = 0;
	function_map.clear();
	for(int current_der_ord = 0; current_der_ord <= derivative_order; current_der_ord++)
	{
		if(current_der_ord == 0)	// oryginal function
		{
			for(double i = min_arg; i<=max_arg; i += step)
			{
				solveForX(expression, &current_value, i);
				function_map[i].push_back(current_value);
			}
			printf("function done! \n");
		}
		else	// derivatives of function
		{
			std::map<double, std::vector<double>>::iterator it, it_next, it_end;
			it = it_next = function_map.begin();
			it_end = function_map.end();
			for(int i=0; i<current_der_ord; i++)	//avoid calculating derivatives for arguments from ends of range
			{
				it++;
				it_next++;
				it_end--;
			}
			it_next++;
			for(it; it != it_end; it++, it_next++)
			{
				// derivative at point is difference of y over difference of x
				double a = (it_next->second[current_der_ord-1] - it->second[current_der_ord-1]) / (step);
				it->second.push_back(a);
			}
			printf("%d. derivative done! \n");
		}
	}

	return 0;
}

// TODO: dynamic accuracy, changing scale, more functions(x), movement of axis, description of visible functions

int draw_function(Coordinate_system *cs, char expression[], char der_num)
{
	std::map<double, std::vector<double>>::iterator it;
	std::map<double, std::vector<double>>::iterator it_end;

	ALLEGRO_COLOR line_color;
	//for function and every derivative
	for(int current_derivative = 0; current_derivative <= der_num; current_derivative++)
	{
		it = function_map.begin();
		it_end = function_map.end();
		for(int i=0; i<=current_derivative; i++)
		{
			it++;
			it_end--;
		}
		
		switch(current_derivative)	// choose color for line
		{
		case 0: line_color = al_map_rgb(255, 0, 0); break;
		case 1: line_color = al_map_rgb(0, 255, 0); break;
		case 2: line_color = al_map_rgb(100, 100, 255); break;
			//	if derivative of higher order take random, bright enough color
		default: unsigned char r; unsigned char g; unsigned char b ;
			do
			{
				r = rand()%255;
				g = rand()%255;
				b = rand()%255;
			}
			while( (r+g+b) > 200 );
			line_color = al_map_rgb(r, g, b);
		}
		for(it; it != it_end; it++)
		{
			cs->draw_function_line(it->first, it->second[current_derivative], 1, 1 /*it++->first, it++->second[current_derivative]*/, line_color);
		}
		al_flip_display();

	}

	//double last_value, current_value;
	//solveForX(expression, &last_value, 0.0);
	//double pixel_unit = cs->calculate_diff_between_pixels_on_X_axis();
	//number_of_calculations = 0;
	//for(float arg_px=(-(cs->read_axis_coord('y'))); arg_px < GRAPH_WIDHT - cs->read_axis_coord('y'); arg_px += cs->read_accuracy())
	//{
	//	solveForX(expression, &current_value, arg_px*pixel_unit);
	//	cs->draw_function_line(cs->read_axis_coord('y')+arg_px-1, cs->read_axis_coord('x')-last_value*cs->read_scale_of_axis('y'), cs->read_axis_coord('y')+arg_px, cs->read_axis_coord('x')-current_value*cs->read_scale_of_axis('y'), line_color);
	//	last_value = current_value;
	//}

	//printf("Number of calcs: %d\n", number_of_calculations);
	al_flip_display();
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
	//al_draw_line(Y_axis_coord+Xscale, MARGIN, Y_axis_coord+Xscale, winYsize-MARGIN, al_map_rgb(255, 0, 0), 1);	// Y axis
	//al_draw_line(MARGIN, X_axis_coord-Yscale, winXsize-MARGIN, X_axis_coord-Yscale, al_map_rgb(255, 0, 0), 1);	// X axis
	al_draw_filled_triangle(Y_axis_coord, MARGIN, Y_axis_coord-5, MARGIN+8, Y_axis_coord+5, MARGIN+8, al_map_rgb(255, 255, 255));	// arrowhead Y axis
	al_draw_filled_triangle(winXsize-MARGIN, X_axis_coord, winXsize-MARGIN-8, X_axis_coord-5, winXsize-MARGIN-8, X_axis_coord+5, al_map_rgb(255, 255, 255));	// arrowhead X axis
	//al_draw_text(font, al_map_rgb(255, 200, 200), 5, 5, 0, expr);
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
				//printf("%s\n", exprdup);
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

	free(exprdup);
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
	//else
	//	printf("Allegro initialized\n");

	al_set_new_display_option(ALLEGRO_SAMPLE_BUFFERS, 1, ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_SAMPLES, 8, ALLEGRO_SUGGEST);

	display = al_create_display(widht, height);
	if(!display) 
	{
		al_show_native_message_box(display, "Error", "Error", "failed to create display!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		return -1;
	}
	//else
	//	printf("Display created\n");

	event_queue = al_create_event_queue();
	if(!event_queue) 
	{
		al_show_native_message_box(display, "Error", "Error", "failed to create event_queue!", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		al_destroy_display(display);
		return -1;
	}
	//else
	//	printf("Event queue created\n");
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
	//else
	//	printf("Fonts installed\n");

	al_install_keyboard();
	al_register_event_source(event_queue, al_get_display_event_source(display));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	ALLEGRO_EVENT ev;
 
	al_init_primitives_addon();
	//al_clear_to_color(al_map_rgb(0,0,20));
	//al_flip_display();
	return 0;
}