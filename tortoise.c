// Simple backend for a Logo-like tortoise drawer
// https://www.gnu.org/software/guile/docs/guile-tut/tutorial.html

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
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

int main(int argc, char* argv[])
{
    start_gnuplot();
    tortoise_reset();
    atexit(stop_gnuplot);

    tortoise_pendown (); /* This is unnecessary, but makes it clearer.  */
    for (int i = 1; i <= 4; ++i) {
        tortoise_move (3.0);
        tortoise_turn (90.0);
    }
    return EXIT_SUCCESS;
}
