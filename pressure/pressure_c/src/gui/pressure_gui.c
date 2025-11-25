#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <cairo.h>
#include <glib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

#include "../ads1115.h"
#include "../pressure_conv.h"

// configuration
#define DEFAULT_I2C_BUS 4
#define DEFAULT_ADDR 0x48
#define SAMPLES 300

typedef struct {
    double samples[SAMPLES];
    int head;
    int count;
    pthread_mutex_t lock;
    double latest;
} sample_buffer_t;

static sample_buffer_t buf;
static ads1115_t ads;
static int g_mock_mode = 0; // set 1 to simulate data if no hardware

static gboolean on_draw(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    int w = alloc.width;
    int h = alloc.height;

    // background
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // get data
    double data[SAMPLES];
    int n = 0;
    pthread_mutex_lock(&buf.lock);
    n = buf.count;
    for (int i = 0; i < n; ++i) {
        int idx = (buf.head + i) % SAMPLES;
        data[i] = buf.samples[idx];
    }
    double latest = buf.latest;
    pthread_mutex_unlock(&buf.lock);

    if (n <= 0) return FALSE;

    // compute min/max for autoscale
    double minv = data[0], maxv = data[0];
    for (int i=0;i<n;++i) {
        if (data[i] < minv) minv = data[i];
        if (data[i] > maxv) maxv = data[i];
    }
    if (fabs(maxv - minv) < 1e-6) { maxv = minv + 1.0; }

    // draw axes
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    cairo_rectangle(cr, 40, 10, w-50, h-60);
    cairo_fill(cr);

    // axis lines
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 40, 10, w-50, h-60);
    cairo_stroke(cr);

    // draw line graph
    cairo_set_source_rgb(cr, 0.0, 0.5, 1.0);
    cairo_set_line_width(cr, 2.0);

    for (int i=0;i<n;i++) {
        double x = 40 + ((double)i / (n-1)) * (w - 90);
        double y = 10 + (1.0 - (data[i] - minv) / (maxv - minv)) * (h - 70);
        if (i==0) cairo_move_to(cr, x, y);
        else cairo_line_to(cr, x, y);
    }
    cairo_stroke(cr);

    // draw latest value big
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 22.0);
    cairo_set_source_rgb(cr, 0,0,0);
    char label[128];
    snprintf(label, sizeof(label), "Latest: %.2f kPa", latest);
    cairo_move_to(cr, 50, h - 20);
    cairo_show_text(cr, label);

    // draw scale labels
    cairo_set_font_size(cr, 12.0);
    char minlbl[64], maxlbl[64];
    snprintf(minlbl, sizeof(minlbl), "%.2f", minv);
    snprintf(maxlbl, sizeof(maxlbl), "%.2f", maxv);
    cairo_move_to(cr, 5, h - 20);
    cairo_show_text(cr, minlbl);
    cairo_move_to(cr, 5, 20);
    cairo_show_text(cr, maxlbl);

    return FALSE;
}

static GtkWidget *drawing_area;
static GtkWidget *window;

static void schedule_draw() {
    if (GTK_IS_WIDGET(drawing_area)) {
        gtk_widget_queue_draw(drawing_area);
    }
}

// thread reading sensor
static void *sensor_thread(void *arg) {
    (void)arg;
    int16_t raw;
    while (1) {
        double voltage = 0.0;
        if (!g_mock_mode) {
            if (ads1115_read_raw(&ads, &raw) == 0) {
                voltage = ads1115_raw_to_voltage(raw, 4.096);
            } else {
                // read failed, sleep and continue
                usleep(200000);
                continue;
            }
        } else {
            // mock: sine wave
            static double t = 0.0;
            voltage = 1.0 + 0.5 * sin(t);
            t += 0.2;
        }

        double pressure = voltage_to_pressure_kpa(voltage);
        pthread_mutex_lock(&buf.lock);
        buf.samples[(buf.head + buf.count) % SAMPLES] = pressure;
        if (buf.count < SAMPLES) buf.count++;
        else buf.head = (buf.head + 1) % SAMPLES;
        buf.latest = pressure;
        pthread_mutex_unlock(&buf.lock);

        // schedule draw on main thread
        g_idle_add((GSourceFunc) schedule_draw, NULL);

        usleep(200000); // 200 ms sampling
    }
    return NULL;
}

int main(int argc, char **argv) {
    gtk_init();

    // load calibration
    load_calibration("config/calibration.cfg");

    // parse optional args
    int i2c_bus = DEFAULT_I2C_BUS;
    int addr = DEFAULT_ADDR;
    for (int i=1;i<argc;i++) {
        if (strcmp(argv[i], "--mock") == 0) g_mock_mode = 1;
        else if (strncmp(argv[i], "--bus=",6)==0) i2c_bus = atoi(argv[i]+6);
        else if (strncmp(argv[i], "--addr=",7)==0) addr = (int)strtol(argv[i]+7, NULL, 0);
    }

    if (!g_mock_mode) {
        if (ads1115_init(&ads, i2c_bus, (uint8_t)addr) != 0) {
            fprintf(stderr, "Failed to init ADS1115 on i2c-%d addr 0x%02x\n", i2c_bus, addr);
            fprintf(stderr, "Run with --mock to use simulated data\n");
            return 1;
        }
    }

    // init sample buffer
    pthread_mutex_init(&buf.lock, NULL);
    buf.head = 0;
    buf.count = 0;
    buf.latest = 0.0;

    // build GUI
    window = gtk_window_new();
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Pressure Monitor");

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_vexpand(drawing_area, TRUE);
    gtk_widget_set_hexpand(drawing_area, TRUE);
    gtk_window_set_child(GTK_WINDOW(window), drawing_area);

    g_signal_connect(drawing_area, "draw", G_CALLBACK(on_draw), NULL);
    g_signal_connect(window, "close-requested", G_CALLBACK(gtk_window_destroy), NULL);

    // start sensor thread
    pthread_t thr;
    pthread_create(&thr, NULL, sensor_thread, NULL);
    pthread_detach(thr);

    gtk_widget_show(window);
    gtk_main();

    // cleanup
    if (!g_mock_mode) ads1115_close(&ads);
    return 0;
}

