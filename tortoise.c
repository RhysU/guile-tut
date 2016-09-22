// Simple backend for a Logo-like tortoise drawer
// https://www.gnu.org/software/guile/docs/guile-tut/tutorial.html

#include <libguile.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const int WIDTH = 10;
static const int HEIGHT = 10;

static FILE* gp;
static double x, y;
static double direction;
static int pendown;

static void start_gnuplot()
{
    int pipes[2];
    pipe(pipes);
    const pid_t pid = fork();

    if (!pid) {
        dup2(pipes[0], STDIN_FILENO);
        execlp("gnuplot", "gnuplot", "-persist", NULL);  // No return
    }

    gp = fdopen(pipes[1], "w");
    fprintf(gp, "set multiplot\n");
    fprintf(gp, "set parametric\n");
    fprintf(gp, "set xrange[-%d:%d]\n", WIDTH, HEIGHT);
    fprintf(gp, "set yrange[-%d:%d]\n", WIDTH, HEIGHT);
    fprintf(gp, "set size ratio -1\n");
    fprintf(gp, "unset xtics\n");
    fprintf(gp, "unset ytics\n");
    fflush(gp);
}

static void stop_gnuplot()
{
    if (gp) {
        fprintf(gp, "quit\n");
        fflush(gp);
    }
}

static void tortoise_reset()
{
    x = y = direction = 0;
    pendown = 1;
    fprintf(gp, "clear\n");
    fflush(gp);
}

static void draw_line(double x1, double y1, double x2, double y2)
{
    fprintf(
        gp,
        "plot [0:1] %f + %f * t, %f + %f * t notitle\n",
        x1, x2 - x1, y1, y2 - y1);
    fflush(gp);
}

static void tortoise_pendown()
{
    pendown = 1;
}

static void tortoise_penup()
{
    pendown = 0;
}

static void tortoise_turn(double degrees)
{
    direction += M_PI / 180.0 * degrees;
}

static void tortoise_move(double length)
{
    const double newX = x + length * cos(direction);
    const double newY = y + length * sin(direction);

    if (pendown) {
        draw_line(x, y, newX, newY);
    }

    x = newX;
    y = newY;
}

static void* register_functions (void* data)
{
    return NULL;
}

int main(int argc, char* argv[])
{
    start_gnuplot();
    tortoise_reset();
    atexit(stop_gnuplot);
    scm_with_guile(&register_functions, NULL);
    scm_shell(argc, argv);
    return EXIT_SUCCESS;
}
