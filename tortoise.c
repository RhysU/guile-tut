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

static SCM tortoise_reset()
{
    x = y = direction = 0;
    pendown = 1;
    fprintf(gp, "clear\n");
    fflush(gp);
    return SCM_UNSPECIFIED;
}

static void draw_line(double x1, double y1, double x2, double y2)
{
    fprintf(
        gp,
        "plot [0:1] %f + %f * t, %f + %f * t notitle\n",
        x1, x2 - x1, y1, y2 - y1);
    fflush(gp);
}

static SCM tortoise_pendown()
{
    SCM result = scm_from_bool(pendown);
    pendown = 1;
    return result;
}

static SCM tortoise_penup()
{
    SCM result = scm_from_bool(pendown);
    pendown = 0;
    return result;
}

static SCM tortoise_turn(SCM degrees)
{
    const double value = scm_to_double (degrees);
    direction += M_PI / 180.0 * value;
    return scm_from_double (direction * 180.0 / M_PI);
}

static SCM tortoise_move(SCM length)
{
    const double value = scm_to_double(length);
    const double newX = x + value * cos(direction);
    const double newY = y + value * sin(direction);

    if (pendown) {
        draw_line(x, y, newX, newY);
    }

    x = newX;
    y = newY;

    return scm_list_2(scm_from_double(x), scm_from_double(y));
}

static void* register_functions (void* data)
{
    scm_c_define_gsubr("tortoise-reset",   0, 0, 0, &tortoise_reset);
    scm_c_define_gsubr("tortoise-penup",   0, 0, 0, &tortoise_penup);
    scm_c_define_gsubr("tortoise-pendown", 0, 0, 0, &tortoise_pendown);
    scm_c_define_gsubr("tortoise-turn",    1, 0, 0, &tortoise_turn);
    scm_c_define_gsubr("tortoise-move",    1, 0, 0, &tortoise_move);
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
